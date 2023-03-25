#include "NativeScript.hpp"
#include "NativeScriptSource.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScript::NativeScript(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource,
                                       UniqueRef<NativeScriptImpl> scriptImpl)
                : Script(view, scriptSource), scriptImpl(std::move(scriptImpl))
            {
            }
            NativeScript::~NativeScript()
            {
            }
            Ref<Script> NativeScript::Create(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource)
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
                if (!attributeMap.contains(name))
                {
                    // Parse attribute
                    rapidjson::Document document;

                    document.Parse(json);

                    // Add attribute to list
                    if (!document.HasParseError())
                    {
                        attributeMap[name] = std::move(document);
                        return true;
                    }
                }

                return false;
            }
            bool NativeScript::RemoveAttribute(const std::string& name)
            {
                return attributeMap.erase(name);
            }
            void NativeScript::ClearAttributes()
            {
                attributeMap.clear();
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
                    if (property.GetGetterFunction() != nullptr)
                    {
                        // Invoke getter according to property type
                        switch (property.GetType())
                        {
                        case ValueType::kBooleanType:
                            return Value((*property.GetGetterFunction<bool>())(scriptImpl.get()));
                        case ValueType::kNumberType:
                            return Value((*property.GetGetterFunction<double>())(scriptImpl.get()));
                        case ValueType::kStringType:
                            return Value((*property.GetGetterFunction<std::string>())(scriptImpl.get()));
                        case ValueType::kEndpointType:
                            return Value((*property.GetGetterFunction<Endpoint>())(scriptImpl.get()));
                        case ValueType::kColorType:
                            return Value((*property.GetGetterFunction<Color>())(scriptImpl.get()));
                        case ValueType::kRoomIDType:
                            return Value::Create<ValueType::kRoomIDType>(
                                (*property.GetGetterFunction<identifier_t>())(scriptImpl.get()));
                        case ValueType::kDeviceIDType:
                            return Value::Create<ValueType::kDeviceIDType>(
                                (*property.GetGetterFunction<identifier_t>())(scriptImpl.get()));
                        case ValueType::kServiceIDType:
                            return Value::Create<ValueType::kServiceIDType>(
                                (*property.GetGetterFunction<identifier_t>())(scriptImpl.get()));
                        default:
                            break;
                        }
                    }
                }

                return Value();
            }

            void NativeScript::SetProperty(const std::string& name, const Value& value)
            {
                robin_hood::unordered_node_map<std::string, Property>::const_iterator it = propertyList.find(name);
                if (it != propertyList.end())
                {
                    const Property& property = it->second;

                    if (property.GetGetterFunction() != nullptr && value.GetType() == property.GetType())
                    {
                        // Invoke getter according to property type
                        switch (property.GetType())
                        {
                        case ValueType::kBooleanType:
                            (*property.GetSetterFunction<bool>())(scriptImpl.get(), value.GetBoolean());
                            break;
                        case ValueType::kNumberType:
                            (*property.GetSetterFunction<double>())(scriptImpl.get(), value.GetNumber());
                            break;
                        case ValueType::kStringType:
                            (*property.GetSetterFunction<std::string>())(scriptImpl.get(), value.GetString());
                            break;
                        case ValueType::kEndpointType:
                            (*property.GetSetterFunction<Endpoint>())(scriptImpl.get(), value.GetEndpoint());
                            break;
                        case ValueType::kColorType:
                            (*property.GetSetterFunction<Color>())(scriptImpl.get(), value.GetColor());
                            break;
                        case ValueType::kRoomIDType:
                            (*property.GetSetterFunction<identifier_t>())(scriptImpl.get(), value.GetRoomID());
                            break;
                        case ValueType::kDeviceIDType:
                            (*property.GetSetterFunction<identifier_t>())(scriptImpl.get(), value.GetDeviceID());
                            break;
                        case ValueType::kServiceIDType:
                            (*property.GetSetterFunction<identifier_t>())(scriptImpl.get(), value.GetServiceID());
                            break;
                        default:
                            break;
                        }
                    }
                }
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
                            return (*method.GetFunction<bool>())(scriptImpl.get(), name, parameter.GetBoolean());
                        case ValueType::kNumberType:
                            return (*method.GetFunction<double>())(scriptImpl.get(), name, parameter.GetNumber());
                        case ValueType::kStringType:
                            return (*method.GetFunction<std::string>())(scriptImpl.get(), name, parameter.GetString());
                        case ValueType::kEndpointType:
                            return (*method.GetFunction<Endpoint>())(scriptImpl.get(), name, parameter.GetEndpoint());
                        case ValueType::kColorType:
                            return (*method.GetFunction<Color>())(scriptImpl.get(), name, parameter.GetColor());
                        case ValueType::kRoomIDType:
                            return (*method.GetFunction<identifier_t>())(scriptImpl.get(), name, parameter.GetRoomID());
                        case ValueType::kDeviceIDType:
                            return (*method.GetFunction<identifier_t>())(scriptImpl.get(), name, parameter.GetDeviceID());
                        case ValueType::kServiceIDType:
                            return (*method.GetFunction<identifier_t>())(scriptImpl.get(), name, parameter.GetServiceID());
                        case ValueType::kNullType:
                            return (*method.GetFunction<Void>())(scriptImpl.get(), name, Void());
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

            Event NativeScript::AddEvent(const std::string& name)
            {
                return eventMap.emplace(name, Event()).first->second;
            }
            bool NativeScript::RemoveEvent(const std::string& name)
            {
                return eventMap.erase(name);
            }
            void NativeScript::ClearEvents()
            {
                eventMap.clear();
            }

            void NativeScript::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
            {
                assert(output.IsObject());

                output.MemberReserve(propertyList.size(), allocator);
                for (auto& [name, property] : propertyList)
                {
                    // Add property
                    output.AddMember(rapidjson::Value(name.data(), name.size(), allocator),
                                     GetProperty(name).JsonGet(allocator), allocator);
                }
            }
            void NativeScript::JsonSetState(rapidjson::Value& input)
            {
                assert(input.IsObject());

                for (rapidjson::Value::MemberIterator propertyIt = input.MemberBegin(); propertyIt != input.MemberEnd();
                     propertyIt++)
                {
                    SetProperty(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()),
                                Value::Create(propertyIt->value));
                }
            }
        }
    }
}