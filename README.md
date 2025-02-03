# Electron Native Module w/OBS

**Streamlabs Project by Giacomo Tumini**

This is an electron application with a native module that uses the OBS project (https://github.com/obsproject) for its backend in order to stream to Twitch. 

The main electron application is based on the template at https://github.com/electron/electron-quick-start

The node native module was built using the information at:
- https://nodejs.org/dist/latest/docs/api/addons.html#addons_c_addons
- https://github.com/nodejs/node-addon-api/blob/master/doc/setup.md

## Dependencies
- obs-studio: https://github.com/obsproject/obs-studio
- Node.js: https://nodejs.org/en
- electron: https://www.electronjs.org
- electron-rebuild: https://github.com/electron/rebuild
- node-gyp: https://github.com/nodejs/node-gyp
- node-addon-api: https://github.com/nodejs/node-addon-api
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

## Running the application

To retrieve the source code for electron-node-native-obs application and setup the necessary modules to run it, use the following command (requires git):

```bash
git clone https://github.com/jacktumi/electron-node-native-obs.git
cd electron-node-native-obs
npm install
npm install electron
npm install --save-dev @electron/rebuild
npm install --save-dev node-gyp
npm install node-addon-api
```
Once the source code and node modules have been properly installed, update the `node_native\binding.gyp` file to properly specify the obs-studio paths for `include_dirs` and `libraries`:

```gyp
"include_dirs": [
"<!(node -p \"require('node-addon-api').include_dir\")",
"...\\obs-studio\\libobs"
],
```
```gyp
"libraries": [
    "...\\obs-studio\\build_x64\\libobs\\Release\\obs.lib"
]
```

Build the application using the following command:

```bash
npm run build
npm run rebuild
```

If the build is successful, you can now start/launch the application. To ensure the obs library can be properly initialized at runtime, update the `settings.json` file by using the appropriate paths for the obs modules/data. Also, make sure to update the Twitch streaming key associated to your Twitch account:

```javascript
{
    "obs_data_path": "...\\obs-studio\\build_x64\\rundir\\Release\\data\\libobs",
    "obs_module_bin_path": "...\\obs-studio\\build_x64\\rundir\\Release\\obs-plugins\\64bit",
    "obs_module_data_path": "...\\obs-studio\\build_x64\\rundir\\Release\\data\\obs-plugins",
    "twitch_rtmp": "rtmp://live.twitch.tv/app",
    "twitch_key": "live_sub_..."
}
```

Run the application:

```bash
npm start
```