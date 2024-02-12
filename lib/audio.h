//
// Created by otaviohribas on 2/12/24.
//

#ifndef IPOVERMUSIC_ENCRYPT_H
#define IPOVERMUSIC_ENCRYPT_H

#include <math.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SAMPLE_RATE 44100
#define BIT_DURATION 0.1

// Function to encode binary data into audio tones
void encode_data_to_audio(const char* data, int len, float* buffer);

// Function to encode UDP packet data into audio tones
void encode_udp_packet_to_audio(const unsigned char* packet_data, int packet_size, float* buffer);

// Function to decode audio tones into binary data
void decode_audio_to_data(float* buffer, int len, char* data);

// Function to capture audio tones
void capture_audio(float* buffer, int buffer_size);

// Function to play audio tones representing the encoded UDP packet
void play_audio(const float* audio_buffer, int buffer_size);

#endif //IPOVERMUSIC_ENCRYPT_H
