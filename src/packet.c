#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/packet.h"

// Function to create a UDP packet with example data
void create_udp_packet(char* packet_data, int* packet_size) {
    // Example UDP packet data
    char example_data[] = "This is a UDP packet!";
    *packet_size = strlen(example_data);
    memcpy(packet_data, example_data, *packet_size);
}
