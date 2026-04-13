#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "enki_network.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

struct MemoriJaringan {
    char *respon;
    size_t ukuran;
};

static size_t tulis_memori_callback(void *konten, size_t ukuran, size_t nmemb, void *userp) {
    size_t ukuran_asli = ukuran * nmemb;
    struct MemoriJaringan *mem = (struct MemoriJaringan *)userp;

    // 🟢 MENGGUNAKAN ENKI_REALOKASI (Mode Dinamis = 1)
    char *ptr = (char*)enki_realokasi(mem->respon, mem->ukuran, mem->ukuran + ukuran_asli + 1, 1);
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
    chunk.respon = (char*)enki_alokasi(1, 1); // 🟢 MENGGUNAKAN ENKI_ALOKASI
    chunk.ukuran = 0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, tulis_memori_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36");
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            enki_bebas(chunk.respon, 1); // 🟢 MENGGUNAKAN ENKI_BEBAS
            curl_easy_cleanup(curl);
            return enki_salin_teks("🚨 ERROR: Gagal mengambil data.", 1); // 🟢 MENGGUNAKAN ENKI_SALIN_TEKS
        }
        curl_easy_cleanup(curl);
    }
    return chunk.respon;
}

char* sihir_setor(const char* url, const char* muatan_json) {
    CURL *curl;
    CURLcode res;
    struct MemoriJaringan chunk;
    chunk.respon = (char*)enki_alokasi(1, 1); // 🟢 MENGGUNAKAN ENKI_ALOKASI
    chunk.ukuran = 0;

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
            enki_bebas(chunk.respon, 1); // 🟢 MENGGUNAKAN ENKI_BEBAS
            return enki_salin_teks("🚨 ERROR: Gagal menyetor data.", 1); // 🟢 MENGGUNAKAN ENKI_SALIN_TEKS
        }
    }
    return chunk.respon;
}