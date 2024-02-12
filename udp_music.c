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
        for (j = 7; j >= 0; j--) {
            float frequency = (data[i] >> j) & 1 ? 2000.0 : 1000.0; // Use 1000 Hz for '0' and 2000 Hz for '1'
            for (int k = 0; k < (int)(BIT_DURATION * SAMPLE_RATE); k++) {
                float t = (float)k / SAMPLE_RATE;
                buffer[index++] = sin(2 * M_PI * frequency * t);
                
            }
        }
    }
}

// Function to decode audio tones into binary data
void decode_audio_to_data(float* buffer, int len, char* data) {
    int i, j;
    int index = 0;
    for (i = 0; i < len; i += 8 * (int)(BIT_DURATION * SAMPLE_RATE)) {
        char byte = 0;
        for (j = 0; j < 8; j++) {
            float sum = 0.0;
            for (int k = 0; k < (int)(BIT_DURATION * SAMPLE_RATE); k++) {
                sum += fabs(buffer[i + j * (int)(BIT_DURATION * SAMPLE_RATE) + k]);
            }
            byte |= (sum / (BIT_DURATION * SAMPLE_RATE) < 0.5 ? 0 : 1) << (7 - j);
        }
        printf("%c",byte);
        data[index++] = byte;
    }
}

// Function to create a UDP packet with example data
void create_udp_packet(char* packet_data, int* packet_size) {
    // Example UDP packet data
    char example_data[] = "This is a UDP packet!";
    *packet_size = strlen(example_data);
    memcpy(packet_data, example_data, *packet_size);
}

// Function to play audio tones
void play_audio(const float* audio_buffer, int buffer_size) {
    pa_simple *s;
    pa_sample_spec ss;

    // Set up PulseAudio sample specification
    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = SAMPLE_RATE;
    ss.channels = 1;

    // Create new PulseAudio simple connection
    if (!(s = pa_simple_new(NULL, "UDP Packet Playback", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, NULL))) {
        fprintf(stderr, "Error: pa_simple_new() failed\n");
        return;
    }

    // Write audio data to PulseAudio stream
    if (pa_simple_write(s, audio_buffer, buffer_size * sizeof(float), NULL) < 0) {
        fprintf(stderr, "Error: pa_simple_write() failed\n");
        pa_simple_free(s);
        return;
    }

    // Close PulseAudio connection
    pa_simple_drain(s, NULL);
    pa_simple_free(s);
}

// Function to capture audio tones
void capture_audio(float* audio_buffer, int buffer_size) {
    pa_simple *s;
    pa_sample_spec ss;

    // Set up PulseAudio sample specification
    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = SAMPLE_RATE;
    ss.channels = 1;

    // Create new PulseAudio simple connection
    if (!(s = pa_simple_new(NULL, "UDP Packet Capture", PA_STREAM_RECORD, NULL, "capture", &ss, NULL, NULL, NULL))) {
        fprintf(stderr, "Error: pa_simple_new() failed\n");
        return;
    }

    // Read audio data from PulseAudio stream
    if (pa_simple_read(s, audio_buffer, buffer_size * sizeof(float), NULL) < 0) {
        fprintf(stderr, "Error: pa_simple_read() failed\n");
        pa_simple_free(s);
        return;
    }

    // Close PulseAudio connection
    pa_simple_free(s);
}

int main(void) {
    char udp_packet_data[1024]; // Buffer for UDP packet data
    int packet_size;

    // Create a UDP packet with example data
    
    create_udp_packet(udp_packet_data, &packet_size);

    float audio_buffer[packet_size * 8 * (int)(BIT_DURATION * SAMPLE_RATE)]; // Buffer for audio data

    // Encode UDP packet data into audio tones
    
    encode_data_to_audio(udp_packet_data, packet_size, audio_buffer);

    // Play audio tones representing the encoded UDP packet
    play_audio(audio_buffer, sizeof(audio_buffer) / sizeof(float));

    // Wait for audio playback to finish
    usleep(1000000); // Sleep for 1 second

    // Capture audio tones
    float captured_audio[sizeof(audio_buffer) / sizeof(float)];
    capture_audio(captured_audio, sizeof(captured_audio) / sizeof(float));

    // Decode captured audio to binary data
    char captured_data[1024];
    decode_audio_to_data(captured_audio, sizeof(captured_audio) / sizeof(float), captured_data);

    // Print the decoded captured data
    printf("Decoded Captured Data: %s\n", captured_data);

    return 0;
}
