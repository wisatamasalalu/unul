#include "ku.h"
#include <string.h>
#include <stdio.h> // Pastikan stdio.h ada untuk snprintf

EnkiObject* ambil_kunci_objek(EnkiObject* obj) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong();

    EnkiObject* hasil_ku;

    if (obj->tipe == ENKI_OBJEK) {
        hasil_ku = ciptakan_array(obj->panjang);
        hasil_ku->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            // 🟢 GUNAKAN DEEP COPY DI SINI
            hasil_ku->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.objek_peta.kunci[i]); 
        }
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        hasil_ku = ciptakan_array(obj->panjang);
        hasil_ku->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", i + 1); 
            // ciptakan_teks sudah membuat salinan baru secara internal, jadi aman
            hasil_ku->nilai.array_elemen[i] = ciptakan_teks(buf);
        }
    }
    else {
        hasil_ku = ciptakan_array(1);
        hasil_ku->panjang = 1;
        hasil_ku->nilai.array_elemen[0] = ciptakan_teks("1");
    }

    return hasil_ku;
}