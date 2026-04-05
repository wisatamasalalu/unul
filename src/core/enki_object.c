#include "enki_object.h"

// ==========================================
// 🏭 PABRIK PENCIPTAAN (CONSTRUCTORS)
// ==========================================

EnkiObject* ciptakan_angka(double nilai) {
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    obj->tipe = ENKI_ANGKA;
    obj->nilai.angka = nilai;
    obj->panjang = 0;
    return obj;
}

EnkiObject* ciptakan_teks(const char* nilai) {
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    obj->tipe = ENKI_TEKS;
    if (nilai) {
        obj->nilai.teks = strdup(nilai);
        obj->panjang = strlen(nilai);
    } else {
        obj->nilai.teks = strdup("");
        obj->panjang = 0;
    }
    return obj;
}

EnkiObject* ciptakan_array(int kapasitas) {
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    obj->tipe = ENKI_ARRAY;
    obj->panjang = 0; // Mulai dari 0, bertambah saat diisi
    if (kapasitas > 0) {
        obj->nilai.array_elemen = (EnkiObject**)calloc(kapasitas, sizeof(EnkiObject*));
    } else {
        obj->nilai.array_elemen = NULL;
    }
    return obj;
}

EnkiObject* ciptakan_objek_peta(int kapasitas) {
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    obj->tipe = ENKI_OBJEK;
    obj->panjang = 0;
    if (kapasitas > 0) {
        obj->nilai.objek_peta.kunci = (EnkiObject**)calloc(kapasitas, sizeof(EnkiObject*));
        obj->nilai.objek_peta.konten = (EnkiObject**)calloc(kapasitas, sizeof(EnkiObject*));
    } else {
        obj->nilai.objek_peta.kunci = NULL;
        obj->nilai.objek_peta.konten = NULL;
    }
    return obj;
}

EnkiObject* ciptakan_kosong() {
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    obj->tipe = ENKI_KOSONG;
    obj->panjang = 0;
    return obj;
}

// ==========================================
// ☠️ PENGHANCUR DIMENSI (DESTRUCTOR)
// ==========================================

void hancurkan_objek(EnkiObject* obj) {
    if (obj == NULL) return;

    if (obj->tipe == ENKI_TEKS && obj->nilai.teks != NULL) {
        free(obj->nilai.teks);
    } 
    else if (obj->tipe == ENKI_ARRAY && obj->nilai.array_elemen != NULL) {
        for (int i = 0; i < obj->panjang; i++) {
            hancurkan_objek(obj->nilai.array_elemen[i]);
        }
        free(obj->nilai.array_elemen);
    }
    else if (obj->tipe == ENKI_OBJEK) {
        for (int i = 0; i < obj->panjang; i++) {
            if (obj->nilai.objek_peta.kunci) hancurkan_objek(obj->nilai.objek_peta.kunci[i]);
            if (obj->nilai.objek_peta.konten) hancurkan_objek(obj->nilai.objek_peta.konten[i]);
        }
        if (obj->nilai.objek_peta.kunci) free(obj->nilai.objek_peta.kunci);
        if (obj->nilai.objek_peta.konten) free(obj->nilai.objek_peta.konten);
    }
    else if (obj->tipe == ENKI_BLOB && obj->nilai.blob.data != NULL) {
        free(obj->nilai.blob.data);
    }

    free(obj);
}

// ==========================================
// 🖨️ UTILITAS TAMPILAN
// ==========================================
void cetak_objek(EnkiObject* obj) {
    if (!obj) return;
    
    if (obj->tipe == ENKI_ANGKA) {
        if (obj->nilai.angka == (int)obj->nilai.angka) {
            printf("%d", (int)obj->nilai.angka);
        } else {
            printf("%g", obj->nilai.angka);
        }
    } 
    else if (obj->tipe == ENKI_TEKS) {
        printf("%s", obj->nilai.teks);
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        printf("[Array: %d elemen]", obj->panjang);
    }
    else if (obj->tipe == ENKI_OBJEK) {
        printf("{Objek: %d entri}", obj->panjang);
    }
    else if (obj->tipe == ENKI_KOSONG) {
        printf("(kosong)");
    }
}