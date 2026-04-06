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
// 🖨️ UTILITAS TAMPILAN (VERSI REKURSIF)
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
        // Kita cetak teks apa adanya
        printf("%s", obj->nilai.teks);
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        // 🟢 BEDAH ARRAY: Cetak kurung siku dan isinya
        printf("[");
        for (int i = 0; i < obj->panjang; i++) {
            cetak_objek(obj->nilai.array_elemen[i]); // Panggil diri sendiri (Sihir Rekursif)
            if (i < obj->panjang - 1) {
                printf(", "); // Kasih koma jika bukan elemen terakhir
            }
        }
        printf("]");
    }
    else if (obj->tipe == ENKI_OBJEK) {
        // 🟢 BEDAH OBJEK: Cetak kurung kurawal, kunci, dan konten
        printf("{");
        for (int i = 0; i < obj->panjang; i++) {
            cetak_objek(obj->nilai.objek_peta.kunci[i]);
            printf(": ");
            cetak_objek(obj->nilai.objek_peta.konten[i]);
            if (i < obj->panjang - 1) {
                printf(", ");
            }
        }
        printf("}");
    }
    else if (obj->tipe == ENKI_KOSONG) {
        printf("(kosong)");
    }
}

// Sihir Pengganda Objek (Deep Copy)
EnkiObject* ciptakan_salinan_objek(EnkiObject* sumber) {
    if (!sumber) return ciptakan_kosong();

    if (sumber->tipe == ENKI_ANGKA) {
        return ciptakan_angka(sumber->nilai.angka);
    } 
    else if (sumber->tipe == ENKI_TEKS) {
        return ciptakan_teks(sumber->nilai.teks);
    } 
    else if (sumber->tipe == ENKI_ARRAY) {
        EnkiObject* salinan = ciptakan_array(sumber->panjang);
        salinan->panjang = sumber->panjang;
        for (int i = 0; i < sumber->panjang; i++) {
            salinan->nilai.array_elemen[i] = ciptakan_salinan_objek(sumber->nilai.array_elemen[i]);
        }
        return salinan;
    } 

    else if (sumber->tipe == ENKI_BLOB) {
        return ciptakan_blob(sumber->nilai.blob.data, sumber->nilai.blob.ukuran);
    }

    else if (sumber->tipe == ENKI_OBJEK) {
        EnkiObject* salinan = ciptakan_objek_peta(sumber->panjang);
        salinan->panjang = sumber->panjang;
        for (int i = 0; i < sumber->panjang; i++) {
            salinan->nilai.objek_peta.kunci[i] = ciptakan_salinan_objek(sumber->nilai.objek_peta.kunci[i]);
            salinan->nilai.objek_peta.konten[i] = ciptakan_salinan_objek(sumber->nilai.objek_peta.konten[i]);
        }
        return salinan;
    }
    
    return ciptakan_kosong();
}

// 📦 PABRIK BLOB (Data Biner Mentah)
EnkiObject* ciptakan_blob(const unsigned char* data, size_t ukuran) {
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    obj->tipe = ENKI_BLOB;
    obj->panjang = (int)ukuran; // Simpan untuk kompatibilitas panjang()
    
    if (data && ukuran > 0) {
        obj->nilai.blob.data = (unsigned char*)malloc(ukuran);
        memcpy(obj->nilai.blob.data, data, ukuran);
        obj->nilai.blob.ukuran = ukuran;
    } else {
        obj->nilai.blob.data = NULL;
        obj->nilai.blob.ukuran = 0;
    }
    return obj;
}