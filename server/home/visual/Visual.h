#pragma once
#include "../common.h"
#include "../helper/Types.h"

namespace home
{
	enum VisualTypes : uint8_t
	{
		// Default
		kEmptyVisual = 0,
		kBooleanVisual,
		kIntegerVisual,
		kNumberVisual,
		kStringVisual,
		kEndpointVisual,
		kColorVisual,

		// Special
		kDeviceVisual,
		kDeviceManagerVisual,
		kLightPowerVisual,

		// Interface
		kReaderFlagVisual = 0x80,	// 1... only read
		kModifierFlagVisual = 0x00, // 0... read and write
	};

	enum VisualUnits : uint8_t
	{
		kNoUnit,

		// Energy
		kUnitThermalCalorie,
		kUnitJoule,
		kUnitKilojoule,

		// Mass
		kUnitMilligram,
		kUnitGram,
		kUnitKilogram,
		kUnitTonne,

		// Length
		kUnitMillimeter,
		kUnitCentimeter,
		kUnitMeter,
		kUnitKilometer,

		// Area
		kUnitMillimeter2,
		kUnitCentimeter2,
		kUnitMeter2,
		kUnitKilometer2,

		// Volume
		kUnitMillimeter3,
		kUnitCentimeter3,
		kUnitMeter3,
		kUnitKilometer3,

		// Angle
		kUnitDegree,

		// Time
		kUnitMillisecond,
		kUnitSecond,
		kUnitMinute,
		kUnitHour,
		kUnitDay,
		kUnitYear,

		// Speed
		kUnitKilomterPerHour,
		kUnitMeterPerSecond,

		// Temperature
		kUnitCelsius,

		// Pressure
		kUnitPascal,

