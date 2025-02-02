#ifndef OBSSTREAMER_H
#define OBSSTREAMER_H

#include <string>
#include <obs.h>

namespace OBS_App {

    //  Start / stop the output

    class ObsStreamer {
    public:
        /// @brief Singleton instance getter
        /// @return The singleton instance
        static ObsStreamer& getInstance();

        /// @brief Destructor
        virtual ~ObsStreamer();

        /// @brief Get the version of OBS
        /// @return The version of OBS as a string (e.g. "26.1.2")
        std::string getVersion() const;

        /// @brief Initialize the OBS streamer object
		/// @param obsDataPath The path to the OBS data (where all the OBS data .effect modules are stored)
		/// @param obsModulesBinPath The path to the OBS modules binary (where all the OBS modules .dll files are stored)
		/// @param obsModulesDataPath The path to the OBS modules data (where all the OBS modules data is stored)
        void initialize(const std::string& obsDataPath, const std::string& obsModulesBinPath, const std::string& obsModulesDataPath);

		/// @brief Setup streaming to Twitch
		/// @param server The server URL for the Twitch service (e.g. "rtmp://live.twitch.tv/app")
		/// @param key The stream key for the Twitch service
		/// Sets up the RTMP service with the given Twitch server URL and key. It also creates the
		/// output object necessary to start streaming.
        /// If the service or output objects are already created, they will be
		/// cleaned-up and recreated.
		void setupStreaming(const std::string& server, const std::string& key);

		/// @brief Start streaming
		/// Starts the streaming output. If the output is already started, the request is disregarded.
		void startStreaming();

		/// @brief Stop streaming
		/// Stops the streaming output. If the output is already stopped, the request is disregarded.
		void stopStreaming();

    private:
		/// @brief Initialize the data paths for OBS
		/// @param obsDataPath The path to the OBS data (where all the OBS data .effect modules are stored)
        void initializeData(const std::string& obsDataPath);

        /// @brief Initialize the audio context
        /// @param samplesPerSec The samples per second (e.g. 44100)
        /// @param speakers The speaker layout (e.g. SPEAKERS_STEREO)
        void initializeAudioContext(uint32_t samplesPerSec, enum speaker_layout speakers);

        /// @brief Initialize the video context
        /// @param width The width of the video (e.g. 1920)
        /// @param height The height of the video (e.g. 1080)
        void initializeVideoContext(uint32_t width, uint32_t height);

		/// @brief Load OBS modules
		/// @param obsModulesBinPath The path to the OBS modules binary (where all the OBS modules .dll files are stored)
		/// @param obsModulesDataPath The path to the OBS modules data (where all the OBS modules data is stored)
        void loadObsModules(const std::string& obsModulesBinPath, const std::string& obsModulesDataPath);

		/// @brief Create an audio encoder
		/// @param id The ID of the audio encoder (e.g. "ffmpeg_aac")
		/// @return The audio encoder pointer, or nullptr if failed
        obs_encoder_t* createAudioEncoder(const std::string& id);

		/// @brief Create a video encoder
		/// @param id The ID of the video encoder (e.g. "obs_x264")
		/// @return The video encoder pointer, or nullptr if failed
        obs_encoder_t* createVideoEncoder(const std::string& id);

		/// @brief Create a Twitch service
		/// @param server The server URL for the Twitch service (e.g. "rtmp://live.twitch.tv/app")
		/// @param key The stream key for the Twitch service
        obs_service_t* createTwichService(const std::string& server, const std::string& key);
        
        /// @brief Create a RTMP stream output
		/// @param audioEncoder The audio encoder to connect to the output
		/// @param videoEncoder The video encoder to connect to the output
		/// @param service The Twich service to connect to the output
		/// @return The RTMP output pointer, or nullptr if failed
        obs_output_t* createRTMPOutput(obs_encoder_t* audioEncoder, obs_encoder_t* videoEncoder, obs_service_t* service);

		/// @brief Cleanup the service and output objects
		/// If the service or output objects are already created, they will be
		/// destroyed and set to nullptr.
        void cleanupStreaming();

		/// @brief Cleanup all allocated resources
		void cleanup();

    private:
        /// @brief Constructor
        ObsStreamer();

        /// @brief Copy constructor
        /// @param other The other instance
        ObsStreamer(const ObsStreamer&) = delete;

        /// @brief Assignment operator
        ObsStreamer& operator=(const ObsStreamer&) = delete;

	private:
		bool m_initialized = false;                 ///< Flag indicating if the OBS streamer is initialized
		bool m_streaming = false;                   ///< Flag indicating if the OBS streamer is streaming
		obs_encoder_t* m_audioEncoder = nullptr;    ///< The audio encoder
		obs_encoder_t* m_videoEncoder = nullptr;    ///< The video encoder
		obs_service_t* m_service = nullptr;         ///< The Twitch service
		obs_output_t* m_output = nullptr;           ///< The RTMP output
    };
}

#endif // OBSSTREAMER_H
