#pragma once
#include "entity.hpp"
#include "common.hpp"
#include <scripting/script.hpp>
#include <scripting_sdk/view/main/service_view.hpp>

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

            virtual EntityType GetType() const override
            {
                return EntityType::kServiceEntityType;
            }

            /// @brief Get view
            ///
            /// @return Ref<scripting::sdk::View> Service view
            virtual Ref<scripting::sdk::View> GetView() override
            {
                return boost::static_pointer_cast<scripting::sdk::View>(view);
            }

            /// @brief Get service view
            ///
            /// @return Ref<ServiceView> Service view
            inline Ref<ServiceView> GetServiceView()
            {
                return view;
            }

            virtual void JsonGetAttributes(rapidjson::Value& output,
                                       rapidjson::Document::AllocatorType& allocator) const override;
            virtual bool JsonSetAttributes(const rapidjson::Value& input) override;
        };

        class ServiceView : public scripting::sdk::ServiceView
        {
          private:
            WeakRef<Service> service;

          public:
            ServiceView(const Ref<Service>& service);
            virtual ~ServiceView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::sdk::Value& parameter) override;

            virtual void Publish() override;
            virtual void PublishState() override;
        };
    }
}