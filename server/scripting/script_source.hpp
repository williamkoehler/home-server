#pragma once
#include "common.hpp"
#include "view/view.hpp"
#include <api/message.hpp>
#include <api/user.hpp>
#include <api/websocket_session.hpp>

namespace server
{
    namespace scripting
    {
        class Script;

        enum ScriptFlags
        {
            kScriptFlag_None = 0x00,
            kScriptFlag_RoomSupport = 0x01,
            kScriptFlag_DeviceSupport = 0x02,
            kScriptFlag_ServiceSupport = 0x04,
        };

        enum class ScriptLanguage
        {
            kUnknownScriptLanguage,
            kNativeScriptLanguage,
            kJSScriptLanguage
        };

        std::string StringifyScriptLanguage(ScriptLanguage language);
        ScriptLanguage ParseScriptLanguage(const std::string& language);

        class ScriptSource : public boost::enable_shared_from_this<ScriptSource>
        {
          protected:
            const identifier_t id;

            std::string name;

            std::string content;

            /// @brief Checksum (changes when the data changes)
            boost::atomic_uint64_t checksum;

            inline void UpdateChecksum()
            {
                checksum = XXH64(content.data(), content.size(), 0x323435367A683567);
            }

            bool updateNeeded;

          public:
            ScriptSource(identifier_t id, const std::string& name, const std::string_view& content);
            virtual ~ScriptSource();

            /// @brief Get script source id
            ///
            /// @return identifier_t Script source id
            inline identifier_t GetID() const
            {
                return id;
            }

            /// @brief Get script source name
            ///
            /// @return std::string Script source name
            inline std::string GetName() const
            {
                return name;
            }

            /// @brief Set script source name
            ///
            /// @param v New script source name
            inline void SetName(const std::string& v)
            {
                name = v;
                updateNeeded = true;
            }

            /// @brief Get script flags
            ///
            /// @return uint8_t Script flags
            virtual uint8_t GetFlags() const = 0;

            /// @brief Get script language
            ///
            /// @return ScriptLanguage Script language
            virtual ScriptLanguage GetLanguage() const = 0;

            virtual std::string GetContent() const
            {
                return content;
            }

            /// @brief Set content / source code
            ///
            /// @param v Source code
            virtual void SetContent(const std::string_view& v)
            {
                content = v;
                updateNeeded = true;
            }

            /// @brief Get content checksum
            ///
            /// @return uint64_t Content checksum
            inline uint64_t GetChecksum() const
            {
                return checksum;
            }

            /// @brief Create script from script source
            ///
            /// @param view Sender view
            /// @return Script
            virtual Ref<Script> CreateScript(const Ref<View>& view) = 0;

            /// @brief Save/update entity data in database
            ///
            /// @return Successfulness
            bool Save() const;

            /// @brief Save/update entity content in database
            ///
            /// @return Successfulness
            bool SaveContent() const;

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;
            bool JsonSet(const rapidjson::Value& input);

            virtual void JsonGetConfig(rapidjson::Value& output,
                                       rapidjson::Document::AllocatorType& allocator) const = 0;
            virtual bool JsonSetConfig(const rapidjson::Value& input) = 0;
            void JsonGetContent(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;
            bool JsonSetContent(const rapidjson::Value& input);

            void ApiGet(const api::ApiRequestMessage& request, api::ApiResponseMessage& response,
                        const Ref<api::WebSocketSession>& session) const;
            bool ApiSet(const api::ApiRequestMessage& request, api::ApiResponseMessage& response,
                        const Ref<api::WebSocketSession>& session);

            void ApiGetContent(const api::ApiRequestMessage& request, api::ApiResponseMessage& response,
                               const Ref<api::WebSocketSession>& session) const;
            bool ApiSetContent(const api::ApiRequestMessage& request, api::ApiResponseMessage& response,
                               const Ref<api::WebSocketSession>& session);
        };
    }
}