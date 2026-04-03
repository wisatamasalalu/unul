#ifndef ENKI_INTERPRETER_H
#define ENKI_INTERPRETER_H

#include "enki_parser.h"
#include <setjmp.h>

// --- FORWARD DECLARATION ---
typedef struct EnkiRAM EnkiRAM;
typedef struct KavlingMemori KavlingMemori;
typedef struct JejakMasaLalu JejakMasaLalu;

// --- TIPE DOMAIN MEMORI ---
typedef enum {
    TIPE_TEKS = 0,   
    TIPE_OBJEK = 1,  
    TIPE_ARRAY = 2   
} TipeKavling;

// 🔥 MESIN WAKTU: Kapsul penyimpan kenangan
struct JejakMasaLalu {
    TipeKavling tipe;
    char* nilai_teks;
    EnkiRAM* anak_anak;
};

// 1. WUJUD KAVLING MEMORI (DIROMBAK)
struct KavlingMemori {
    char* nama;
    TipeKavling tipe;          
    char* nilai_teks;          
    EnkiRAM* anak_anak;        
    ASTNode* simpul_fungsi;
    int apakah_konstanta;
    
    // 🔥 SARAF MESIN WAKTU (RIWAYAT) 🔥
    JejakMasaLalu* riwayat;
    int jumlah_riwayat;
    int kapasitas_riwayat;
};

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