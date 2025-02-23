#include <Arduino.h>

// 16bit, monoral, 44100Hz,  linear PCM
void CreateWavHeader(byte* header, int waveDataSize, int sampleRate, int numChannels);  // size of header is 44
