#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_os.h"

// ====================================================================
// 🌍 MENCARI TITIK PUSAT USER (MARKAS/HOME)
// ====================================================================
char* dapatkan_jalur_markas_user() {
    char* jalur = NULL;

    // 1. Logika Dimensi Windows
#if defined(_WIN32) || defined(_WIN64)
    jalur = getenv("USERPROFILE"); // Biasanya C:\Users\NamaUser
    
    // Fallback rahasia jika USERPROFILE gagal
    if (jalur == NULL) {
        char* drive = getenv("HOMEDRIVE");
        char* path = getenv("HOMEPATH");
        // Catatan: Di C murni kita harus gabungkan, tapi untuk sementara 
        // kita kembalikan HOMEDRIVE saja jika darurat.
        if (drive && path) {
            jalur = drive; 
        }
    }

    // 2. Logika Dimensi POSIX (Linux, Mac, Android/Termux, BSD, Solaris)
#else
    // Hampir seluruh OS berbasis Unix menggunakan $HOME
    jalur = getenv("HOME"); 
#endif

    // 3. Fallback Kiamat (Jika OS benar-benar asing)
    if (jalur == NULL) {
        return "."; // Kembalikan folder saat ini agar sistem tidak meledak
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
        
        // Hitung panjang: panjang_markas + panjang_sisa_jalur (tanpa '~') + 1 (null terminator)
        size_t panjang_baru = strlen(markas) + strlen(jalur_mentah + 1) + 1;
        char* jalur_final = (char*)malloc(panjang_baru);
        
        strcpy(jalur_final, markas);
        strcat(jalur_final, jalur_mentah + 1); // Gabungkan sisa teks setelah '~'
        
        // Opsional: Normalisasi garis miring sesuai OS (untuk Windows)
        #if defined(_WIN32) || defined(_WIN64)
            for (int i = 0; jalur_final[i] != '\0'; i++) {
                if (jalur_final[i] == '/') jalur_final[i] = '\\';
            }
        #endif
        
        return jalur_final;
    }

    // Jika tidak ada '~', kembalikan duplikat jalur aslinya
    return strdup(jalur_mentah);
}

char* os_eksekusi_perintah(const char* perintah) {
    char buffer[256];
    char* hasil = malloc(1); 
    if (hasil) hasil[0] = '\0';
    int ukuran = 1;

    // Kompatibilitas Lintas Dimensi (Windows vs POSIX)
#ifdef _WIN32
    FILE* pipe = _popen(perintah, "r");
#else
    FILE* pipe = popen(perintah, "r");
#endif

    if (!pipe) return NULL;

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        ukuran += strlen(buffer);
        hasil = realloc(hasil, ukuran);
        strcat(hasil, buffer);
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return hasil;
}