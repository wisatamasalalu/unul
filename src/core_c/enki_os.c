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

// ====================================================================
// 🧬 IMPLEMENTASI JEMBATAN KUANTUM (THREADING WRAPPER)
// ====================================================================

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

    // Di bagian Windows (#if defined(_WIN32) ...):
    // Bungkus khusus Windows karena CreateThread butuh format fungsi berbeda
    typedef struct { void* (*fungsi)(void*); void* arg; } OsUtasBungkus;
    DWORD WINAPI win32_utas_pelari(LPVOID arg) {
        OsUtasBungkus* b = (OsUtasBungkus*)arg;
        b->fungsi(b->arg); // Eksekusi fungsi
        enki_bebas(b, 1);
        return 0;
    }

    void* os_utas_ciptakan(void* (*fungsi)(void*), void* argumen) {
        OsUtasBungkus* b = (OsUtasBungkus*)enki_alokasi(sizeof(OsUtasBungkus), 1);
        b->fungsi = fungsi; b->arg = argumen;
        return (void*)CreateThread(NULL, 0, win32_utas_pelari, b, 0, NULL);
    }
    
    void os_utas_lepas(void* utas) {
        CloseHandle((HANDLE)utas);
    }

    void* os_gembok_ciptakan(int mode_dinamis) {
        CRITICAL_SECTION* cs = (CRITICAL_SECTION*)enki_alokasi(sizeof(CRITICAL_SECTION), mode_dinamis);
        InitializeCriticalSection(cs);
        return (void*)cs;
    }
    void os_gembok_kunci(void* gembok) { EnterCriticalSection((CRITICAL_SECTION*)gembok); }
    void os_gembok_buka(void* gembok) { LeaveCriticalSection((CRITICAL_SECTION*)gembok); }
    void os_gembok_hancurkan(void* gembok, int mode_dinamis) { 
        DeleteCriticalSection((CRITICAL_SECTION*)gembok); 
        enki_bebas(gembok, mode_dinamis); 
    }

    void* os_sinyal_ciptakan(int mode_dinamis) {
        CONDITION_VARIABLE* cv = (CONDITION_VARIABLE*)enki_alokasi(sizeof(CONDITION_VARIABLE), mode_dinamis);
        InitializeConditionVariable(cv);
        return (void*)cv;
    }
    void os_sinyal_tunggu(void* sinyal, void* gembok) { 
        SleepConditionVariableCS((CONDITION_VARIABLE*)sinyal, (CRITICAL_SECTION*)gembok, INFINITE); 
    }
    void os_sinyal_bangunkan(void* sinyal) { WakeConditionVariable((CONDITION_VARIABLE*)sinyal); }
    void os_sinyal_hancurkan(void* sinyal, int mode_dinamis) { enki_bebas(sinyal, mode_dinamis); }

#else
    #include <pthread.h>

    // Di bagian Linux/Mac (#else):
    void* os_utas_ciptakan(void* (*fungsi)(void*), void* argumen) {
        pthread_t* id = (pthread_t*)enki_alokasi(sizeof(pthread_t), 1);
        // Tidak perlu casting paksa lagi! GCC akan senang!
        pthread_create(id, NULL, fungsi, argumen);
        return (void*)id;
    }
    
    void os_utas_lepas(void* utas) {
        pthread_detach(*(pthread_t*)utas);
        enki_bebas(utas, 1);
    }

    void* os_gembok_ciptakan(int mode_dinamis) {
        pthread_mutex_t* mtx = (pthread_mutex_t*)enki_alokasi(sizeof(pthread_mutex_t), mode_dinamis);
        pthread_mutex_init(mtx, NULL);
        return (void*)mtx;
    }
    void os_gembok_kunci(void* gembok) { pthread_mutex_lock((pthread_mutex_t*)gembok); }
    void os_gembok_buka(void* gembok) { pthread_mutex_unlock((pthread_mutex_t*)gembok); }
    void os_gembok_hancurkan(void* gembok, int mode_dinamis) { 
        pthread_mutex_destroy((pthread_mutex_t*)gembok); 
        enki_bebas(gembok, mode_dinamis); 
    }

    void* os_sinyal_ciptakan(int mode_dinamis) {
        pthread_cond_t* cnd = (pthread_cond_t*)enki_alokasi(sizeof(pthread_cond_t), mode_dinamis);
        pthread_cond_init(cnd, NULL);
        return (void*)cnd;
    }
    void os_sinyal_tunggu(void* sinyal, void* gembok) { 
        pthread_cond_wait((pthread_cond_t*)sinyal, (pthread_mutex_t*)gembok); 
    }
    void os_sinyal_bangunkan(void* sinyal) { pthread_cond_signal((pthread_cond_t*)sinyal); }
    void os_sinyal_hancurkan(void* sinyal, int mode_dinamis) { 
        pthread_cond_destroy((pthread_cond_t*)sinyal); 
        enki_bebas(sinyal, mode_dinamis); 
    }
#endif