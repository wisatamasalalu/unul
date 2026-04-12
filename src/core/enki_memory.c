#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_memory.h"

// ==========================================
// 🌊 KOLAM MEMORI STATIS (THE ARENA)
// ==========================================
static char* kolam_statis = NULL;
static size_t penunjuk_kolam = 0; 

void inisialisasi_kolam_memori() {
    if (kolam_statis == NULL) {
        // Pesan 5MB RAM dari OS HANYA SEKALI!
        kolam_statis = (char*)malloc(UKURAN_KOLAM_STATIS);
        if (!kolam_statis) {
            printf("🚨 KIAMAT HARDWARE: RAM Anda tidak sanggup menampung Urantia OS!\n");
            exit(1);
        }
        penunjuk_kolam = 0;
    }
}

// ==========================================
// 🚦 GERBANG TOL ALOKASI MEMORI
// ==========================================
void* enki_alokasi(size_t ukuran, int mode_dinamis) {
    if (ukuran == 0) return NULL;

    if (mode_dinamis == 1) {
        // 🔵 MODE DINAMIS: Gunakan malloc biasa
        return malloc(ukuran);
    } else {
        // 🟢 MODE STATIS: Potong dari Kolam 5MB
        if (kolam_statis == NULL) inisialisasi_kolam_memori();

        if (penunjuk_kolam + ukuran > UKURAN_KOLAM_STATIS) {
            printf("🚨 KIAMAT MEMORI: Kolam Statis 5MB Habis! Gunakan 'untuk array.dinamis'.\n");
            exit(1);
        }

        void* ptr = kolam_statis + penunjuk_kolam;
        penunjuk_kolam += ukuran;
        
        // Bersihkan memori agar tidak ada 'hantu' data lama
        memset(ptr, 0, ukuran);
        return ptr;
    }
}

void* enki_realokasi(void* pointer_lama, size_t ukuran_lama, size_t ukuran_baru, int mode_dinamis) {
    if (ukuran_baru == 0) {
        enki_bebas(pointer_lama, mode_dinamis);
        return NULL;
    }
    if (!pointer_lama) return enki_alokasi(ukuran_baru, mode_dinamis);

    if (mode_dinamis == 1) {
        return realloc(pointer_lama, ukuran_baru);
    } else {
        // Mode Statis: Alokasi blok baru, lupakan yang lama (tanpa fragmen di arena)
        void* pointer_baru = enki_alokasi(ukuran_baru, 0); 
        if (pointer_baru && pointer_lama) {
            size_t ukuran_copy = (ukuran_lama < ukuran_baru) ? ukuran_lama : ukuran_baru;
            memcpy(pointer_baru, pointer_lama, ukuran_copy);
        }
        return pointer_baru;
    }
}

void enki_bebas(void* pointer, int mode_dinamis) {
    if (pointer == NULL) return;

    if (mode_dinamis == 1) {
        // 🔵 Di Mode Dinamis, kembalikan ke OS
        free(pointer);
    } else {
        // 🟢 Di Mode Statis, No-Op (Sangat hemat CPU!)
    }
}