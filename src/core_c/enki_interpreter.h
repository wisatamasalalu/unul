#ifndef ENKI_INTERPRETER_H
#define ENKI_INTERPRETER_H

#include "enki_parser.h"
#include <setjmp.h>
#include "../core/enki_object.h" // 🟢 SUNTIKAN JANTUNG BARU!

// --- FORWARD DECLARATION ---
typedef struct EnkiRAM EnkiRAM;
typedef struct KavlingMemori KavlingMemori;
typedef struct JejakMasaLalu JejakMasaLalu;

// --- TIPE DOMAIN MEMORI ---
// Karena tipe data (Teks, Angka, Array) sekarang diurus oleh EnkiObject,
// TipeKavling kini fokus pada SIFAT Variabel (Soft vs Hard/Konstanta).
typedef enum {
    TIPE_VARIABEL_SOFT = 0,   
    TIPE_VARIABEL_HARD = 1  
} TipeKavling;

// 🔥 MESIN WAKTU: Kapsul penyimpan kenangan
struct JejakMasaLalu {
    TipeKavling tipe;
    EnkiObject* objek; // 🟢 GANTI: char* nilai_teks -> EnkiObject* objek
    EnkiRAM* anak_anak;
};

// 1. WUJUD KAVLING MEMORI (DIROMBAK)
struct KavlingMemori {
    char* nama;
    TipeKavling tipe;          
    EnkiObject* objek; // 🟢 GANTI: char* nilai_teks -> EnkiObject* objek          
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
    
    struct EnkiRAM* induk;       // 🟢 DITAMBAHKAN: Untuk melacak Scope (Global/Lokal)
    
    // --- SARAF HUKUM TABU ---
    int dalam_mode_coba;         
    jmp_buf titik_kembali;       
    char* pesan_error_tabu;      

    // --- SARAF KONTROL & SIKLUS ---
    int status_pulang;           
    EnkiObject* nilai_kembalian; // 🟢 GANTI: Agar fungsi bisa me-return Array/Objek JSON!
    int status_terus;            // lompat ke putaran berikutnya
    int status_henti;            // hancurkan loop sepenuhnya  
};

// 3. DEKLARASI FUNGSI UTAMA
EnkiRAM inisialisasi_ram();
EnkiRAM* ciptakan_ram_mini(EnkiRAM* induk); // 🟢 Ditambah parameter induk

// Fungsi bantuan Utas & Jadwal
EnkiRAM* salin_ram_untuk_utas(EnkiRAM* sumber);

// Manajemen Memori
void bebaskan_ram(EnkiRAM* ram);
void simpan_ke_ram(EnkiRAM* ram, const char* nama, EnkiObject* nilai_objek); // 🟢 UBAH PARAMETER
EnkiObject* baca_dari_ram(EnkiRAM* ram, const char* nama);                   // 🟢 UBAH RETURN TYPE

// Mesin Eksekusi
EnkiObject* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram);                  // 🟢 UBAH RETURN TYPE
void eksekusi_node(ASTNode* node, EnkiRAM* ram);
void eksekusi_program(ASTNode* program, EnkiRAM* ram);

// --- SISTEM KEAMANAN & RAHASIA ---
void pemicu_kernel_panic(EnkiRAM* ram, const char* pesan);
void pemicu_kiamat_presisi(ASTNode* node, EnkiRAM* ram, const char* judul, const char* pesan);
void muat_anu(EnkiRAM* ram);

#endif // ENKI_INTERPRETER_H