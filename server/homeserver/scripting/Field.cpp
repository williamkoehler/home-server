#include "Field.hpp"

namespace scripting
{
	Field::Field(Ref<home::Visual> visual)
		: visual(std::move(visual))
	{ }
	Field::~Field()
	{ }
	Ref<Field> Field::CreateBoolean(identifier_t id, std::string text, home::VisualUnit unit)
	{
		class BooleanField : public Field
		{
		public:
			bool value;
			BooleanField(identifier_t id, std::string text, home::VisualUnit unit)
				: Field(home::Visuals::CreateBooleanModifier(id, text, unit))
			{ };
			virtual bool GetBoolean() override { return value; }
			virtual void SetBoolean(bool v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				json.SetBool(value);
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsBool())
				{
					value = json.GetBool();
					return true;
				}
				else
					return false;
			}
		};
		return boost::make_shared<BooleanField>(id, text, unit);
	}
	Ref<Field> Field::CreateInteger(identifier_t id, std::string text, home::VisualUnit unit, int64_t min, int64_t max)
	{
		class IntegerField : public Field
		{
		public:
			int64_t value;
			IntegerField(identifier_t id, std::string text, home::VisualUnit unit, int64_t min, int64_t max)
				: Field(home::Visuals::CreateIntegerModifier(id, text, unit, min, max))
			{ };
			virtual int64_t GetInteger() override { return value; }
			virtual void SetInteger(int64_t v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				json.SetInt64(value);
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsInt64())
				{
					value = json.GetInt64();
					return true;
				}
				else
					return false;
			}
		};
		return boost::make_shared<IntegerField>(id, text, unit, min, max);
	}
	Ref<Field> Field::CreateNumber(identifier_t id, std::string text, home::VisualUnit unit, double_t min, double_t max)
	{
		class NumberField : public Field
		{
		public:
			double_t value;
			NumberField(identifier_t id, std::string text, home::VisualUnit unit, double_t min, double_t max)
				: Field(home::Visuals::CreateNumberModifier(id, text, unit, min, max))
			{ };
			virtual double_t GetNumber() override { return value; }
			virtual void SetNumber(double_t v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				json.SetDouble(value);
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsDouble())
				{
					value = json.GetDouble();
					return true;
				}
				else
					return false;
			}
		};
		return boost::make_shared<NumberField>(id, text, unit, min, max);
	}
	Ref<Field> Field::CreateString(identifier_t id, std::string text, home::VisualUnit unit)
	{
		class TextField : public Field
		{
		public:
			std::string value;
			TextField(identifier_t id, std::string text, home::VisualUnit unit)
				: Field(home::Visuals::CreateStringModifier(id, text, unit))
			{ };
			virtual std::string GetString() override { return value; }
			virtual void SetString(std::string v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				json.SetString(value.c_str(), value.size(), allocator);
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsString())
				{
					value.assign(json.GetString(), json.GetStringLength());
					return true;
				}
				else
					return false;
			}
		};
		return boost::make_shared<TextField>(id, text, unit);
	}
	Ref<Field> Field::CreateEndpoint(identifier_t id, std::string text)
	{
		class EndpointField : public Field
		{
		public:
			home::Endpoint value;
			EndpointField(identifier_t id, std::string text)
				: Field(home::Visuals::CreateEndpointModifier(id, text))
			{ };
			virtual home::Endpoint GetEndpoint() override { return value; }
			virtual void SetEndpoint(home::Endpoint v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				std::string v = value.host;
				v += ":";
				v += std::to_string(value.port);
				json.SetString(v.c_str(), allocator);
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsString())
				{
					boost::container::vector<std::string> components;
					boost::split(components, std::string(json.GetString(), json.GetStringLength()), boost::is_any_of(":"));

					if (components.size() == 2)
					{
						value.host = components[0];
						value.port = std::stoi(components[1]);

						return true;
					}
				}

				return false;
			}
		};
		return boost::make_shared<EndpointField>(id, text);
	}
	Ref<Field> Field::CreateColor(identifier_t id, std::string text)
	{
		class ColorField : public Field
		{
		public:
			home::Color value;
			ColorField(identifier_t id, std::string text)
				: Field(home::Visuals::CreateColorModifier(id, text))
			{ };
			virtual home::Color GetColor() override { return value; }
			virtual void SetColor(home::Color v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				std::string v;
				v += std::to_string(value.red);
				v += ";";
				v += std::to_string(value.green);
				v += ";";
				v += std::to_string(value.blue);

				json.SetString(v.c_str(), v.size(), allocator);
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsString())
				{
					boost::container::vector<std::string> components;
					boost::split(components, std::string(json.GetString(), json.GetStringLength()), boost::is_any_of(";"));

					if (components.size() == 3)
					{
						value.red = std::stoi(components[0]);
						value.green = std::stoi(components[1]);
						value.blue = std::stoi(components[2]);

						return true;
					}
				}

				return false;
			}
		};
		return boost::make_shared<ColorField>(id, text);
	}
	Ref<Field> Field::CreateDevice(identifier_t id, std::string text)
	{
		class DeviceField : public Field
		{
		public:
			int32_t value;
			DeviceField(identifier_t id, std::string text)
				: Field(home::Visuals::CreateDeviceModifier(id, text, 0))
			{ };
			virtual int64_t GetInteger() override { return value; }
			virtual void SetInteger(int64_t v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				json.SetInt64(static_cast<int64_t>(value));
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsInt64())
				{
					value = static_cast<int32_t>(json.GetInt64());
					return true;
				}
				else
					return false;
			}
		};
		return boost::make_shared<DeviceField>(id, text);
	}
	Ref<Field> Field::CreateDeviceManager(identifier_t id, std::string text)
	{
		class DeviceManagerField : public Field
		{
		public:
			int32_t value;
			DeviceManagerField(identifier_t id, std::string text)
				: Field(home::Visuals::CreateDeviceManagerModifier(id, text, 0))
			{ };
			virtual int64_t GetInteger() override { return value; }
			virtual void SetInteger(int64_t v) override { value = v; }
			virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
			{
				json.SetInt64(static_cast<int64_t>(value));
			}
			virtual bool Set(rapidjson::Value& json) override
			{
				if (json.IsInt64())
				{
					value = static_cast<int32_t>(json.GetInt64());
					return true;
				}
				else
					return false;
			}
		};
		return boost::make_shared<DeviceManagerField>(id, text);
	}

	void Field::GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		assert(visual != nullptr);

		visual->GetRepresentation(json, allocator);
	}
	void Field::Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{ }
	bool Field::Set(rapidjson::Value& json)
	{
		return true;
	}
}