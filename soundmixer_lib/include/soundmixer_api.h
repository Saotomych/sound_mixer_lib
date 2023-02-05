/// @file soundmixer_api.h
/// @brief Interface class for a sound mixer implementation
/// @details It uses pimpl pattern for a mixer entity

#pragma once

#ifndef SOUNDMIXER_API_H
#define SOUNDMIXER_API_H

#include <memory>

namespace soundmixer {
    class SoundMixer;

    class SoundMixerApi {
    public:

        /// @brief constructor
        SoundMixerApi();

        /// @brief destructor
        ~SoundMixerApi();

        /// @brief Start playing by filename
        /// @param [in] fileName Sound file name
        /// @return Sound handle
        /// @throw std::invalid_argument when file not found
        int32_t PlaySound(std::string fileName);

        /// @brief Stop playing definite sound by handle
        /// @param [in] handle Sound handle
        void StopSound(int32_t handle) noexcept;

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

        std::unique_ptr <soundmixer::SoundMixer> mixer;

    };

}

#endif // SOUNDMIXER_API_H
