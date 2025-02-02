# Electron Native Module w/OBS

**Streamlabs Project by Giacomo Tumini**

This is an electron application that will interact with a native module that uses the OBS project (https://github.com/obsproject) for its backend in order to stream to Twitch. 

The main electron application was built by simply following the steps at https://github.com/electron/electron-quick-start

The node native module was built based on the information at:
- https://nodejs.org/dist/latest/docs/api/addons.html#addons_c_addons
- https://github.com/nodejs/node-addon-api/blob/master/doc/setup.md

## Dependencies
- obs-studio: https://github.com/obsproject/obs-studio
- Node.js: https://nodejs.org/en
- node-gyp: `nmp i node-gyp`
- node-addon-api: `nmp i node-api`
- Microsoft Visual Studio 2022: https://visualstudio.microsoft.com

## Expected Behavior
The Electron application uses the node native module in `main.js`:

```javascript
const addon = require("./node_native/build/Release/native_module");
console.log(`INFO: ${addon.getVersion()}`);
```

The native module code (C++) uses the 'Node Addon API' (`napi.h`) to marshall data between the JavaScript code and the C++ code. For more information see https://github.com/nodejs/node-addon-api/tree/main.

```C++
Napi::String GetVersion(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string version = NODE_NATIVE_MODULE_NAME " v" NODE_NATIVE_MODULE_VERSION " - " OBS_STUDIO_NAME " v";

    try
    {
        OBS_App::ObsStreamer& streamer = OBS_App::ObsStreamer::getInstance();
        version.append(streamer.getVersion());
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        version.append("???");
    }
    catch(...)
    {
        std::cerr << "ERROR: An unknown error occurred." << std::endl;
        version.append("???");
    }

    return Napi::String::New(env, version);
}
```

The bulk of the work is done in `ObsStreamer.cpp`, where the class exposes a public interface to:

- Startup OBS
- Initialize the audio and video contexts
- Load OBS modules
- Create audio and video encoders
- Create Twitch service with server URL + streamkey
- Create an RTMP output
- Set the encoders and the service to the output
- Start / stop the output
- Shutdown OBS

## Node Native Module API
The following API is exposed by the node native module:
- `getVersion()`: returns a string with both the native module versioning info and the obs-studio versioning info (e.g. `"OBS Link Native Module v1.0.0 - OBS Studio v31.0.0-67-g83934acb4"`)
- `initStreamer(obs_data_path, obs_module_bin_path, obs_module_data_path)`: initializes the graphic module (DirectX), and loads all necessary obs-studio modules. The parameters `obs_data_path`, `obs_module_bin_path`, and `obs_module_data_path` are used to facilitate the search for the libraries and data used by the modules. Once all the modules are successfully loaded, the audio and video context and codecs are initialized.
- `setupStream(twitch_rtmp, twitch_key)`: initializes the Twitch service (`rtmp`), and the stream output. The audio/video encoders, and the Twitch service are then connected to the stream output. At this point, the Twitch stream is ready to be started.
- `startStreaming()`: starts the live stream using the stream output setup in `setupStream()`.
- `stopStreaming()`: stops the live stream.

```C++
Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "getVersion"), Napi::Function::New(env, GetVersion));
    exports.Set(Napi::String::New(env, "initStreamer"), Napi::Function::New(env, InitializeStreamer));
    exports.Set(Napi::String::New(env, "setupStream"), Napi::Function::New(env, SetupStream));
    exports.Set(Napi::String::New(env, "startStreaming"), Napi::Function::New(env, StartStreaming));
    exports.Set(Napi::String::New(env, "stopStreaming"), Napi::Function::New(env, StopStreaming));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitModule)
```