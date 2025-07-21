#include "libretro/libretro-common/include/libretro.h"
#include <stdio.h>
#include <stdlib.h>

// Minimal libretro callbacks
void video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
    printf("Video refresh called\n");
}
void audio_sample(int16_t left, int16_t right) {}
size_t audio_sample_batch(const int16_t *data, size_t frames) { return frames; }
void input_poll() {}
int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id) { return 0; }
bool environment(unsigned cmd, void *data) { 
    printf("Environment callback called with cmd=%u\n", cmd);
    return false; 
}

int main() {
    printf("Starting simple GPSP test...\n");
    
    // Initialize libretro core
    printf("Setting callbacks...\n");
    retro_set_video_refresh(video_refresh);
    retro_set_audio_sample(audio_sample);
    retro_set_audio_sample_batch(audio_sample_batch);
    retro_set_input_poll(input_poll);
    retro_set_input_state(input_state);
    retro_set_environment(environment);
    
    printf("Calling retro_init...\n");
    retro_init();
    
    printf("retro_init completed successfully!\n");
    
    printf("Calling retro_deinit...\n");
    retro_deinit();
    
    printf("Test completed successfully!\n");
    return 0;
}