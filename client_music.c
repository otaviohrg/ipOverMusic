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

int main(int argc, char* argv[]) {
    //float captured_audio[MAX_BUF_SIZE * 8 * (int)(BIT_DURATION * SAMPLE_RATE)]={}; // Buffer for captured audio data
    //float* captured_audio=(float*)malloc(MAX_BUF_SIZE * 8 * (int)(BIT_DURATION * SAMPLE_RATE)*sizeof(float));
    // //Capture audio tones
    // capture_audio(captured_audio, sizeof(captured_audio) / sizeof(float));
    
    // // Decode captured audio to binary data
    // char captured_data[MAX_BUF_SIZE]; // Buffer for decoded captured data
    // memset(captured_data, 0, MAX_BUF_SIZE);
    // decode_audio_to_data(captured_audio, sizeof(captured_audio) / sizeof(float), captured_data);

    // //Print the decoded captured data
    //printf("Decoded Captured Data: %s\n", captured_data);
    // Receiving audio data from the server using sockets
    if (argc<2){
        return -1;
    }
    int sockfd=socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in * to=(struct sockaddr_in * )malloc(sizeof(struct sockaddr_in));
    int port=0;
    sscanf(argv[1],"%d",&port);
    to->sin_port=htons(port);
    to->sin_family=AF_INET;
    int b=bind(sockfd,(struct sockaddr*)to,sizeof(*to));
    printf("%d",b);
    char *buf=(char*)malloc(BUFSIZ);
    unsigned int* len = (unsigned int*)malloc(sizeof(int));
    *len=sizeof(struct sockaddr);
    ssize_t recnum=0;
    while ((int) recnum>=0){
    recnum=recvfrom(sockfd,buf,BUFSIZ,0,(struct sockaddr*)to,len);
    printf("received: %s",buf);
    ssize_t sendnum=sendto(sockfd,buf,strlen(buf),0,(struct sockaddr*)to, sizeof(*to));
    if ((int)sendnum<0){
        perror("tosend failure");
    }}
    return 0;
}
