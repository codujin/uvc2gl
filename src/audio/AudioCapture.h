#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace uvc2gl {

struct AudioFrame {
    std::vector<int16_t> samples;  // Interleaved stereo samples
    size_t frameCount;
    unsigned int sampleRate;
    unsigned int channels;
    
    AudioFrame() : frameCount(0), sampleRate(0), channels(0) {}
    AudioFrame(size_t frames, unsigned int rate, unsigned int ch)
        : samples(frames * ch), frameCount(frames), sampleRate(rate), channels(ch) {}
};

class AudioCapture {
public:
    AudioCapture(const std::string& device = "default", 
                 unsigned int sampleRate = 48000,
                 unsigned int channels = 2,
                 snd_pcm_uframes_t periodSize = 1024);
    ~AudioCapture();
    
    AudioCapture(const AudioCapture&) = delete;
    AudioCapture& operator=(const AudioCapture&) = delete;
    
    void Start();
    void Stop();
    bool IsRunning() const { return m_Running.load(); }
    
    // Get latest audio data (non-blocking)
    bool GetAudioFrame(AudioFrame& frame);
    
private:
    void CaptureLoop();
    bool InitializeALSA();
    void CleanupALSA();
    
    std::string m_Device;
    unsigned int m_SampleRate;
    unsigned int m_Channels;
    snd_pcm_uframes_t m_PeriodSize;
    
    snd_pcm_t* m_Handle;
    std::atomic<bool> m_Running;
    std::thread m_CaptureThread;
    
    // Simple double buffer for audio data
    AudioFrame m_BufferA;
    AudioFrame m_BufferB;
    AudioFrame* m_WriteBuffer;
    AudioFrame* m_ReadBuffer;
    std::mutex m_BufferMutex;
};

} // namespace uvc2gl
