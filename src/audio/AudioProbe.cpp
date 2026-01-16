#include <alsa/asoundlib.h>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== ALSA Capture Devices ===" << std::endl;
    
    // List hardware devices
    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char* name;
        if (snd_card_get_name(card, &name) < 0) {
            continue;
        }
        
        std::cout << "\nCard " << card << ": " << name << std::endl;
        
        // Try each device on this card
        int device = -1;
        snd_ctl_t* ctl;
        char hwname[32];
        snprintf(hwname, sizeof(hwname), "hw:%d", card);
        
        if (snd_ctl_open(&ctl, hwname, 0) < 0) {
            continue;
        }
        
        while (snd_ctl_pcm_next_device(ctl, &device) >= 0 && device >= 0) {
            snd_pcm_info_t* info;
            snd_pcm_info_alloca(&info);
            snd_pcm_info_set_device(info, device);
            snd_pcm_info_set_subdevice(info, 0);
            snd_pcm_info_set_stream(info, SND_PCM_STREAM_CAPTURE);
            
            if (snd_ctl_pcm_info(ctl, info) >= 0) {
                std::cout << "  Device " << device << ": " << snd_pcm_info_get_name(info) << std::endl;
                std::cout << "    ALSA name: hw:" << card << "," << device << std::endl;
                std::cout << "    Can be opened as: plughw:" << card << "," << device << std::endl;
            }
        }
        
        snd_ctl_close(ctl);
    }
    
    std::cout << "\n=== ALSA Device Hints ===" << std::endl;
    void** hints;
    if (snd_device_name_hint(-1, "pcm", &hints) == 0) {
        void** hint = hints;
        while (*hint != nullptr) {
            char* name = snd_device_name_get_hint(*hint, "NAME");
            char* desc = snd_device_name_get_hint(*hint, "DESC");
            char* ioid = snd_device_name_get_hint(*hint, "IOID");
            
            if (name && (ioid == nullptr || std::string(ioid) == "Input")) {
                std::cout << "\nDevice: " << name << std::endl;
                if (desc) {
                    std::cout << "Description: " << desc << std::endl;
                }
                
                // Try to open it
                snd_pcm_t* pcm;
                if (snd_pcm_open(&pcm, name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK) >= 0) {
                    std::cout << "Status: ✓ Can be opened" << std::endl;
                    snd_pcm_close(pcm);
                } else {
                    std::cout << "Status: ✗ Cannot be opened" << std::endl;
                }
            }
            
            if (name) free(name);
            if (desc) free(desc);
            if (ioid) free(ioid);
            
            hint++;
        }
        snd_device_name_free_hint(hints);
    }
    
    std::cout << "\n=== Testing capture card devices ===" << std::endl;
    std::cout << "Look for devices with 'USB', 'Video', or capture card manufacturer names" << std::endl;
    
    return 0;
}
