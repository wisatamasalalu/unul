#include "ku.h"
#include <string.h>
#include <stdio.h> 

EnkiObject* ambil_kunci_objek(EnkiObject* obj, int mode_dinamis) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong(mode_dinamis); 

    EnkiObject* hasil_ku;

    if (obj->tipe == ENKI_OBJEK) {
        hasil_ku = ciptakan_array(obj->panjang, mode_dinamis); 
        hasil_ku->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ku->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.objek_peta.kunci[i], mode_dinamis); 
        }
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        hasil_ku = ciptakan_array(obj->panjang, mode_dinamis); 
        hasil_ku->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", i + 1); 
            hasil_ku->nilai.array_elemen[i] = ciptakan_teks(buf, mode_dinamis); 
        }
    }
    else {
        hasil_ku = ciptakan_array(1, mode_dinamis); 
        hasil_ku->panjang = 1;
        hasil_ku->nilai.array_elemen[0] = ciptakan_teks("1", mode_dinamis); 
    }

    return hasil_ku;
}