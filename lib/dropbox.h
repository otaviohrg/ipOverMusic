//
// Created by otaviohribas on 2/13/24.
//

#ifndef IPOVERMUSIC_DROPBOX_H
#define IPOVERMUSIC_DROPBOX_H

enum type {
    CLIENT,
    SERVER
};
typedef enum type type;

struct dropbox_client {
    type role;
    char* client_id;
    char* client_secret;
    char* authorization_code;
    char* bearer_token;
};
typedef struct dropbox_client dropbox_client;

/**
 * Authenticate function.
 *
 * @param client: client object for sending requests to Dropbox;
 * @param client_id: environment key to get the client id for the Dropbox app;
 * @param client_secret: environment key to get the secret key for the Dropbox app;
 * @param authorization_code: environment key to get the chat authorization_code for connecting the Dropbox app;
 *
 * @return: -1 in case of any error and 0 otherwise
*/

int auth(dropbox_client* client, char* client_id, char* client_secret, char* authorization_code);

/**
 * Upload function.
 *
 * @param client: client object for sending requests to Dropbox;
 * @param file_path: string containing the path of the file to be uploaded;
 * @param uploaded_file_name: string containing the name of the file as it is uploaded to Dropbox;
 *
 * @return: -1 in case of any error and 0 otherwise
*/

int upload_file(dropbox_client* client, char* file_path, char* uploaded_file_name);

/**
 * Download function.
 *
 * @param client: client object for sending requests to Dropbox;
 * @param file_name: string containing the name of the file to be downloaded from Dropbox;
 *
 * @return: -1 in case of any error and 0 otherwise
*/

int download_file(dropbox_client* client, char* file_name);

#endif //IPOVERMUSIC_DROPBOX_H
