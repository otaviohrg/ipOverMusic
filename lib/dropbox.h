//
// Created by otaviohribas on 2/13/24.
//

#ifndef IPOVERMUSIC_DROPBOX_H
#define IPOVERMUSIC_DROPBOX_H

enum identity {
    CLIENT,
    SERVER
};
typedef enum identity identity;

struct dropbox_client {
    identity id;
    char* client_id;
    char* client_secret;
    char* authorization_code;
    char* bearer_token;
};
typedef struct dropbox_client dropbox_client;

/**
 * Authenticate function.
 *
 * @param client: configuration object connecting to Dropbox;
 * @param client_id: environment key to get the client id for the Dropbox app;
 * @param client_secret: environment key to get the secret key for the Dropbox app;
 * @param authorization_code: environment key to get the chat authorization_code for connecting the Dropbox app;
 *
 * @return: -1 in case of any error and 0 otherwise
*/

int auth(dropbox_client* client, char* client_id, char* client_secret, char* authorization_code);

int upload_file(dropbox_client* client, char* file_path, char* message_id);


#endif //IPOVERMUSIC_DROPBOX_H
