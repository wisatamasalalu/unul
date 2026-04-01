#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "enki_lexer.h"

// =================================================================
// DAPUR MESIN LEXER (CHARACTER STEPPER)
// Mengiris teks huruf demi huruf tanpa Regex. Selamat datang di C!
// =================================================================

// Fungsi internal untuk memesan RAM awal
void inisialisasi_array(TokenArray* array) {
    array->kapasitas = 64; // Mulai dengan pesanan 64 kavling token
    array->jumlah = 0;
    // Ini dia kerumitan C: Memesan RAM manual (malloc)
    array->data = (Token*)malloc(array->kapasitas * sizeof(Token));
}

// Fungsi internal untuk menambah token dan melebarkan RAM jika penuh
void tambah_token(TokenArray* array, TokenJenis jenis, const char* teks_isi, int baris, int kolom) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas *= 2; // Gandakan kapasitas kavling
        // Minta OS untuk memperbesar ukuran RAM (realloc)
        array->data = (Token*)realloc(array->data, array->kapasitas * sizeof(Token));
    }
    
    Token* t = &array->data[array->jumlah++];
    t->jenis = jenis;
    // Menyalin teks asli ke dalam memori token (strdup)
    t->isi = teks_isi ? strdup(teks_isi) : NULL; 
    t->baris = baris;
    t->kolom = kolom;
}

// Fungsi wajib untuk membuang sampah memori agar tidak bocor (Memory Leak)
void bebaskan_token_array(TokenArray* array) {
    for (int i = 0; i < array->jumlah; i++) {
        if (array->data[i].isi != NULL) {
            free(array->data[i].isi); // Bebaskan teks
        }
    }
    free(array->data); // Bebaskan array-nya
    array->data = NULL;
    array->jumlah = array->kapasitas = 0;
}

// Cek apakah karakter adalah bagian dari nama variabel/fungsi
bool is_identitas(char c) {
    return isalnum(c) || c == '_' || c == '.'; // Kita izinkan titik untuk 'takdir.soft' dll
}

