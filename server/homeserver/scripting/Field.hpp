#pragma once
#include "../common.hpp"
#include <helper/Types.hpp>
#include <visual/Visual.hpp>

namespace scripting
{
	class Field
	{
	private:
		Ref<home::Visual> visual;

	public:
		Field(Ref<home::Visual> visual);
		~Field();
		static boost::shared_ptr<Field> CreateBoolean(identifier_t id, std::string name, home::VisualUnit unit);
		static boost::shared_ptr<Field> CreateInteger(identifier_t id, std::string name, home::VisualUnit unit, int64_t min, int64_t max);
		static boost::shared_ptr<Field> CreateNumber(identifier_t id, std::string text, home::VisualUnit unit, double_t min, double_t max);
		static boost::shared_ptr<Field> CreateString(identifier_t id, std::string text, home::VisualUnit unit);
		static boost::shared_ptr<Field> CreateEndpoint(identifier_t id, std::string text);
		static boost::shared_ptr<Field> CreateColor(identifier_t id, std::string text);
		static boost::shared_ptr<Field> CreateDevice(identifier_t id, std::string text);
		static boost::shared_ptr<Field> CreateDeviceManager(identifier_t id, std::string text);

		virtual bool GetBoolean() { return false; }
		virtual int64_t GetInteger() { return 0; }
		virtual double_t GetNumber() { return 0.0; }
		virtual std::string GetString() { return ""; }
		virtual home::Endpoint GetEndpoint() { return {}; }
		virtual home::Color GetColor() { return {}; }

		virtual void SetBoolean(bool value) {}
		virtual void SetInteger(int64_t value) {}
		virtual void SetNumber(double_t value) {}
		virtual void SetString(std::string value) {}
		virtual void SetEndpoint(home::Endpoint value) {}
		virtual void SetColor(home::Color value) {}

		virtual void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
		virtual void Get(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
		virtual bool Set(rapidjson::Value& json);
	};
}