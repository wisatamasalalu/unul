#include "ko.h"
#include <string.h>

EnkiObject* ambil_konten_objek(EnkiObject* obj) {
    if (obj == NULL) return ciptakan_objek(ENKI_KOSONG);

    // Siapkan wadah Array baru untuk menampung daftar konten
    EnkiObject* hasil_ko = ciptakan_objek(ENKI_ARRAY);

    // Berlaku untuk tipe Kolektif (ARRAY dan OBJEK)
    if (obj->tipe == ENKI_ARRAY || obj->tipe == ENKI_OBJEK) {
        hasil_ko->jumlah = obj->jumlah;
        
        // Alokasikan ruang untuk menampung pointer konten
        hasil_ko->konten = (EnkiObject**)malloc(sizeof(EnkiObject*) * obj->jumlah);
        
        for (int i = 0; i < obj->jumlah; i++) {
            // Kita arahkan konten hasil_ko ke konten objek asli
            // (Shallow copy of pointers for efficiency)
            hasil_ko->konten[i] = obj->konten[i];
        }
    } 
    // Jika user iseng memanggil .ko pada Teks atau Angka
    else {
        hasil_ko->jumlah = 1;
        hasil_ko->konten = (EnkiObject**)malloc(sizeof(EnkiObject*));
        hasil_ko->konten[0] = obj; // Isinya ya dirinya sendiri
    }

    return hasil_ko;
}