#pragma once
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/main/ServiceView.hpp>

namespace server
{
    namespace main
    {
        class ServiceView;

        class Service : public boost::enable_shared_from_this<Service>
        {
          protected:
            const identifier_t id;
            std::string name;
            Ref<scripting::Script> script;

            Ref<ServiceView> view;

          public:
            Service(identifier_t id, const std::string& name);
            virtual ~Service();
            static Ref<Service> Create(identifier_t id, const std::string& name, identifier_t scriptSourceID);

            /// @brief Get service id
            ///
            /// @return Service id
            inline identifier_t GetID()
            {
                return id;
            }

            /// @brief Get service name
            ///
            /// @return Service name
            std::string GetName();

            /// @brief Set service name
            ///
            /// @param v New service name
            /// @return Successfulness
            bool SetName(const std::string& v);

            /// @brief Set script source id
            ///
            /// @param scriptSource Script source id
            /// @return Successfulness
            bool SetScriptSourceID(identifier_t scriptSourceID);

            /// @brief Get script source id
            ///
            /// @return Script source id
            identifier_t GetScriptSourceID();

            /// @brief Get service view
            ///
            /// @return Get service view of this object
            Ref<ServiceView> GetView();

            /// @brief Invoke script method
            ///
            /// @param event Method name
            void Invoke(const std::string& method, const scripting::Value& parameter);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetState(rapidjson::Value& input);
        };

        class ServiceView : public scripting::ServiceView
        {
          private:
            WeakRef<Service> service;

          public:
            ServiceView(const Ref<Service>& service);
            virtual ~ServiceView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::Value& parameter) override;
        };
    }
}