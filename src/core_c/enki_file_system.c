#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "enki_file_system.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

char* sihir_cari(const char* pola) {
    glob_t hasil_glob;
    int status = glob(pola, 0, NULL, &hasil_glob);
    
    if (status != 0) {
        globfree(&hasil_glob);
        return enki_salin_teks("", 1); // 🟢 MENGGUNAKAN ENKI_SALIN_TEKS (Mode 1)
    }

    // Hitung total panjang string yang dibutuhkan
    size_t total_panjang = 0;
    for (size_t i = 0; i < hasil_glob.gl_pathc; i++) {
        total_panjang += strlen(hasil_glob.gl_pathv[i]) + 1; // +1 untuk pemisah (koma atau spasi)
    }

    // 🟢 MENGGUNAKAN ENKI_ALOKASI (Mode 1)
    char* hasil_akhir = (char*)enki_alokasi(total_panjang + 1, 1);
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
    if (!f) return enki_salin_teks("🚨 ERROR: File tidak bisa dibuka.", 1); // 🟢 MENGGUNAKAN ENKI_SALIN_TEKS
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // 🟢 MENGGUNAKAN ENKI_ALOKASI (Mode 1)
    char* string = (char*)enki_alokasi(fsize + 1, 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

void sihir_tulis_file(const char* path, const char* konten) {
    // Mode "w" akan membuat file baru, atau menimpa jika sudah ada
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s", konten);
        fclose(f);
    } else {
        printf("🚨 SISTEM FILE: Gagal menciptakan takdir pada '%s'\n", path);
    }
}