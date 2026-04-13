#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_os.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

// ====================================================================
// 🌍 MENCARI TITIK PUSAT USER (MARKAS/HOME)
// ====================================================================
char* dapatkan_jalur_markas_user() {
    char* jalur = NULL;

#if defined(_WIN32) || defined(_WIN64)
    jalur = getenv("USERPROFILE"); 
    if (jalur == NULL) {
        char* drive = getenv("HOMEDRIVE");
        char* path = getenv("HOMEPATH");
        if (drive && path) {
            jalur = drive; 
        }
    }
#else
    jalur = getenv("HOME"); 
#endif

    if (jalur == NULL) {
        return "."; 
    }

    return jalur;
}

// ====================================================================
// 🦅 MENCETAK LAPORAN RADAR DIMENSI
// ====================================================================
void cetak_info_dimensi() {
    printf("\n");
    printf("==================================================\n");
    printf(" 🌍 RADAR LINTAS DIMENSI UNUL AKTIF \n");
    printf("==================================================\n");
    printf(" [*] Sistem Operasi Terdeteksi : %s\n", OS_DIMENSI);
    printf(" [*] Karakter Pemisah Jalur    : '%c'\n", PEMISAH_JALUR);
    printf(" [*] Markas Utama User (Home)  : %s\n", dapatkan_jalur_markas_user());
    printf("==================================================\n\n");
}

// ====================================================================
// 🦅 SIHIR EKSPANSI JALUR (Menerjemahkan '~')
// ====================================================================
char* ekspansi_jalur(const char* jalur_mentah) {
    if (jalur_mentah == NULL) return NULL;

    // Jika jalur dimulai dengan '~'
    if (jalur_mentah[0] == '~') {
        char* markas = dapatkan_jalur_markas_user();
        
        size_t panjang_baru = strlen(markas) + strlen(jalur_mentah + 1) + 1;
        char* jalur_final = (char*)enki_alokasi(panjang_baru, 1); // 🟢 MENGGUNAKAN ENKI_ALOKASI
        
        strcpy(jalur_final, markas);
        strcat(jalur_final, jalur_mentah + 1); 
        
        #if defined(_WIN32) || defined(_WIN64)
            for (int i = 0; jalur_final[i] != '\0'; i++) {
                if (jalur_final[i] == '/') jalur_final[i] = '\\';
            }
        #endif
        
        return jalur_final;
    }

    // Jika tidak ada '~', kembalikan duplikat jalur aslinya
    return enki_salin_teks(jalur_mentah, 1); // 🟢 MENGGUNAKAN ENKI_SALIN_TEKS
}

char* os_eksekusi_perintah(const char* perintah) {
    char buffer[256];
    char* hasil = (char*)enki_alokasi(1, 1); // 🟢 MENGGUNAKAN ENKI_ALOKASI
    if (hasil) hasil[0] = '\0';
    int ukuran = 1;

#ifdef _WIN32
    FILE* pipe = _popen(perintah, "r");
#else
    FILE* pipe = popen(perintah, "r");
#endif

    if (!pipe) return NULL;

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        ukuran += strlen(buffer);
        // 🟢 MENGGUNAKAN ENKI_REALOKASI (Asumsikan ukuran lama adalah strlen(hasil) + 1, ini aman untuk realokasi LLVM)
        int ukuran_lama = strlen(hasil) + 1;
        hasil = (char*)enki_realokasi(hasil, ukuran_lama, ukuran, 1); 
        strcat(hasil, buffer);
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return hasil;
}