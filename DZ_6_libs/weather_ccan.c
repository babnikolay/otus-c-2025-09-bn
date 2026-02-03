/*
sudo apt-get install libcurl4-openssl-dev
gcc weather.c -o weather -lcurl -lccan/json
./weather Moscow
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ccan/json/json.h>

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
            json_value *parsed_json = json_parse(chunk.data, chunk.size);
            if (parsed_json == NULL) {
                fprintf(stderr, "Failed to parse JSON\n");
            } else if (parsed_json->type == json_array) {
                json_value *current_condition = parsed_json->u.array.values[0];
                if (current_condition->type == json_object) {
                    json_value *weather_desc = json_object_get(current_condition, "weatherDesc");
                    json_value *wind_speed = json_object_get(current_condition, "windspeedKmph");
                    json_value *temp_C = json_object_get(current_condition, "temp_C");

                    printf("Weather in %s:\n", city);
                    printf("Description: %s\n", weather_desc->u.array.values[0]->u.object.values[0].value->u.string.ptr);
                    printf("Wind Speed: %s km/h\n", wind_speed->u.string.ptr);
                    printf("Temperature: %s °C\n", temp_C->u.string.ptr);
                }
                json_value_free(parsed_json);
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
