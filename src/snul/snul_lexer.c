#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "snul_lexer.h"

// Asisten Alokasi Memori Dinamis
static void tambah_snul_token(SnulTokenArray* array, SnulToken token) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas = (array->kapasitas == 0) ? 16 : array->kapasitas * 2;
        array->data = realloc(array->data, array->kapasitas * sizeof(SnulToken));
    }
    array->data[array->jumlah++] = token;
}

SnulTokenArray snul_lexer(const char* kode_sumber) {
    SnulTokenArray tokens = {NULL, 0, 0};
    int i = 0;
    int baris = 1, kolom = 1;
    int dalam_blok = 0; // 0 = Luar (Cari Selektor), 1 = Dalam (Cari Properti)

    while (kode_sumber[i] != '\0') {
        // 1. Lewati Spasi
        if (isspace(kode_sumber[i])) {
            if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
            i++; kolom++;
            continue;
        }

        // 2. Abaikan Komentar UNUL (^^)
        if (kode_sumber[i] == '^' && kode_sumber[i+1] == '^') {
            while (kode_sumber[i] != '\n' && kode_sumber[i] != '\0') i++;
            continue;
        }

        // 3. Pintu Masuk Gaya '{'
        if (kode_sumber[i] == '{') {
            SnulToken t = {TOKEN_SNUL_LBRACE, NULL, baris, kolom};
            tambah_snul_token(&tokens, t);
            dalam_blok = 1;
            i++; kolom++;
            continue;
        }

        // 4. Pintu Keluar Gaya '}'
        if (kode_sumber[i] == '}') {
            SnulToken t = {TOKEN_SNUL_RBRACE, NULL, baris, kolom};
            tambah_snul_token(&tokens, t);
            dalam_blok = 0;
            i++; kolom++;
            continue;
        }

        // 5. Tangkap Target Selektor (Misal: @wadah.utama)
        if (!dalam_blok && kode_sumber[i] == '@') {
            int awal = i;
            while (kode_sumber[i] != '{' && !isspace(kode_sumber[i]) && kode_sumber[i] != '\0') {
                i++; kolom++;
            }
            int panjang = i - awal;
            SnulToken t = {TOKEN_SNUL_SELECTOR, strndup(&kode_sumber[awal], panjang), baris, kolom};
            tambah_snul_token(&tokens, t);
            continue;
        }

        // 6. Tangkap Properti dan Nilai (Di dalam kurung {})
        if (dalam_blok && (isalpha(kode_sumber[i]) || kode_sumber[i] == '_')) {
            // A. Menangkap Nama Properti (Misal: warna_latar)
            int awal_prop = i;
            while (kode_sumber[i] != ':' && !isspace(kode_sumber[i]) && kode_sumber[i] != '\0') {
                i++; kolom++;
            }
            int panjang_prop = i - awal_prop;
            SnulToken t_prop = {TOKEN_SNUL_PROPERTI, strndup(&kode_sumber[awal_prop], panjang_prop), baris, kolom};
            tambah_snul_token(&tokens, t_prop);

            // Lewati spasi dan titik dua ':' pemisah
            while ((isspace(kode_sumber[i]) || kode_sumber[i] == ':') && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }

            // B. Menangkap Nilainya (Misal: #1A1A1A atau 20px otomatis)
            int awal_nilai = i;
            while (kode_sumber[i] != ';' && kode_sumber[i] != '}' && kode_sumber[i] != '\n' && kode_sumber[i] != '\0') {
                i++; kolom++;
            }
            int panjang_nilai = i - awal_nilai;
            
            // Simpan nilai (trim spasi manual jika Anda mau di masa depan, saat ini ditelan utuh)
            SnulToken t_nilai = {TOKEN_SNUL_NILAI, strndup(&kode_sumber[awal_nilai], panjang_nilai), baris, kolom};
            tambah_snul_token(&tokens, t_nilai);

            // Lewati titik koma ';'
            if (kode_sumber[i] == ';') { i++; kolom++; }
            continue;
        }

        // Abaikan karakter yang tidak dimengerti agar Lexer tidak tersedak
        i++; kolom++;
    }

    SnulToken t_eof = {TOKEN_SNUL_EOF, NULL, baris, kolom};
    tambah_snul_token(&tokens, t_eof);
    return tokens;
}

void bebaskan_snul_token(SnulTokenArray* array) {
    for (int i = 0; i < array->jumlah; i++) {
        if (array->data[i].teks) free(array->data[i].teks);
    }
    free(array->data);
    array->data = NULL;
    array->jumlah = 0;
    array->kapasitas = 0;
}