#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "otim_lexer.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

void tambah_otim_token(OtimTokenArray* array, OtimToken token) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas = (array->kapasitas == 0) ? 16 : array->kapasitas * 2;
        array->data = (OtimToken*)enki_realokasi(array->data, (array->kapasitas/2) * sizeof(OtimToken), array->kapasitas * sizeof(OtimToken), 1);
    }
    array->data[array->jumlah++] = token;
}

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

        // 🟢 TANGKAP TAG & ATRIBUT (Format Kunci="Nilai")
        if (kode_sumber[i] == '<') {
            int awal_tag = ++i; kolom++;
            int is_tutup = 0;
            
            if (kode_sumber[i] == '/') { is_tutup = 1; i++; awal_tag++; kolom++; }
            while (kode_sumber[i] != '>' && kode_sumber[i] != '\0') { i++; kolom++; }
            
            int panjang_tag = i - awal_tag;
            
            // 🟢 BERSIH: enki_alokasi
            char* isi_tag_mentah = (char*)enki_alokasi(panjang_tag + 1, 1);
            strncpy(isi_tag_mentah, &kode_sumber[awal_tag], panjang_tag);
            isi_tag_mentah[panjang_tag] = '\0';
            
            OtimToken t = {0};
            t.baris = baris; t.kolom = kolom;
            
            if (is_tutup) {
                t.jenis = TOKEN_OTIM_TAG_TUTUP;
                trim_string(isi_tag_mentah);
                t.tag_nama = enki_salin_teks(isi_tag_mentah, 1);
            } else {
                t.jenis = TOKEN_OTIM_TAG_BUKA;
                
                // Pisahkan Nama Tag dari Atributnya
                char* spasi_pertama = strchr(isi_tag_mentah, ' ');
                if (spasi_pertama) {
                    *spasi_pertama = '\0';
                    t.tag_nama = enki_salin_teks(isi_tag_mentah, 1);
                    
                    char* sisa_atribut = spasi_pertama + 1;
                    trim_string(sisa_atribut);
                    
                    char* temukan_id = strstr(sisa_atribut, "id=\"");
                    if (!temukan_id) temukan_id = strstr(sisa_atribut, "id='"); 
                    
                    if (temukan_id) {
                        char kutip = temukan_id[3];
                        char* awal_id = temukan_id + 4;
                        char* akhir_id = strchr(awal_id, kutip);
                        if (akhir_id) {
                            *akhir_id = '\0';
                            t.tag_id = enki_salin_teks(awal_id, 1); 
                            *akhir_id = kutip; 
                        }
                    }
                    
                    if (strlen(sisa_atribut) > 0) {
                        t.atribut = enki_salin_teks(sisa_atribut, 1);
                    }
                } else {
                    t.tag_nama = enki_salin_teks(isi_tag_mentah, 1);
                }
            }
            // 🟢 BERSIH: enki_bebas
            enki_bebas(isi_tag_mentah, 1);
            
            tambah_otim_token(&tokens, t);
            if (kode_sumber[i] == '>') { i++; kolom++; }
            continue;
        }

        // 🟢 TANGKAP TEKS BEBAS
        if (kode_sumber[i] != '<' && kode_sumber[i] != '#' && kode_sumber[i] != '^') {
            int awal = i;
            while (kode_sumber[i] != '<' && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }
            
            int panjang = i - awal;
            
            // 🟢 BERSIH: enki_alokasi
            char* teks_mentah = (char*)enki_alokasi(panjang + 1, 1);
            strncpy(teks_mentah, &kode_sumber[awal], panjang);
            teks_mentah[panjang] = '\0';
            
            if (teks_mentah[0] == '"') teks_mentah[0] = ' ';
            if (teks_mentah[panjang-1] == '"') teks_mentah[panjang-1] = ' ';
            
            trim_string(teks_mentah);
            
            if (strlen(teks_mentah) > 0) {
                OtimToken t = {TOKEN_OTIM_TEKS, NULL, NULL, NULL, enki_salin_teks(teks_mentah, 1), baris, kolom};
                tambah_otim_token(&tokens, t);
            }
            
            // 🟢 BERSIH: enki_bebas
            enki_bebas(teks_mentah, 1);
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
        if (array->data[i].tag_nama) enki_bebas(array->data[i].tag_nama, 1);
        if (array->data[i].tag_id) enki_bebas(array->data[i].tag_id, 1);
        if (array->data[i].atribut) enki_bebas(array->data[i].atribut, 1);
        if (array->data[i].isi_teks) enki_bebas(array->data[i].isi_teks, 1);
    }
    enki_bebas(array->data, 1);
    array->data = NULL;
    array->jumlah = 0;
    array->kapasitas = 0;
}