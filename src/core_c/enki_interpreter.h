#ifndef ENKI_INTERPRETER_H
#define ENKI_INTERPRETER_H

#include "enki_parser.h"
#include <setjmp.h>

// --- FORWARD DECLARATION ---
// Mengabari kompilator bahwa wujud EnkiRAM itu ada dan sah
typedef struct EnkiRAM EnkiRAM;

// --- TIPE DOMAIN MEMORI ---
typedef enum {
    TIPE_TEKS = 0,   // Nilai tunggal biasa (string, angka)
    TIPE_OBJEK = 1,  // Domain bersarang (Objek JSON / Kamus)
    TIPE_ARRAY = 2   // (Untuk Array Dinamis masa depan)
} TipeKavling;

// 1. WUJUD KAVLING MEMORI (DIROMBAK MULTIDIMENSI)
typedef struct {
    char* nama;
    TipeKavling tipe;          // Penanda apakah ini teks atau objek
    char* nilai_teks;          // (Teks Unlimited)
    
    // 🔥 INTI DARI SIHIR BERSARANG 🔥
    EnkiRAM* anak_anak;        // Pointer ke RAM baru (jika tipe == TIPE_OBJEK)
    
    ASTNode* simpul_fungsi;
    int apakah_konstanta;
} KavlingMemori;

// 2. RAM UTAMA SISTEM OPERASI
struct EnkiRAM {
    KavlingMemori* kavling;      // Pointer dinamis untuk variabel
    int kapasitas;               // Batas ruang saat ini
    int jumlah;                  // Jumlah variabel terisi
    int butuh_anu_aktif;         // Bendera file .anu
    
    // --- SARAF HUKUM TABU ---
    int dalam_mode_coba;         
    jmp_buf titik_kembali;       
    char* pesan_error_tabu;      // DIUBAH JADI POINTER (Unlimited!)

    // --- SARAF KONTROL & SIKLUS ---
    int status_pulang;           
    char* nilai_kembalian;       // DIUBAH JADI POINTER (Unlimited!)
    int status_terus;            // lompat ke putaran berikutnya
    int status_henti;            // hancurkan loop sepenuhnya  
};

// 3. DEKLARASI FUNGSI UTAMA
EnkiRAM inisialisasi_ram();
EnkiRAM* ciptakan_ram_mini(); // Fungsi pencipta dimensi anak

// Manajemen Memori
void bebaskan_ram(EnkiRAM* ram);
void simpan_ke_ram(EnkiRAM* ram, const char* nama, const char* nilai);
const char* baca_dari_ram(EnkiRAM* ram, const char* nama);

// Mesin Eksekusi
char* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram);
void eksekusi_node(ASTNode* node, EnkiRAM* ram);
void eksekusi_program(ASTNode* program, EnkiRAM* ram);

// --- SISTEM KEAMANAN & RAHASIA ---
void pemicu_kernel_panic(EnkiRAM* ram, const char* pesan);
void muat_anu(EnkiRAM* ram);

#endif // ENKI_INTERPRETER_H