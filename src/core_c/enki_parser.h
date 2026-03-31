#ifndef ENKI_PARSER_H
#define ENKI_PARSER_H

#include "enki_lexer.h"

// =================================================================
// KITAB POHON LOGIKA (AST - Abstract Syntax Tree)
// Mengubah barisan Token menjadi rantai instruksi berstruktur
// =================================================================

// 1. DAFTAR WUJUD LOGIKA (Tipe Node AST)
typedef enum {
    AST_PROGRAM,          // Akar utama (Kumpulan seluruh baris kode)
    AST_DEKLARASI_TAKDIR, // takdir.soft x = ...
    AST_PERINTAH_KETIK,   // ketik(...)
    AST_FUNGSI_DENGAR,    // dengar()
    AST_LITERAL_TEKS,     // "Sebuah Teks"
    AST_IDENTITAS,        // nama_variabel
    AST_TIDAK_DIKENAL,     // Node kosong/error
    AST_OPERASI_MATEMATIKA, // AST Matematika Dasar
    AST_HUKUM_KARMA,      // Blok jika...maka...lain...putus
    AST_KONDISI,          // Kondisi (misal: umur > 10)
    AST_DEKLARASI_DATANG, // datang
    AST_PRAGMA_MEMORI,    // untuk array.dinamis / .statis
    AST_PERINTAH_PERGI    // pergi
} ASTJenis;

// 2. STRUKTUR NODE POHON (Wujud 1 Blok Logika)
// Karena C tidak dinamis, 1 Struct ini harus bisa "menyamar" menjadi berbagai bentuk.
typedef struct ASTNode {
    ASTJenis jenis;
    
    // --- Cabang untuk AST_PROGRAM (Menyimpan daftar perintah) ---
    struct ASTNode** anak_anak; // Array of pointers (Banyak anak)
    int jumlah_anak;
    int kapasitas_anak;

    // --- Cabang untuk Literal / Identitas ---
    char* nilai_teks;           // Menyimpan teks (misal: "Siapa nama anda?")
    char* operator_math; // Buat ngitung

    // --- Cabang untuk Deklarasi (Kiri = Kanan) atau Panggilan Fungsi ---
    struct ASTNode* kiri;       // Contoh: Menunjuk ke node identitas (nama_user)
    struct ASTNode* kanan;      // Contoh: Menunjuk ke nilai (dengar())

    struct ASTNode* syarat;     // Menunjuk ke AST_KONDISI
    struct ASTNode* blok_maka;  // Menunjuk ke blok perintah jika SAH
    struct ASTNode* blok_lain;  // Menunjuk ke blok perintah jika GAGAL (else)
    char* pembanding;           // Teks operator, misal "==" atau ">"

} ASTNode;

// 3. STRUKTUR MESIN PARSER (Pelacak Jejak Token)
typedef struct {
    TokenArray tokens;          // Daftar token dari Lexer
    int kursor;                 // Posisi token yang sedang dibaca saat ini
} Parser;

// 4. FUNGSI-FUNGSI PARSER UTAMA
Parser inisialisasi_parser(TokenArray tokens);
ASTNode* parse_program(Parser* parser);

// Fungsi Wajib untuk membuang Pohon dari RAM saat program selesai
void bebaskan_ast(ASTNode* node);

#endif // ENKI_PARSER_H