#include <stdint.h>

typedef int16_t audio_sample_t;
void sound_init();
size_t sound_read_samples(audio_sample_t *buffer, uint16_t sample_count);