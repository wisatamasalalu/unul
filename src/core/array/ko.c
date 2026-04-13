#include "ko.h"
#include <string.h>

EnkiObject* ambil_konten_objek(EnkiObject* obj, int mode_dinamis) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong(mode_dinamis); 

    EnkiObject* hasil_ko;

    if (obj->tipe == ENKI_ARRAY) {
        hasil_ko = ciptakan_array(obj->panjang, mode_dinamis); 
        hasil_ko->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ko->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.array_elemen[i], mode_dinamis); 
        }
    } 
    else if (obj->tipe == ENKI_OBJEK) {
        hasil_ko = ciptakan_array(obj->panjang, mode_dinamis); 
        hasil_ko->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ko->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.objek_peta.konten[i], mode_dinamis); 
        }
    } 
    else {
        hasil_ko = ciptakan_array(1, mode_dinamis); 
        hasil_ko->panjang = 1;
        hasil_ko->nilai.array_elemen[0] = ciptakan_salinan_objek(obj, mode_dinamis); 
    }

    return hasil_ko;
}