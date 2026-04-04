#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "enki_file_system.h"

char* sihir_cari(const char* pola) {
    glob_t hasil_glob;
    int status = glob(pola, 0, NULL, &hasil_glob);
    
    if (status != 0) {
        globfree(&hasil_glob);
        return strdup(""); // Kembalikan kosong jika tidak ada yang cocok
    }

    // Hitung total panjang string yang dibutuhkan
    size_t total_panjang = 0;
    for (size_t i = 0; i < hasil_glob.gl_pathc; i++) {
        total_panjang += strlen(hasil_glob.gl_pathv[i]) + 1; // +1 untuk pemisah (koma atau spasi)
    }

    char* hasil_akhir = malloc(total_panjang + 1);
    hasil_akhir[0] = '\0';

    for (size_t i = 0; i < hasil_glob.gl_pathc; i++) {
        strcat(hasil_akhir, hasil_glob.gl_pathv[i]);
        if (i < hasil_glob.gl_pathc - 1) {
            strcat(hasil_akhir, ","); // Gunakan koma sebagai pemisah antar file
        }
    }

    globfree(&hasil_glob);
    return hasil_akhir;
}

char* sihir_baca_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return strdup("🚨 ERROR: File tidak bisa dibuka.");
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}