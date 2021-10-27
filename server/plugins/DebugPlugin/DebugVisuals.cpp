//#include "DebugVisuals.hpp"
//
//const identifier_t BOOLEAN_I = 1;
//const identifier_t INTEGER_I = 2;
//const identifier_t DOUBLE_I = 3;
//const identifier_t STRING_I = 4;
//const identifier_t ENDPOINT_I = 5;
//const identifier_t COLOR_I = 6;
//const identifier_t DEVICE_I = 7;
//const identifier_t DEVICEMANAGER_I = 8;
//
//void DebugVisuals::OnRegister(home::DeviceScriptDescription* description)
//{
//	description->AddProperty(home::Visual::Create(
//R"(
//{
//	"name": "Text",
//	"type": "text-field",
//	"format": "[0-9.]",
//	"keyboard": "number",
//	"value": {
//		"type": "string",
//		"id": 4
//	}
//})"));
//}
//
//bool DebugVisuals::OnInitialize()
//{
//	return true;
//}
//bool DebugVisuals::OnTerminate()
//{
//	return true;
//}
//
//bool DebugVisuals::HasError()
//{
//	return false;
//}
//
//std::string DebugVisuals::GetError()
//{
//	return "";
//}
//
//void DebugVisuals::OnUpdate(Ref<home::SignalManager> signalManager, size_t cycle)
//{
//}
//
//void DebugVisuals::GetFields(home::WriteableFieldCollection& collection)
//{
//	collection.AddBool(BOOLEAN_I, fields.booleanV);
//	collection.AddInt64(INTEGER_I, fields.integerV);
//	collection.AddDouble(DOUBLE_I, fields.doubleV);
//	collection.AddString(STRING_I, fields.stringV);
//	collection.AddEndpoint(ENDPOINT_I, fields.endpointV);
//	collection.AddColor(COLOR_I, fields.colorV);
//	collection.AddUInt32(DEVICE_I, fields.deviceV);
//	collection.AddUInt32(DEVICEMANAGER_I, fields.deviceControllerV);
//}
//bool DebugVisuals::GetField(home::WriteableField& field)
//{
//	switch (field.GetID())
//	{
//		case BOOLEAN_I:
//			field.SetBool(fields.booleanV);
//			break;
//		case INTEGER_I:
//			field.SetInt64(fields.integerV);
//			break;
//		case DOUBLE_I:
//			field.SetDouble(fields.doubleV);
//			break;
//		case STRING_I:
//			field.SetString(fields.stringV);
//			break;
//		case ENDPOINT_I:
//			field.SetEndpoint(fields.endpointV);
//			break;
//		case COLOR_I:
//			field.SetColor(fields.colorV);
//			break;
//		case DEVICE_I:
//			field.SetUInt32(fields.deviceV);
//			break;
//		case DEVICEMANAGER_I:
//			field.SetUInt32(fields.deviceControllerV);
//			break;
//		default:
//			return false;
//	}
//
//	return true;
//}
//bool DebugVisuals::SetField(home::ReadableField& field)
//{
//	switch (field.GetID())
//	{
//		case BOOLEAN_I:
//			return field.GetBool(fields.booleanV);
//		case INTEGER_I:
//			return field.GetInt64(fields.integerV);
//		case DOUBLE_I:
//			return field.GetDouble(fields.doubleV);
//		case STRING_I:
//			return field.GetString(fields.stringV);
//		case ENDPOINT_I:
//			return field.GetEndpoint(fields.endpointV);
//		case COLOR_I:
//			return field.GetColor(fields.colorV);
//		case DEVICE_I:
//			return field.GetUInt32(fields.deviceV);
//		case DEVICEMANAGER_I:
//			return field.GetUInt32(fields.deviceControllerV);
//		default:
//			return false;
//	}
//}
