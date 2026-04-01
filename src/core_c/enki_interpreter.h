#ifndef ENKI_INTERPRETER_H
#define ENKI_INTERPRETER_H

#include "enki_parser.h"
#include <setjmp.h>

// 1. WUJUD KAVLING MEMORI
typedef struct {
    char* nama;
    char* nilai_teks;
    ASTNode* simpul_fungsi;
} KavlingMemori;

// 2. RAM UTAMA SISTEM OPERASI
typedef struct {
    KavlingMemori* kavling;      // Pointer dinamis untuk variabel
    int kapasitas;               // Batas ruang saat ini
    int jumlah;                  // Jumlah variabel terisi
    int butuh_anu_aktif;         // Bendera file .anu
    
    // --- SARAF HUKUM TABU ---
    int dalam_mode_coba;         
    jmp_buf titik_kembali;       
    char pesan_error_tabu[1024]; 

    // --- SUNTIKAN BARU: SARAF PULANG (RETURN) ---
    int status_pulang;           
    char nilai_kembalian[256];   
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