#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <mutex>
#include <cstdint>

namespace uvc2gl {

class AudioPlayback {
public:
    AudioPlayback(unsigned int sampleRate = 48000, unsigned int channels = 2);
    ~AudioPlayback();
    
    AudioPlayback(const AudioPlayback&) = delete;
    AudioPlayback& operator=(const AudioPlayback&) = delete;
    
    void Start();
    void Stop();
    bool IsRunning() const { return m_Running; }
    
    // Queue audio samples for playback (called from main thread)
    void QueueAudio(const int16_t* samples, size_t frameCount);
    
    // Volume control (0.0 to 1.0)
    void SetVolume(float volume);
    float GetVolume() const { return m_Volume; }
    
private:
    static void AudioCallback(void* userdata, uint8_t* stream, int len);
    void FillAudioBuffer(int16_t* stream, int frameCount);
    
    SDL_AudioDeviceID m_DeviceID;
    SDL_AudioSpec m_AudioSpec;
    bool m_Running;
    
    // Ring buffer for audio samples
    std::vector<int16_t> m_Buffer;
    size_t m_ReadPos;
    size_t m_WritePos;
    size_t m_BufferSize;
    std::mutex m_Mutex;
    float m_Volume;
};

} // namespace uvc2gl
