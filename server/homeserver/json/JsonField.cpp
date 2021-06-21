#include "JsonField.h"

namespace server
{
	JsonReadableField::JsonReadableField(identifier_t id, rapidjson::Value& value)
		: id(id), value(value)
	{
	}
	JsonWriteableField::JsonWriteableField(identifier_t id, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
		: id(id), value(value), type(home::FieldTypes::kUnknownFieldType), allocator(allocator)
	{
	}

	bool JsonReadableField::GetBool(bool& v)
	{
		if (value.IsBool())
		{
			v = value.GetBool();
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetBool(bool v)
	{
		type = home::FieldTypes::kBooleanFieldType;
		value.SetBool(v);
	}

	bool JsonReadableField::GetInt8(int8_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<int8_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetInt8(int8_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}
	bool JsonReadableField::GetUInt8(uint8_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<uint8_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetUInt8(uint8_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}

	bool JsonReadableField::GetInt16(int16_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<int16_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetInt16(int16_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}
	bool JsonReadableField::GetUInt16(uint16_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<uint16_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetUInt16(uint16_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}

	bool JsonReadableField::GetInt32(int32_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<int32_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetInt32(int32_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}
	bool JsonReadableField::GetUInt32(uint32_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<uint32_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetUInt32(uint32_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}

	bool JsonReadableField::GetInt64(int64_t& v)
	{
		if (value.IsInt64())
		{
			v = value.GetInt64();
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetInt64(int64_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(v);
	}
	bool JsonReadableField::GetUInt64(uint64_t& v)
	{
		if (value.IsInt64())
		{
			v = static_cast<uint64_t>(value.GetInt64());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetUInt64(uint64_t v)
	{
		type = home::FieldTypes::kIntegerFieldType;
		value.SetInt64(static_cast<int64_t>(v));
	}

	bool JsonReadableField::GetFloat(float_t& v)
	{
		if (value.IsDouble())
		{
			v = static_cast<float_t>(value.GetDouble());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetFloat(float_t v)
	{
		type = home::FieldTypes::kNumberFieldType;

		value.SetFloat(v);
	}

	bool JsonReadableField::GetDouble(double_t& v)
	{
		if (value.IsDouble())
		{
			v = value.GetDouble();
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetDouble(double_t v)
	{
		type = home::FieldTypes::kNumberFieldType;
		value.SetDouble(v);
	}

	bool JsonReadableField::GetString(std::string& v)
	{
		if (value.IsString())
		{
			v = std::string(value.GetString(), value.GetStringLength());
			return true;
		}
		else
			return false;
	}
	void JsonWriteableField::SetString(std::string v)
	{
		type = home::FieldTypes::kStringFieldType;
		value.SetString(v.c_str(), v.size(), allocator);
	}

	bool JsonReadableField::GetEndpoint(home::Endpoint& v)
	{
		if (value.IsString())
		{
			boost::container::vector<std::string> components;
			boost::split(components, std::string(value.GetString(), value.GetStringLength()), boost::is_any_of(":"));

			if (components.size() == 2)
			{
				v.host = components[0];
				v.port = std::stoi(components[1]);

				return true;
			}
		}

		return false;
	}
	void JsonWriteableField::SetEndpoint(home::Endpoint endpoint)
	{
		type = home::FieldTypes::kEndpointFieldType;

		std::string v = endpoint.host;
		v += ":";
		v += std::to_string(endpoint.port);
		value.SetString(v.c_str(), allocator);
	}
	void JsonWriteableField::SetEndpoint(std::string host, uint16_t port)
	{
		type = home::FieldTypes::kEndpointFieldType;

		std::string v = host;
		v += ":";
		v += std::to_string(port);
		value.SetString(v.c_str(), v.size(), allocator);
	}

	bool JsonReadableField::GetColor(home::Color& v)
	{
		if (value.IsString())
		{
			boost::container::vector<std::string> components;
			boost::split(components, std::string(value.GetString(), value.GetStringLength()), boost::is_any_of(";"));

			if (components.size() == 3)
			{
				v.red = std::stoi(components[0]);
				v.green = std::stoi(components[1]);
				v.blue = std::stoi(components[2]);

				return true;
			}
		}

		return false;
	}
	void JsonWriteableField::SetColor(home::Color color)
	{
		type = home::FieldTypes::kColorFieldType;

		std::string v;
		v += std::to_string(color.red);
		v += ";";
		v += std::to_string(color.green);
		v += ";";
		v += std::to_string(color.blue);

		value.SetString(v.c_str(), v.size(), allocator);
	}
	void JsonWriteableField::SetColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		type = home::FieldTypes::kColorFieldType;

		std::string v;
		v += std::to_string(red);
		v += ";";
		v += std::to_string(green);
		v += ";";
		v += std::to_string(blue);

		value.SetString(v.c_str(), v.size(), allocator);
	}
	void JsonWriteableField::SetColor(float_t red, float_t green, float_t blue)
	{
		type = home::FieldTypes::kColorFieldType;

		std::string v;
		v += std::to_string(static_cast<uint8_t>(red * 255.0f));
		v += ";";
		v += std::to_string(static_cast<uint8_t>(green * 255.0f));
		v += ";";
		v += std::to_string(static_cast<uint8_t>(blue * 255.0f));

		value.SetString(v.c_str(), v.size(), allocator);
	}

	JsonWriteableFieldCollection::JsonWriteableFieldCollection(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
		: json(json), allocator(allocator)
	{
		assert(json.IsArray());
	}

	void JsonWriteableFieldCollection::AddField(JsonWriteableField& field)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", field.id, allocator);
		value.AddMember("value", field.value, allocator);

		json.PushBack(value, allocator);
	}

	void JsonWriteableFieldCollection::AddBool(identifier_t id, bool v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", v, allocator);

		json.PushBack(value, allocator);
	}

	void JsonWriteableFieldCollection::AddInt8(identifier_t id, int8_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddUInt8(identifier_t id, uint8_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddInt16(identifier_t id, int16_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddUInt16(identifier_t id, uint16_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddInt32(identifier_t id, int32_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddUInt32(identifier_t id, uint32_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddInt64(identifier_t id, int64_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", v, allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddUInt64(identifier_t id, uint64_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<int64_t>(v), allocator);

		json.PushBack(value, allocator);
	}

	void JsonWriteableFieldCollection::AddFloat(identifier_t id, float_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", static_cast<double_t>(v), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddDouble(identifier_t id, double_t v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", v, allocator);

		json.PushBack(value, allocator);
	}

	void JsonWriteableFieldCollection::AddString(identifier_t id, std::string v)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);
		value.AddMember("value", rapidjson::Value(v.c_str(), v.size(), allocator), allocator);

		json.PushBack(value, allocator);
	}

	void JsonWriteableFieldCollection::AddEndpoint(identifier_t id, home::Endpoint endpoint)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);

		std::string v = endpoint.host;
		v += ":";
		v += std::to_string(endpoint.port);
		value.AddMember("value", rapidjson::Value(v.c_str(), v.size(), allocator), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddEndpoint(identifier_t id, std::string host, uint16_t port)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);

		std::string v = host;
		v += ":";
		v += std::to_string(port);
		value.AddMember("value", rapidjson::Value(v.c_str(), v.size(), allocator), allocator);

		json.PushBack(value, allocator);
	}

	void JsonWriteableFieldCollection::AddColor(identifier_t id, home::Color color)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);

		std::string v;
		v += std::to_string(color.red);
		v += ";";
		v += std::to_string(color.green);
		v += ";";
		v += std::to_string(color.blue);

		value.AddMember("value", rapidjson::Value(v.c_str(), v.size(), allocator), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddColor(identifier_t id, uint8_t red, uint8_t green, uint8_t blue)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);

		std::string v;
		v += std::to_string(red);
		v += ";";
		v += std::to_string(green);
		v += ";";
		v += std::to_string(blue);

		value.AddMember("value", rapidjson::Value(v.c_str(), v.size(), allocator), allocator);

		json.PushBack(value, allocator);
	}
	void JsonWriteableFieldCollection::AddColor(identifier_t id, float_t red, float_t green, float_t blue)
	{
		rapidjson::Value value = rapidjson::Value(rapidjson::kObjectType);
		value.AddMember("id", id, allocator);

		std::string v;
		v += std::to_string(static_cast<uint8_t>(red * 255.0f));
		v += ";";
		v += std::to_string(static_cast<uint8_t>(green * 255.0f));
		v += ";";
		v += std::to_string(static_cast<uint8_t>(blue * 255.0f));

		value.AddMember("value", rapidjson::Value(v.c_str(), v.size(), allocator), allocator);

		json.PushBack(value, allocator);
	}
}