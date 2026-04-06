#ifndef SNUL_LEXER_H
#define SNUL_LEXER_H

typedef enum {
    TOKEN_SNUL_SELECTOR,    // Contoh: "@wadah.utama" atau "@judul"
    TOKEN_SNUL_LBRACE,      // {
    TOKEN_SNUL_RBRACE,      // }
    TOKEN_SNUL_PROPERTI,    // Contoh: "warna_latar" atau "ruang_dalam"
    TOKEN_SNUL_NILAI,       // Contoh: "#1A1A1A" atau "20px otomatis"
    TOKEN_SNUL_EOF          // Batas Akhir
} SnulTokenJenis;

typedef struct {
    SnulTokenJenis jenis;
    char* teks;             // Teks asli dari kode sumber
    int baris;
    int kolom;
} SnulToken;

typedef struct {
    SnulToken* data;
    int jumlah;
    int kapasitas;
} SnulTokenArray;

SnulTokenArray snul_lexer(const char* kode_sumber);
void bebaskan_snul_token(SnulTokenArray* array);

#endif