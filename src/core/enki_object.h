#ifndef ENKI_OBJECT_H
#define ENKI_OBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. Identitas Zat Dimensi
typedef enum {
    ENKI_ANGKA,     // Desimal / Integer (1, 3.14)
    ENKI_TEKS,      // String ("Halo")
    ENKI_ARRAY,     // Daftar berindeks biasa ([1, 2, 3]) -> Hanya punya .ko
    ENKI_OBJEK,     // Peta Kunci-Nilai (JSON) -> Punya .ku dan .ko
    ENKI_BLOB,      // Biner/Berkas Mentah
    ENKI_KOSONG     // Void / Null
} TipeEnki;

// 2. Struktur Anatomi Gabungan (Sintesis Hemat Memori & Fitur Ku/Ko)
typedef struct EnkiObject {
    TipeEnki tipe;
    
    // RAM HANYA akan memesan memori untuk SALAH SATU bentuk di bawah ini!
    union {
        double angka;
        char* teks;
        
        // Wujud untuk ENKI_ARRAY
        struct EnkiObject** array_elemen; 
        
        // Wujud untuk ENKI_OBJEK (Sintesis dari kode lama Anda: Mendukung .ku dan .ko)
        struct {
            struct EnkiObject** kunci;
            struct EnkiObject** konten;
        } objek_peta;
        
        // Wujud untuk ENKI_BLOB
        struct {
            unsigned char* data;
            size_t ukuran;
        } blob;
        
    } nilai;
    
    int panjang; // Ukuran (Bisa untuk panjang teks, jumlah array, atau jumlah objek)
} EnkiObject;

// 3. Mantra Penciptaan
EnkiObject* ciptakan_angka(double nilai);
EnkiObject* ciptakan_teks(const char* nilai);
EnkiObject* ciptakan_array(int kapasitas);
EnkiObject* ciptakan_objek_peta(int kapasitas); // Untuk Dictionary/JSON
EnkiObject* ciptakan_blob(const unsigned char* data, size_t ukuran);
EnkiObject* ciptakan_salinan_objek(EnkiObject* sumber);
EnkiObject* ciptakan_kosong();

// ... [Enum dan Struct EnkiObject tetap sama] ...

// 🟢 MODIFIKASI: Sekarang semua fungsi menerima (..., int mode_dinamis)
EnkiObject* ciptakan_angka(double nilai, int mode_dinamis);
EnkiObject* ciptakan_teks(const char* nilai, int mode_dinamis);
EnkiObject* ciptakan_array(int kapasitas, int mode_dinamis);
EnkiObject* ciptakan_objek_peta(int kapasitas, int mode_dinamis); 
EnkiObject* ciptakan_blob(const unsigned char* data, size_t ukuran, int mode_dinamis);
EnkiObject* ciptakan_salinan_objek(EnkiObject* sumber, int mode_dinamis);
EnkiObject* ciptakan_kosong(int mode_dinamis);

void hancurkan_objek(EnkiObject* obj, int mode_dinamis);
void cetak_objek(EnkiObject* obj);

// 4. Mantra Penghancur
void hancurkan_objek(EnkiObject* obj);

// 5. Utilitas
void cetak_objek(EnkiObject* obj);

#endif