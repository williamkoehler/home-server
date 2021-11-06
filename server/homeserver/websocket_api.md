# WebSocket Api

The websocket api is the main api used to communicate with the server. It allows fast response time and live change.

## Connection

The connection is estabilished using the `\ws` url. 
Additionally a `Autorization` header field must be passed to authenticate, either with a simple Basic Auth or a Bearer token. The bearer can be generated using the HTTP Api.

## Request

Every request must contain a two important fields:
- `msgid`: A number with will be sent in the response (allows for multiple request to be sent at the same time)
- `msg`: A string which tells the server which method to used to process the request

## Response

Every response (to a request) allways contain the following fields:
- `msgid`: The same message id used in the request
- `log`: A list of strings containing logs
- `error`: The error code that resultet from the method (0 = no error)

## Methods

📓 Note:
- **\<value desc: value type\>** describe the content of a value and its type
- Fields with a **?** are not necessary or can be omited

### Settings

#### Method `get-settings`

##### Request

##### Response
```json
{
	"core": {
		"name": <server name: string>
	},
	"email": {
		"recipients": [
			<... email recipients: string>
		]
	}
}
```

#### Method `set-settings`

##### Request ⚠️ Only administrators
```json
{
}
```

##### Response
```json
{
	"core"?: {
		"name"?: "<server name>"
	},
	"email"?: {
		"recipients": [
			<... email recipients: string>
		]
	}
}
```

### User
#### Method `get-users`

### Plugins
#### Method `get-plugins`

### Scripting
#### Method `get-scriptsources`

#### Method `add-scriptsource`
#### Method `rem-scriptsource`

#### Method `get-scriptsource`
#### Method `set-scriptsource`

#### Method `get-scriptsource?data`
#### Method `set-scriptsource?data`

### Home
#### Method `get-home`
#### Method `set-home`

### Room
#### Method `add-room`
#### Method `rem-room`

#### Method `get-room`
#### Method `set-room`

### Device
#### Method `add-device`
#### Method `rem-device`

#### Method `get-device`
#### Method `set-device`

#### Method `get-devices?state`
#### Method `get-device?state`
#### Method `set-device?state`

### DeviceController
#### Method `add-devicecontroller`
#### Method `rem-devicecontroller`

#### Method `get-devicecontroller`
#### Method `set-devicecontroller`

#### Method `get-devicecontrollers?state`
#### Method `get-devicecontroller?state`
#### Method `set-devicecontroller?state`