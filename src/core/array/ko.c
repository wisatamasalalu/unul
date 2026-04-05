#include "ko.h"
#include <string.h>

EnkiObject* ambil_konten_objek(EnkiObject* obj) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong();

    // Siapkan wadah Array baru untuk menampung daftar konten
    EnkiObject* hasil_ko;

    if (obj->tipe == ENKI_ARRAY) {
        hasil_ko = ciptakan_array(obj->panjang);
        hasil_ko->panjang = obj->panjang;
        
        for (int i = 0; i < obj->panjang; i++) {
            // Salin pointer elemen dari array asli
            hasil_ko->nilai.array_elemen[i] = obj->nilai.array_elemen[i];
        }
    } 
    else if (obj->tipe == ENKI_OBJEK) {
        hasil_ko = ciptakan_array(obj->panjang);
        hasil_ko->panjang = obj->panjang;
        
        for (int i = 0; i < obj->panjang; i++) {
            // Salin pointer konten dari objek peta JSON asli
            hasil_ko->nilai.array_elemen[i] = obj->nilai.objek_peta.konten[i];
        }
    } 
    // Jika user iseng memanggil .ko pada Teks atau Angka
    else {
        hasil_ko = ciptakan_array(1);
        hasil_ko->panjang = 1;
        hasil_ko->nilai.array_elemen[0] = obj; // Isinya ya dirinya sendiri
    }

    return hasil_ko;
}