#ifndef ENKI_OBJECT_H
#define ENKI_OBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. Identitas Zat Dimensi
typedef enum {
    ENKI_ANGKA,     
    ENKI_TEKS,      
    ENKI_ARRAY,     
    ENKI_OBJEK,     
    ENKI_BLOB,      
    ENKI_KOSONG     
} TipeEnki;

// 2. Struktur Anatomi Gabungan
typedef struct EnkiObject {
    TipeEnki tipe;
    union {
        double angka;
        char* teks;
        struct EnkiObject** array_elemen; 
        struct {
            struct EnkiObject** kunci;
            struct EnkiObject** konten;
        } objek_peta;
        struct {
            unsigned char* data;
            size_t ukuran;
        } blob;
    } nilai;
    int panjang; 
} EnkiObject;

// 3. Mantra Penciptaan (HANYA INI YANG BOLEH ADA!)
EnkiObject* ciptakan_angka(double nilai, int mode_dinamis);
EnkiObject* ciptakan_teks(const char* nilai, int mode_dinamis);
EnkiObject* ciptakan_array(int kapasitas, int mode_dinamis);
EnkiObject* ciptakan_objek_peta(int kapasitas, int mode_dinamis); 
EnkiObject* ciptakan_blob(const unsigned char* data, size_t ukuran, int mode_dinamis);
EnkiObject* ciptakan_salinan_objek(EnkiObject* sumber, int mode_dinamis);
EnkiObject* ciptakan_kosong(int mode_dinamis);

// 4. Mantra Penghancur
void hancurkan_objek(EnkiObject* obj, int mode_dinamis);

// 5. Utilitas
void cetak_objek(EnkiObject* obj);

#endif