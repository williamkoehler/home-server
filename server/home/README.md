# How to create a user interface

## How to define a Plugin

``` c++
//Include "Plugin.hpp" to have access to all the plugin features
#include <Plugin.hpp>
#include "MyDevice.hpp"

extern "C"
{
    // Define a "void RegisterPlugin(Ref<home::Plugin> plugin)" function 
    // that will be called when the plugin is loaded.
	void RegisterPlugin(Ref<home::Plugin> plugin)
	{
        // To add device script plugin call
        // "plugin->RegisterDeviceScript<DEVSCRIPTNAME>()
        // and replace DEVSCRIPTNAME with the name of the script class
		plugin->RegisterDeviceScript<MyDeviceScript>();
	}
}
```

Every plugin library **must** contain the "**RegisterPlugin**" or else the plugin will not be loaded

## How to define a Device Script and Device Script Plugin 

A Device Script is a class that inherits from "home::DeviceScript".
Inside the class the "**SCRIPT_CLASS**" macro **must** be used to define some methods so you don't need to add them manually.
The "**DEVICESCRIPT_CLASS**" macro has a few arguments:
- The **class name**. In this example "MyDeviceClass".
- The **name of the device script plugin**.
- The **device type**. This is important for mobile and web apps.
- The **device state**. This constains every device state.

```c++
// MyDevice.hpp
#pragma once
#include <home/Home.hpp>
#include <home/DeviceScript.hpp>

class MyDeviceScript : public home::DeviceScript
{
	struct MyState
	{
	};

	DEVICESCRIPT_CLASS(
	MyDeviceScript,
	"Device name",
	home::DeviceType::kLightDeviceType,
	MyState);

private:

public:
	static void OnRegister(home::DeviceScriptDescription* description);

	virtual bool OnInitialize() override;
	virtual bool OnTerminate() override;

	virtual bool HasError() override;
	virtual std::string GetError() override;

	virtual void OnUpdate(Ref<home::SignalManager> signalManager, size_t cycle) override;

	virtual void GetFields(home::WriteableFieldCollection& collection) override;
	virtual bool GetField(home::WriteableField& field) override;
	virtual bool SetField(home::ReadableField& field) override;
};
```

Now, all these methods need to be implemented

```c++
#include "DebugVisuals.hpp"

void DebugVisuals::OnRegister(home::DeviceScriptDescription* description)
{
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
}
bool DebugVisuals::GetField(home::WriteableField& field)
{
    return false;
}
bool DebugVisuals::SetField(home::ReadableField& field)
{
    return false;
}

```

Currently, this example is not finished, since nothing is really fully explained.