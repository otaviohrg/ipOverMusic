//
// Created by otaviohribas on 2/13/24.
//

#ifndef IPOVERMUSIC_JSON_H
#define IPOVERMUSIC_JSON_H

typedef struct json_tokener json_tokener;
typedef struct json_object json_object;

int extract_json_value(char *key, char **value, char *json);

#endif //IPOVERMUSIC_JSON_H
