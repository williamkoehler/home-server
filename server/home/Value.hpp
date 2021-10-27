#pragma once
#include "common.hpp"
#include "helper/Types.hpp"

#define DEVICEPROP 0x8000 |

namespace home
{
	enum ValueType : uint8_t
	{
		kUnknownFieldType = 0,
		kBooleanFieldType,
		kNumberFieldType,
		kStringFieldType,
		kEndpointFieldType,
		kColorFieldType,
	};

	class ReadableValue
	{
	public:
		virtual identifier_t GetID() = 0;
		virtual ValueType GetType() = 0;

		virtual bool GetBool() = 0;

		virtual int8_t GetInt8() = 0;
		virtual uint8_t GetUInt8() = 0;
		virtual int16_t GetInt16() = 0;
		virtual uint16_t GetUInt16() = 0;
		virtual int32_t GetInt32() = 0;
		virtual uint32_t GetUInt32() = 0;
		virtual int64_t GetInt64() = 0;
		virtual uint64_t GetUInt64() = 0;

		virtual float_t GetFloat() = 0;
		virtual double_t GetDouble() = 0;

		virtual std::string GetString() = 0;

		virtual Endpoint GetEndpoint() = 0;
		virtual Color GetColor() = 0;
	};

	class WriteableValue
	{
	public:
		virtual identifier_t GetID() = 0;
		virtual ValueType GetType() = 0;

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
}