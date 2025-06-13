#include "libretro/libretro-common/include/libretro.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Minimal libretro callbacks
void video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {}
void audio_sample(int16_t left, int16_t right) {}
size_t audio_sample_batch(const int16_t *data, size_t frames) { return frames; }
void input_poll() {}
int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id) { return 0; }
bool environment(unsigned cmd, void *data) { return false; }

int main(int argc, char *argv[]) {
    printf("Starting GPSP performance test...\n");
    
    // Initialize libretro core
    retro_set_video_refresh(video_refresh);
    retro_set_audio_sample(audio_sample);
    retro_set_audio_sample_batch(audio_sample_batch);
    retro_set_input_poll(input_poll);
    retro_set_input_state(input_state);
    retro_set_environment(environment);
    
    retro_init();
    
    struct retro_game_info game_info = {0};
    if (argc > 1) {
        // Try to load ROM if provided
        FILE *f = fopen(argv[1], "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);
            void *data = malloc(size);
            fread(data, 1, size, f);
            fclose(f);
            
            game_info.path = argv[1];
            game_info.data = data;
            game_info.size = size;
            printf("Loading ROM: %s (%zu bytes)\n", argv[1], size);
        }
    }
    
    if (!retro_load_game(&game_info)) {
        printf("Failed to load game, running without ROM\n");
    }
    
    printf("Running 1000 frames for profiling...\n");
    clock_t start = clock();
    
    // Run 1000 frames
    for (int i = 0; i < 1000; i++) {
        retro_run();
        if (i % 100 == 0) printf("Frame %d\n", i);
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Completed 1000 frames in %.2f seconds (%.1f FPS)\n", 
           time_taken, 1000.0 / time_taken);
    
    retro_deinit();
    return 0;
}