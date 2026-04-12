#ifndef ENKI_MEMORY_H
#define ENKI_MEMORY_H

#include <stddef.h>

// Ukuran Kolam Utama: 5 Megabyte (Arena Statis)
// Cocok untuk aplikasi cepat dan sistem tertanam
#define UKURAN_KOLAM_STATIS (5 * 1024 * 1024)

// 🟢 INISIALISASI (Panggil sekali di main.c)
void inisialisasi_kolam_memori();

// 🟢 FUNGSI ALOKASI UTAMA (Ganti semua malloc dengan ini!)
void* enki_alokasi(size_t ukuran, int mode_dinamis);

// 🟢 FUNGSI REALOKASI (Ganti semua realloc dengan ini!)
void* enki_realokasi(void* pointer_lama, size_t ukuran_lama, size_t ukuran_baru, int mode_dinamis);

// 🟢 FUNGSI BEBAS (Ganti semua free dengan ini!)
void enki_bebas(void* pointer, int mode_dinamis);

#endif