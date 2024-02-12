#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <unistd.h>
#define SAMPLE_RATE 44100
#define BIT_DURATION 0.1

// Function to encode binary data into audio tones
void encode_data_to_audio(const char* data, int len, float* buffer) {
    int i, j;
    int index = 0;
    for (i = 0; i < len; i++) {
        for (j = 0; j < (int)(BIT_DURATION * SAMPLE_RATE); j++) {
            float t = (float)j / SAMPLE_RATE;
            float frequency = data[i] == '0' ? 1000.0 : 2000.0;
            buffer[index++] = sin(2 * M_PI * frequency * t);
        }
    }
}

// Function to decode audio tones into binary data
void decode_audio_to_data(float* buffer, int len, char* data) {
    int i, j;
    int index = 0;
    for (i = 0; i < len; i += (int)(BIT_DURATION * SAMPLE_RATE)) {
        float sum = 0.0;
        for (j = 0; j < (int)(BIT_DURATION * SAMPLE_RATE); j++) {
            sum += fabs(buffer[i + j]);
            //printf("added :%f",fabs(buffer[i + j]));
        }
        
        data[index++] = (sum / (BIT_DURATION * SAMPLE_RATE)) <= 0.02? '0' : '1';
    }
}

// Main function for audio playback
void audio_playback(const char* data_to_transmit) {
    pa_simple *s;
    pa_sample_spec ss;
    float buffer[204800]; // Buffer for audio data

    // Set up PulseAudio sample specification
    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = SAMPLE_RATE;
    ss.channels = 1;
    // Create new PulseAudio simple connection
    if (!(s = pa_simple_new(NULL, "IP over Music (Playback)", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, NULL))) {
        fprintf(stderr, "Error: pa_simple_new() for playback failed\n");
        return;
    }
    
    // Encode data to audio tones
    encode_data_to_audio(data_to_transmit, strlen(data_to_transmit), buffer);
    // Write audio data to PulseAudio stream
    if (pa_simple_write(s, buffer, sizeof(buffer), NULL) < 0) {
        fprintf(stderr, "Error: pa_simple_write() for playback failed\n");
        pa_simple_free(s);
        return;
    }

    // Close PulseAudio connection
    pa_simple_drain(s, NULL);
    pa_simple_free(s);
}

// Main function for audio capture
void audio_capture(char* captured_data) {
    pa_simple *s;
    pa_sample_spec ss;
    float buffer[204800]; // Buffer for audio data

    // Set up PulseAudio sample specification
    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = SAMPLE_RATE;
    ss.channels = 1;

    // Create new PulseAudio simple connection
    if (!(s = pa_simple_new(NULL, "IP over Music (Capture)", PA_STREAM_RECORD, NULL, "capture", &ss, NULL, NULL, NULL))) {
        fprintf(stderr, "Error: pa_simple_new() for capture failed\n");
        return;
    }

    // Read audio data from PulseAudio stream
    if (pa_simple_read(s, buffer, sizeof(buffer), NULL) < 0) {
        fprintf(stderr, "Error: pa_simple_read() for capture failed\n");
        pa_simple_free(s);
        return;
    }

    // Decode audio data to binary
    decode_audio_to_data(buffer, sizeof(buffer) / sizeof(float), captured_data);

    // Close PulseAudio connection
    pa_simple_free(s);
}

// Main function
int main(void) {
    char data_to_transmit[] = "1101010101000101"; // Example data to transmit
    char captured_data[204800]; // Buffer for captured audio data

    // Playback audio
    printf("Playing audio...\n");
    audio_playback(data_to_transmit);

    // Simulate transmission delay
    printf("Transmission delay...\n");
    sleep(2);

    // Capture audio
    printf("Capturing audio...\n");
    audio_capture(captured_data);

    // Print captured data
    printf("Captured data: %s\n", captured_data);

    return 0;
}

