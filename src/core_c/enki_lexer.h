#ifndef ENKI_LEXER_H
#define ENKI_LEXER_H

// =================================================================
// KITAB HUKUM ENLIL: DEFINISI TOKEN (LEXER)
// Diadaptasi dari Blueprint: src/legacy/enki_lexer.py
// =================================================================

// 1. DAFTAR TAKDIR (Enum Jenis Token)
typedef enum {
    TOKEN_HEADER,        // datang
    TOKEN_PRAGMA,        // untuk array.dinamis, untuk array.statis, butuh .anu
    TOKEN_TAKDIR,        // takdir.soft, takdir.hard
    TOKEN_FUNGSI,        // ketik, dengar, tunggu, jeda, (dan juga kata 'fungsi')
    TOKEN_KARMA,         // jika, maka, lain, putus
    TOKEN_COCOKKAN,      // cocokkan
    TOKEN_KASUS,         // kasus
    TOKEN_SIKLUS,        // effort, kali
    
    TOKEN_PENCIPTAAN,    // (Cadangan)
    TOKEN_CIPTAKAN,      // ciptakan
    
    TOKEN_KONTROL,       // henti, pergi, balikan, terus
    TOKEN_PULANG,        // pulang
    
    TOKEN_SOWAN,         // sowan
    TOKEN_UTAS,          // utas, gaib
    
    // Hukum Tabu (Error Handling)
    TOKEN_COBA,          // coba
    TOKEN_TABU,          // tabu
    TOKEN_MELANGGAR,     // melanggar
    TOKEN_TEBUS,         // tebus
    TOKEN_BUKAN,         // Untuk sihir negasi logika
    TOKEN_PASRAH,        // Untuk pelepasan memori manual

    // Operator, Logika, & Pembanding
    TOKEN_PEMBANDING,    // ==, !=, >, <, >=, <=, berisi
    TOKEN_LOGIKA,        // dan, atau, bukan, &&, ||, !
    TOKEN_OPERATOR,      // +, -, *, /, %, ^
    TOKEN_ASSIGN,        // =
    TOKEN_PANAH,         // =>
    TOKEN_PIPA,          // |>

    // Tipe Data Dasar & Identitas
    TOKEN_TEKS,          // "String teks" atau 'String'
    TOKEN_ANGKA,         // 123, 3.14, 0xFF, 0b10, 0o10
    TOKEN_IDENTITAS,     // nama_variabel, fungsi_kustom
    
    // Tanda Baca (Kavling & Domain)
    TOKEN_TITIK,         // .
    TOKEN_TITIK_DUA,     // :
    TOKEN_KOMA,          // ,
    TOKEN_KURUNG_B,      // (
    TOKEN_KURUNG_T,      // )
    TOKEN_KURUNG_S_B,    // [
    TOKEN_KURUNG_S_T,    // ]
    TOKEN_KURUNG_K_B,    // {
    TOKEN_KURUNG_K_T,    // }

    TOKEN_EOF,           // End of File (Penanda akhir kitab)
    TOKEN_MISMATCH       // Karakter asing (Trigger Kernel Panic)
} TokenJenis;

// 2. WUJUD TOKEN (Struct)
typedef struct {
    TokenJenis jenis;    // Tipe token
    char* isi;           // Teks asli dari token
    int baris;           // 🟢 Dimensi Waktu (Baris)
    int kolom;           // 🟢 Dimensi Ruang (Kolom)
    char* nama_file;     // 🟢 Identitas Alam (Nama File)
} Token;

// 3. DAFTAR TOKEN (Array Dinamis di C)
typedef struct {
    Token* data;         
    int jumlah;          
    int kapasitas;       
} TokenArray;

// 4. DEKLARASI FUNGSI PEMINDAI (Lexer)
// 🟢 UBAH: Sekarang meminta nama_file_sumber untuk pelacakan error presisi!
TokenArray enki_lexer(const char* kode_sumber, const char* nama_file_sumber);

// Fungsi Wajib di C: Mengembalikan memori ke sistem operasi agar tidak bocor
void bebaskan_token_array(TokenArray* array);

#endif // ENKI_LEXER_H