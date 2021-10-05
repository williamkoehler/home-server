#include "DebugLight.hpp"

const id_t STATE = home::DefaultFields::Light::STATE;
const id_t DIMMABLE = home::DefaultFields::Light::DIMMABLE;

void DebugLight::OnRegister(home::DeviceScriptDescription* description)
{
    description->AddProperty(home::Visuals::CreateIntegerReader(STATE, "Boolean", home::VisualUnit::kNoUnit));
    description->AddProperty(home::Visuals::CreateIntegerModifier(STATE, "Boolean", home::VisualUnit::kNoUnit));
}

bool DebugLight::OnInitialize()
{
    return true;
}

bool DebugLight::HasError()
{
    return false;
}
std::string DebugLight::GetError()
{
    return "";
}

void DebugLight::OnUpdate(Ref<home::SignalManager> signalManager, size_t cycle)
{
}



void DebugLight::GetFields(home::WriteableFieldCollection& collection)
{
    collection.AddBool(STATE, fields.state);
}

bool DebugLight::GetField(home::WriteableField& field)
{
    switch (field.GetID()) 
    {
    case STATE:
        field.SetBool(fields.state);
        break;
    default:
        return false;
    }

    return true;
}

bool DebugLight::SetField(home::ReadableField& field)
{
    switch (field.GetID())
    {
    case STATE:
        return field.GetBool(fields.state);
    default:
        return false;
    }
}

bool DebugLight::OnTerminate()
{
    return true;
}