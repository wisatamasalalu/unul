#ifndef OTIM_LEXER_H
#define OTIM_LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_OTIM_HEADER,      // #!datang
    TOKEN_OTIM_FOOTER,      // pergi!#
    TOKEN_OTIM_TAG_BUKA,    // <wadah: utama> atau <tebal>
    TOKEN_OTIM_TAG_TUTUP,   // </wadah>
    TOKEN_OTIM_TEKS,        // "Teks yang akan tampil di layar"
    TOKEN_OTIM_EOF          // Batas Akhir Dimensi
} OtimTokenJenis;

typedef struct {
    OtimTokenJenis jenis;
    char* tag_nama;         // Contoh: "wadah", "butir", "masukan"
    char* tag_id;           // Contoh: "utama", "sandi_user" (bisa NULL)
    char* atribut;          // Contoh: "tipe='rahasia'" (bisa NULL)
    char* isi_teks;         // Isi dari "..." 
    int baris;
    int kolom;
} OtimToken;

typedef struct {
    OtimToken* data;
    int jumlah;
    int kapasitas;
} OtimTokenArray;

OtimTokenArray otim_lexer(const char* kode_sumber);
void bebaskan_otim_token(OtimTokenArray* array);

#endif