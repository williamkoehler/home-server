#include "WSSession.h"
#include "../Core.h"
#include "NetworkManager.h"
#include "../json/JsonApi.h"
#include "../home/Home.h"
#include "../home/Room.h"
#include "../home/Device.h"
#include "../user/User.h"
#include <streambuf>

namespace server
{
	WSSession::WSSession(Ref<ssl_socket_t> socket, Ref<User> user)
		: strand(socket->get_executor()), socket(boost::make_shared<websocket_t>(std::move(*socket))), user(user)
	{ }
	WSSession::~WSSession()
	{ }

	void WSSession::Run(boost::beast::http::request<boost::beast::http::string_body>& request)
	{
		socket->next_layer().next_layer().expires_after(std::chrono::seconds(12));
		socket->set_option(
			boost::beast::websocket::stream_base::decorator(
				[](boost::beast::websocket::response_type& response) -> void
				{
					response.set(boost::beast::http::field::server, Core::GetInstance()->GetName());
				}
			)
		);
		socket->async_accept(request, boost::asio::bind_executor(strand, boost::bind(&WSSession::OnAccept, shared_from_this(), boost::placeholders::_1)));
	}

	void WSSession::OnAccept(boost::system::error_code error)
	{
		if (error)
			return;

		socket->next_layer().next_layer().expires_never();

		//Add ws to publish list
		{
			Ref<NetworkManager> networkManager = NetworkManager::GetInstance();
			boost::lock_guard lock(networkManager->mutex);
			networkManager->sessionList.push_back(shared_from_this());
		}

		socket->text(true);
		socket->async_read(buffer, boost::asio::bind_executor(strand, boost::bind(&WSSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
	}

	void WSSession::OnRead(boost::system::error_code error, size_t receivedBytes)
	{
		if (error)
			return;

		if (!socket->got_text())
		{
			DoWSShutdown(boost::beast::websocket::close_code::bad_payload);
			return;
		}
		if (!socket->is_message_done())
		{
			socket->async_read(buffer, boost::asio::bind_executor(strand, boost::bind(&WSSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
			return;
		}

		buffer.reserve(buffer.size() + 1);
		char* data = static_cast<char*>(buffer.data().data());
		data[buffer.size()] = '\0';
		rapidjson::StringStream stream = rapidjson::StringStream(static_cast<const char*>(buffer.data().data()));

		rapidjson::Document requestDocument;
		requestDocument.ParseStream(stream);

		if (requestDocument.HasParseError() || !requestDocument.IsObject())
		{
			DoWSShutdown(boost::beast::websocket::close_code::bad_payload, "Invalid JSON");
			return;
		}

		buffer.consume(receivedBytes);

		rapidjson::Value::MemberIterator messageIDIt = requestDocument.FindMember("msgid");
		rapidjson::Value::MemberIterator messageIt = requestDocument.FindMember("msg");
		if (messageIDIt == requestDocument.MemberEnd() || !messageIDIt->value.IsUint64() ||
			messageIt == requestDocument.MemberEnd() || !messageIt->value.IsString())
		{
			DoWSShutdown(boost::beast::websocket::close_code::bad_payload, "Invalid JSON");
			return;
		}

		rapidjson::Document responseDocument = rapidjson::Document(rapidjson::kObjectType);

		if (ProcessJsonApi(messageIDIt->value.GetUint64(), messageIt->value.GetString(), requestDocument, responseDocument))
			Send(responseDocument);

		//Wait for data
		socket->async_read(buffer, boost::asio::bind_executor(strand, boost::bind(&WSSession::OnRead, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
	}

	bool WSSession::ProcessJsonApi(size_t id, const char* msg, rapidjson::Document& input, rapidjson::Document& output)
	{
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		rapidjson::Value errorsJson;
		errorsJson.SetArray();

		ExecutionContext context(errorsJson, allocator);

		if (strncmp(msg, "get-", 4) == 0)
		{
			msg += 4;
			if (strncmp(msg, "device", 6) == 0)
			{
				msg += 6;
				if (strncmp(msg, "?state", 6) == 0) // get-device?state
					JsonApi::ProcessJsonGetDeviceStateMessageWS(input, output, context);
				else if (strncmp(msg, "s?state", 7) == 0) // get-devices?state
					JsonApi::ProcessJsonGetDeviceStatesMessageWS(input, output, context);
				else if (strncmp(msg, "?info", 5) == 0) // get-device?info
					JsonApi::ProcessJsonGetDeviceMessageWS(input, output, context);
				else
					return false;
			}
			else if (strncmp(msg, "devicemanager", 13) == 0)
			{
				msg += 13;
				if (strncmp(msg, "?state", 6) == 0) // get-devicemanager?state
					JsonApi::ProcessJsonGetDeviceManagerStateMessageWS(input, output, context);
				else if (strncmp(msg, "s?state", 7) == 0) // get-devicemanagers?state
					JsonApi::ProcessJsonGetDeviceManagerStatesMessageWS(input, output, context);
				else if (strncmp(msg, "?info", 5) == 0) // get-devicemanager?info
					JsonApi::ProcessJsonGetDeviceManagerMessageWS(input, output, context);
				else
					return false;
			}
			else if (strncmp(msg, "action", 6) == 0)
			{
				msg += 6;
				if (strncmp(msg, "?state", 6) == 0) // get-action?state
					JsonApi::ProcessJsonGetActionStateMessageWS(input, output, context);
				else if (strncmp(msg, "s?state", 7) == 0) // get-actions?state
					JsonApi::ProcessJsonGetDeviceStatesMessageWS(input, output, context); // Todo
				else if (strncmp(msg, "?info", 5) == 0) // get-action?info
					JsonApi::ProcessJsonGetActionMessageWS(input, output, context);
				else
					return false;
			}
			else if (strncmp(msg, "draft", 5) == 0)
			{
				msg += 5;
				if (strncmp(msg, "?info", 5) == 0) // get-draft?info
					JsonApi::ProcessJsonGetDraftSourceMessageWS(input, output, context);
				else if (strncmp(msg, "?content", 4) == 0) // get-draft?content
					JsonApi::ProcessJsonGetDraftSourceContentMessageWS(user, input, output, context); // Todo
				else if (strncmp(msg, "s?info", 4) == 0) // get-drafts?info
					JsonApi::ProcessJsonGetDraftSourcesMessageWS(input, output, context); // Todo
				else
					return false;
			}
			else if (strncmp(msg, "home?info", 9) == 0) // get-home?info
				JsonApi::ProcessJsonGetHomeMessageWS(input, output, context);
			else if (strncmp(msg, "room?info", 9) == 0) // get-room?info
				JsonApi::ProcessJsonGetRoomMessageWS(input, output, context);
			else if (strncmp(msg, "users?info", 10) == 0) // get-users?info
				JsonApi::ProcessJsonGetUsersMessageWS(input, output, context);
			else if (strncmp(msg, "plugins?info", 12) == 0) // get-plugins?info
				JsonApi::ProcessJsonGetPluginsMessageWS(input, output, context);
			else if (strncmp(msg, "settings?info", 13) == 0) // get-settings?info
				JsonApi::ProcessJsonGetSettingsMessageWS(user, input, output, context);
			else if (strncmp(msg, "changes?info", 12) == 0) // get-changes?info
				JsonApi::ProcessJsonGetTimestampsMessageWS(input, output, context);
			else
				return false;
		}
		else if (strncmp(msg, "set-", 4) == 0)
		{
			msg += 4;
			if (strncmp(msg, "home?info", 9) == 0) // set-home?info
				JsonApi::ProcessJsonSetHomeMessageWS(input, output, context);
			else if (strncmp(msg, "room?info", 9) == 0) // set-room?info
				JsonApi::ProcessJsonSetRoomMessageWS(input, output, context);
			else if (strncmp(msg, "device", 6) == 0)
			{
				msg += 6;
				if (strncmp(msg, "?state", 6) == 0) // set-device?state
					JsonApi::ProcessJsonSetDeviceStateMessageWS(input, output, context);
				else if (strncmp(msg, "?info", 5) == 0) // get-device?info
					JsonApi::ProcessJsonSetDeviceMessageWS(input, output, context);
				else
					return false;
			}
			else if (strncmp(msg, "devicemanager", 13) == 0)
			{
				msg += 13;
				if (strncmp(msg, "?state", 6) == 0) // set-devicemanager?state
					JsonApi::ProcessJsonSetDeviceManagerStateMessageWS(input, output, context);
				else if (strncmp(msg, "?info", 5) == 0) // get-devicemanager?info
					JsonApi::ProcessJsonSetDeviceManagerMessageWS(input, output, context);
				else
					return false;
			}
			else if (strncmp(msg, "action", 6) == 0)
			{
				msg += 6;
				if (strncmp(msg, "?state", 6) == 0) // set-action?state
					JsonApi::ProcessJsonSetActionStateMessageWS(input, output, context);
				else if (strncmp(msg, "?info", 5) == 0) // get-action?info
					JsonApi::ProcessJsonSetActionMessageWS(input, output, context);
				else
					return false;
			}
			else if (strncmp(msg, "draft", 5) == 0)
			{
				msg += 5;
				if (strncmp(msg, "?info", 5) == 0) // set-draft?info
					JsonApi::ProcessJsonSetDraftSourceMessageWS(input, output, context);
				else if (strncmp(msg, "?content", 4) == 0) // set-draft?content
					JsonApi::ProcessJsonSetDraftSourceContentMessageWS(user, input, output, context); // Todo
				else
					return false;
			}
			else if (strncmp(msg, "settings?info", 13) == 0) // set-settings?info
				JsonApi::ProcessJsonSetSettingsMessageWS(user, input, output, context);
			else
				return false;
		}
		else if (strncmp(msg, "add-", 4) == 0)
		{
			msg += 4;
			if (strncmp(msg, "devicemanager", 13) == 0) // add-devicemanager
				JsonApi::ProcessJsonAddDeviceManagerMessageWS(user, input, output, context);
			else if (strncmp(msg, "device", 6) == 0) // add-device
			{
				msg += 6;
				if (strncmp(msg, "to-room", 7) == 0) // add-device-to-room
					JsonApi::ProcessJsonAddDeviceToRoomMessageWS(user, input, output, context);
				else // add-device
					JsonApi::ProcessJsonAddDeviceMessageWS(user, input, output, context);
			}
			else if (strncmp(msg, "action", 6) == 0) // add-action
			{
				msg += 6;
				if (strncmp(msg, "to-room", 7) == 0) // add-action-to-room
					JsonApi::ProcessJsonAddDeviceToRoomMessageWS(user, input, output, context); // Todo
				else // add-action
					JsonApi::ProcessJsonAddActionMessageWS(user, input, output, context);
			}
			else if (strncmp(msg, "room", 4) == 0) // add-room
				JsonApi::ProcessJsonAddRoomMessageWS(user, input, output, context);
			else if (strncmp(msg, "draft", 5) == 0) // add-draft
				JsonApi::ProcessJsonAddDraftSourceMessageWS(user, input, output, context);
			else
				return false;
		}
		else if (strncmp(msg, "rem-", 4) == 0)
		{
			msg += 4;
			if (strncmp(msg, "devicemanager", 13) == 0) // rem-devicemanager
				JsonApi::ProcessJsonRemoveDeviceManagerMessageWS(user, input, output, context);
			else if (strncmp(msg, "device", 6) == 0) // rem-device
			{
				msg += 6;
				if (strncmp(msg, "from-room", 9) == 0) // rem-device-to-room
					JsonApi::ProcessJsonRemoveDeviceFromRoomMessageWS(user, input, output, context);
				else // rem-device
					JsonApi::ProcessJsonRemoveDeviceMessageWS(user, input, output, context);
			}
			else if (strncmp(msg, "action", 6) == 0) // rem-action
			{
				msg += 6;
				if (strncmp(msg, "from-room", 9) == 0) // rem-action-to-room
					JsonApi::ProcessJsonRemoveDeviceFromRoomMessageWS(user, input, output, context); // Todo
				else // rem-action
					JsonApi::ProcessJsonRemoveDeviceMessageWS(user, input, output, context);
			}
			else if (strncmp(msg, "room", 4) == 0) // rem-room
				JsonApi::ProcessJsonRemoveRoomMessageWS(user, input, output, context);
			else if (strncmp(msg, "draft", 5) == 0) // rem-draft
				JsonApi::ProcessJsonRemoveDraftSourceMessageWS(user, input, output, context);
			else
				return false;
		}
		else if (strncmp(msg, "exe-", 4) == 0)
		{
			msg += 4;
			if (strncmp(msg, "action", 6) == 0) // exe-action
				JsonApi::ProcessJsonExecuteActionMessageWS(input, output, context);
			else
				return false;
		}
		else
			return false;

		output.AddMember("errors", errorsJson, allocator);
		output.AddMember("msgid", rapidjson::Value(id), allocator);

		return true;
	}

	void WSSession::Send(rapidjson::Document& document)
	{
		Ref<rapidjson::StringBuffer> message = boost::make_shared<rapidjson::StringBuffer>();
		rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(*message);
		document.Accept(writer);

		boost::lock_guard lock(queueMutex);

		messageQueue.push_back(message);

		if (messageQueue.size() == 1)
		{
			socket->async_write(
				boost::asio::buffer(message->GetString(), message->GetSize()),
				boost::asio::bind_executor(strand, boost::bind(&WSSession::OnWrite, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, message)));
		}
	}
	void WSSession::Send(rapidjson::StringBuffer& buffer)
	{
		Ref<rapidjson::StringBuffer> message = boost::make_shared<rapidjson::StringBuffer>();
		message->Push(buffer.GetSize());
		memcpy((void*)message->GetString(), buffer.GetString(), buffer.GetSize());

		boost::lock_guard lock(queueMutex);

		messageQueue.push_back(message);

		if (messageQueue.size() == 1)
		{
			socket->async_write(
				boost::asio::buffer(message->GetString(), message->GetSize()),
				boost::asio::bind_executor(strand, boost::bind(&WSSession::OnWrite, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, message)));
		}
	}
	void WSSession::Send(Ref<rapidjson::StringBuffer> buffer)
	{
		boost::lock_guard lock(queueMutex);

		messageQueue.push_back(buffer);

		if (messageQueue.size() == 1)
		{
			socket->async_write(
				boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
				boost::asio::bind_executor(strand, boost::bind(&WSSession::OnWrite, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, buffer)));
		}
	}

	void WSSession::OnWrite(boost::system::error_code error, size_t sentBytes, Ref<rapidjson::StringBuffer> message)
	{
		if (error)
			return;

		boost::lock_guard lock(queueMutex);

		messageQueue.erase(messageQueue.begin());

		if (messageQueue.size())
		{
			Ref<rapidjson::StringBuffer> message = messageQueue.front();

			socket->async_write(
				boost::asio::buffer(message->GetString(), message->GetSize()),
				boost::asio::bind_executor(strand, boost::bind(&WSSession::OnWrite, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2, message)));
		}
	}

	void WSSession::DoWSShutdown(boost::beast::websocket::close_code code, const char* reason)
	{
		//Shutdown
		socket->next_layer().next_layer().expires_after(std::chrono::seconds(6));
		socket->async_close(boost::beast::websocket::close_reason(code, reason), boost::asio::bind_executor(strand, boost::bind(&WSSession::DoSSLShutdown, shared_from_this(), boost::placeholders::_1)));
	}
	void WSSession::DoSSLShutdown(boost::system::error_code error)
	{
		if (error)
			return;

		//Shutdown
		socket->next_layer().next_layer().expires_after(std::chrono::seconds(6));
		socket->next_layer().async_shutdown(boost::asio::bind_executor(strand, boost::bind(&WSSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
	}
	void WSSession::OnShutdown(boost::system::error_code error)
	{
		if (error)
			return;

		socket->next_layer().next_layer().close();
	}
}