#ifndef AUDIO_API_H
#define AUDIO_API

#include <memory>

namespace audiomixer {
    class AudioMixer;

    class AudioMixerApi {
    public:

        /// @brief ctor
        AudioMixerApi();

        /// @brief dtor
        ~AudioMixerApi();

        /// @brief Start playing by filename
        /// @param [in] filename Sound file name
        /// @return Sound handle
        /// @throw std::invalid_argument when file not found
        int32_t PlaySound(std::string fileName);

        /// @brief Stop playing definite sound by handle
        /// @param [in] handle Sound handle
        void StopSound(int32_t handle) noexcept;

        /// @brief Pause playing definite sound by handle
        /// @param [in] handle
        void PauseSound(int32_t handle) noexcept;

        /// @brief Resume playing definite sound by handle
        /// @param [in] handle
        void ResumeSound(int32_t handle) noexcept;

        /// @brief Pause all sounds
        void Pause() noexcept;

        /// @brief Resume all sounds
        void Resume() noexcept;

        /// @brief Get rest playing as time in seconds
        /// @return Time in seconds
        double LeftTime() noexcept;

        /// @brief Get rest playing as size of the longest file
        uint64_t LeftSize() noexcept;

        /// @brief Play mixed sound for the certain time
        /// @param [in] ms time in ms
        void DelayPlay(uint32_t ms) noexcept;

        /// @brief Break mixed sound playing
        void BreakPlay() noexcept;

    private:

        std::unique_ptr <audiomixer::AudioMixer> mixer;

    };

}

#endif // AUDIO_API
