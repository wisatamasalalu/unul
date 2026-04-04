#include "ku.h"
#include <string.h>

EnkiObject* ambil_kunci_objek(EnkiObject* obj) {
    if (obj == NULL) return ciptakan_objek(ENKI_KOSONG);

    // Siapkan wadah Array baru untuk menampung daftar kunci
    EnkiObject* hasil_ku = ciptakan_objek(ENKI_ARRAY);

    if (obj->tipe == ENKI_OBJEK) {
        // ⚡ KASUS OBJEK (MAP): Salin daftar kunci yang sudah ada
        hasil_ku->jumlah = obj->jumlah;
        hasil_ku->konten = (EnkiObject**)malloc(sizeof(EnkiObject*) * obj->jumlah);
        
        for (int i = 0; i < obj->jumlah; i++) {
            // Kita ambil kunci dari objek asal dan jadikan konten di hasil_ku
            hasil_ku->konten[i] = obj->kunci[i]; 
        }
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        // ⚡ KASUS ARRAY: Hasilkan indeks otomatis (1, 2, 3...)
        hasil_ku->jumlah = obj->jumlah;
        hasil_ku->konten = (EnkiObject**)malloc(sizeof(EnkiObject*) * obj->jumlah);
        
        for (int i = 0; i < obj->jumlah; i++) {
            // Buat objek teks baru berisi angka indeks (dimulai dari 1 sesuai tradisi UNUL)
            EnkiObject* idx = ciptakan_objek(ENKI_TEKS);
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", i + 1);
            idx->v_teks = strdup(buf);
            
            hasil_ku->konten[i] = idx;
        }
    }

    return hasil_ku;
}