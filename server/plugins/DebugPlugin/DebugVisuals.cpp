#include "DebugVisuals.hpp"

const identifier_t STATE = home::DefaultFields::Light::STATE;

const identifier_t BOOLEAN_I = 1;
const identifier_t INTEGER_I = 2;
const identifier_t DOUBLE_I = 3;
const identifier_t STRING_I = 4;
const identifier_t ENDPOINT_I = 5;
const identifier_t COLOR_I = 6;
const identifier_t DEVICE_I = 7;
const identifier_t DEVICEMANAGER_I = 8;

void DebugVisuals::OnRegister(home::DeviceScriptDescription* description)
{
    // Add additional visuals
    description->AddProperty(home::Visuals::CreateIntegerReader(BOOLEAN_I, "Boolean", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateIntegerModifier(BOOLEAN_I, "Boolean", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateIntegerReader(INTEGER_I, "Integer", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateIntegerModifier(INTEGER_I, "Integer", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Number", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Number", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateStringReader(STRING_I, "String", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateStringModifier(STRING_I, "String", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Energy", home::VisualUnit::kUnitJoule));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Energy", home::VisualUnit::kUnitJoule));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Mass", home::VisualUnit::kUnitGram));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Mass", home::VisualUnit::kUnitGram));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Length", home::VisualUnit::kUnitMeter));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Length", home::VisualUnit::kUnitMeter));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Area", home::VisualUnit::kUnitMeter2));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Area", home::VisualUnit::kUnitMeter2));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Volume", home::VisualUnit::kUnitMeter3));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Volume", home::VisualUnit::kUnitMeter3));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Degree", home::VisualUnit::kUnitDegree));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Degree", home::VisualUnit::kUnitDegree));
    description->AddProperty(home::Visuals::CreateNumberReader(INTEGER_I, "Time", home::VisualUnit::kUnitSecond));
    description->AddProperty(home::Visuals::CreateNumberModifier(INTEGER_I, "Time", home::VisualUnit::kUnitSecond));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Speed", home::VisualUnit::kUnitKilomterPerHour));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Speed", home::VisualUnit::kUnitKilomterPerHour));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Temperature", home::VisualUnit::kUnitCelsius));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Temperature", home::VisualUnit::kUnitCelsius));
    description->AddProperty(home::Visuals::CreateNumberReader(DOUBLE_I, "Pressure", home::VisualUnit::kUnitPascal));
    description->AddProperty(home::Visuals::CreateNumberModifier(DOUBLE_I, "Pressure", home::VisualUnit::kUnitPascal));
    description->AddProperty(home::Visuals::CreateIntegerReader(INTEGER_I, "Storage", home::VisualUnit::kUnitByte));
    description->AddProperty(home::Visuals::CreateIntegerModifier(INTEGER_I, "Storage", home::VisualUnit::kUnitByte));
    description->AddProperty(home::Visuals::CreateEndpointReader(ENDPOINT_I, "Endpoint"));
    description->AddProperty(home::Visuals::CreateEndpointModifier(ENDPOINT_I, "Endpoint"));
    description->AddProperty(home::Visuals::CreateColorReader(COLOR_I, "Color"));
    description->AddProperty(home::Visuals::CreateColorModifier(COLOR_I, "Color"));

    description->AddProperty(home::Visuals::CreateDeviceModifier(DEVICE_I, "Device", DebugVisuals::_getScriptID()));
    description->AddProperty(home::Visuals::CreateDeviceManagerModifier(DEVICEMANAGER_I, "Device manager", DebugVisuals::_getScriptID()));
    description->AddProperty(home::Visuals::CreateLightSwitchModifier(INTEGER_I, "Light Switch", true));
}

bool DebugVisuals::OnInitialize()
{
    return true;
}
bool DebugVisuals::OnTerminate()
{
    return true;
}

bool DebugVisuals::HasError()
{
    return false;
}

std::string DebugVisuals::GetError()
{
    return "";
}

void DebugVisuals::OnUpdate(Ref<home::SignalManager> signalManager, size_t cycle)
{
}

void DebugVisuals::GetFields(home::WriteableFieldCollection& collection)
{
    collection.AddBool(BOOLEAN_I, fields.booleanV);
    collection.AddInt64(INTEGER_I, fields.integerV);
    collection.AddDouble(DOUBLE_I, fields.doubleV);
    collection.AddString(STRING_I, fields.stringV);
    collection.AddEndpoint(ENDPOINT_I, fields.endpointV);
    collection.AddColor(COLOR_I, fields.colorV);
    collection.AddUInt32(DEVICE_I, fields.deviceV);
    collection.AddUInt32(DEVICEMANAGER_I, fields.deviceManagerV);
}
bool DebugVisuals::GetField(home::WriteableField& field)
{
    switch (field.GetID())
    {
    case BOOLEAN_I:
        field.SetBool(fields.booleanV);
        break;
    case INTEGER_I:
        field.SetInt64(fields.integerV);
        break;
    case DOUBLE_I:
        field.SetDouble(fields.doubleV);
        break;
    case STRING_I:
        field.SetString(fields.stringV);
        break;
    case ENDPOINT_I:
        field.SetEndpoint(fields.endpointV);
        break;
    case COLOR_I:
        field.SetColor(fields.colorV);
        break;
    case DEVICE_I:
        field.SetUInt32(fields.deviceV);
        break;
    case DEVICEMANAGER_I:
        field.SetUInt32(fields.deviceManagerV);
        break;
    default:
        return false;
    }

    return true;
}
bool DebugVisuals::SetField(home::ReadableField& field)
{
    switch (field.GetID())
    {
    case BOOLEAN_I:
        return field.GetBool(fields.booleanV);
    case INTEGER_I:
        return field.GetInt64(fields.integerV);
    case DOUBLE_I:
        return field.GetDouble(fields.doubleV);
    case STRING_I:
        return field.GetString(fields.stringV);
    case ENDPOINT_I:
        return field.GetEndpoint(fields.endpointV);
    case COLOR_I:
        return field.GetColor(fields.colorV);
    case DEVICE_I:
        return field.GetUInt32(fields.deviceV);
    case DEVICEMANAGER_I:
        return field.GetUInt32(fields.deviceManagerV);
    default:
        return false;
    }
}