		// Storage
		kUnitBit,
		kUnitByte,
		kUnitKilobyte,
		kUnitMegabyte,
		kUnitGigabyte,
		kUnitTerabyte,
		kUnitPetabyte,
		kUnitExabyte,
		kUnitZetabyte,
		kUnitYotabyte,
	};

#define CREATEMETHOD(name, content, inter) \
	static Ref<Visual> Create ## name(identifier_t variable, std::string text, VisualUnits unit) \
	{ \
		class name : public Visual \
		{ \
		public: \
			VisualUnits unit; \
			name(identifier_t variable, std::string text, VisualUnits unit) : Visual(variable, std::move(text)), unit(unit) {}; \
			virtual VisualTypes GetType() override \
			{ \
				return (VisualTypes)(inter | content); \
			} \
			void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override \
			{ \
				Visual::GetRepresentation(json, allocator); \
				json.AddMember("unit", unit, allocator); \
			} \
		}; \
		return (Ref<Visual>)boost::make_shared<name>(variable, text, unit); \
	}
#define CREATEMETHODBOUNDED(name, content, inter, type, defmin, defmax) \
	static Ref<Visual> Create ## name(identifier_t variable, std::string text, VisualUnits unit, type min = defmin, type max = defmax) \
	{ \
		class name : public Visual \
		{ \
		public: \
			VisualUnits unit; \
			type min, max; \
			name(identifier_t variable, std::string text, VisualUnits unit, type min, type max) : Visual(variable, std::move(text)), unit(unit), min(min), max(max) {}; \
			virtual VisualTypes GetType() override \
			{ \
				return (VisualTypes)(inter | content); \
			} \
			void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override \
			{ \
				Visual::GetRepresentation(json, allocator); \
				json.AddMember("unit", unit, allocator); \
				json.AddMember("min", min, allocator); \
				json.AddMember("max", max, allocator); \
			} \
		}; \
		return (Ref<Visual>)boost::make_shared<name>(variable, text, unit, min, max); \
	}

#define CREATEMETHODS(name, content) \
	CREATEMETHOD(name ## Reader, VisualTypes::kReaderFlagVisual, content) \
	CREATEMETHOD(name ## Modifier, VisualTypes::kModifierFlagVisual, content)

#define CREATEMETHODSBOUNDED(name, content, type, defmin, defmax) \
	CREATEMETHOD(name ## Reader, VisualTypes::kReaderFlagVisual, content) \
	CREATEMETHODBOUNDED(name ## Modifier, VisualTypes::kModifierFlagVisual, content, type, defmin, defmax)

	class HOME_EXPORT Visual
	{
	protected:
		identifier_t variable;
		std::string text;

		virtual VisualTypes GetType() = 0;

	public:
		Visual(identifier_t variable, std::string text);

		virtual void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};

	class Visuals
	{
	public:
		CREATEMETHODS(Boolean, VisualTypes::kBooleanVisual);
		CREATEMETHODSBOUNDED(Integer, VisualTypes::kIntegerVisual, int64_t, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
		CREATEMETHODSBOUNDED(Number, VisualTypes::kNumberVisual, double_t, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
		CREATEMETHODS(String, VisualTypes::kStringVisual);


		static Ref<Visual> CreateEndpointReader(identifier_t variable, std::string text)
		{
			class EndpointReader : public Visual
			{
			public:
				EndpointReader(identifier_t variable, std::string text) : Visual(variable, std::move(text)) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kReaderFlagVisual | VisualTypes::kEndpointVisual);
				}
			};
			return boost::make_shared<EndpointReader>(variable, text);
		}
		static Ref<Visual> CreateEndpointModifier(identifier_t variable, std::string text)
		{
			class EndpointReader : public Visual
			{
			public:
				EndpointReader(identifier_t variable, std::string text) : Visual(variable, std::move(text)) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kModifierFlagVisual | VisualTypes::kEndpointVisual);
				}
			};
			return boost::make_shared<EndpointReader>(variable, text);
		}

		static Ref<Visual> CreateColorReader(identifier_t variable, std::string text)
		{
			class ColorReader : public Visual
			{
			public:
				ColorReader(identifier_t variable, std::string text) : Visual(variable, std::move(text)) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kReaderFlagVisual | VisualTypes::kColorVisual);
				}
			};
			return boost::make_shared<ColorReader>(variable, text);
		}
		static Ref<Visual> CreateColorModifier(identifier_t variable, std::string text)
		{
			class ColorReader : public Visual
			{
			public:
				ColorReader(identifier_t variable, std::string text) : Visual(variable, std::move(text)) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kModifierFlagVisual | VisualTypes::kColorVisual);
				}
			};
			return boost::make_shared<ColorReader>(variable, text);
		}

		static Ref<Visual> CreateDeviceModifier(identifier_t variable, std::string text, uint32_t scriptID)
		{
			class DeviceModifier : public Visual
			{
			public:
				uint32_t scriptID;

				DeviceModifier(identifier_t variable, std::string text, uint32_t scriptID) : Visual(variable, std::move(text)), scriptID(scriptID) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kModifierFlagVisual | VisualTypes::kDeviceVisual);
				}

				void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
				{
					Visual::GetRepresentation(json, allocator);
					json.AddMember("scriptid", scriptID, allocator);
				}
			};
			return boost::make_shared<DeviceModifier>(variable, text, scriptID); \
		}
		static Ref<Visual> CreateDeviceManagerModifier(identifier_t variable, std::string text, uint32_t scriptID)
		{
			class DeviceManagerModifier : public Visual
			{
			public:
				uint32_t scriptID;

				DeviceManagerModifier(identifier_t variable, std::string text, uint32_t scriptID) : Visual(variable, std::move(text)), scriptID(scriptID) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kModifierFlagVisual | VisualTypes::kDeviceManagerVisual);
				}

				void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
				{
					Visual::GetRepresentation(json, allocator);
					json.AddMember("scriptid", scriptID, allocator);
				}
			};
			return boost::make_shared<DeviceManagerModifier>(variable, text, scriptID); \
		}

		// Special
		static Ref<Visual> CreateLightSwitchModifier(identifier_t variable, std::string text, bool dimmable)
		{
			class LightSwitchModifier : public Visual
			{
			public:
				bool dimmable;

				LightSwitchModifier(identifier_t variable, std::string text, bool dimmable) : Visual(variable, std::move(text)), dimmable(dimmable) {};
				virtual VisualTypes GetType() override
				{
					return static_cast<VisualTypes>(VisualTypes::kModifierFlagVisual | VisualTypes::kLightPowerVisual);
				}

				void GetRepresentation(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) override
				{
					Visual::GetRepresentation(json, allocator);
					json.AddMember("dimmable", dimmable, allocator);
				}
			};
			return boost::make_shared<LightSwitchModifier>(variable, text, dimmable); \
		}
	};

#undef CREATEMETHODS
#undef CREATEMETHODSBOUNDED

#undef CREATEMETHOD
#undef CREATEMETHODBOUNDED
}