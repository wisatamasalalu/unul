#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "otim_lexer.h"

// Fungsi pembantu untuk memori
void tambah_otim_token(OtimTokenArray* array, OtimToken token) {
    if (array->jumlah >= array->kapasitas) {
        array->kapasitas = (array->kapasitas == 0) ? 16 : array->kapasitas * 2;
        array->data = realloc(array->data, array->kapasitas * sizeof(OtimToken));
    }
    array->data[array->jumlah++] = token;
}

OtimTokenArray otim_lexer(const char* kode_sumber) {
    OtimTokenArray tokens = {NULL, 0, 0};
    int i = 0;
    int baris = 1, kolom = 1;

    while (kode_sumber[i] != '\0') {
        // 1. Lewati Spasi (indentasi tidak bermakna di OTIM)
        if (isspace(kode_sumber[i])) {
            if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
            i++; kolom++;
            continue;
        }

        // 2. Abaikan Komentar (^^)
        if (kode_sumber[i] == '^' && kode_sumber[i+1] == '^') {
            while (kode_sumber[i] != '\n' && kode_sumber[i] != '\0') i++;
            continue;
        }

        // 3. Tangkap Header (#!datang)
        if (strncmp(&kode_sumber[i], "#!datang", 8) == 0) {
            OtimToken t = {TOKEN_OTIM_HEADER, NULL, NULL, NULL, NULL, baris, kolom};
            tambah_otim_token(&tokens, t);
            i += 8; kolom += 8;
            continue;
        }

        // 4. Tangkap Footer (pergi!#)
        if (strncmp(&kode_sumber[i], "pergi!#", 7) == 0) {
            OtimToken t = {TOKEN_OTIM_FOOTER, NULL, NULL, NULL, NULL, baris, kolom};
            tambah_otim_token(&tokens, t);
            i += 7; kolom += 7;
            continue;
        }

        // 5. Tangkap TAG (<...>)
        if (kode_sumber[i] == '<') {
            int awal_tag = ++i; kolom++;
            int is_tutup = 0;
            
            // Cek apakah ini </tag>
            if (kode_sumber[i] == '/') {
                is_tutup = 1;
                i++; awal_tag++; kolom++;
            }

            // Cari batas akhir tag '>'
            while (kode_sumber[i] != '>' && kode_sumber[i] != '\0') { i++; kolom++; }
            
            int panjang_tag = i - awal_tag;
            char* isi_tag_mentah = strndup(&kode_sumber[awal_tag], panjang_tag);
            
            OtimToken t = {0};
            t.baris = baris; t.kolom = kolom;
            
            if (is_tutup) {
                t.jenis = TOKEN_OTIM_TAG_TUTUP;
                t.tag_nama = strdup(isi_tag_mentah); // Contoh: "wadah"
            } else {
                t.jenis = TOKEN_OTIM_TAG_BUKA;
                // PARSING TAG BUKA: <nama: id, atribut>
                char* titik_dua = strchr(isi_tag_mentah, ':');
                if (titik_dua) {
                    *titik_dua = '\0'; // Pisahkan nama tag
                    t.tag_nama = strdup(isi_tag_mentah);
                    
                    char* sisa = titik_dua + 1;
                    while(isspace(*sisa)) sisa++; // bersihkan spasi di depan ID
                    
                    char* koma = strchr(sisa, ',');
                    if (koma) {
                        *koma = '\0'; // Pisahkan ID dan Atribut
                        
                        // 🟢 SUNTIKAN: BERSIHKAN SPASI DI BELAKANG ID!
                        char* akhir_id = sisa + strlen(sisa) - 1;
                        while(akhir_id > sisa && isspace(*akhir_id)) {
                            *akhir_id = '\0';
                            akhir_id--;
                        }
                        t.tag_id = strdup(sisa);
                        
                        // Bersihkan spasi di depan Atribut
                        char* atr = koma + 1;
                        while(isspace(*atr)) atr++;
                        t.atribut = strdup(atr);
                        
                    } else {
                        // 🟢 SUNTIKAN: BERSIHKAN SPASI DI BELAKANG ID (JIKA TANPA ATRIBUT)!
                        char* akhir_id = sisa + strlen(sisa) - 1;
                        while(akhir_id > sisa && isspace(*akhir_id)) {
                            *akhir_id = '\0';
                            akhir_id--;
                        }
                        t.tag_id = strdup(sisa);
                    }
                } else {
                    t.tag_nama = strdup(isi_tag_mentah); // Tag sederhana tanpa ID
                }
            }
            free(isi_tag_mentah);
            tambah_otim_token(&tokens, t);
            
            if (kode_sumber[i] == '>') { i++; kolom++; }
            continue;
        }

        // 6. Tangkap TEKS (Diapit tanda kutip ganda)
        if (kode_sumber[i] == '"') {
            int awal_teks = ++i; kolom++;
            while (kode_sumber[i] != '"' && kode_sumber[i] != '\0') {
                if (kode_sumber[i] == '\\' && kode_sumber[i+1] == '"') { i += 2; kolom += 2; continue; } // Escape character
                if (kode_sumber[i] == '\n') { baris++; kolom = 0; }
                i++; kolom++;
            }
            int panjang = i - awal_teks;
            char* teks = strndup(&kode_sumber[awal_teks], panjang);
            
            OtimToken t = {TOKEN_OTIM_TEKS, NULL, NULL, NULL, teks, baris, kolom};
            tambah_otim_token(&tokens, t);
            
            if (kode_sumber[i] == '"') { i++; kolom++; }
            continue;
        }

        // Jika karakter tidak dikenali, lewati saja (agar tidak kiamat di spasi liar)
        i++; kolom++;
    }

    // Akhiri dengan EOF
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