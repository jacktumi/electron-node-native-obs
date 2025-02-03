#include "ObsStreamer.h"
#include <stdexcept>
#include <filesystem>
#include <iostream>

namespace OBS_App {

    ObsStreamer::ObsStreamer() {
        //  Startup OBS
        if (!obs_startup("en-US", NULL, NULL)) {
            throw std::runtime_error("Failed to start OBS");
        }
    }

    ObsStreamer::~ObsStreamer() {
        try
        {
            stopStreaming();
            cleanup();
            //  Shutdown OBS
            obs_shutdown();
        }
        catch(...)
        {
            //  Ignore exceptions
        }        
    }

    ObsStreamer& ObsStreamer::getInstance() {
        static ObsStreamer instance;
        return instance;
    }

    std::string ObsStreamer::getVersion() const {
        return std::string(obs_get_version_string());
    }

    void ObsStreamer::initialize(const std::string& obsDataPath, const std::string& obsModulesBinPath, const std::string& obsModulesDataPath) {
		if (m_initialized) {
			throw std::runtime_error("OBS already initialized");
		}
        // Initialize paths for libobs data and modules
        initializeData(obsDataPath);

        //  Initialize audio context
        initializeAudioContext(44100, SPEAKERS_STEREO);

        //  Initialize video context
        initializeVideoContext(1280, 720);

        //  Load OBS modules
        loadObsModules(obsModulesBinPath, obsModulesDataPath);

		//  Create audio and video encoders
        const std::string AUDIO_ENCODER_ID("ffmpeg_aac");
        const std::string VIDEO_ENCODER_ID("obs_x264");
		m_audioEncoder = createAudioEncoder(AUDIO_ENCODER_ID);
		m_videoEncoder = createVideoEncoder(VIDEO_ENCODER_ID);

		m_initialized = true;
    }

	void ObsStreamer::setupStreaming(const std::string& server, const std::string& key) {
		if (!m_initialized) {
			throw std::runtime_error("OBS not initialized");
		}
		try {
			if (m_service || m_output) {
                stopStreaming(); // just in case
				cleanupStreaming();
			}
			//  Create a Twitch service
			m_service = createTwichService(server, key);
			// Create RTMP output
			m_output = createRTMPOutput(m_audioEncoder, m_videoEncoder, m_service);
        } catch (...) {
			cleanupStreaming();
            throw;
		}
	}

    // TODO: Handle 'start' signal
	void ObsStreamer::startStreaming() {
		if (!m_initialized) {
			throw std::runtime_error("OBS not initialized");
		}
		if (!m_service || !m_output) {
			throw std::runtime_error("Streaming not set up");
		}
		if (!m_streaming) {
			m_streaming = obs_output_start(m_output);
			if (!m_streaming) {
                std::string outReason("Unknown Error");
                const char* outErr = obs_output_get_last_error(m_output);
                if (outErr) {
                    outReason = std::string(outErr);
                }
				throw std::runtime_error("Failed to start streaming: " + outReason);
			}
		}
	}

    // TODO: Handle 'stop' signal
	void ObsStreamer::stopStreaming() {
        if (!m_streaming) {
            return;
        }
		obs_output_stop(m_output);
		m_streaming = false;
	}

    void ObsStreamer::initializeData(const std::string& obsDataPath) {
		//  Add data path for OBS data (for example "C:\\Data\\SourceCode\\Streamlabs\\assignment\\obs-studio\\build_x64\\rundir\\Release\\data\\libobs")
        obs_add_data_path(obsDataPath.c_str());
        // TODO: somehow when loading the graphic module during the video initialization, the application
        // loads the default.effect module from the folder specified in obs_add_data_path(), and then
        // fails to open the file color.effect from the current working directory. Shouldn't the 
        // obs_add_data_path() be handling this?
		// The following code is a workaround to set the current working directory to the OBS data path.
        std::filesystem::current_path(obsDataPath);
    }

    void ObsStreamer::initializeAudioContext(uint32_t samplesPerSec, enum speaker_layout speakers) {
        //  Initialize audio context. Example: 44100, SPEAKERS_STEREO
        struct obs_audio_info oai = {
            .samples_per_sec = samplesPerSec,
            .speakers = speakers
        };
        bool resetAudioCode = obs_reset_audio(&oai);
        if (!resetAudioCode) {
            throw std::runtime_error("Failed to initialize audio: " + std::to_string(resetAudioCode));
        }
    }

