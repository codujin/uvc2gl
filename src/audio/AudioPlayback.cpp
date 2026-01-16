#include "AudioPlayback.h"
#include <iostream>
#include <cstring>

namespace uvc2gl {

AudioPlayback::AudioPlayback(unsigned int sampleRate, unsigned int channels)
    : m_DeviceID(0)
    , m_Running(false)
    , m_ReadPos(0)
    , m_WritePos(0)
    , m_BufferSize(sampleRate * channels * 2) // 2 seconds of audio
    , m_Volume(1.0f)
{
    m_Buffer.resize(m_BufferSize);
    
    // Get current audio driver
    const char* driver = SDL_GetCurrentAudioDriver();
    if (driver) {
        std::cout << "SDL Audio Driver: " << driver << std::endl;
    }
    
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = sampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = channels;
    desired.samples = 1024;
    desired.callback = AudioCallback;
    desired.userdata = this;
    
    m_DeviceID = SDL_OpenAudioDevice(nullptr, 0, &desired, &m_AudioSpec, 0); // Don't allow changes
    if (m_DeviceID == 0) {
        throw std::runtime_error(std::string("Failed to open audio device: ") + SDL_GetError());
    }
    
    if (m_AudioSpec.freq != sampleRate || m_AudioSpec.channels != channels) {
        std::cerr << "Warning: Audio format changed by SDL. Requested: " << sampleRate 
                  << "Hz " << channels << "ch, Got: " << m_AudioSpec.freq 
                  << "Hz " << (int)m_AudioSpec.channels << "ch" << std::endl;
    }
    
    std::cout << "Audio playback initialized: " << m_AudioSpec.freq << "Hz, " 
              << (int)m_AudioSpec.channels << " channels, buffer size: " 
              << m_AudioSpec.samples << " frames" << std::endl;
}

AudioPlayback::~AudioPlayback() {
    Stop();
    if (m_DeviceID != 0) {
        SDL_CloseAudioDevice(m_DeviceID);
    }
}

void AudioPlayback::Start() {
    if (m_Running) {
        return;
    }
    
    m_Running = true;
    SDL_PauseAudioDevice(m_DeviceID, 0); // Unpause
    std::cout << "Audio playback started" << std::endl;
}

void AudioPlayback::Stop() {
    if (!m_Running) {
        return;
    }
    
    m_Running = false;
    SDL_PauseAudioDevice(m_DeviceID, 1); // Pause
    std::cout << "Audio playback stopped" << std::endl;
}

void AudioPlayback::QueueAudio(const int16_t* samples, size_t frameCount) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t sampleCount = frameCount * m_AudioSpec.channels;
    
    for (size_t i = 0; i < sampleCount; ++i) {
        m_Buffer[m_WritePos] = samples[i];
        m_WritePos = (m_WritePos + 1) % m_BufferSize;
        
        // If buffer is full, overwrite oldest data
        if (m_WritePos == m_ReadPos) {
            m_ReadPos = (m_ReadPos + 1) % m_BufferSize;
        }
    }
}

void AudioPlayback::AudioCallback(void* userdata, uint8_t* stream, int len) {
    AudioPlayback* self = static_cast<AudioPlayback*>(userdata);
    int16_t* output = reinterpret_cast<int16_t*>(stream);
    int frameCount = len / (sizeof(int16_t) * self->m_AudioSpec.channels);
    
    self->FillAudioBuffer(output, frameCount);
}

void AudioPlayback::FillAudioBuffer(int16_t* stream, int frameCount) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t sampleCount = frameCount * m_AudioSpec.channels;
    size_t samplesRead = 0;
    
    for (size_t i = 0; i < sampleCount; ++i) {
        if (m_ReadPos != m_WritePos) {
            // Apply volume by scaling the sample
            int32_t sample = static_cast<int32_t>(m_Buffer[m_ReadPos]);
            sample = static_cast<int32_t>(sample * m_Volume);
            // Clamp to int16_t range
            if (sample > 32767) sample = 32767;
            if (sample < -32768) sample = -32768;
            stream[i] = static_cast<int16_t>(sample);
            m_ReadPos = (m_ReadPos + 1) % m_BufferSize;
            samplesRead++;
        } else {
            // No data available, output silence
            stream[i] = 0;
        }
    }
}

void AudioPlayback::SetVolume(float volume) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Volume = std::max(0.0f, std::min(1.0f, volume)); // Clamp between 0 and 1
}

} // namespace uvc2gl
