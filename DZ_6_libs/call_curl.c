/*
**************************************************************************
 * _ _ ____ _
 * Проект ___| | | | _ \| |
 * / __| | | | |_) | |
 * | (__| |_| | _ <| |___
 * \___|\___/|_| \_\_____|
 *
 * Авторские права (C) Даниэль Стенберг, <daniel@haxx.se>, и др.
 *
 * Данное программное обеспечение распространяется по лицензии, описанной в файле COPYING, который
 * Вы должны были получить это в рамках данной рассылки. Условия
 * Также доступны по адресу https://curl.se/docs/copyright.html.
 *
 * Вы можете использовать, копировать, изменять, объединять, публиковать, распространять и/или продавать по своему усмотрению.
 * копии Программного обеспечения и разрешить лицам, которым Программное обеспечение предназначено
 * Предоставляется для выполнения этой задачи в соответствии с условиями файла КОПИРОВАНИЯ.
 *
 * Данное программное обеспечение распространяется «как есть», без каких-либо гарантий.
 * Доброта, выраженная или подразумеваемая.
 *
 * Идентификатор лицензии SPDX: curl
 *
 **************************************************************************
 */ 
/* <DESC>
 * Показано, как функция обратного вызова записи может использоваться для загрузки данных в
 * использовать фрагмент памяти вместо хранения в файле.
 * </DESC>
 */
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>  
#include <curl/curl.h>
 
struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t write_cb( void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = ( struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
   if (!ptr) {
     /* Недостаточно памяти! */ 
    printf( "Недостаточно памяти (realloc вернул NULL)\n" );
     return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
 
int  main ()
{
  CURL *curl;
  CURLcode result;
 
  struct MemoryStruct chunk;
 
  result = curl_global_init(CURL_GLOBAL_ALL);
  if(result)
    return (int)result;
 
  chunk.memory = malloc(1); /* увеличивается по мере необходимости с помощью realloc выше */ 
  chunk.size = 0;            /* на данный момент нет данных */
 
  /* Инициализация сессии curl */ 
  curl = curl_easy_init();
   if (curl) {
 
    /* Укажите URL для получения */ 
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.example.com/" );
 
    /* Отправляем все данные в эту функцию */ 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
 
    /* Мы передаем нашу структуру 'chunk' в функцию обратного вызова */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, ( void *)&chunk);
 
    /* Некоторые серверы не обрабатывают запросы, отправленные без user-gent.
       поле, поэтому мы предоставляем одно */ 
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
 
    /* Получите это! */ 
    result = curl_easy_perform(curl);
 
    /* Проверка на ошибки */ 
    if (result != CURLE_OK) {
      fprintf(stderr, " curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
    }
    else {
       /*
       * Теперь наш chunk.memory указывает на блок памяти размером chunk.size.
       * Размер файла составляет байты и содержит удаленный файл.
       *
       * Сделайте с этим что-нибудь приятное!
       */
 
      printf("%lu байт получен\n" , ( unsigned  long )chunk.size);
    }
 
    /* Очистка кода curl */ 
    curl_easy_cleanup(curl);
  }
 
  free(chunk.memory);
 
  /* Мы закончили работу с libcurl, поэтому очищаем его */ 
  curl_global_cleanup();
 
  return (int)result;
}