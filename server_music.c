#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SAMPLE_RATE 44100
#define BIT_DURATION 0.1
#define PORT 8080
#define MAX_BUF_SIZE 1024
#define CHUNK_SIZE 512
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

// Function to create a UDP packet with example data
void create_udp_packet(char* packet_data, int* packet_size) {
    // Example UDP packet data
    char example_data[] = "This is a UDP packet!";
    *packet_size = strlen(example_data);
    memcpy(packet_data, example_data, *packet_size);
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

int main(int argc, char* argv[]) {
    // Create a UDP packet with example data
    char udp_packet_data[MAX_BUF_SIZE];
    int packet_size;
    create_udp_packet(udp_packet_data, &packet_size);

    // Encode UDP packet data into audio tones
    float audio_buffer[packet_size * 8 * (int)(BIT_DURATION * SAMPLE_RATE)];
    encode_udp_packet_to_audio(udp_packet_data, packet_size, audio_buffer);

    // Play audio tones representing the encoded UDP packet
    play_audio(audio_buffer, sizeof(audio_buffer) / sizeof(float));
    if (argc<2){
        return -1;
    }
    int sockfd=socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in * to=(struct sockaddr_in * )malloc(sizeof(struct sockaddr_in));
    int port=0;
    char* addr_raw=argv[1];
    
    struct in_addr* address=(struct in_addr *)malloc(sizeof(struct in_addr));
    
    if (inet_pton(AF_INET,addr_raw,address)<1){
        printf("address is:%s",addr_raw);
        return -1;
    }
    sscanf(argv[2],"%d",&port);
    //printf("%s,%d",addr_raw,port);
    to->sin_port=htons(port);
    to->sin_addr=*address;
    to->sin_family=AF_INET;
    to->sin_zero[8]=0;

    char *tosend=(char*)malloc(BUFSIZ);
    size_t len = 0;
    ssize_t read;
    char* buf=(char*)malloc(BUFSIZ);
    printf("Send: ");
    int stat=0;
    unsigned int* lenfrom = (unsigned int*)malloc(sizeof(unsigned int));
    *lenfrom=sizeof(struct sockaddr);
    // Send the audio data in chunks
    int num_chunks = sizeof(audio_buffer) / sizeof(float) / CHUNK_SIZE;
    int remaining_samples = sizeof(audio_buffer) / sizeof(float);
    float* chunk_ptr = audio_buffer;

    for (int i = 0; i < num_chunks; i++) {
        int chunk_size = CHUNK_SIZE * sizeof(float);
        sendto(sockfd, chunk_ptr, chunk_size, 0, (const struct sockaddr *) &to, sizeof(to));
        chunk_ptr += CHUNK_SIZE;
        remaining_samples -= CHUNK_SIZE;
    }

    // Send the remaining samples
    if (remaining_samples > 0) {
        sendto(sockfd, chunk_ptr, remaining_samples * sizeof(float), 0, (const struct sockaddr *) &to, sizeof(to));
    }
    while (stat != -1) {
       stat=(read = getline(&tosend, &len, stdin));
       ssize_t num=sendto(sockfd,tosend,(int)strlen(tosend),0,(const struct sockaddr*)to, (socklen_t)sizeof(*to));
       //perror("sendto failed");
       if ((int)num<0){
            perror("sendto failed");
       }
       ssize_t recnum=recvfrom(sockfd,buf,BUFSIZ,0,(struct sockaddr*)to,lenfrom);
       if ((int)recnum<0){
            perror("receive failed");
       }
       printf("\n received: %s",buf);
       printf("\n Send: ");
    }
    return 0;
}