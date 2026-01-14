/*
https://github.com/DaveGamble/cJSON
sudo apt-get install libcurl4-openssl-dev libcjson-dev
gcc weather.c -o weather -lcurl -lcjson
./weather Moscow
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)userp;
 
  char *ptr = realloc(mem->data, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("Недостаточно памяти (realloc вернул NULL)\n");
    return 0;
  }
 
  mem->data = ptr;
  memcpy(&(mem->data[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->data[mem->size] = 0;
 
  return realsize;
}

int fetch_weather(const char *city) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk;
    chunk.data = malloc(1);     // увеличивается по мере необходимости с помощью realloc выше 
    chunk.size = 0;             // на данный момент нет данных

    char buffer[256];
    int len_url = 0;
    len_url = snprintf(buffer, sizeof(buffer), "https://wttr.in/%s?format=j1", city);
    char url[len_url];
    for(int i = 0; i < len_url; ++i) {
        url[i] = buffer[i];
    }
    printf("url = %s\n", url);

    // res = curl_global_init(CURL_GLOBAL_DEFAULT);
    res = curl_global_init(CURL_GLOBAL_ALL);
    if(res)
        return (int)res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT , "curl/7.64.1");
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        res = curl_easy_perform(curl);
        // printf("res = %d\n", res);
        if (res != CURLE_OK) {
            fprintf(stderr, "Ошибка curl_easy_perform(): %s\n", curl_easy_strerror(res));
        } 
        else {
            printf("%lu байт получено\n", (unsigned long)chunk.size);
            // printf("chunk.data:\n%s\n", chunk.data);

            // Разбор JSON
            cJSON* json = cJSON_Parse(chunk.data);
            if (json == NULL) {
                fprintf(stderr, "Ошибка cJSON_Parse\n");
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    fprintf(stderr, "Error before: %s\n", error_ptr);
                }
            } 
            else {
                // Доступ к полям
                cJSON *nearest_area = cJSON_GetObjectItem(json, "nearest_area");
                cJSON *area = cJSON_GetArrayItem(nearest_area, 0);

                const char *region = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(area, "region"), 0), "value")->valuestring;
                const char *country = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(area, "country"), 0), "value")->valuestring;

                printf("Страна: %s\n", country);
                printf("Регион: %s\n", region);

                cJSON* current_condition = cJSON_GetObjectItem(json, "current_condition");
                cJSON* condition = cJSON_GetArrayItem(current_condition, 0);

                const char* local_date_time = cJSON_GetObjectItem(condition, "localObsDateTime")->valuestring;
                const char* weather_desc = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(condition, "weatherDesc"), 0), "value")->valuestring;
                const char* temperature_c = cJSON_GetObjectItem(condition, "temp_C")->valuestring;
                const char* feels_like_c = cJSON_GetObjectItem(condition, "FeelsLikeC")->valuestring;
                const char* humidity = cJSON_GetObjectItem(condition, "humidity")->valuestring;
                const char* visibility = cJSON_GetObjectItem(condition, "visibility")->valuestring;
                const char* wind_speed = cJSON_GetObjectItem(condition, "windspeedKmph")->valuestring;
                const char* wind_direction = cJSON_GetObjectItem(condition, "winddir16Point")->valuestring;
                const char* pressure = cJSON_GetObjectItem(condition, "pressure")->valuestring;

                // Вывод данных
                printf("Текущая дата: %s\n", local_date_time);
                printf("Описание погоды: %s\n", weather_desc);
                printf("Температура (C): %s\n", temperature_c);
                printf("Ощущается как (C): %s\n", feels_like_c);
                printf("Влажность: %s%%\n", humidity);
                printf("Видимость (км): %s\n", visibility);
                printf("Скорость ветра (км/ч): %s\n", wind_speed);
                printf("Направление ветра: %s\n", wind_direction);
                printf("Давление (гПа): %s\n", pressure);

            }
            // Освобождение памяти
            cJSON_Delete(json);
        }
        curl_easy_cleanup(curl);
    }
    free(chunk.data);
    curl_global_cleanup();
    curl = NULL;

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использовать: %s <Название города>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fetch_weather(argv[1]);
    return EXIT_SUCCESS;
}
