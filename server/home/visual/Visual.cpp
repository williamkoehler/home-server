#include "Visual.hpp"

namespace home
{
	Visual::Visual(identifier_t variable, std::string text)
		: variable(variable), text(std::move(text))
	{

	}

	void Visual::GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		json.AddMember("type", GetType(), allocator);
		json.AddMember("var", variable, allocator);
		json.AddMember("text", rapidjson::Value(text.c_str(), text.size()), allocator);
	}
}