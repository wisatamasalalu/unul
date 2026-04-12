#include "ku.h"
#include <string.h>
#include <stdio.h> 

EnkiObject* ambil_kunci_objek(EnkiObject* obj) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong(1); // 🟢 SUNTIKAN 1

    EnkiObject* hasil_ku;

    if (obj->tipe == ENKI_OBJEK) {
        hasil_ku = ciptakan_array(obj->panjang, 1); // 🟢 SUNTIKAN 1
        hasil_ku->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ku->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.objek_peta.kunci[i], 1); // 🟢 SUNTIKAN 1
        }
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        hasil_ku = ciptakan_array(obj->panjang, 1); // 🟢 SUNTIKAN 1
        hasil_ku->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", i + 1); 
            hasil_ku->nilai.array_elemen[i] = ciptakan_teks(buf, 1); // 🟢 SUNTIKAN 1
        }
    }
    else {
        hasil_ku = ciptakan_array(1, 1); // 🟢 SUNTIKAN 1
        hasil_ku->panjang = 1;
        hasil_ku->nilai.array_elemen[0] = ciptakan_teks("1", 1); // 🟢 SUNTIKAN 1
    }

    return hasil_ku;
}