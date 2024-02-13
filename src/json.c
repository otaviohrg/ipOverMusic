//
// Created by otaviohribas on 2/13/24.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../lib/json.h"

int extract_json_value(char *key, char **value, char *json){
    char *ptr = (char*) malloc(strlen(json));
    strcpy(ptr, json);
    char *end_ptr;

    ptr = strstr(ptr, key);
    if (ptr == NULL) {
        return -1;
    }
    ptr = strchr(ptr, '\"');
    if (ptr == NULL) {
        return -1;
    }
    ptr++;
    ptr = strchr(ptr, '\"');
    if (ptr == NULL) {
        return -1;
    }
    ptr++;
    ptr = strchr(ptr, '\"');
    if (ptr == NULL) {
        return -1;
    }
    ptr++;
    end_ptr = strchr(ptr, '\"');
    if (end_ptr == NULL) {
        return -1;
    }
    *end_ptr ='\0';
    *value = (char*)malloc(strlen(ptr));
    strcpy(*value, ptr);
    printf("%s\n", *value);

    return 0;
}