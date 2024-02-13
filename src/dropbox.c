//
// Created by otaviohribas on 2/13/24.
//
#include "stdlib.h"
#include "string.h"
#include <curl/curl.h>

#include "../lib/dropbox.h"
#include "../lib/json.h"

/**
 * FOUND FROM THE OFFICIAL DOCUMENTATION FOR LIBCURL
 *
 * https://curl.se/libcurl/c/getinmemory.html
*/

struct MemoryStruct
{
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

    /* parse output to extract bearer token */
    int ret = extract_json_value("\"access_token\"", &client->bearer_token, chunk.memory);

    return ret;
}

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *file = (FILE *)userdata;
    return fread(ptr, size, nmemb, file);
}

int upload_file(dropbox_client* client, char* file_path, char* message_id){
    CURLcode res;
    CURL *hnd;
    struct curl_slist *headers;

    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    fseek(file, 0, SEEK_END); // seek to end of file
    int size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET);

    char *auth_header = (char*) malloc(
        strlen("Authorization: Bearer ") +
        strlen(client->bearer_token)
            );
    char *args_header = (char*) malloc(
            strlen("Dropbox-API-Arg: {\"autorename\":false,\"mode\":\"add\",\"mute\":false,\"path\":\"") +
            strlen(message_id) +
            strlen("\",\"strict_conflict\":false}")
    );

    strcpy(auth_header, "Authorization: Bearer ");
    strcat(auth_header, client->bearer_token);

    strcpy(args_header, "Dropbox-API-Arg: {\"autorename\":false,\"mode\":\"add\",\"mute\":false,\"path\":\"");
    if(client->id == CLIENT) strcat(args_header, "/client/");
    else if(client->id == SERVER) strcat(args_header, "/server/");
    else{
        printf("Invalid entity\n");
        return -1;
    }
    strcat(args_header, message_id);
    strcat(args_header, "\",\"strict_conflict\":false}");

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
    curl_easy_setopt(hnd, CURLOPT_READFUNCTION, read_callback);
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

    return 0;
}
