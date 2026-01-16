#include "ALSACapabilities.h"
#include <alsa/asoundlib.h>
#include <iostream>

namespace uvc2gl {

std::vector<AudioDevice> ALSACapabilities::EnumerateDevices() {
    std::vector<AudioDevice> devices;
    
    // Add PulseAudio device (most common on modern Linux)
    devices.push_back({"pulse", "PulseAudio (System Default)"});
    
    // Enumerate hardware cards
    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char* name;
        if (snd_card_get_name(card, &name) >= 0) {
            // Try each device on this card
            int device = -1;
            snd_ctl_t* ctl;
            char hwname[32];
            snprintf(hwname, sizeof(hwname), "hw:%d", card);
            
            if (snd_ctl_open(&ctl, hwname, 0) >= 0) {
                while (snd_ctl_pcm_next_device(ctl, &device) >= 0 && device >= 0) {
                    snd_pcm_info_t* info;
                    snd_pcm_info_alloca(&info);
                    snd_pcm_info_set_device(info, device);
                    snd_pcm_info_set_subdevice(info, 0);
                    snd_pcm_info_set_stream(info, SND_PCM_STREAM_CAPTURE);
                    
                    if (snd_ctl_pcm_info(ctl, info) >= 0) {
                        char deviceName[64];
                        snprintf(deviceName, sizeof(deviceName), "hw:%d,%d", card, device);
                        
                        char description[256];
                        snprintf(description, sizeof(description), "%s - %s", 
                                name, snd_pcm_info_get_name(info));
                        
                        devices.push_back({deviceName, description});
                    }
                }
                snd_ctl_close(ctl);
            }
        }
    }
    
    // Add default device
    devices.push_back({"default", "Default Audio Device"});
    
    std::cout << "Found " << devices.size() << " audio device(s)" << std::endl;
    return devices;
}

} // namespace uvc2gl
