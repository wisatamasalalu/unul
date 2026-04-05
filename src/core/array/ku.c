#include "ku.h"
#include <string.h>

EnkiObject* ambil_kunci_objek(EnkiObject* obj) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong();

    // Siapkan wadah Array baru untuk menampung daftar kunci
    EnkiObject* hasil_ku;

    if (obj->tipe == ENKI_OBJEK) {
        // ⚡ KASUS OBJEK (MAP): Salin daftar kunci yang sudah ada
        hasil_ku = ciptakan_array(obj->panjang);
        hasil_ku->panjang = obj->panjang;
        
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ku->nilai.array_elemen[i] = obj->nilai.objek_peta.kunci[i]; 
        }
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        // ⚡ KASUS ARRAY: Hasilkan indeks otomatis (1, 2, 3...)
        hasil_ku = ciptakan_array(obj->panjang);
        hasil_ku->panjang = obj->panjang;
        
        for (int i = 0; i < obj->panjang; i++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", i + 1); // Tradisi UNUL mulai dari 1
            EnkiObject* idx = ciptakan_teks(buf);
            
            hasil_ku->nilai.array_elemen[i] = idx;
        }
    }
    // Jika iseng dipanggil di teks/angka, kembalikan indeks 1
    else {
        hasil_ku = ciptakan_array(1);
        hasil_ku->panjang = 1;
        hasil_ku->nilai.array_elemen[0] = ciptakan_teks("1");
    }

    return hasil_ku;
}