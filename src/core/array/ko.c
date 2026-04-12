#include "ko.h"
#include <string.h>

EnkiObject* ambil_konten_objek(EnkiObject* obj) {
    if (obj == NULL || obj->tipe == ENKI_KOSONG) return ciptakan_kosong(1); // 🟢 SUNTIKAN 1

    EnkiObject* hasil_ko;

    if (obj->tipe == ENKI_ARRAY) {
        hasil_ko = ciptakan_array(obj->panjang, 1); // 🟢 SUNTIKAN 1
        hasil_ko->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ko->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.array_elemen[i], 1); // 🟢 SUNTIKAN 1
        }
    } 
    else if (obj->tipe == ENKI_OBJEK) {
        hasil_ko = ciptakan_array(obj->panjang, 1); // 🟢 SUNTIKAN 1
        hasil_ko->panjang = obj->panjang;
        for (int i = 0; i < obj->panjang; i++) {
            hasil_ko->nilai.array_elemen[i] = ciptakan_salinan_objek(obj->nilai.objek_peta.konten[i], 1); // 🟢 SUNTIKAN 1
        }
    } 
    else {
        hasil_ko = ciptakan_array(1, 1); // 🟢 SUNTIKAN 1
        hasil_ko->panjang = 1;
        hasil_ko->nilai.array_elemen[0] = ciptakan_salinan_objek(obj, 1); // 🟢 SUNTIKAN 1
    }

    return hasil_ko;
}