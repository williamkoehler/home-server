#pragma once
#define HIDDEN_SYMBOL(x) ("\xFF" x)

//! Basics
#define ATTRIBUTES_PROPERTY ("attributes")
#define ATTRIBUTES_PROPERTY_SIZE (sizeof(ATTRIBUTES_PROPERTY) - 1)

#define PROPERTIES_PROPERTY ("properties")
#define PROPERTIES_PROPERTY_SIZE (sizeof(PROPERTIES_PROPERTY) - 1)

#define METHODS_PROPERTY ("methods")
#define METHODS_PROPERTY_SIZE (sizeof(METHODS_PROPERTY) - 1)

#define EVENTS_PROPERTY ("events")
#define EVENTS_PROPERTY_SIZE (sizeof(EVENTS_PROPERTY) - 1)

#define INITIALIZE_FUNCTION ("_initialize")
#define INITIALIZE_FUNCTION_SIZE (sizeof(INITIALIZE_FUNCTION) - 1)

//! Value
#define ENDPOINT_OBJECT ("Endpoint")
#define ENDPOINT_OBJECT_SIZE (sizeof(ENDPOINT_OBJECT) - 1)

#define COLOR_OBJECT ("Color")
#define COLOR_OBJECT_SIZE (sizeof(COLOR_OBJECT) - 1)

#define ROOM_ID_OBJECT ("RoomID")
#define ROOM_ID_OBJECT_SIZE (sizeof(ROOM_ID_OBJECT) - 1)

#define DEVICE_ID_OBJECT ("DeviceID")
#define DEVICE_ID_OBJECT_SIZE (sizeof(DEVICE_ID_OBJECT) - 1)

#define SERVICE_ID_OBJECT ("ServiceID")
#define SERVICE_ID_OBJECT_SIZE (sizeof(SERVICE_ID_OBJECT) - 1)


#define TYPE_PROPERTY HIDDEN_SYMBOL("type")
#define TYPE_PROPERTY_SIZE (sizeof(TYPE_PROPERTY) - 1)


#define HOST_PROPERTY ("host")
#define HOST_PROPERTY_SIZE (sizeof(HOST_PROPERTY) - 1)

#define PORT_PROPERTY ("port")
#define PORT_PROPERTY_SIZE (sizeof(PORT_PROPERTY) - 1)


#define RED_PROPERTY ("red")
#define RED_PROPERTY_SIZE (sizeof(RED_PROPERTY) - 1)

#define GREEN_PROPERTY ("green")
#define GREEN_PROPERTY_SIZE (sizeof(GREEN_PROPERTY) - 1)

#define BLUE_PROPERTY ("blue")
#define BLUE_PROPERTY_SIZE (sizeof(BLUE_PROPERTY) - 1)


#define ID_PROPERTY ("id")
#define ID_PROPERTY_SIZE (sizeof(ID_PROPERTY) - 1)


#define ENDPOINT_TYPE_NAME ("endpoint")
#define ENDPOINT_TYPE_NAME_SIZE (sizeof(ENDPOINT_TYPE_NAME) - 1)

#define COLOR_TYPE_NAME ("color")
#define COLOR_TYPE_NAME_SIZE (sizeof(COLOR_TYPE_NAME) - 1)

#define ROOM_TYPE_NAME ("room")
#define ROOM_TYPE_NAME_SIZE (sizeof(ROOM_TYPE_NAME) - 1)

#define DEVICE_TYPE_NAME ("device")
#define DEVICE_TYPE_NAME_SIZE (sizeof(DEVICE_TYPE_NAME) - 1)

#define SERVICE_TYPE_NAME ("service")
#define SERVICE_TYPE_NAME_SIZE (sizeof(SERVICE_TYPE_NAME) - 1)

//! Home

#define HOME_OBJECT ("Home")
#define HOME_OBJECT_SIZE (sizeof(HOME_OBJECT) - 1)


#define DEVICE_OBJECT ("Device")
#define DEVICE_OBJECT_SIZE (sizeof(DEVICE_OBJECT) - 1)

#define DEVICE_ID_PROPERTY HIDDEN_SYMBOL("device_id")
#define DEVICE_ID_PROPERTY_SIZE (sizeof(DEVICE_ID_PROPERTY) - 1)


#define ROOM_OBJECT ("Room")
#define ROOM_OBJECT_SIZE (sizeof(ROOM_OBJECT) - 1)

#define ROOM_ID_PROPERTY HIDDEN_SYMBOL("room_id")
#define ROOM_ID_PROPERTY_SIZE (sizeof(ROOM_ID_PROPERTY) - 1)
