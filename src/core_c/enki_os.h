#ifndef ENKI_OS_H
#define ENKI_OS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ====================================================================
// 🌍 RADAR DIMENSI MUTLAK (CROSS-PLATFORM OS DETECTOR)
// ====================================================================

// 1. KELUARGA WINDOWS
#if defined(_WIN32) || defined(_WIN64)
    #define OS_DIMENSI "Windows"
    #define PEMISAH_JALUR '\\'

// 2. KELUARGA APPLE (Membutuhkan TargetConditionals.h untuk detail)
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define OS_DIMENSI "iOS (iPhone/iPad)"
    #elif TARGET_OS_MAC
        #define OS_DIMENSI "MacOS"
    #else
        #define OS_DIMENSI "Apple_Lainnya"
    #endif
    #define PEMISAH_JALUR '/'

// 3. KELUARGA ANDROID & TERMUX
#elif defined(__ANDROID__)
    #define OS_DIMENSI "Android (Termux/Native)"
    #define PEMISAH_JALUR '/'

// 4. KELUARGA LINUX & RASPBERRY PI
#elif defined(__linux__)
    #define OS_DIMENSI "Linux"
    #define PEMISAH_JALUR '/'

// 5. KELUARGA BSD (FreeBSD, OpenBSD, MacOS Core)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__bsdi__)
    #define OS_DIMENSI "BSD_Sistem"
    #define PEMISAH_JALUR '/'

// 6. KELUARGA UNIX ENTERPRISE (Server Tua nan Tangguh)
#elif defined(__sun) && defined(__SVR4)
    #define OS_DIMENSI "Solaris"
    #define PEMISAH_JALUR '/'
#elif defined(_AIX)
    #define OS_DIMENSI "IBM_AIX"
    #define PEMISAH_JALUR '/'
#elif defined(__hpux)
    #define OS_DIMENSI "HP-UX"
    #define PEMISAH_JALUR '/'

// 7. KELUARGA WEB (WebAssembly / Emscripten)
// Jika suatu saat UNUL dijalankan langsung di dalam Browser!
#elif defined(__EMSCRIPTEN__) || defined(__wasm__)
    #define OS_DIMENSI "WebAssembly (Browser)"
    #define PEMISAH_JALUR '/'

// 8. ALAM TAK DIKENAL
#else
    #define OS_DIMENSI "Alam_Gaib_Tak_Dikenal"
    #define PEMISAH_JALUR '/'
#endif

// ====================================================================
// 🦅 PROTOTIPE FUNGSI INTELIJEN OS
// ====================================================================

// Mengambil koordinat markas utama user (C:\Users\X atau /home/X)
char* dapatkan_jalur_markas_user(); 

// Mencetak laporan dimensi saat ini
void cetak_info_dimensi();  

// Mengekspansi simbol '~' menjadi jalur markas mutlak
char* ekspansi_jalur(const char* jalur_mentah);

// Fungsi Eksekusi Perintah Lintas OS
char* os_eksekusi_perintah(const char* perintah);

// ====================================================================
// 🧬 JEMBATAN KUANTUM (CROSS-PLATFORM THREADING & SYNCHRONIZATION)
// ====================================================================

// 1. Sihir Utas (Thread)
void* os_utas_ciptakan(void* (*fungsi)(void*), void* argumen);
void  os_utas_lepas(void* utas); // Membiarkan utas berjalan mandiri (Detach)

// 2. Sihir Gembok (Mutex / Critical Section)
void* os_gembok_ciptakan(int mode_dinamis);
void  os_gembok_kunci(void* gembok);
void  os_gembok_buka(void* gembok);
void  os_gembok_hancurkan(void* gembok, int mode_dinamis);

// 3. Sihir Sinyal Waktu (Condition Variable)
void* os_sinyal_ciptakan(int mode_dinamis);
void  os_sinyal_tunggu(void* sinyal, void* gembok);
void  os_sinyal_bangunkan(void* sinyal);
void  os_sinyal_hancurkan(void* sinyal, int mode_dinamis);

#endif // ENKI_OS_H