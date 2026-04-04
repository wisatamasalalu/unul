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
    array->kapasitas = 64; 
    array->jumlah = 0;
    array->data = (Token*)malloc(array->kapasitas * sizeof(Token));
}

// 🟢 SUNTIKAN: Tambahkan parameter nama_file
void tambah_token(TokenArray* array, TokenJenis jenis, const char* teks_isi, int baris, int kolom, const char* nama_file) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas *= 2; 
        array->data = (Token*)realloc(array->data, array->kapasitas * sizeof(Token));
    }
    Token* t = &array->data[array->jumlah++];
    t->jenis = jenis;
    t->isi = teks_isi ? strdup(teks_isi) : NULL; 
    t->baris = baris;
    t->kolom = kolom;
    t->nama_file = nama_file ? strdup(nama_file) : strdup("TakDiketahui"); 
}

// Fungsi wajib untuk membuang sampah memori agar tidak bocor
void bebaskan_token_array(TokenArray* array) {
    for (int i = 0; i < array->jumlah; i++) {
        if (array->data[i].isi != NULL) free(array->data[i].isi); 
        if (array->data[i].nama_file != NULL) free(array->data[i].nama_file);
    }
    free(array->data); 
    array->data = NULL;
    array->jumlah = array->kapasitas = 0;
}

// Cek apakah karakter adalah bagian dari nama variabel/fungsi
bool is_identitas(char c) {
    return isalnum(c) || c == '_'; 
}

