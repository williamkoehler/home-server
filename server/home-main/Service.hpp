#pragma once
#include "Entity.hpp"
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/main/ServiceView.hpp>

namespace server
{
    namespace main
    {
        class ServiceView;

        class Service final : public Entity
        {
          protected:
            Ref<ServiceView> view;

          public:
            Service(identifier_t id, const std::string& name);
            virtual ~Service();
            static Ref<Service> Create(identifier_t id, const std::string& name);

            virtual EntityType GetType() override
            {
                return EntityType::kServiceEntityType;
            }

            /// @brief Get view
            ///
            /// @return Ref<scripting::View> Service view
            virtual Ref<scripting::View> GetView() override
            {
                return boost::static_pointer_cast<scripting::View>(view);
            }

            /// @brief Get service view
            ///
            /// @return Ref<ServiceView> Service view
            inline Ref<ServiceView> GetServiceView()
            {
                return view;
            }

            virtual void JsonGetConfig(rapidjson::Value& output,
                                       rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSetConfig(const rapidjson::Value& input) override;
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