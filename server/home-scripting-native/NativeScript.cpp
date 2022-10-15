#include "NativeScript.hpp"
#include "NativeScriptSource.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScript::NativeScript(Ref<View> view, Ref<NativeScriptSource> scriptSource,
                                       UniqueRef<NativeScriptImpl> scriptImpl)
                : Script(view, scriptSource), scriptImpl(std::move(scriptImpl))
            {
            }
            NativeScript::~NativeScript()
            {
            }
            Ref<Script> NativeScript::Create(Ref<View> view, Ref<NativeScriptSource> scriptSource)
            {
                UniqueRef<NativeScriptImpl> scriptImpl = scriptSource->GetCreateCallback()();

                if (scriptImpl != nullptr)
                {
                    Ref<NativeScript> script =
                        boost::make_shared<NativeScript>(view, scriptSource, std::move(scriptImpl));

                    // Initialize script implementation
                    script->scriptImpl->script = script;

                    return script->shared_from_this();
                }
                else
                    return nullptr;
            }

            bool NativeScript::Initialize()
            {
                return scriptImpl->Initialize();
            }

            bool NativeScript::AddAttribute(const std::string& name, const char* json)
            {
                // Check existance
                if (!attributeList.contains(name))
                {
                    // Parse attribute
                    rapidjson::Document document;

                    document.Parse(json);

                    // Add attribute to list
                    if (!document.HasParseError())
                    {
                        attributeList[name] = std::move(document);
                        return true;
                    }
                }

                return false;
            }
            bool NativeScript::RemoveAttribute(const std::string& name)
            {
                return attributeList.erase(name);
            }
            void NativeScript::ClearAttributes()
            {
                attributeList.clear();
            }

            bool NativeScript::AddProperty(const std::string& name, const Property& property)
            {
                // Check existance
                if (!propertyList.contains(name))
                {
                    // Add property to list
                    propertyList[name] = property;
                    return true;
                }

                return false;
            }
            bool NativeScript::RemoveProperty(const std::string& name)
            {
                return propertyList.erase(name);
            }
            void NativeScript::ClearProperties()
            {
                propertyList.clear();
            }

            bool NativeScript::AddMethod(const std::string& name, const Method& method)
            {
                // Check existance
                if (!methodList.contains(name))
                {
                    // Add method to list
                    methodList[name] = method;
                    return true;
                }
                else
                    return false;
            }

            Value NativeScript::GetProperty(const std::string& name)
            {
                robin_hood::unordered_node_map<std::string, Property>::const_iterator it = propertyList.find(name);
                if (it != propertyList.end())
                {
                    const Property& property = it->second;
                    if (property.GetGetter() != nullptr)
                    {
                        // Invoke getter according to property type
                        switch (property.GetType())
                        {
                        case ValueType::kBooleanType:
                            return Value((scriptImpl.get()->*(property.GetGetter<bool>()))());
                        case ValueType::kNumberType:
                            return Value((scriptImpl.get()->*(property.GetGetter<double>()))());
                        case ValueType::kStringType:
                            return Value((scriptImpl.get()->*(property.GetGetter<std::string>()))());
                        case ValueType::kEndpointType:
                            return Value((scriptImpl.get()->*(property.GetGetter<Endpoint>()))());
                        case ValueType::kColorType:
                            return Value((scriptImpl.get()->*(property.GetGetter<Color>()))());
                        case ValueType::kRoomIDType:
                            return Value::Create<ValueType::kRoomIDType>(
                                (scriptImpl.get()->*(property.GetGetter<identifier_t>()))());
                        case ValueType::kDeviceIDType:
                            return Value::Create<ValueType::kDeviceIDType>(
                                (scriptImpl.get()->*(property.GetGetter<identifier_t>()))());
                        case ValueType::kServiceIDType:
                            return Value::Create<ValueType::kServiceIDType>(
                                (scriptImpl.get()->*(property.GetGetter<identifier_t>()))());
                        default:
                            break;
                        }
                    }
                }

                return Value();
            }

            void NativeScript::SetProperty(const std::string& name, const Value& value)
            {
            }

            bool NativeScript::Invoke(const std::string& name, const Value& parameter)
            {
                robin_hood::unordered_node_map<std::string, Method>::const_iterator it = methodList.find(name);
                if (it != methodList.end())
                {
                    // Check method validity
                    const Method& method = it->second;
                    if (method.GetMethod() != nullptr && parameter.GetType() == method.GetParameterType())
                    {
                        // Invoke method according to parameter type
                        switch (method.GetParameterType())
                        {
                        case ValueType::kBooleanType:
                            return (scriptImpl.get()->*(method.GetMethod<bool>()))(name, parameter.GetBoolean());
                        case ValueType::kNumberType:
                            return (scriptImpl.get()->*(method.GetMethod<double>()))(name, parameter.GetNumber());
                        case ValueType::kStringType:
                            return (scriptImpl.get()->*(method.GetMethod<std::string>()))(name, parameter.GetString());
                        case ValueType::kEndpointType:
                            return (scriptImpl.get()->*(method.GetMethod<Endpoint>()))(name, parameter.GetEndpoint());
                        case ValueType::kColorType:
                            return (scriptImpl.get()->*(method.GetMethod<Color>()))(name, parameter.GetColor());
                        case ValueType::kRoomIDType:
                            return (scriptImpl.get()->*(method.GetMethod<identifier_t>()))(name, parameter.GetRoomID());
                        case ValueType::kDeviceIDType:
                            return (scriptImpl.get()->*(method.GetMethod<identifier_t>()))(name,
                                                                                           parameter.GetDeviceID());
                        case ValueType::kServiceIDType:
                            return (scriptImpl.get()->*(method.GetMethod<identifier_t>()))(name,
                                                                                           parameter.GetServiceID());
                        case ValueType::kNullType:
                            return (scriptImpl.get()->*(method.GetMethod<Void>()))(name, Void());
                        default:
                            return false;
                        }
                    }
                }

                return false;
            }

            bool NativeScript::RemoveMethod(const std::string& name)
            {
                return methodList.erase(name);
            }
            void NativeScript::ClearMethods()
            {
                methodList.clear();
            }

            Ref<Event> NativeScript::AddEvent(const std::string& name)
            {
                // Check existance
                if (!eventList.contains(name))
                {
                    // Create event instance
                    Ref<Event> r = Event::Create();

                    // Add event to list
                    if (r != nullptr)
                    {
                        eventList[name] = r;
                        return r;
                    }
                }

                return nullptr;
            }
            bool NativeScript::RemoveEvent(const std::string& name)
            {
                return eventList.erase(name);
            }
            void NativeScript::ClearEvents()
            {
                eventList.clear();
            }
        }
    }
}