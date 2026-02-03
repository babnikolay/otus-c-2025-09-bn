/*
https://github.com/json-c/json-c
Использует для сборки CMake
sudo apt-get install libcurl4-openssl-dev libjson-c-dev
gcc weather.c -o weather -lcurl -ljson-c
./weather Moscow
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, MemoryStruct *userp) {
    size_t total_size = size * nmemb;
    userp->data = realloc(userp->data, userp->size + total_size + 1);
    if (userp->data == NULL) {
        fprintf(stderr, "Not enough memory\n");
        return 0;
    }

    memcpy(&(userp->data[userp->size]), contents, total_size);
    userp->size += total_size;
    userp->data[userp->size] = 0;

    return total_size;
}

void fetch_weather(const char *city) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};

    char url[256];
    snprintf(url, sizeof(url), "https://wttr.in/%s?format=j1", city);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            struct json_object *parsed_json;
            struct json_object *current_condition;
            struct json_object *weather_desc;
            struct json_object *wind_speed;
            struct json_object *temp_C;

            parsed_json = json_tokener_parse(chunk.data);
            if (parsed_json == NULL) {
                fprintf(stderr, "Failed to parse JSON\n");
            } else {
                json_object *condition = json_object_object_get(parsed_json, "current_condition");
                if (condition) {
                    weather_desc = json_object_array_get_idx(condition, 0);
                    wind_speed = json_object_object_get(weather_desc, "windspeedKmph");
                    temp_C = json_object_object_get(weather_desc, "temp_C");

                    printf("Weather in %s:\n", city);
                    printf("Description: %s\n", json_object_get_string(json_object_object_get(weather_desc, "weatherDesc")));
                    printf("Wind Speed: %s km/h\n", json_object_get_string(wind_speed));
                    printf("Temperature: %s °C\n", json_object_get_string(temp_C));
                } else {
                    fprintf(stderr, "Could not find current condition in response.\n");
                }
                json_object_put(parsed_json);
            }
        }
        free(chunk.data);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <City Name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fetch_weather(argv[1]);
    return EXIT_SUCCESS;
}
