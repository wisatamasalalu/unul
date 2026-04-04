#ifndef ENKI_OBJECT_H
#define ENKI_OBJECT_H

#include <stdio.h>
#include <stdlib.h>

// --- 1. ENUMERASI TIPE (Identitas Zat) ---
typedef enum {
    ENKI_TEKS,      // String biasa
    ENKI_ANGKA,     // Double / Desimal
    ENKI_ARRAY,     // Daftar berindeks (1, 2, 3...)
    ENKI_OBJEK,     // Peta Kunci-Nilai (Map/JSON)
    ENKI_BLOB,      // Data Biner Mentah (Untuk .unuls atau File)
    ENKI_KOSONG     // Status hampa (NULL/VOID)
} TipeEnki;

// --- 2. STRUKTUR UTAMA (EnkiObject) ---
typedef struct EnkiObject {
    TipeEnki tipe;
    
    // Data Tunggal (Untuk Teks/Angka)
    char* v_teks;
    double v_angka;

    // Data Kolektif (Inilah jantung .ku dan .ko)
    // Kita menggunakan pointer ke pointer agar bisa menampung list of objects
    struct EnkiObject** kunci;   // Wadah untuk .ku
    struct EnkiObject** konten;  // Wadah untuk .ko
    
    int jumlah;                  // Wadah untuk .panjang
    size_t ukuran_biner;         // Khusus untuk ENKI_BLOB
} EnkiObject;

// --- 3. PROTOTIPE FUNGSI DASAR ---
EnkiObject* ciptakan_objek(TipeEnki tipe);
void hancurkan_objek(EnkiObject* obj);

#endif