#xui-command-adapter
*proof-of-concept version*

Forwards commands from 3rd party software to the target software through websockets.
Makes use of [libwebsockets](https://libwebsockets.org/) to do its magic :)

*For this proof-of-concept, we just made an example C# software that changes*
*scenes, a websocket software with hosted html which should be added to the*
*global script of XBC, and a C based websocket software (though I’m using .cpp,*
*I just simply used C’s standard libraries)*

##Getting Started

1. Download the example software by cloning this repository, or downloading the _packaged
2. Run the command-adapter (`your_path/xui-command-adapter/server/Release/command-adapter.exe`)
3. Open XSplit Broadcaster and log-in (only tested on version 2.3+)
4. Click Tools > Script Plugins > Add Script Plugin (**If this does not exist, Enable it on the General Settings > Advanced > Check Enable Global Script**)
5. Type on the textbox of Add Script Plugin: “http://127.0.0.1:7877” and then click OK
6. Click Tools > Script Plugins > Click “http://127.0.0.1:7877”
7. Run the C# example (`your_path/command-adapter/sample-client/bin/Release/CAdapterExample.exe`)
8. Now play with the scenes and click Change Scene

*The example software currently has a limitation. You need to start the server first before executing the sample software*

##So what’s going on you might ask? 

Basically, the C# software would send commands (strings) to the websocket (`ws://127.0.0.1:7877`) wherein that same websocket is being listened by the global script (http://127.0.0.1:7877). The global script would then execute the command if the keyword is available on the external calls that is exposed on the global script by XSplit Broadcaster.

The example software limits the possible commands that would be passed to XSplit Broadcaster. You can edit and add additional commands on `your_path/command-adapter/Debug/html/js/external.js` under the window.External object.

**Components:**

- Websocket component
- Global Script
- 3rd party software (C#)

###Websocket component

Required external libraries:
libwebsockets - https://libwebsockets.org

This component would create a websocket server that would listen on a specified port. The default port used by the example server linked above is 7877 (**http: http://127.0.0.1:7877, websocket: ws://127.0.0.1:7877**).

The example software above also includes the required h, dll, and lib files (libwebsockets.h, websockets.dll, and websockets.lib).

Aside from the expected usage of this component, this would also serve the html file that would be used on the global script.


###Global Script

This could be a plain html file with a bunch of javascript codes. All this needs to do is to forward the data retrieved from the websocket and then forward it to the exposed external methods of XSplit Broadcaster.

The JavaScript file should create a websocket object that would listen to `ws://127.0.0.1:7877` and specify the sub-protocol as `lws-mirror-protocol`. This would allow the html file to listen to your local websocket.

*(We will update this with the complete links to our awesome plugin documentation once it is released, or when it is on public beta stage)*


###3rd party software (C#)

Required external libraries:
WebSocket4Net - https://websocket4net.codeplex.com/
Newton.Json - http://json.codeplex.com/

I used WebSocket4Net to connect to the websockets, since WebSockets isn’t available on VS2010 running on Win7. But feel free to use any websocket library that you would prefer as long as it supports sub-protocols.

You’ll have to simply register/listen to `ws://127.0.0.1:7877` with the sub-protocol as “lws-mirror-protocol”.

**Example (using websocket4net)**
```csharp
WebSocket websocket = new WebSocket("ws://127.0.0.1:7877", "lws-mirror-protocol");
websocket.Opened += new EventHandler(websocket_Opened);
websocket.MessageReceived += new EventHandler<MessageReceivedEventArgs>(websocket_MessageReceived);
websocket.Open();

// You’ll need to create the methods to handle the handlers. lol
```

As for the the JSON parser, you’ll have to note that we could only send strings to the websocket, thus, we’re sending stringified JSON objects and that would simply mean that we need to parse it back to a JSON object somehow.

That’s where Newton.Json comes in. Feel free to use any other JSON parser for C# if you prefer something else. Sending data to our global script requires a “name” and its corresponding “value”.

**Example**
```csharp
JObject obj  = new JObject();
obj["name"]  = "setScene";
obj["value"] = "0";

try
{
    obj["value"] = Convert.ToInt32(textScene.Text);
}
catch (Exception)
{
    // Do nothing
}

websocket.Send(obj.ToString());
```