#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "snul_lexer.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

static void tambah_snul_token(SnulTokenArray* array, SnulToken token) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas = (array->kapasitas == 0) ? 16 : array->kapasitas * 2;
        array->data = (SnulToken*)enki_realokasi(array->data, (array->kapasitas/2) * sizeof(SnulToken), array->kapasitas * sizeof(SnulToken), 1);
    }
    array->data[array->jumlah++] = token;
}

SnulTokenArray snul_lexer(const char* kode_sumber) {
    SnulTokenArray tokens = {NULL, 0, 0};
    int i = 0;
    int baris = 1, kolom = 1;
    int dalam_blok = 0; 

    while (kode_sumber[i] != '\0') {
        if (isspace(kode_sumber[i])) {
            if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
            i++; kolom++;
            continue;
        }

        if (kode_sumber[i] == '^' && kode_sumber[i+1] == '^') {
            while (kode_sumber[i] != '\n' && kode_sumber[i] != '\0') i++;
            continue;
        }

        if (kode_sumber[i] == '{') {
            SnulToken t = {TOKEN_SNUL_LBRACE, NULL, baris, kolom};
            tambah_snul_token(&tokens, t);
            dalam_blok = 1;
            i++; kolom++;
            continue;
        }

        if (kode_sumber[i] == '}') {
            SnulToken t = {TOKEN_SNUL_RBRACE, NULL, baris, kolom};
            tambah_snul_token(&tokens, t);
            dalam_blok = 0;
            i++; kolom++;
            continue;
        }

        // 🟢 TANGKAP SELEKTOR
        if (!dalam_blok && kode_sumber[i] == '@') {
            int awal = i;
            while (kode_sumber[i] != '{' && !isspace(kode_sumber[i]) && kode_sumber[i] != '\0') {
                i++; kolom++;
            }
            int panjang = i - awal;
            
            // 🟢 BERSIH
            char* selektor_mentah = (char*)enki_alokasi(panjang + 1, 1);
            strncpy(selektor_mentah, &kode_sumber[awal], panjang);
            selektor_mentah[panjang] = '\0';
            
            SnulToken t = {TOKEN_SNUL_SELECTOR, enki_salin_teks(selektor_mentah, 1), baris, kolom};
            tambah_snul_token(&tokens, t);
            enki_bebas(selektor_mentah, 1);
            continue;
        }

        if (dalam_blok && (isalpha(kode_sumber[i]) || kode_sumber[i] == '_')) {
            int awal_prop = i;
            while (kode_sumber[i] != ':' && !isspace(kode_sumber[i]) && kode_sumber[i] != '\0') {
                i++; kolom++;
            }
            int panjang_prop = i - awal_prop;
            
            // 🟢 BERSIH
            char* prop_mentah = (char*)enki_alokasi(panjang_prop + 1, 1);
            strncpy(prop_mentah, &kode_sumber[awal_prop], panjang_prop);
            prop_mentah[panjang_prop] = '\0';
            
            SnulToken t_prop = {TOKEN_SNUL_PROPERTI, enki_salin_teks(prop_mentah, 1), baris, kolom};
            tambah_snul_token(&tokens, t_prop);
            enki_bebas(prop_mentah, 1);

            while ((isspace(kode_sumber[i]) || kode_sumber[i] == ':') && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }

            int awal_nilai = i;
            while (kode_sumber[i] != ';' && kode_sumber[i] != '}' && kode_sumber[i] != '\n' && kode_sumber[i] != '\0') {
                i++; kolom++;
            }
            int panjang_nilai = i - awal_nilai;
            
            // 🟢 BERSIH
            char* nilai_mentah = (char*)enki_alokasi(panjang_nilai + 1, 1);
            strncpy(nilai_mentah, &kode_sumber[awal_nilai], panjang_nilai);
            nilai_mentah[panjang_nilai] = '\0';
            
            SnulToken t_nilai = {TOKEN_SNUL_NILAI, enki_salin_teks(nilai_mentah, 1), baris, kolom};
            tambah_snul_token(&tokens, t_nilai);
            enki_bebas(nilai_mentah, 1);

            if (kode_sumber[i] == ';') { i++; kolom++; }
            continue;
        }

        i++; kolom++;
    }

    SnulToken t_eof = {TOKEN_SNUL_EOF, NULL, baris, kolom};
    tambah_snul_token(&tokens, t_eof);
    return tokens;
}

void bebaskan_snul_token(SnulTokenArray* array) {
    for (int i = 0; i < array->jumlah; i++) {
        if (array->data[i].teks) enki_bebas(array->data[i].teks, 1);
    }
    enki_bebas(array->data, 1);
    array->data = NULL;
    array->jumlah = 0;
    array->kapasitas = 0;
}