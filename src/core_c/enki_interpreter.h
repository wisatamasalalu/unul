#ifndef ENKI_INTERPRETER_H
#define ENKI_INTERPRETER_H

#include "enki_parser.h"
#include <setjmp.h>

// 1. WUJUD KAVLING MEMORI
typedef struct {
    char* nama;
    char* nilai_teks;
} KavlingMemori;

// 2. RAM UTAMA SISTEM OPERASI
typedef struct {
    KavlingMemori* kavling;      // Pointer dinamis untuk variabel
    int kapasitas;               // Batas ruang saat ini
    int jumlah;                  // Jumlah variabel terisi
    int butuh_anu_aktif;         // Bendera file .anu
    
    // --- SARAF HUKUM TABU ---
    int dalam_mode_coba;         // 1 jika sedang di dalam blok coba
    jmp_buf titik_kembali;       // Kordinat mesin waktu untuk longjmp
    char pesan_error_tabu[1024]; // Menyimpan pesan error agar tidak hilang
} EnkiRAM;

// 3. DEKLARASI FUNGSI UTAMA
EnkiRAM inisialisasi_ram();
void bebaskan_ram(EnkiRAM* ram);

// Manajemen Memori
void simpan_ke_ram(EnkiRAM* ram, const char* nama, const char* nilai);
const char* baca_dari_ram(EnkiRAM* ram, const char* nama);

// Mesin Eksekusi
char* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram);
void eksekusi_node(ASTNode* node, EnkiRAM* ram);
void eksekusi_program(ASTNode* program, EnkiRAM* ram);

// --- SISTEM KEAMANAN & RAHASIA ---

// Fungsi Panic dengan Diary (HANYA SATU VERSI!)
void pemicu_kernel_panic(EnkiRAM* ram, const char* pesan);

// Pemuat rahasia
void muat_anu(EnkiRAM* ram);

#endif // ENKI_INTERPRETER_H