#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "enki_network.h"

struct MemoriJaringan {
    char *respon;
    size_t ukuran;
};

static size_t tulis_memori_callback(void *konten, size_t ukuran, size_t nmemb, void *userp) {
    size_t ukuran_asli = ukuran * nmemb;
    struct MemoriJaringan *mem = (struct MemoriJaringan *)userp;

    char *ptr = realloc(mem->respon, mem->ukuran + ukuran_asli + 1);
    if(!ptr) return 0; 

    mem->respon = ptr;
    memcpy(&(mem->respon[mem->ukuran]), konten, ukuran_asli);
    mem->ukuran += ukuran_asli;
    mem->respon[mem->ukuran] = 0;

    return ukuran_asli;
}

char* sihir_ambil(const char* url) {
    CURL *curl;
    CURLcode res;
    struct MemoriJaringan chunk;
    chunk.respon = malloc(1); chunk.ukuran = 0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, tulis_memori_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36");
        // curl_easy_setopt(curl, CURLOPT_USERAGENT, "UNUL-OS-Engine/1.0");
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            free(chunk.respon);
            curl_easy_cleanup(curl);
            return strdup("🚨 ERROR: Gagal mengambil data.");
        }
        curl_easy_cleanup(curl);
    }
    return chunk.respon;
}

char* sihir_setor(const char* url, const char* muatan_json) {
    CURL *curl;
    CURLcode res;
    struct MemoriJaringan chunk;
    chunk.respon = malloc(1); chunk.ukuran = 0;

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, muatan_json);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, tulis_memori_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK) {
            free(chunk.respon);
            return strdup("🚨 ERROR: Gagal menyetor data.");
        }
    }
    return chunk.respon;
}