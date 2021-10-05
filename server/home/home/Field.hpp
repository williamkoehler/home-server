#pragma once
#include "../common.hpp"
#include "../helper/Types.hpp"

#define DEVICEPROP 0x8000 |

namespace home
{
	enum FieldType : uint8_t
	{
		kUnknownFieldType = 0,
		kBooleanFieldType,
		kIntegerFieldType,
		kNumberFieldType,
		kStringFieldType,
		kEndpointFieldType,
		kColorFieldType,
	};

	class DefaultFields
	{
	public:
		struct Light
		{
			static const identifier_t STATE = DEVICEPROP 1;
			static const identifier_t DIMMABLE = DEVICEPROP 2;
		};
		struct ColoredLight : public Light
		{
			static const identifier_t COLOR = DEVICEPROP 3;
		};
		struct Thermometer
		{
			static const identifier_t TEMPERATURE = DEVICEPROP 1;
		};
	};

	class ReadableField
	{
	public:
		virtual identifier_t GetID() = 0;

		virtual bool GetBool(bool& v) = 0;

		virtual bool GetInt8(int8_t& v) = 0;
		virtual bool GetUInt8(uint8_t& v) = 0;
		virtual bool GetInt16(int16_t& v) = 0;
		virtual bool GetUInt16(uint16_t& v) = 0;
		virtual bool GetInt32(int32_t& v) = 0;
		virtual bool GetUInt32(uint32_t& v) = 0;
		virtual bool GetInt64(int64_t& v) = 0;
		virtual bool GetUInt64(uint64_t& v) = 0;

		virtual bool GetFloat(float_t& v) = 0;
		virtual bool GetDouble(double_t& v) = 0;

		virtual bool GetString(std::string& v) = 0;

		virtual bool GetEndpoint(Endpoint& v) = 0;
		virtual bool GetColor(Color& v) = 0;
	};

	class WriteableField
	{
	public:
		virtual identifier_t GetID() = 0;

		virtual void SetBool(bool v) = 0;

		virtual void SetInt8(int8_t v) = 0;
		virtual void SetUInt8(uint8_t v) = 0;
		virtual void SetInt16(int16_t v) = 0;
		virtual void SetUInt16(uint16_t v) = 0;
		virtual void SetInt32(int32_t v) = 0;
		virtual void SetUInt32(uint32_t v) = 0;
		virtual void SetInt64(int64_t v) = 0;
		virtual void SetUInt64(uint64_t v) = 0;

		virtual void SetFloat(float_t v) = 0;
		virtual void SetDouble(double_t v) = 0;

		virtual void SetString(std::string v) = 0;

		virtual void SetEndpoint(Endpoint endpoint) = 0;
		virtual void SetEndpoint(std::string host, uint16_t port) = 0;
		virtual void SetColor(Color color) = 0;
		virtual void SetColor(uint8_t red, uint8_t green, uint8_t blue) = 0;
		virtual void SetColor(float_t red, float_t green, float_t blue) = 0;
	};

	class WriteableFieldCollection
	{
	public:
		virtual void AddBool(identifier_t id, bool v) = 0;

		virtual void AddInt8(identifier_t id, int8_t v) = 0;
		virtual void AddUInt8(identifier_t id, uint8_t v) = 0;
		virtual void AddInt16(identifier_t id, int16_t v) = 0;
		virtual void AddUInt16(identifier_t id, uint16_t v) = 0;
		virtual void AddInt32(identifier_t id, int32_t v) = 0;
		virtual void AddUInt32(identifier_t id, uint32_t v) = 0;
		virtual void AddInt64(identifier_t id, int64_t v) = 0;
		virtual void AddUInt64(identifier_t id, uint64_t v) = 0;

		virtual void AddFloat(identifier_t id, float_t v) = 0;
		virtual void AddDouble(identifier_t id, double_t v) = 0;

		virtual void AddString(identifier_t id, std::string v) = 0;

		virtual void AddEndpoint(identifier_t id, Endpoint endpoint) = 0;
		virtual void AddEndpoint(identifier_t id, std::string host, uint16_t port) = 0;
		virtual void AddColor(identifier_t id, Color color) = 0;
		virtual void AddColor(identifier_t id, uint8_t red, uint8_t green, uint8_t blue) = 0;
		virtual void AddColor(identifier_t id, float_t red, float_t green, float_t blue) = 0;
	};
}