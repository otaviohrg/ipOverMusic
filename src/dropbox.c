//
// Created by otaviohribas on 2/13/24.
//
#include "stdlib.h"
#include "string.h"
#include <curl/curl.h>
#include "json-c/json_tokener.h"
#include "json-c/json_object.h"

#include "../lib/dropbox.h"
#include "../lib/json.h"

/**
 * FOUND FROM THE OFFICIAL DOCUMENTATION FOR LIBCURL
 *
 * https://curl.se/libcurl/c/getinmemory.html
*/

struct MemoryStruct{
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

//Authenticate function
int auth(dropbox_client* client, char* client_id, char* client_secret, char* authorization_code){
    CURLcode res;
    CURL *hnd;
    struct curl_slist *headers;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0;           /* no data at this point */

    char *parameters = (char*)malloc(
            strlen("code=")+
            strlen(authorization_code)+
            strlen("&grant_type=authorization_code&client_id=")+
            strlen(client_id)+
            strlen("&client_secret=")+
            strlen(client_secret)
    );

    strcpy(parameters, "code=");
    strcat(parameters, authorization_code);
    strcat(parameters, "&grant_type=authorization_code&client_id=");
    strcat(parameters, client_id);
    strcat(parameters, "&client_secret=");
    strcat(parameters, client_secret);

    headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.dropbox.com/oauth2/token");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, parameters);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)134);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.2.1");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    /* send all data to this function  */
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(headers);
    headers = NULL;

    /* check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }

    printf("%s\n", chunk.memory);

    /* parse output to extract bearer token */
    json_object *obj = json_tokener_parse(chunk.memory);
    obj = json_object_object_get(obj, "access_token");
    if(obj == NULL){
        printf("Auth Error: Could not parse bearer token");
        return -1;
    }
    client->bearer_token = (char*)json_object_get_string(obj);

    free(chunk.memory);
    return 0;
}

//Upload function
int upload_file(dropbox_client* client, char* file_path, char* uploaded_file_name){
    CURLcode res;
    CURL *hnd;
    struct curl_slist *headers;

    printf("HERE F\n");

    //Process File
    char *command = (char*)malloc(
            strlen("tar czvf send.tar.gz ")+
            strlen(file_path)+1
    );
    strcpy(command, "tar czvf send.tar.gz ");
    strcat(command, file_path);
    system(command);
    FILE *file = fopen("send.tar.gz", "rb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }
    printf("HERE E\n");

    fseek(file, 0, SEEK_END); // seek to end of file
    long size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET);

    printf("HERE D - %ld\n", size);

    //Create Headers
    char *auth_header = (char*) malloc(
        strlen("Authorization: Bearer ") +
        strlen(client->bearer_token)+1
            );
    char *args_header = (char*) malloc(
            strlen("Dropbox-API-Arg: {\"autorename\":false,\"mode\":\"add\",\"mute\":false,\"path\":\"") +
            strlen(uploaded_file_name) +
            strlen("\",\"strict_conflict\":false}")
    );
    strcpy(auth_header, "Authorization: Bearer ");
    strcat(auth_header, client->bearer_token);
    strcpy(args_header, "Dropbox-API-Arg: {\"autorename\":false,\"mode\":\"add\",\"mute\":false,\"path\":\"");

    if(client->role == CLIENT) strcat(args_header, "/client/");
    else if(client->role == SERVER) strcat(args_header, "/server/");
    else{
        printf("Invalid entity\n");
        return -1;
    }
    strcat(args_header, uploaded_file_name);
    strcat(args_header, "\",\"strict_conflict\":false}");

    // Create cURL command
    headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, args_header);
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, NULL);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)size);
    curl_easy_setopt(hnd, CURLOPT_READDATA, (void *)file);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.2.1");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    res = curl_easy_perform(hnd);

    fclose(file);
    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(headers);
    headers = NULL;

    /* check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }

    free(command);
    free(auth_header);
    free(args_header);

    return 0;
}

//Download function
int download_file(dropbox_client* client, char* file_name){
    CURLcode res;
    CURL *hnd;
    struct curl_slist *headers;

    //Process File
    char *directory_path = (char *) malloc(
            strlen("files/")+
            6+
            strlen("/receive/"));
    strcpy(directory_path, "files/");

    if(client->role == CLIENT) strcat(directory_path, "client");
    else if(client->role == SERVER) strcat(directory_path, "server");
    else{
        printf("Invalid entity\n");
        return -1;
    }

    strcat(directory_path, "/receive/");

    char *file_path = (char *) malloc(
            strlen(directory_path)+
            strlen(file_name));
    strcpy(file_path, directory_path);
    strcat(file_path, file_name);

    FILE *file = fopen(file_path, "wb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    //Create Headers
    char *auth_header = (char*) malloc(
            strlen("Authorization: Bearer ") +
            strlen(client->bearer_token)
    );
    char *args_header = (char*) malloc(
            strlen("Dropbox-API-Arg: {\"path\":\"/") +
            7 +
            strlen(file_name) +
            strlen("\"}")
    );
    strcpy(auth_header, "Authorization: Bearer ");
    strcat(auth_header, client->bearer_token);

    strcpy(args_header, "Dropbox-API-Arg: {\"path\":\"/");

    if(client->role == CLIENT) strcat(args_header, "server/");
    else if(client->role == SERVER) strcat(args_header, "client/");
    else{
        printf("Invalid entity\n");
        return -1;
    }
    strcat(args_header, file_name);
    strcat(args_header, "\"}");

    // Create cURL command
    headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, args_header);

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.2.1");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    res = curl_easy_perform(hnd);

    fclose(file);
    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(headers);
    headers = NULL;

    /* check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }

    /* Decompress file */
    char *command = (char*)malloc(
            strlen("tar -xvf ")+
            strlen(file_path)+
            strlen(" -C ") +
            strlen(directory_path)+
            strlen(" && rm ") +
            strlen(file_path)
    );
    strcpy(command, "tar -xvf ");
    strcat(command, file_path);
    strcat(command, " -C ");
    strcat(command, directory_path);
    strcat(command, " && rm ");
    strcat(command, file_path);
    printf("%s\n", command);
    system(command);

    /* Cleanup */
    free(command);
    free(auth_header);
    free(args_header);
    free(directory_path);
    free(file_path);

    return 0;
}
