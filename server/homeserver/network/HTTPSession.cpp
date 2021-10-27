#include "HTTPSession.hpp"
#include "../Core.hpp"
#include "io/WebPageFiles.hpp"
#include "../user/UserManager.hpp"
#include "../home/Home.hpp"
#include "../home/Room.hpp"
#include "../home/Device.hpp"
#include "../plugin/PluginManager.hpp"
#include "json/JsonApi.hpp"
#include "WSSession.hpp"

#define ERROR404 \
	"<html>" \
	"<style>div{width:100%;height:100%;display:flex;align-items:center;justify-content:center;}h1{text-align:center;font-family:sans-serif;}</style>" \
	"<body><div><h1>error 404<br><br>Woops. Looks like this page doesn't exist.<br>This message only shows up when 'index.html' doesn't exist.</h1></div></body>" \
	"</html>"

namespace server
{
	HTTPSession::HTTPSession(Ref<ssl_socket_t> socket)
		: strand(socket->get_executor()), socket(socket)
	{ }
	HTTPSession::~HTTPSession()
	{ }

	void HTTPSession::Run()
	{
		socket->next_layer().expires_after(std::chrono::seconds(12));
		boost::beast::http::async_read(*socket, buffer, request, boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
	}

	void HTTPSession::OnRead(boost::system::error_code error, size_t size)
	{
		if (error)
			return;

		try
		{

			std::string_view target = std::string_view(request.target().data(), request.target().size());

			if (strncmp(target.data(), "/res/", 5) == 0)
			{
				Ref<User> user = Authenticate();
				if (user == nullptr)
				{
					WriteError("Invalid authentication token.");
					return;
				}

				target.remove_prefix(5); // Remove /api/

				//Create response
				boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::buffer_body>>();
				response->version(request.version());
				response->set(boost::beast::http::field::server, "HomeAutomation Server");

				// Resource
				std::string contentType = "text/plain";

				responseBuffer.Clear();

				if (JsonApi::ProcessResApiCallHTTP(request.method(), target, user, std::string_view(request.body()), responseBuffer, contentType))
				{
					response->result(boost::beast::http::status::ok);
					response->keep_alive(request.keep_alive());
				}
				else
				{
					response->result(boost::beast::http::status::bad_request);
					response->keep_alive(false);
				}

				if (responseBuffer.GetSize() == 0)
				{
					// A response cannot be empty
					response->set(boost::beast::http::field::content_type, "test/plain");
					memcpy(responseBuffer.Push(10), "NO CONTENT", 10);
				}
				else
					response->set(boost::beast::http::field::content_type, contentType);

				boost::beast::http::buffer_body::value_type& buf = response->body();
				buf.data = (void*)responseBuffer.GetString();
				buf.size = responseBuffer.GetSize();
				buf.more = false;

				response->prepare_payload();

				buffer.consume(size);

				//Send response
				boost::beast::http::async_write(*socket, *response,
					boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteBuffer, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
			}
			else if (strncmp(target.data(), "/auth", 5) == 0)
			{
				rapidjson::StringStream stream = rapidjson::StringStream(request.body().c_str());

				rapidjson::Document document;
				document.ParseStream(stream);

				buffer.consume(size);

				if (document.HasParseError() || !document.IsObject())
				{
					std::string error = "Invalid body. Please call /help for more details. JSON Message : ";
					error += rapidjson::GetParseError_En(document.GetParseError());

					WriteError(error.c_str());
					return;
				}

				rapidjson::Value::MemberIterator tokenIt = document.FindMember("token");

				std::string token;

				if (tokenIt != document.MemberEnd() && tokenIt->value.IsString())
				{
					rapidjson::Value& tokenJson = tokenIt->value;

					token.append(tokenJson.GetString(), tokenJson.GetStringLength());
					try
					{
						jwt::decoded_jwt decodedToken = jwt::decode(token);

						identifier_t userID = UserManager::GetInstance()->VerifyToken(decodedToken);
					#ifndef _DEBUG
						/*if (jwt::date::clock::now() + std::chrono::hours(120) > decodedToken.get_expires_at())
							token = UserManager::GetInstance()->GenerateToken(userID);*/
					#endif
					}
					catch (std::exception)
					{
						WriteError("Invalid authentication token.");
						return;
					}
				}
				else
				{
					rapidjson::Value::MemberIterator nameIt = document.FindMember("name");
					rapidjson::Value::MemberIterator passwdIt = document.FindMember("passwd");

					if (nameIt == document.MemberEnd() || !nameIt->value.GetString() ||
						passwdIt == document.MemberEnd() || !passwdIt->value.GetString())
					{
						WriteError("Invalid body. Please call /help for more details.");
						return;
					}

					try
					{
						token = UserManager::GetInstance()->GenerateToken(
							std::string_view(nameIt->value.GetString(), nameIt->value.GetStringLength()),
							std::string_view(passwdIt->value.GetString(), passwdIt->value.GetStringLength()));
					}
					catch (std::exception)
					{
						WriteError("Invalid credentials.");
						return;
					}
				}

				//Create response
				boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
				response->result(boost::beast::http::status::ok);
				response->version(request.version());
				response->keep_alive(request.keep_alive());
				response->set(boost::beast::http::field::server, "HomeAutomation Server");
				response->set(boost::beast::http::field::content_type, "application/json");
				std::string& body = response->body();
				body.reserve(12 + token.size());
				body += "{\"token\":\"";
				body += token;
				body += "\"}";

				response->prepare_payload();

				//Send response
				boost::beast::http::async_write(*socket, *response,
					boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
			}
			else if (strncmp(target.data(), "/ws", 3) == 0)
			{
				if (boost::beast::websocket::is_upgrade(request))
				{
					Ref<User> user = Authenticate();
					if (user == nullptr)
					{
						WriteError("Invalid authentication token.");
						return;
					}

					socket->next_layer().expires_never();

					Ref<WSSession> ws = boost::make_shared<WSSession>(socket, user);
					ws->Run(request);

					buffer.consume(size);
				}
				else
				{
					WriteError("Invalid websocket upgrade. Please call /help for more details.");
					return;
				}
			}
			else if (strncmp(target.data(), "/ping", 5) == 0)
			{
				//Create response
				boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
				response->result(boost::beast::http::status::ok);
				response->version(request.version());
				response->keep_alive(request.keep_alive());
				response->set(boost::beast::http::field::server, "HomeAutomation Server");
				response->set(boost::beast::http::field::content_type, "application/json");
				std::string& body = response->body();
				body.reserve(38);
				body += "{\"pong\":\"57494c4c49414d4bd6484c4552\"}";
				response->prepare_payload();

				//Send response
				boost::beast::http::async_write(*socket, *response,
					boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
			}
			else if (strncmp(target.data(), "/help", 5) == 0)
			{
				//Create response
				boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
				response->result(boost::beast::http::status::ok);
				response->version(request.version());
				response->keep_alive(request.keep_alive());
				response->set(boost::beast::http::field::server, "HomeAutomation Server");
				response->set(boost::beast::http::field::content_type, "test/plain");

				// Write help log
				{
					std::stringstream ss(response->body());

					ss << "You called for help!\n";
				}

				response->prepare_payload();

				//Send response
				boost::beast::http::async_write(*socket, *response,
					boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
			}
			else
			{
				if (request.method() == boost::beast::http::verb::get)
				{
					WebPageFile* file = nullptr;

					if (target.size() == 1)
						file = WebPageFiles::GetFile("/index.html");
					else
					{
						file = WebPageFiles::GetFile(target.data());

						if (file == nullptr) //Serve index.html
							file = WebPageFiles::GetFile("/index.html");
					}

					buffer.consume(size);

					if (file == nullptr)
					{
						boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
						response->result(boost::beast::http::status::not_found);
						response->version(request.version());
						response->keep_alive(false);
						response->set(boost::beast::http::field::server, "HomeAutomation Server");
						response->set(boost::beast::http::field::content_type, "text/html");
						response->body() += ERROR404;

						response->prepare_payload();

						boost::beast::http::async_write(*socket, *response, boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
						return;
					}

					//Create response
					boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::buffer_body>>();
					response->result(boost::beast::http::status::ok);
					response->version(request.version());
					response->keep_alive(request.keep_alive());
					response->set(boost::beast::http::field::server, "HomeAutomation Server");
					response->set(boost::beast::http::field::content_type, GetMimeFromFileType(file->fileType));

					boost::beast::http::buffer_body::value_type& body = response->body();
					body.size = file->size;
					body.data = file->data;
					body.more = false;

					response->prepare_payload();

					//Send response
					boost::beast::http::async_write(*socket, *response,
						boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteBuffer, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
				}
				else
				{
					buffer.consume(size);

					WriteError("Invalid method.");
					return;
				}
			}

		}
		catch (std::exception e)
		{
			LOG_ERROR("Ups : {0}", e.what());

			buffer.clear();

			WriteError("Internal error.");
		}
	}
	Ref<User> HTTPSession::Authenticate()
	{
		boost::beast::http::fields::iterator it = request.find(boost::beast::http::field::authorization);
		if (it == request.end())
		{
			WriteError("Missing authorization field. Please call /api/help for more details.");
			return nullptr;
		}

		boost::beast::string_view authorization = it->value();
		if (strncmp(authorization.data(), "Bearer ", 7))
		{
			WriteError("Invalid authorization field. Please call /api/help for more details.");
			return nullptr;
		}

		authorization.remove_prefix(7);

		identifier_t userID = 0;
		try
		{
			jwt::decoded_jwt decodedToken = jwt::decode(authorization.to_string());

			userID = UserManager::GetInstance()->VerifyToken(decodedToken);
		}
		catch (std::exception)
		{
			return nullptr;
		}

		return UserManager::GetInstance()->GetUser(userID);
	}
	void HTTPSession::WriteError(const char* error)
	{
		boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response = boost::make_shared<boost::beast::http::response<boost::beast::http::buffer_body>>();
		response->result(boost::beast::http::status::bad_request);
		response->version(request.version());
		response->keep_alive(false);
		response->set(boost::beast::http::field::server, "HomeAutomation Server");
		response->set(boost::beast::http::field::content_type, "application/json");

		rapidjson::Document responseDocument = rapidjson::Document(rapidjson::kObjectType);

		JsonApi::BuildJsonErrorMessageHTTP(error, responseDocument);

		responseBuffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(responseBuffer);
		responseDocument.Accept(writer);

		boost::beast::http::buffer_body::value_type& buf = response->body();
		buf.data = (void*)responseBuffer.GetString();
		buf.size = responseBuffer.GetSize();
		buf.more = false;

		response->prepare_payload();

		boost::beast::http::async_write(*socket, *response, boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteBuffer, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, response)));
	}

	void HTTPSession::OnWrite(boost::system::error_code error, size_t size, boost::shared_ptr<boost::beast::http::response<boost::beast::http::empty_body>> response)
	{
		if (error)
			return;

		if (response->keep_alive())
		{
			request = {};
			socket->next_layer().expires_after(std::chrono::seconds(12));
			boost::beast::http::async_read(*socket, buffer, request, boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
		}
		else
		{
			//Shutdown
			socket->next_layer().expires_after(std::chrono::seconds(6));
			socket->async_shutdown(boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
		}
	}
	void HTTPSession::OnWriteString(boost::system::error_code error, size_t size, boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response)
	{
		if (error)
			return;

		if (response->keep_alive())
		{
			request = {};
			socket->next_layer().expires_after(std::chrono::seconds(12));
			boost::beast::http::async_read(*socket, buffer, request, boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
		}
		else
		{
			//Shutdown
			socket->next_layer().expires_after(std::chrono::seconds(6));
			socket->async_shutdown(boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
		}
	}
	void HTTPSession::OnWriteBuffer(boost::system::error_code error, size_t size, boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response)
	{
		if (error)
			return;

		if (response->keep_alive())
		{
			request = {};
			socket->next_layer().expires_after(std::chrono::seconds(12));
			boost::beast::http::async_read(*socket, buffer, request, boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
		}
		else
		{
			//Shutdown
			socket->next_layer().expires_after(std::chrono::seconds(6));
			socket->async_shutdown(boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
		}
	}

	void HTTPSession::OnShutdown(boost::system::error_code error)
	{
		if (error)
			return;

		//At this point the session will close itself
		socket->next_layer().close();
	}
}