// ==========================================
// MESIN UTAMA: PEMINDAI KARAKTER
// ==========================================
TokenArray enki_lexer(const char* kode_sumber, const char* nama_file_sumber) {
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
            baris++; kolom = 1; i++; continue; // 🟢 Ruang dan waktu tereset
        }

        // 2. Abaikan Komentar (^^)
        if (c == '^' && kode_sumber[i+1] == '^') {
            while (kode_sumber[i] != '\n' && kode_sumber[i] != '\0') i++;
            continue; 
        }

        // 3. Tangkap Tanda Baca Dasar 
        if (c == '(') { tambah_token(&token_list, TOKEN_KURUNG_B, "(", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == ')') { tambah_token(&token_list, TOKEN_KURUNG_T, ")", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == '{') { tambah_token(&token_list, TOKEN_KURUNG_K_B, "{", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == '}') { tambah_token(&token_list, TOKEN_KURUNG_K_T, "}", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == '[') { tambah_token(&token_list, TOKEN_KURUNG_S_B, "[", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == ']') { tambah_token(&token_list, TOKEN_KURUNG_S_T, "]", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == ',') { tambah_token(&token_list, TOKEN_KOMA, ",", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == ':') { tambah_token(&token_list, TOKEN_TITIK_DUA, ":", baris, kolom, nama_file_sumber); i++; kolom++; continue; }
        if (c == '.') { tambah_token(&token_list, TOKEN_TITIK, ".", baris, kolom, nama_file_sumber); i++; kolom++; continue; }

        // 4. Tangkap Teks ("..." atau '...')
        if (c == '"' || c == '\'') {
            char kutip = c;
            int awal = i;
            int awal_kolom = kolom; 
            i++; kolom++;
            while (kode_sumber[i] != kutip && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }
            if (kode_sumber[i] == kutip) { i++; kolom++; }
            
            int panjang = i - awal;
            char* teks_buffer = (char*)malloc(panjang + 1);
            strncpy(teks_buffer, &kode_sumber[awal], panjang);
            teks_buffer[panjang] = '\0';
            
            tambah_token(&token_list, TOKEN_TEKS, teks_buffer, baris, awal_kolom, nama_file_sumber);
            free(teks_buffer);
            continue;
        }

        // 5. Tangkap Angka (Mendukung Desimal sederhana)
        if (isdigit(c)) {
            int awal = i;
            int awal_kolom = kolom;
            while (isdigit(kode_sumber[i]) || kode_sumber[i] == '.') { i++; kolom++; }
            
            int panjang = i - awal;
            char* angka = (char*)malloc(panjang + 1);
            strncpy(angka, &kode_sumber[awal], panjang);
            angka[panjang] = '\0';
            
            tambah_token(&token_list, TOKEN_ANGKA, angka, baris, awal_kolom, nama_file_sumber);
            free(angka);
            continue;
        }

        // 6. Tangkap Operator Matematika, Logika & Assignment
        // 🔥 BUG FIX: Menambahkan c == '^' agar pangkat tidak dianggap Mismatch!
        if (c == '=' || c == '!' || c == '>' || c == '<' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '|') {
            int awal_kolom = kolom;
            char op_str[3] = {c, '\0', '\0'};
            
            // Cek Fungsi Panah (=>)
            if (c == '=' && kode_sumber[i+1] == '>') {
                tambah_token(&token_list, TOKEN_PANAH, "=>", baris, awal_kolom, nama_file_sumber);
                i += 2; kolom += 2;
                continue;
            }

            // Cek Pipa Aliran (|>)
            if (c == '|' && kode_sumber[i+1] == '>') {
                tambah_token(&token_list, TOKEN_PIPA, "|>", baris, awal_kolom, nama_file_sumber);
                i += 2; kolom += 2;
                continue;
            }

            // Cek Double Operator (==, !=, >=, <=)
            if ((c == '=' || c == '!' || c == '>' || c == '<') && kode_sumber[i+1] == '=') {
                op_str[1] = '=';
                tambah_token(&token_list, TOKEN_PEMBANDING, op_str, baris, awal_kolom, nama_file_sumber);
                i += 2; kolom += 2;
                continue;
            }
            
            TokenJenis tj;
            if (c == '=') tj = TOKEN_ASSIGN;
            else if (c == '>' || c == '<') tj = TOKEN_PEMBANDING;
            else tj = TOKEN_OPERATOR;

            tambah_token(&token_list, tj, op_str, baris, awal_kolom, nama_file_sumber);
            i++; kolom++;
            continue;
        }

        // 7. Tangkap Kata (Identitas, Keyword, Fungsi)
        if (isalpha(c) || c == '_') {
            int awal_kolom = kolom;
            
            // Sihir Lookahead untuk Takdir & Pragma
            if (strncmp(&kode_sumber[i], "takdir.soft", 11) == 0 && !is_identitas(kode_sumber[i+11])) {
                tambah_token(&token_list, TOKEN_TAKDIR, "takdir.soft", baris, awal_kolom, nama_file_sumber);
                i += 11; kolom += 11; continue;
            }
            if (strncmp(&kode_sumber[i], "takdir.hard", 11) == 0 && !is_identitas(kode_sumber[i+11])) {
                tambah_token(&token_list, TOKEN_TAKDIR, "takdir.hard", baris, awal_kolom, nama_file_sumber);
                i += 11; kolom += 11; continue;
            }

            int awal = i;
            while (is_identitas(kode_sumber[i])) { i++; kolom++; }
            
            int panjang = i - awal;
            char* kata = (char*)malloc(panjang + 1);
            strncpy(kata, &kode_sumber[awal], panjang);
            kata[panjang] = '\0';

            // Pragma Lanjutan
            if (strcmp(kata, "butuh") == 0 && strncmp(&kode_sumber[i], " .anu", 5) == 0) {
                tambah_token(&token_list, TOKEN_PRAGMA, "butuh .anu", baris, awal_kolom, nama_file_sumber);
                i += 5; kolom += 5; free(kata); continue;
            }
            if (strcmp(kata, "untuk") == 0) {
                if (strncmp(&kode_sumber[i], " array.dinamis", 14) == 0) {
                    tambah_token(&token_list, TOKEN_PRAGMA, "untuk array.dinamis", baris, awal_kolom, nama_file_sumber);
                    i += 14; kolom += 14; free(kata); continue;
                }
                if (strncmp(&kode_sumber[i], " array.statis", 13) == 0) {
                    tambah_token(&token_list, TOKEN_PRAGMA, "untuk array.statis", baris, awal_kolom, nama_file_sumber);
                    i += 13; kolom += 13; free(kata); continue;
                }
            }

            // Keyword Cek Mutlak
            if (strcmp(kata, "datang") == 0) tambah_token(&token_list, TOKEN_HEADER, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "cocokkan") == 0) tambah_token(&token_list, TOKEN_COCOKKAN, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "kasus") == 0) tambah_token(&token_list, TOKEN_KASUS, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "jika") == 0 || strcmp(kata, "maka") == 0 || strcmp(kata, "lain") == 0 || strcmp(kata, "putus") == 0) tambah_token(&token_list, TOKEN_KARMA, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "effort") == 0 || strcmp(kata, "kali") == 0) tambah_token(&token_list, TOKEN_SIKLUS, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "dan") == 0 || strcmp(kata, "atau") == 0 || strcmp(kata, "bukan") == 0) tambah_token(&token_list, TOKEN_LOGIKA, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "coba") == 0) tambah_token(&token_list, TOKEN_COBA, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "tabu") == 0) tambah_token(&token_list, TOKEN_TABU, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "melanggar") == 0) tambah_token(&token_list, TOKEN_MELANGGAR, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "tebus") == 0) tambah_token(&token_list, TOKEN_TEBUS, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "pasrah") == 0) tambah_token(&token_list, TOKEN_PASRAH, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "ciptakan") == 0) tambah_token(&token_list, TOKEN_CIPTAKAN, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "fungsi") == 0) tambah_token(&token_list, TOKEN_FUNGSI, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "pulang") == 0) tambah_token(&token_list, TOKEN_PULANG, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "sowan") == 0) tambah_token(&token_list, TOKEN_SOWAN, kata, baris, awal_kolom, nama_file_sumber);
            else if (strcmp(kata, "pergi") == 0 || strcmp(kata, "henti") == 0 || strcmp(kata, "balikan") == 0 || strcmp(kata, "terus") == 0) tambah_token(&token_list, TOKEN_KONTROL, kata, baris, awal_kolom, nama_file_sumber);
            else tambah_token(&token_list, TOKEN_IDENTITAS, kata, baris, awal_kolom, nama_file_sumber);

            free(kata);
            continue;
        }

        // 8. Tangkap Karakter Asing
        char op_str[2] = {c, '\0'};
        tambah_token(&token_list, TOKEN_MISMATCH, op_str, baris, kolom, nama_file_sumber);
        i++; kolom++;
    }

    tambah_token(&token_list, TOKEN_EOF, "EOF", baris, kolom, nama_file_sumber);
    return token_list;
}