// ==========================================
// MESIN UTAMA: PEMINDAI KARAKTER
// ==========================================
TokenArray enki_lexer(const char* kode_sumber) {
    TokenArray token_list;
    inisialisasi_array(&token_list);

    int baris = 1;
    int kolom = 1;
    int i = 0; // Indeks/Kursor pembaca
    
    while (kode_sumber[i] != '\0') {
        char c = kode_sumber[i];

        // 1. Abaikan Spasi dan Baris Baru
        if (c == ' ' || c == '\t' || c == '\r') {
            i++; kolom++; continue;
        }
        if (c == '\n') {
            baris++; kolom = 1; i++; continue;
        }

        // 2. Abaikan Komentar (^^)
        if (c == '^' && kode_sumber[i+1] == '^') {
            while (kode_sumber[i] != '\n' && kode_sumber[i] != '\0') {
                i++;
            }
            continue; // Ulangi dari baris baru
        }

        // 3. Tangkap Tanda Baca Dasar (Kavling)
        if (c == '(') { tambah_token(&token_list, TOKEN_KURUNG_B, "(", baris, kolom); i++; kolom++; continue; }
        if (c == ')') { tambah_token(&token_list, TOKEN_KURUNG_T, ")", baris, kolom); i++; kolom++; continue; }
        if (c == '{') { tambah_token(&token_list, TOKEN_KURUNG_K_B, "{", baris, kolom); i++; kolom++; continue; }
        if (c == '}') { tambah_token(&token_list, TOKEN_KURUNG_K_T, "}", baris, kolom); i++; kolom++; continue; }
        if (c == '[') { tambah_token(&token_list, TOKEN_KURUNG_S_B, "[", baris, kolom); i++; kolom++; continue; }
        if (c == ']') { tambah_token(&token_list, TOKEN_KURUNG_S_T, "]", baris, kolom); i++; kolom++; continue; }
        if (c == ',') { tambah_token(&token_list, TOKEN_KOMA, ",", baris, kolom); i++; kolom++; continue; }

        // 4. Tangkap Teks ("..." atau '...')
        if (c == '"' || c == '\'') {
            char kutip = c;
            int awal = i;
            i++; kolom++;
            while (kode_sumber[i] != kutip && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }
            if (kode_sumber[i] == kutip) {
                i++; kolom++; // Lewati kutip penutup
            }
            // Ekstrak string murni (termasuk kutipnya untuk saat ini)
            int panjang = i - awal;
            char* teks_buffer = (char*)malloc(panjang + 1);
            strncpy(teks_buffer, &kode_sumber[awal], panjang);
            teks_buffer[panjang] = '\0';
            
            tambah_token(&token_list, TOKEN_TEKS, teks_buffer, baris, kolom - panjang);
            free(teks_buffer);
            continue;
        }

        // [TAMBAHKAN INI SEBELUM BLOK 5 (IDENTITAS)]
        // 4.5 Tangkap Angka (Mendukung Desimal sederhana)
        if (isdigit(c)) {
            int awal = i;
            while (isdigit(kode_sumber[i]) || kode_sumber[i] == '.') {
                i++; kolom++;
            }
            int panjang = i - awal;
            char* angka = (char*)malloc(panjang + 1);
            strncpy(angka, &kode_sumber[awal], panjang);
            angka[panjang] = '\0';
            tambah_token(&token_list, TOKEN_ANGKA, angka, baris, kolom - panjang);
            free(angka);
            continue;
        }

        // 4.6 Tangkap Operator Matematika, Logika & Assignment (Lebih Cerdas)
        if (c == '=' || c == '!' || c == '>' || c == '<' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
            char op_str[3] = {c, '\0', '\0'};
            
            // Cek apakah ini operator 2 karakter (==, !=, >=, <=)
            if ((c == '=' || c == '!' || c == '>' || c == '<') && kode_sumber[i+1] == '=') {
                op_str[1] = '=';
                tambah_token(&token_list, TOKEN_PEMBANDING, op_str, baris, kolom);
                i += 2; kolom += 2;
                continue;
            }
            
            // Jika hanya 1 karakter
            TokenJenis tj;
            if (c == '=') tj = TOKEN_ASSIGN;
            else if (c == '>' || c == '<') tj = TOKEN_PEMBANDING;
            else tj = TOKEN_OPERATOR;

            tambah_token(&token_list, tj, op_str, baris, kolom);
            i++; kolom++;
            continue;
        }

        // 5. Tangkap Kata (Identitas, Keyword, Fungsi)
        if (isalpha(c) || c == '_') {
    int awal = i;
    while (is_identitas(kode_sumber[i])) {
        i++; kolom++;
    }
    int panjang = i - awal;
    char* kata = (char*)malloc(panjang + 1);
    strncpy(kata, &kode_sumber[awal], panjang);
    kata[panjang] = '\0';

    // --- SIHIR INTIP (LOOKAHEAD) DIMULAI DI SINI ---

    // 1. Cek Mantra "butuh .anu"
    if (strcmp(kata, "butuh") == 0) {
        // Intip apakah 5 karakter kedepan adalah " .anu"
        if (strncmp(&kode_sumber[i], " .anu", 5) == 0) {
            tambah_token(&token_list, TOKEN_PRAGMA, "butuh .anu", baris, kolom - panjang);
            i += 5; // Loncat melewati " .anu"
            kolom += 5;
            free(kata); continue;
        }
    }
    
    // 2. Cek Mantra "untuk array.dinamis"
    if (strcmp(kata, "untuk") == 0) {
        // Intip apakah setelahnya adalah " array.dinamis"
        if (strncmp(&kode_sumber[i], " array.dinamis", 14) == 0) {
            tambah_token(&token_list, TOKEN_PRAGMA, "untuk array.dinamis", baris, kolom - panjang);
            i += 14; // Loncat melewati " array.dinamis"
            kolom += 14;
            free(kata); continue;
        }
    }
            // Cek Manual Keyword (Ganti Regex)
            if (strcmp(kata, "datang") == 0) tambah_token(&token_list, TOKEN_HEADER, kata, baris, kolom - panjang);
            else if (strcmp(kata, "untuk array.dinamis") == 0 || strcmp(kata, "butuh .anu") == 0) tambah_token(&token_list, TOKEN_PRAGMA, kata, baris, kolom - panjang);
            else if (strcmp(kata, "takdir.soft") == 0 || strcmp(kata, "takdir.hard") == 0) tambah_token(&token_list, TOKEN_TAKDIR, kata, baris, kolom - panjang);
            else if (strcmp(kata, "jika") == 0 || strcmp(kata, "maka") == 0 || strcmp(kata, "lain") == 0 || strcmp(kata, "putus") == 0) tambah_token(&token_list, TOKEN_KARMA, kata, baris, kolom - panjang);
            else if (strcmp(kata, "effort") == 0 || strcmp(kata, "kali") == 0) tambah_token(&token_list, TOKEN_SIKLUS, kata, baris, kolom - panjang);
            else if (strcmp(kata, "coba") == 0) tambah_token(&token_list, TOKEN_COBA, kata, baris, kolom - panjang);
            else if (strcmp(kata, "tabu") == 0) tambah_token(&token_list, TOKEN_TABU, kata, baris, kolom - panjang);
            else if (strcmp(kata, "melanggar") == 0) tambah_token(&token_list, TOKEN_MELANGGAR, kata, baris, kolom - panjang);
            else if (strcmp(kata, "tebus") == 0) tambah_token(&token_list, TOKEN_TEBUS, kata, baris, kolom - panjang);
            else if (strcmp(kata, "pasrah") == 0) tambah_token(&token_list, TOKEN_PASRAH, kata, baris, kolom - panjang);
            
            // --- INI YANG DIPISAH ---
            else if (strcmp(kata, "ciptakan") == 0) tambah_token(&token_list, TOKEN_CIPTAKAN, kata, baris, kolom - panjang);
            else if (strcmp(kata, "fungsi") == 0) tambah_token(&token_list, TOKEN_FUNGSI, kata, baris, kolom - panjang);
            else if (strcmp(kata, "pulang") == 0) tambah_token(&token_list, TOKEN_PULANG, kata, baris, kolom - panjang);
            
            // --- SISA KONTROL YANG LAMA ---
            else if (strcmp(kata, "sowan") == 0) tambah_token(&token_list, TOKEN_SOWAN, kata, baris, kolom - panjang);
            // --->> PERUBAHAN HANYA DI BARIS INI (Tambahkan "terus" di ujungnya):
            else if (strcmp(kata, "pergi") == 0 || strcmp(kata, "henti") == 0 || strcmp(kata, "balikan") == 0 || strcmp(kata, "terus") == 0) tambah_token(&token_list, TOKEN_KONTROL, kata, baris, kolom - panjang);
            // Jika tidak ada yang cocok, berarti ini Variabel atau Nama Fungsi buatan user!
            else tambah_token(&token_list, TOKEN_IDENTITAS, kata, baris, kolom - panjang);

            free(kata);
            continue;
        }

        // 6. Tangkap Operator yang tertinggal (Kita handle sederhana dulu)
        char op_str[2] = {c, '\0'};
        tambah_token(&token_list, TOKEN_MISMATCH, op_str, baris, kolom);
        i++; kolom++;
    }

    // Akhir dari file
    tambah_token(&token_list, TOKEN_EOF, "EOF", baris, kolom);
    return token_list;
}