    void ObsStreamer::initializeVideoContext(uint32_t width, uint32_t height) {
        //  Initialize video context. Example: 1920, 1080
        struct obs_video_info ovi = {
#ifndef SWIG
            .graphics_module = "libobs-d3d11",
#endif
            .fps_num = 60,
            .fps_den = 1,
            .base_width = width,
            .base_height = height,
            .output_width = width,
            .output_height = height,
            .output_format = VIDEO_FORMAT_NV12,
            .adapter = 0,
            .gpu_conversion = true,
            .colorspace = VIDEO_CS_DEFAULT,
            .range = VIDEO_RANGE_DEFAULT,
            .scale_type = OBS_SCALE_BILINEAR
        };

        int resetVideoCode = obs_reset_video(&ovi);
        if (resetVideoCode != 0) {
            throw std::runtime_error("Failed to initialize video: " + std::to_string(resetVideoCode));
        }
    }

    void ObsStreamer::loadObsModules(const std::string& obsModulesBinPath, const std::string& obsModulesDataPath) {
        //  Load OBS modules
        obs_add_module_path(obsModulesBinPath.c_str(), obsModulesDataPath.c_str());
        obs_load_all_modules();
        obs_log_loaded_modules();
        obs_post_load_modules();
    }

    obs_encoder_t* ObsStreamer::createAudioEncoder(const std::string& id) {
		//  Create an audio encoder
		obs_encoder_t* audioEncoder = obs_audio_encoder_create(id.c_str(), "audio_encoder", nullptr, 0, nullptr);
		if (!audioEncoder) {
			throw std::runtime_error("Failed to create audio encoder");
		}
        obs_encoder_set_audio(audioEncoder, obs_get_audio());
        return audioEncoder;
	}

    obs_encoder_t* ObsStreamer::createVideoEncoder(const std::string& id) {
		//  Create a video encoder
        obs_encoder_t* videoEncoder = obs_video_encoder_create(id.c_str(), "video_encoder", nullptr, nullptr);
        if (!videoEncoder) {
			throw std::runtime_error("Failed to create video encoder");
		}
        obs_encoder_set_video(videoEncoder, obs_get_video());
		return videoEncoder;
	}

    obs_service_t* ObsStreamer::createTwichService(const std::string& server, const std::string& key) {
		//  Create a Twitch service
        obs_data_t* settings = obs_data_create();
        obs_data_set_string(settings, "service", "Twitch");
        obs_data_set_string(settings, "server", server.c_str());
        obs_data_set_string(settings, "key", key.c_str());
        obs_service_t* twitchService = obs_service_create("rtmp_common", "twitch_service", settings, nullptr);
        obs_data_release(settings);
        if (!twitchService) {
			throw std::runtime_error("Failed to create Twitch service");
		}
		return twitchService;
    }

    obs_output_t* ObsStreamer::createRTMPOutput(obs_encoder_t* audioEncoder, obs_encoder_t* videoEncoder, obs_service_t* service) {
        obs_output_t* output = obs_output_create("rtmp_output", "rtmp_stream", nullptr, nullptr);
        if (!output) {
            throw std::runtime_error("Failed to create output");
        }
        // Connect the audio/video encoders, and the Twich service to the output stream
        obs_output_set_video_encoder(output, videoEncoder);
        obs_output_set_audio_encoder(output, audioEncoder, 0);
        obs_output_set_service(output, service);
        return output;
    }

    void ObsStreamer::cleanupStreaming() {
        //  Cleanup streaming
        if (m_output) {
            obs_output_release(m_output);
            m_output = nullptr;
        }
        if (m_service) {
            obs_service_release(m_service);
            m_service = nullptr;
        }
    }

	void ObsStreamer::cleanup() {
		//  Cleanup
		cleanupStreaming();
		if (m_videoEncoder) {
			obs_encoder_release(m_videoEncoder);
			m_videoEncoder = nullptr;
		}
		if (m_audioEncoder) {
			obs_encoder_release(m_audioEncoder);
			m_audioEncoder = nullptr;
		}
		m_initialized = false;
	}
}