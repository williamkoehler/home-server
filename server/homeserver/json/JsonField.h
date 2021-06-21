#pragma once
#include "../common.h"
#include <home/Field.h>

namespace server
{
	class JsonReadableField : public home::ReadableField
	{
	protected:
		friend class JsonWriteableFieldCollection;

		identifier_t id;
		rapidjson::Value& value;

	public:
		JsonReadableField(identifier_t id, rapidjson::Value& value);

		virtual identifier_t GetID() override { return id; }

		virtual bool GetBool(bool& v) override;

		virtual bool GetInt8(int8_t& v) override;
		virtual bool GetUInt8(uint8_t& v) override;
		virtual bool GetInt16(int16_t& v) override;
		virtual bool GetUInt16(uint16_t& v) override;
		virtual bool GetInt32(int32_t& v) override;
		virtual bool GetUInt32(uint32_t& v) override;
		virtual bool GetInt64(int64_t& v) override;
		virtual bool GetUInt64(uint64_t& v) override;

		virtual bool GetFloat(float_t& v) override;
		virtual bool GetDouble(double_t& v) override;

		virtual bool GetString(std::string& v) override;

		virtual bool GetEndpoint(home::Endpoint& v) override;
		virtual bool GetColor(home::Color& v) override;
	};

	class JsonWriteableField : public home::WriteableField
	{
	protected:
		friend class JsonWriteableFieldCollection;

		identifier_t id;
		rapidjson::Value& value;
		home::FieldTypes type;
		rapidjson::Document::AllocatorType& allocator;

	public:
		JsonWriteableField(identifier_t id, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator);

		virtual identifier_t GetID() override { return id; }

		inline home::FieldTypes GetType() { return type; }

		virtual void SetBool(bool v) override;

		virtual void SetInt8(int8_t v) override;
		virtual void SetUInt8(uint8_t v) override;
		virtual void SetInt16(int16_t v) override;
		virtual void SetUInt16(uint16_t v) override;
		virtual void SetInt32(int32_t v) override;
		virtual void SetUInt32(uint32_t v) override;
		virtual void SetInt64(int64_t v) override;
		virtual void SetUInt64(uint64_t v) override;

		virtual void SetFloat(float_t v) override;
		virtual void SetDouble(double_t v) override;

		virtual void SetString(std::string v) override;

		virtual void SetEndpoint(home::Endpoint endpoint) override;
		virtual void SetEndpoint(std::string host, uint16_t port) override;
		virtual void SetColor(home::Color color) override;
		virtual void SetColor(uint8_t red, uint8_t green, uint8_t blue) override;
		virtual void SetColor(float_t red, float_t green, float_t blue) override;
	};

	class JsonWriteableFieldCollection : public home::WriteableFieldCollection
	{
	private:
		rapidjson::Value& json;
		rapidjson::Document::AllocatorType& allocator;

	public:
		JsonWriteableFieldCollection(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);

		void AddField(JsonWriteableField& field);

		virtual void AddBool(identifier_t id, bool v) override;

		virtual void AddInt8(identifier_t id, int8_t v) override;
		virtual void AddUInt8(identifier_t id, uint8_t v) override;
		virtual void AddInt16(identifier_t id, int16_t v) override;
		virtual void AddUInt16(identifier_t id, uint16_t v) override;
		virtual void AddInt32(identifier_t id, int32_t v) override;
		virtual void AddUInt32(identifier_t id, uint32_t v) override;
		virtual void AddInt64(identifier_t id, int64_t v) override;
		virtual void AddUInt64(identifier_t id, uint64_t v) override;

		virtual void AddFloat(identifier_t id, float_t v) override;
		virtual void AddDouble(identifier_t id, double_t v) override;

		virtual void AddString(identifier_t id, std::string v) override;

		virtual void AddEndpoint(identifier_t id, home::Endpoint endpoint) override;
		virtual void AddEndpoint(identifier_t id, std::string host, uint16_t port) override;
		virtual void AddColor(identifier_t id, home::Color color) override;
		virtual void AddColor(identifier_t id, uint8_t red, uint8_t green, uint8_t blue) override;
		virtual void AddColor(identifier_t id, float_t red, float_t green, float_t blue) override;
	};
}