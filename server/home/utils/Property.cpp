#include "Property.hpp"

namespace home
{
	Ref<NullProperty> NullProperty::Create() { return boost::make_shared<NullProperty>(); }
	PropertyType NullProperty::GetType() const { return PropertyType::kUnknownType; }

	//! Boolean
	Ref<BooleanProperty> BooleanProperty::Create() { return boost::make_shared<BooleanProperty>(); }
	PropertyType BooleanProperty::GetType() const { return PropertyType::kBooleanType; }
	bool BooleanProperty::GetBoolean() { return value; }
	void BooleanProperty::SetBoolean(bool v) { value = v; }
	rapidjson::Value BooleanProperty::ToJson(rapidjson::Document::AllocatorType &allocator) { return rapidjson::Value(value); }

	//! Integer
	Ref<IntegerProperty> IntegerProperty::Create() { return boost::make_shared<IntegerProperty>(); }
	PropertyType IntegerProperty::GetType() const { return PropertyType::kIntegerType; }
	int64_t IntegerProperty::GetInteger() { return value; }
	void IntegerProperty::SetInteger(int64_t v) { value = v; }
	rapidjson::Value IntegerProperty::ToJson(rapidjson::Document::AllocatorType &allocator) { return rapidjson::Value(value); }

	//! Number
	Ref<NumberProperty> NumberProperty::Create() { return boost::make_shared<NumberProperty>(); }
	PropertyType NumberProperty::GetType() const { return PropertyType::kNumberType; }
	double NumberProperty::GetNumber() { return value; }
	void NumberProperty::SetNumber(double v) { value = v; }
	rapidjson::Value NumberProperty::ToJson(rapidjson::Document::AllocatorType &allocator) { return rapidjson::Value(value); }

	//! String
	Ref<StringProperty> StringProperty::Create() { return boost::make_shared<StringProperty>(); }
	PropertyType StringProperty::GetType() const { return PropertyType::kStringType; }
	std::string StringProperty::GetString() { return value; }
	void StringProperty::SetString(const std::string &v) { value = v; }
	rapidjson::Value StringProperty::ToJson(rapidjson::Document::AllocatorType &allocator) { return rapidjson::Value(value.data(), value.size()); }

	//! Endpoint
	Ref<EndpointProperty> EndpointProperty::Create() { return boost::make_shared<EndpointProperty>(); }
	PropertyType EndpointProperty::GetType() const { return PropertyType::kEndpointType; }
	Endpoint EndpointProperty::GetEndpoint() { return value; }
	void EndpointProperty::SetEndpoint(const Endpoint &v) { value = v; }
	rapidjson::Value EndpointProperty::ToJson(rapidjson::Document::AllocatorType &allocator)
	{
		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

		json.AddMember("_class", rapidjson::Value("endpoint"), allocator);
		json.AddMember("host", rapidjson::Value(value.host.data(), value.host.size(), allocator), allocator);
		json.AddMember("port", rapidjson::Value(value.port), allocator);

		return json;
	}

	//! Color
	Ref<ColorProperty> ColorProperty::Create() { return boost::make_shared<ColorProperty>(); }
	PropertyType ColorProperty::GetType() const { return PropertyType::kColorType; }
	Color ColorProperty::GetColor() { return value; }
	void ColorProperty::SetColor(const Color &v) { value = v; }
	rapidjson::Value ColorProperty::ToJson(rapidjson::Document::AllocatorType &allocator)
	{
		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

		json.AddMember("_class", rapidjson::Value("color"), allocator);
		json.AddMember("r", rapidjson::Value(value.red), allocator);
		json.AddMember("g", rapidjson::Value(value.green), allocator);
		json.AddMember("b", rapidjson::Value(value.blue), allocator);

		return json;
	}
}