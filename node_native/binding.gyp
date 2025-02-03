{
  "targets": [
    {
      "target_name": "native_module",
      "sources": [ "main.cpp", "ObsStreamer.cpp" ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "C:\\Data\\SourceCode\\Streamlabs\\assignment\\obs-studio\\libobs"
      ],
      "defines": [
        "NODE_ADDON_API_CPP_EXCEPTIONS"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags!": [ "-fno-rtti" ],
      "cflags": [ "/EHsc" ],
      "ldflags": [ "/NODEFAULTLIB:libcmt" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "GCC_ENABLE_OBJC_EXCEPTIONS": "YES"
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": "2" # This is the equivalent of /EHsc
        }
      },
      "libraries": [
          "C:\\Data\\SourceCode\\Streamlabs\\assignment\\obs-studio\\build_x64\\libobs\\Release\\obs.lib"
      ]
    }
  ]
}