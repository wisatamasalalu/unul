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
    TOKEN_SIKLUS,        // effort, kali
    
    TOKEN_PENCIPTAAN,    // (Cadangan)
    TOKEN_CIPTAKAN,      // ciptakan <-- BARU DITAMBAHKAN DI SINI
    
    TOKEN_KONTROL,       // henti, pergi, balikan
    TOKEN_PULANG,        // pulang   <-- BARU DITAMBAHKAN DI SINI
    
    TOKEN_SOWAN,         // sowan
    
    // Hukum Tabu (Error Handling)
    TOKEN_COBA,          // coba
    TOKEN_TABU,          // tabu
    TOKEN_MELANGGAR,     // melanggar
    TOKEN_TEBUS,         // tebus
    TOKEN_PASRAH,        // pasrah

    // Operator, Logika, & Pembanding
    TOKEN_PEMBANDING,    // ==, !=, >, <, >=, <=, berisi
    TOKEN_LOGIKA,        // dan, atau, bukan, &&, ||, !
    TOKEN_OPERATOR,      // +, -, *, /, %, ^
    TOKEN_ASSIGN,        // =

    // Tipe Data Dasar & Identitas
    TOKEN_TEKS,          // "String teks" atau 'String'
    TOKEN_ANGKA,         // 123, 3.14, 0xFF, 0b10, 0o10
    TOKEN_IDENTITAS,     // nama_variabel, fungsi_kustom
    
    // Tanda Baca (Kavling & Domain)
    TOKEN_TITIK,         // .
    TOKEN_TITIK_DUA,     // :
    TOKEN_KOMA,          // ,  <-- INI SUDAH ADA DARI AWAL
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
// C tidak dinamis, kita harus mengikat "Jenis" dan "Isi" teksnya secara manual.
typedef struct {
    TokenJenis jenis;    // Tipe token (dari enum di atas)
    char* isi;           // Teks asli dari token (Minta alokasi malloc nanti)
    int baris;           // Jejak baris (Untuk pesan error Hukum Tabu)
    int kolom;           // Jejak letak karakter
} Token;

// 3. DAFTAR TOKEN (Array Dinamis di C)
// Karena C kaku, kita butuh "Kavling" yang bisa melebar jika token bertambah.
typedef struct {
    Token* data;         // Alamat memori yang menunjuk ke deretan Token
    int jumlah;          // Jumlah token saat ini
    int kapasitas;       // Kapasitas maksimal array memori saat ini
} TokenArray;

// 4. DEKLARASI FUNGSI PEMINDAI (Lexer)
// Mesin Pemindai Karakter: Membaca teks sumber dan mengubahnya jadi Barisan Token
TokenArray enki_lexer(const char* kode_sumber);

// Fungsi Wajib di C: Mengembalikan memori ke sistem operasi agar tidak bocor (Memory Leak)
void bebaskan_token_array(TokenArray* array);

#endif // ENKI_LEXER_H