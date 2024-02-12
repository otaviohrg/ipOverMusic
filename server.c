#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "lib/audio.h"
#include "lib/packet.h"


#define PORT 8080
#define MAX_BUF_SIZE 1024
#define CHUNK_SIZE 512


int main(int argc, char* argv[]) {
    // Create a UDP packet with example data
    char udp_packet_data[MAX_BUF_SIZE];
    int packet_size;
    create_udp_packet(udp_packet_data, &packet_size);

    // Encode UDP packet data into audio tones
    float audio_buffer[packet_size * 8 * (int)(BIT_DURATION * SAMPLE_RATE)];
    encode_data_to_audio(udp_packet_data, packet_size, audio_buffer);

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