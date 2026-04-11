#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "otim_lexer.h"

void tambah_otim_token(OtimTokenArray* array, OtimToken token) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas = (array->kapasitas == 0) ? 16 : array->kapasitas * 2;
        array->data = realloc(array->data, array->kapasitas * sizeof(OtimToken));
    }
    array->data[array->jumlah++] = token;
}

// 🟢 MESIN PEMBERSIH SPASI (Trim)
static void trim_string(char* str) {
    if (!str) return;
    char* start = str;
    while(isspace(*start)) start++;
    char* end = start + strlen(start) - 1;
    while(end >= start && isspace(*end)) *end-- = '\0';
    if (start != str) memmove(str, start, strlen(start) + 1);
}

OtimTokenArray otim_lexer(const char* kode_sumber) {
    OtimTokenArray tokens = {NULL, 0, 0};
    int i = 0; int baris = 1, kolom = 1;

    while (kode_sumber[i] != '\0') {
        if (isspace(kode_sumber[i])) {
            if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
            i++; kolom++; continue;
        }

        if (kode_sumber[i] == '^' && kode_sumber[i+1] == '^') {
            while (kode_sumber[i] != '\n' && kode_sumber[i] != '\0') i++;
            continue;
        }

        if (strncmp(&kode_sumber[i], "#!datang", 8) == 0) {
            OtimToken t = {TOKEN_OTIM_HEADER, NULL, NULL, NULL, NULL, baris, kolom};
            tambah_otim_token(&tokens, t);
            i += 8; kolom += 8; continue;
        }

        if (strncmp(&kode_sumber[i], "pergi!#", 7) == 0) {
            OtimToken t = {TOKEN_OTIM_FOOTER, NULL, NULL, NULL, NULL, baris, kolom};
            tambah_otim_token(&tokens, t);
            i += 7; kolom += 7; continue;
        }

        // 🟢 TANGKAP TAG (Sekarang memisahkan ID dan Atribut dengan presisi mutlak)
        if (kode_sumber[i] == '<') {
            int awal_tag = ++i; kolom++;
            int is_tutup = 0;
            
            if (kode_sumber[i] == '/') { is_tutup = 1; i++; awal_tag++; kolom++; }
            while (kode_sumber[i] != '>' && kode_sumber[i] != '\0') { i++; kolom++; }
            
            int panjang_tag = i - awal_tag;
            char* isi_tag_mentah = strndup(&kode_sumber[awal_tag], panjang_tag);
            
            OtimToken t = {0};
            t.baris = baris; t.kolom = kolom;
            
            if (is_tutup) {
                t.jenis = TOKEN_OTIM_TAG_TUTUP;
                trim_string(isi_tag_mentah);
                t.tag_nama = strdup(isi_tag_mentah);
            } else {
                t.jenis = TOKEN_OTIM_TAG_BUKA;
                char* koma = strchr(isi_tag_mentah, ',');
                char* titik_dua = strchr(isi_tag_mentah, ':');
                
                if (titik_dua && koma && titik_dua > koma) { titik_dua = NULL; }

                if (titik_dua) *titik_dua = '\0';
                if (koma) *koma = '\0';

                char* tag_nama = isi_tag_mentah;
                char* tag_id = titik_dua ? titik_dua + 1 : NULL;
                char* tag_atr = koma ? koma + 1 : NULL;

                trim_string(tag_nama);
                if (tag_id) trim_string(tag_id);
                if (tag_atr) trim_string(tag_atr);

                t.tag_nama = strdup(tag_nama);
                if (tag_id && strlen(tag_id) > 0) t.tag_id = strdup(tag_id);
                if (tag_atr && strlen(tag_atr) > 0) t.atribut = strdup(tag_atr);
            }
            free(isi_tag_mentah);
            tambah_otim_token(&tokens, t);
            if (kode_sumber[i] == '>') { i++; kolom++; }
            continue;
        }

        // 🟢 TANGKAP TEKS BEBAS (HTML STYLE)
        // Menghancurkan kebutuhan tanda kutip ("...") di OTIM!
        if (kode_sumber[i] != '<' && kode_sumber[i] != '#' && kode_sumber[i] != '^') {
            int awal = i;
            while (kode_sumber[i] != '<' && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }
            
            int panjang = i - awal;
            char* teks_mentah = strndup(&kode_sumber[awal], panjang);
            
            // Hapus tanda kutip jika developer lama masih memakainya
            if (teks_mentah[0] == '"') teks_mentah[0] = ' ';
            if (teks_mentah[panjang-1] == '"') teks_mentah[panjang-1] = ' ';
            
            trim_string(teks_mentah);
            
            if (strlen(teks_mentah) > 0) {
                OtimToken t = {TOKEN_OTIM_TEKS, NULL, NULL, NULL, strdup(teks_mentah), baris, kolom};
                tambah_otim_token(&tokens, t);
            }
            free(teks_mentah);
            continue;
        }
        i++; kolom++;
    }

    OtimToken t_eof = {TOKEN_OTIM_EOF, NULL, NULL, NULL, NULL, baris, kolom};
    tambah_otim_token(&tokens, t_eof);
    return tokens;
}

void bebaskan_otim_token(OtimTokenArray* array) {
    for (int i = 0; i < array->jumlah; i++) {
        if (array->data[i].tag_nama) free(array->data[i].tag_nama);
        if (array->data[i].tag_id) free(array->data[i].tag_id);
        if (array->data[i].atribut) free(array->data[i].atribut);
        if (array->data[i].isi_teks) free(array->data[i].isi_teks);
    }
    free(array->data);
    array->data = NULL;
    array->jumlah = 0;
    array->kapasitas = 0;
}