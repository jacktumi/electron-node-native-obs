#include <napi.h>
// #include <obs.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "ObsStreamer.h"

#define NODE_NATIVE_MODULE_NAME "OBS Link Native Module"
#define NODE_NATIVE_MODULE_VERSION "1.0.0"
#define OBS_STUDIO_NAME "OBS Studio"

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

Napi::Boolean InitializeStreamer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool initResult = true;

    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    for (int argIdx=0; argIdx<info.Length(); argIdx++) {
        if (!info[argIdx].IsString()) {
            Napi::TypeError::New(env, "Wrong argument types").ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }
    }
    std::string obs_data_path = info[0].As<Napi::String>().Utf8Value();
    std::string obs_module_bin_path = info[1].As<Napi::String>().Utf8Value();
    std::string obs_module_data_path = info[2].As<Napi::String>().Utf8Value();

    try
    {
        std::cout << "INFO: Initializing OBS streamer..." << std::endl;
        std::cout << "  OBS data path: " << obs_data_path << std::endl;
        std::cout << "  OBS module binary path: " << obs_module_bin_path << std::endl;
        std::cout << "  OBS module data path: " << obs_module_data_path << std::endl;

        OBS_App::ObsStreamer& streamer = OBS_App::ObsStreamer::getInstance();
        streamer.initialize(obs_data_path, obs_module_bin_path, obs_module_data_path);

        std::cout << "INFO: OBS streamer initialized." << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        initResult = false;
    }
    catch(...)
    {
        std::cerr << "ERROR: An unknown error occurred." << std::endl;
        initResult = false;
    }
    
    return Napi::Boolean::New(env, initResult);
}

Napi::Boolean SetupStream(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool initResult = true;

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    for (int argIdx=0; argIdx<info.Length(); argIdx++) {
        if (!info[argIdx].IsString()) {
            Napi::TypeError::New(env, "Wrong argument types").ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }
    }
    std::string twitch_rtmp = info[3].As<Napi::String>().Utf8Value();
    std::string twitch_key = info[4].As<Napi::String>().Utf8Value();

    try
    {
        std::cout << "INFO: Setting up OBS stream..." << std::endl;
        std::cout << "  Twitch RTMP: " << twitch_rtmp << std::endl;
        std::cout << "  Twitch key: " << twitch_key << std::endl;

        OBS_App::ObsStreamer& streamer = OBS_App::ObsStreamer::getInstance();
        streamer.setupStreaming(twitch_rtmp, twitch_key);

        std::cout << "INFO: OBS stream successfully setup." << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        initResult = false;
    }
    catch(...)
    {
        std::cerr << "ERROR: An unknown error occurred." << std::endl;
        initResult = false;
    }
    
    return Napi::Boolean::New(env, initResult);
}

Napi::Boolean StartStreaming(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool startResult = true;

    try
    {
        OBS_App::ObsStreamer& streamer = OBS_App::ObsStreamer::getInstance();
        streamer.startStreaming();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        startResult = false;
    }
    catch(...)
    {
        std::cerr << "ERROR: An unknown error occurred." << std::endl;
        startResult = false;
    }
    
    return Napi::Boolean::New(env, startResult);
}

Napi::Boolean StopStreaming(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool stopResult = true;

    try
    {
        OBS_App::ObsStreamer& streamer = OBS_App::ObsStreamer::getInstance();
        streamer.stopStreaming();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        stopResult = false;
    }
    catch(...)
    {
        std::cerr << "ERROR: An unknown error occurred." << std::endl;
        stopResult = false;
    }
    
    return Napi::Boolean::New(env, stopResult);
}

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "getVersion"), Napi::Function::New(env, GetVersion));
    exports.Set(Napi::String::New(env, "initStreamer"), Napi::Function::New(env, InitializeStreamer));
    exports.Set(Napi::String::New(env, "setupStream"), Napi::Function::New(env, SetupStream));
    exports.Set(Napi::String::New(env, "startStreaming"), Napi::Function::New(env, StartStreaming));
    exports.Set(Napi::String::New(env, "stopStreaming"), Napi::Function::New(env, StopStreaming));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitModule)