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
    AST_OPERASI_LOGIKA,   // Gabungan kondisi (A dan B)
    AST_DEKLARASI_DATANG, // datang
    AST_PRAGMA_MEMORI,    // untuk array.dinamis / .statis
    AST_PERINTAH_PERGI,    // pergi
    AST_PERINTAH_TERUS,    // terus sama dengan continue di bahasa pemrogaman umum
    AST_PERINTAH_HENTI,    // henti sama dengan break
    AST_HUKUM_SIKLUS,     // effort X kali maka...
    AST_STRUKTUR_ARRAY,    // buat array.dinamis
    AST_AKSES_ARRAY,       // Akses data array
    AST_PERINTAH_SOWAN ,    // Seperti import atau include, kata kunci pemanggilan file
    AST_COBA_TABU,           // Ranting hukum tabu
    AST_DEKLARASI_FUNGSI, // Untuk: ciptakan fungsi nama(x) maka ... putus
    AST_PANGGILAN_FUNGSI, // Untuk: nama(10)
    AST_PULANG,            // Untuk: pulang x
    AST_OPERASI_BUKAN,     // Menyimpan logika pembalik // Seperti operasi NOT pada bahasa pemrograman umum
    AST_PERINTAH_PASRAH,    // Menyimpan perintah eksekusi free()
    AST_PERINTAH_JEDA,      // Fungsi jeda waktu seperti sleep X di bash
    AST_STRUKTUR_OBJEK,        // Menampung kurung kurawal {}
    AST_PASANGAN_KUNCI_NILAI,  // Menampung format "kunci": nilai
    AST_AKSES_DOMAIN,          // Menampung akses titik (misal: dewa.nama)
    AST_PERINTAH_BALIKAN,       // Membalikan keadaan seperti sebelumnya (mesin waktu)
    AST_FUNGSI_PANAH,           // Fungsi Anonim (x) => ...
    AST_COCOKKAN,              // Blok percocokan pola
    AST_KASUS,                  // Ranting kasus individu
    AST_TERNARI                // Percabangan sebaris (jika A maka B lain C)

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

    // buat error handling
    int baris;       
    int kolom;       
    char* nama_file; 

    // --- Cabang untuk Deklarasi (Kiri = Kanan) atau Panggilan Fungsi ---
    struct ASTNode* kiri;       // Contoh: Menunjuk ke node identitas (nama_user)
    struct ASTNode* kanan;      // Contoh: Menunjuk ke nilai (dengar())

    struct ASTNode* syarat;     // Menunjuk ke AST_KONDISI
    struct ASTNode* blok_maka;  // Menunjuk ke blok perintah jika SAH
    struct ASTNode* blok_lain;  // Menunjuk ke blok perintah jika GAGAL (else)

    struct ASTNode* batas_loop;  // Menunjuk ke node jumlah perulangan
    struct ASTNode* blok_siklus; // Menunjuk ke blok perintah yang diulang
    struct ASTNode* indeks_array; // Buat memetakan index memori
    struct ASTNode* blok_tebus;   // berfungsi seperti finnaly

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
// Fungsi bantuan untuk menambah anak cabang
void tambah_anak(ASTNode* induk, ASTNode* anak);

// --- SUNTIKAN BARU UNTUK SIHIR EVALUASI ---
ASTNode* parse_ekspresi(Parser* parser);
// ------------------------------------------

// Fung// <--- Sowan ke Jantung Eksekusisi Wajib untuk membuang Pohon dari RAM saat program selesai
void bebaskan_ast(ASTNode* node);

#endif // ENKI_PARSER_H