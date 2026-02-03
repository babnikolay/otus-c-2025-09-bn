/********* Пример кода, сгенерированного утилитой командной строки curl **********
 * Все опции cur_easy_setopt() документированы по адресу:
 * https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
 ************************************************************************/
#include <curl/curl.h>

int main()
{
  CURLcode ret;
  CURL *hnd;

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  // curl_easy_setopt(hnd, CURLOPT_URL, "https://google.com");
  curl_easy_setopt(hnd, CURLOPT_URL, "https://wttr.in/Berlin?format=j1");
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.64.1");
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(hnd, CURLOPT_HTTP09_ALLOWED, 1L);
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

  /* Вот список опций, в которых использовался код Curl, который нельзя легко сгенерировать
     как исходный код. Вы можете либо не использовать их, либо реализовать
     их самостоятельно.

  CURLOPT_WRITEDATA установлен в указателе объекта.
  CURLOPT_INTERLEAVEDATA установлен в указателе объекта.
  CURLOPT_WRITEFUNCTION установлен в указателе функции
  CURLOPT_READDATA установлен в указателе объекта.
  CURLOPT_READFUNCTION установлен в указателе функции
  CURLOPT_SEEKDATA установлен в указателе объекта.
  CURLOPT_SEEKFUNCTION установлен в указателе функции
  CURLOPT_ERRORBUFFER установлен в указателе объекта.
  CURLOPT_STDERR установлен в указателе объекта.
  CURLOPT_HEADERFUNCTION установлен в указателе функции
  CURLOPT_HEADERDATA установлен в указателе объекта.

  */

  ret = curl_easy_perform(hnd);

  curl_easy_cleanup(hnd);
  hnd = NULL;

  return (int)ret;
}
/**** Конец кода ****/