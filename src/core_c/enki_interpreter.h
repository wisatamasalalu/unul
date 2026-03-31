#ifndef ENKI_INTERPRETER_H
#define ENKI_INTERPRETER_H

#include "enki_parser.h"

// =================================================================
// KITAB EKSEKUSI (INTERPRETER C)
// Mesin yang menjalankan Pohon Logika dan mengatur RAM Alam Semesta
// =================================================================

// 1. WUJUD KAVLING MEMORI
// Menyimpan variabel (takdir) yang diciptakan user.
typedef struct {
    char* nama;         // Nama identitas (misal: "nama_user")
    char* nilai_teks;   // Isi dari memori (misal: "Nabhan")
} KavlingMemori;

// 2. RAM UTAMA SISTEM OPERASI (Memori Global)
// Pengganti `self.memory = {}` dari versi Python
typedef struct {
    KavlingMemori* kavling; // Deretan kavling memori dinamis
    int jumlah;
    int kapasitas;
} EnkiRAM;

// 3. DEKLARASI FUNGSI-FUNGSI EKSEKUSI UTAMA
// Menyalakan dan mematikan RAM
EnkiRAM inisialisasi_ram();
void bebaskan_ram(EnkiRAM* ram);

// Mengambil nilai dari variabel atau teks literal
char* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram);

// Pengeksekusi setiap cabang dari Pohon Logika
void eksekusi_node(ASTNode* node, EnkiRAM* ram);

// Pintu masuk eksekusi seluruh program
void eksekusi_program(ASTNode* program, EnkiRAM* ram);

#endif // ENKI_INTERPRETER_H