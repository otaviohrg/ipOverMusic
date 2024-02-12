#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/audio.h"

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

// Function to encode UDP packet data into audio tones
void encode_udp_packet_to_audio(const unsigned char* packet_data, int packet_size, float* buffer) {
    int i, j;
    int index = 0;
    for (i = 0; i < packet_size; i++) {
        for (j = 7; j >= 0; j--) {
            float frequency = (packet_data[i] >> j) & 1 ? 2000.0 : 1000.0; // Use 1000 Hz for '0' and 2000 Hz for '1'
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

// Function to capture audio tones
void capture_audio(float* buffer, int buffer_size) {
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
    if (pa_simple_read(s, buffer, buffer_size * sizeof(float), NULL) < 0) {
        fprintf(stderr, "Error: pa_simple_read() failed\n");
        pa_simple_free(s);
        return;
    }

    // Close PulseAudio connection
    pa_simple_free(s);
}

// Function to play audio tones representing the encoded UDP packet
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