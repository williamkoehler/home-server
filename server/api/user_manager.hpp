#pragma once
#include "message.hpp"
#include "user.hpp"
#include "common.hpp"

namespace server
{
    namespace api
    {
        class Verifier;

        class UserManager : public boost::enable_shared_from_this<UserManager>
        {
          private:
            boost::atomic<time_t> timestamp = 0;

            robin_hood::unordered_node_map<identifier_t, Ref<User>> userList;

            // Authentication
            uint8_t* authenticationKey = nullptr;
            std::string issuer;

            Ref<Verifier> verifier;

            void CalculateHash(const std::string_view& password, uint8_t* salt, uint8_t* digest);

            // Database
            bool LoadUser(identifier_t userID, const std::string& name, uint8_t hash[SHA256_SIZE],
                          uint8_t salt[SALT_SIZE], const std::string& accessLevel);

          public:
            UserManager(const std::string& issuer);
            virtual ~UserManager();
            static Ref<UserManager> Create(const std::string& issuer);
            static Ref<UserManager> GetInstance();

            // Timestamp
            void UpdateTimestamp();
            time_t GetLastTimestamp() const
            {
                return timestamp;
            }

            // User

            /// @brief Add user using password
            ///
            /// @param name Name
            /// @param password Password
            /// @param accessLevel Access level
            /// @return User or null in case of an error
            Ref<User> AddUser(const std::string& name, const std::string& password, UserAccessLevel accessLevel);

            /// @brief Add user using hash and salt
            ///
            /// @param name Name
            /// @param hash Hash
            /// @param salt Salt
            /// @param accessLevel Access level
            /// @return User or null in case of an error
            Ref<User> AddUser(const std::string& name, uint8_t* hash, uint8_t* salt, UserAccessLevel accessLevel);

            size_t GetUserCount()
            {
                // boost::shared_lock_guard lock(mutex);
                return userList.size();
            }
            Ref<User> GetUser(identifier_t userID);
            Ref<User> GetUserByName(const std::string_view& name);

            /// @brief Set user password
            ///
            /// @param userID User ID
            /// @param passwd Old password
            /// @param newPasswd New password
            /// @return Successfulness
            bool SetUserPassword(identifier_t userID, const std::string& passwd, const std::string& newPasswd);

            /// @brief Set user password
            ///
            /// @param user User ID
            /// @param passwd
            /// @param newPasswd
            /// @return true
            /// @return false
            bool SetUserPassword(const Ref<User>& user, const std::string& passwd, const std::string& newPasswd);

            /// @brief Authenticate user using name and password
            ///
            /// @param name Name
            /// @param password Password
            /// @return User or null if credentials are not valid
            Ref<User> Authenticate(const std::string_view& name, const std::string_view& password);

            /// @brief Remove user
            ///
            /// @param userID User id
            /// @return Successfulness
            bool RemoveUser(identifier_t userID);

            //! JWT

            /// @brief Generete JWT Token
            ///
            /// @param user Authenticated user (not null)
            /// @return JWT Token
            std::string GenerateJWTToken(const Ref<User>& user);

            /// @brief Verify JWT Token
            ///
            /// @param decoded Decoded
            /// @return User id or null in case of an error
            identifier_t VerifyJWTToken(const std::string& token);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            //! WebSocket Api
            static void WebSocketProcessGetUsersMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                        ApiResponseMessage& response,
                                                        const Ref<WebSocketSession>& session);
            static void WebSocketProcessAddUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                       ApiResponseMessage& response,
                                                       const Ref<WebSocketSession>& session);
            static void WebSocketProcessRemoveUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                          ApiResponseMessage& response,
                                                          const Ref<WebSocketSession>& session);
            static void WebSocketProcessGetUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                       ApiResponseMessage& response,
                                                       const Ref<WebSocketSession>& session);
            static void WebSocketProcessSetUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                       ApiResponseMessage& response,
                                                       const Ref<WebSocketSession>& session);
        };
    }
}