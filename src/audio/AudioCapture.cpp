#include "AudioCapture.h"
#include <iostream>

namespace uvc2gl {

AudioCapture::AudioCapture(const std::string& device, 
                           unsigned int sampleRate,
                           unsigned int channels,
                           snd_pcm_uframes_t periodSize)
    : m_Device(device)
    , m_SampleRate(sampleRate)
    , m_Channels(channels)
    , m_PeriodSize(periodSize)
    , m_Handle(nullptr)
    , m_Running(false)
    , m_BufferA(periodSize, sampleRate, channels)
    , m_BufferB(periodSize, sampleRate, channels)
{
    m_WriteBuffer = &m_BufferA;
    m_ReadBuffer = &m_BufferB;
}

AudioCapture::~AudioCapture() {
    try {
        Stop();
    } catch (...) {
        // Never throw from destructor
        std::cerr << "Exception in AudioCapture destructor" << std::endl;
    }
}

void AudioCapture::Start() {
    if (m_Running.exchange(true)) {
        return; // Already running
    }
    
    m_CaptureThread = std::thread(&AudioCapture::CaptureLoop, this);
}

void AudioCapture::Stop() {
    if (!m_Running.exchange(false)) {
        return; // Not running
    }
    
    try {
        if (m_CaptureThread.joinable()) {
            m_CaptureThread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error joining audio capture thread: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error joining audio capture thread" << std::endl;
    }
}

bool AudioCapture::GetAudioFrame(AudioFrame& frame) {
    std::lock_guard<std::mutex> lock(m_BufferMutex);
    
    if (m_ReadBuffer->samples.empty() || m_ReadBuffer->frameCount == 0) {
        return false;
    }
    
    frame = *m_ReadBuffer;
    
    // Mark this buffer as consumed by clearing it
    // This prevents the same audio from being played multiple times
    m_ReadBuffer->frameCount = 0;
    
    return true;
}

bool AudioCapture::InitializeALSA() {
    int err;
    
    // Open PCM device for capture
    err = snd_pcm_open(&m_Handle, m_Device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        std::cerr << "Cannot open audio device " << m_Device << ": " 
                  << snd_strerror(err) << std::endl;
        return false;
    }
    
    // Allocate hardware parameters object
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    
    // Fill with default values
    err = snd_pcm_hw_params_any(m_Handle, params);
    if (err < 0) {
        std::cerr << "Cannot initialize hardware parameters: " 
                  << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    // Set interleaved mode
    err = snd_pcm_hw_params_set_access(m_Handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    // Set sample format (16-bit signed integer)
    err = snd_pcm_hw_params_set_format(m_Handle, params, SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
        std::cerr << "Cannot set sample format: " << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    // Set number of channels
    err = snd_pcm_hw_params_set_channels(m_Handle, params, m_Channels);
    if (err < 0) {
        std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    // Set sample rate
    unsigned int actualRate = m_SampleRate;
    err = snd_pcm_hw_params_set_rate_near(m_Handle, params, &actualRate, 0);
    if (err < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    if (actualRate != m_SampleRate) {
        std::cout << "Sample rate adjusted from " << m_SampleRate 
                  << " to " << actualRate << " Hz" << std::endl;
        m_SampleRate = actualRate;
        // Update buffer sizes to match actual rate
        m_BufferA = AudioFrame(m_PeriodSize, m_SampleRate, m_Channels);
        m_BufferB = AudioFrame(m_PeriodSize, m_SampleRate, m_Channels);
    }
    
    // Set period size
    snd_pcm_uframes_t actualPeriodSize = m_PeriodSize;
    err = snd_pcm_hw_params_set_period_size_near(m_Handle, params, &actualPeriodSize, 0);
    if (err < 0) {
        std::cerr << "Cannot set period size: " << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    if (actualPeriodSize != m_PeriodSize) {
        std::cout << "Period size adjusted from " << m_PeriodSize 
                  << " to " << actualPeriodSize << " frames" << std::endl;
        m_PeriodSize = actualPeriodSize;
        // Update buffer sizes to match actual period
        m_BufferA = AudioFrame(m_PeriodSize, m_SampleRate, m_Channels);
        m_BufferB = AudioFrame(m_PeriodSize, m_SampleRate, m_Channels);
    }
    
    // Write parameters to device
    err = snd_pcm_hw_params(m_Handle, params);
    if (err < 0) {
        std::cerr << "Cannot set hardware parameters: " 
                  << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    // Prepare device
    err = snd_pcm_prepare(m_Handle);
    if (err < 0) {
        std::cerr << "Cannot prepare audio device: " << snd_strerror(err) << std::endl;
        CleanupALSA();
        return false;
    }
    
    std::cout << "Audio capture initialized: " << m_SampleRate << "Hz, " 
              << m_Channels << " channels, " << m_PeriodSize << " frames/period" << std::endl;
    
    return true;
}

void AudioCapture::CleanupALSA() {
    if (m_Handle) {
        snd_pcm_close(m_Handle);
        m_Handle = nullptr;
    }
}

void AudioCapture::CaptureLoop() {
    if (!InitializeALSA()) {
        std::cerr << "Failed to initialize ALSA" << std::endl;
        return;
    }
    
    std::cout << "Audio capture thread started" << std::endl;
    
    while (m_Running.load()) {
        // Read audio data into write buffer
        snd_pcm_sframes_t frames = snd_pcm_readi(m_Handle, 
                                                  m_WriteBuffer->samples.data(), 
                                                  m_PeriodSize);
        
        if (frames < 0) {
            // Handle errors
            frames = snd_pcm_recover(m_Handle, frames, 0);
            if (frames < 0) {
                std::cerr << "Audio capture error: " << snd_strerror(frames) << std::endl;
                break;
            }
            continue;
        }
        
        if (frames != static_cast<snd_pcm_sframes_t>(m_PeriodSize)) {
            std::cerr << "Short read: expected " << m_PeriodSize 
                      << " frames, got " << frames << std::endl;
        }
        
        // Update the frame count for the write buffer
        m_WriteBuffer->frameCount = frames;
        
        // Swap buffers
        {
            std::lock_guard<std::mutex> lock(m_BufferMutex);
            std::swap(m_WriteBuffer, m_ReadBuffer);
        }
    }
            
            std::cout << "Audio capture thread stopped" << std::endl;
            CleanupALSA();
        }

} // namespace uvc2gl
