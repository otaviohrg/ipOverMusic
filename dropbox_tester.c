//
// Created by otaviohribas on 2/12/24.
//
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
#include "lib/dropbox.h"

int main(int argc, char* argv[]) {

    if (argc<4){
        printf("Missing arguments\n");
        return -1;
    }

    dropbox_client client;
    client.role = CLIENT;
    char *client_id = argv[1];
    char *client_secret = argv[2];
    char *auth_code = argv[3];
    char *bearer_token = argv[4];

    if(bearer_token == NULL){
        if(auth(&client, client_id, client_secret, auth_code)){
            printf("Authentication error\n");
            return -1;
        }
    } else{
        client.bearer_token = (char *) malloc(strlen(bearer_token));
        strcpy(client.bearer_token, bearer_token);
    }

    printf("%s\n", client.bearer_token);

    //upload_file(&client, "natello2.tar.gz", "natello2");

    client.role = SERVER;

    //download_file(&client, "natello2.tar.gz");
    delete_file(&client, "Carapaca");

    free(client.bearer_token);
    return 1;

}
