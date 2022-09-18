#pragma once
#include "../common.hpp"
#include "Controller.hpp"

namespace server
{
    namespace scripting
    {
        enum class HttpStatusCode : uint16_t
        {
            // 1xx Informational
            kContinue = 100,
            kSwitchingprotocols = 101,
            kProcessing = 102,
            kEarlyHints = 103,

            // 2xx Succesful
            kOK = 200,
            kCreated = 201,
            kAccepted = 202,
            kNonAuthoritativeInformation = 203,
            kNoContent = 204,
            kResetContent = 205,
            kPartialContent = 206,
            kMultiStatus = 207,
            kAlreadyReported = 208,
            kIMUsed = 226,

            // 3xx Redirection
            kMultipleChoices = 300,
            kMovedPermanently = 301,
            kFoundPreviously = 302,
            kSeeOther = 303,
            kNotModified = 304,
            kUseProxy = 305,
            kSwitchProxy = 306,
            kTemporaryRedirect = 307,
            kPermanentRedirect = 308,

            // 4xx Client Error
            kBadRequest = 400,
            kUnauthorized = 401,
            kPaymentRequired = 402,
            kForbidden = 403,
            kNotFound = 404,
            kMethodNotAllowed = 405,
            kNotAcceptable = 406,
            kProxyAuthenticationRequired = 407,
            kRequestTimeout = 408,
            kConflict = 409,
            kGone = 410,
            kLengthRequired = 411,
            kPreconditionFailed = 412,
            kPayloadTooLarge = 413,
            kURITooLong = 414,
            kUnsupportedMediaType = 415,
            kRangeNotSatisfiable = 416,
            kExpectationFailed = 417,
            kImATeapot = 418,
            kMisdirectedRequest = 421,
            kUnprocessableEntity = 422,
            kLocked = 423,
            kFailedDependency = 424,
            kTooEarly = 425,
            kUpgradeRequired = 426,
            kPreconditionRequired = 428,
            kTooManyRequests = 429,
            kRequestHeaderFieldsTooLarge = 431,
            kUnavailableForLegalReasons = 451,

            // 5xx Server Error
            kInternalServerError = 500,
            kNotImplemented = 501,
            kBadGateway = 502,
            kServiceUnavailable = 503,
            kGatewayTimeout = 504,
            kHTTPVersionNotSupported = 505,
            kVariantAlsoNegotiates = 506,
            kInsufficientStorage = 507,
            kLoopDetected = 508,
            kNotExtended = 510,
            kNetworkAuthenticationRequired = 511,

            // 10xx Internal Error
            kInternalError = 1000,
            kResolveError = 1001,
            kConnectError = 1002,
            kHandshakeError = 1003,
        };

        enum class HttpMethod
        {
            kUnknown,
            kGet,
            kHead,
            kPost,
            kPut,
            kDelete,
            kConnect,
            kOptions,
            kTrace,
            kPatch,
        };

        class HttpController : public Controller
        {
          protected:
            HttpStatusCode statusCode;

            CallbackMethod<> callback;

          public:
            HttpController(Ref<Script> script, CallbackMethod<> callback);

            virtual ControllerType GetType() const override
            {
                return ControllerType::kHttpController;
            }

            /// @brief Get HTTP status code
            ///
            /// @return Status code
            HttpStatusCode GetStatusCode() const
            {
                return statusCode;
            }

            /// @brief Is status code a internal error (over 1000)
            /// Note: Has nothing to do with HTTP status codes
            ///
            /// @return
            bool HasInternalError()
            {
                return (uint16_t)statusCode >= 1000;
            }

            /// @brief Is status code a success (between 200 and 299)
            ///
            /// @return Success status
            bool IsOK() const
            {
                return (uint16_t)statusCode >= 200 && (uint16_t)statusCode <= 299;
            }

            /// @brief Http response content
            ///
            /// @return Content
            virtual std::string_view GetContent() = 0;
        };

        class Http
        {
          public:
            /// @brief Send http request
            ///
            /// @param callback Method id
            /// @param method Http method
            /// @param target URL Target
            /// @param content Body Content
            /// @return Successfulness
            static bool Send(Ref<Script> script, const std::string& host, uint16_t port,
                             HttpMethod method = HttpMethod::kGet, const std::string& target = "/",
                             const std::string_view& content = std::string_view("", 0),
                             CallbackMethod<> callback = nullptr);

            /// @brief Send http get request
            ///
            /// @param target URL Target
            /// @param content Body Content
            /// @param callback Callback
            /// @return Successfulness
            template <class T>
            inline static bool Get(Ref<Script> script, const std::string& host, uint16_t port,
                                   const std::string& target = "/",
                                   const std::string_view& content = std::string_view("", 0),
                                   CallbackMethod<T> callback = nullptr)
            {
                return Send(script, host, port, HttpMethod::kGet, target, content,
                            CallbackMethodConversion<T>{callback}.f2);
            }

            /// @brief Send http post request
            ///
            /// @param target URL Target
            /// @param content Body Content
            /// @param callback Callback
            /// @return Successfulness
            template <class T>
            inline static bool Post(Ref<Script> script, const std::string& host, uint16_t port,
                                    const std::string& target = "/",
                                    const std::string_view& content = std::string_view("", 0),
                                    CallbackMethod<T> callback = nullptr)
            {
                return Send(script, host, port, HttpMethod::kGet, target, content,
                            CallbackMethodConversion<T>{callback}.f2);
            }
        };

        class Https
        {
          public:
            /// @brief Send https request
            ///
            /// @param callback Method id
            /// @param method Http method
            /// @param target URL Target
            /// @param content Body Content
            /// @return Successfulness
            static bool Send(Ref<Script> script, const std::string& host, uint16_t port,
                             HttpMethod method = HttpMethod::kGet, const std::string& target = "/",
                             const std::string_view& content = std::string_view("", 0),
                             CallbackMethod<> callback = nullptr);

            /// @brief Send https get request
            ///
            /// @param callback Method id
            /// @param target URL Target
            /// @param content Body Content
            /// @return Successfulness
            template <class T>
            inline static bool Get(Ref<Script> script, const std::string& host, uint16_t port,
                                   const std::string& target = "/",
                                   const std::string_view& content = std::string_view("", 0),
                                   CallbackMethod<T> callback = nullptr)
            {
                return Send(script, host, port, HttpMethod::kGet, target, content,
                            CallbackMethodConversion<T>{callback}.f2);
            }

            /// @brief Send https post request
            ///
            /// @param callback Method id
            /// @param target URL Target
            /// @param content Body Content
            /// @return Successfulness
            template <class T>
            inline static bool Post(Ref<Script> script, const std::string& host, uint16_t port,
                                    const std::string& target = "/",
                                    const std::string_view& content = std::string_view("", 0),
                                    CallbackMethod<T> callback = nullptr)
            {
                return Send(script, host, port, HttpMethod::kGet, target, content,
                            CallbackMethodConversion<T>{callback}.f2);
            }
        };
    }
}