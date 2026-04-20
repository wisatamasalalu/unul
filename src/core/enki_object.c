#include "enki_object.h"
#include "enki_memory.h" // HUBUNGKAN DENGAN ARENA
#include "../core_c/enki_os.h" // 🟢 WAJIB ADA UNTUK GEMBOK!

// 🟢 PABRIK PORTAL
EnkiObject* ciptakan_portal(int kapasitas_buffer, int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_PORTAL;
    obj->panjang = 0;
    
    int kap = (kapasitas_buffer > 0) ? kapasitas_buffer : 1; 
    
    obj->nilai.portal.kapasitas = kap;
    obj->nilai.portal.kepala = 0;
    obj->nilai.portal.ekor = 0;
    obj->nilai.portal.jumlah = 0;
    
    obj->nilai.portal.antrian = (EnkiObject**)enki_alokasi(kap * sizeof(EnkiObject*), mode_dinamis);
    if (mode_dinamis == 1) memset(obj->nilai.portal.antrian, 0, kap * sizeof(EnkiObject*));
    
    // Gunakan Wrapper OS! Bersih dan murni!
    obj->nilai.portal.gembok = os_gembok_ciptakan(mode_dinamis);
    obj->nilai.portal.sinyal = os_sinyal_ciptakan(mode_dinamis);
    
    return obj;
}

// ==========================================
// 🏭 PABRIK PENCIPTAAN (CONSTRUCTORS)
// ==========================================

EnkiObject* ciptakan_angka(double nilai, int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_ANGKA;
    obj->nilai.angka = nilai;
    obj->panjang = 0;
    return obj;
}

EnkiObject* ciptakan_teks(const char* nilai, int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_TEKS;
    if (nilai) {
        size_t len = strlen(nilai);
        obj->nilai.teks = (char*)enki_alokasi(len + 1, mode_dinamis);
        strcpy(obj->nilai.teks, nilai);
        obj->panjang = len;
    } else {
        obj->nilai.teks = (char*)enki_alokasi(1, mode_dinamis);
        obj->nilai.teks[0] = '\0';
        obj->panjang = 0;
    }
    return obj;
}

EnkiObject* ciptakan_array(int kapasitas, int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_ARRAY;
    obj->panjang = 0; 
    if (kapasitas > 0) {
        size_t total_ukuran = kapasitas * sizeof(EnkiObject*);
        obj->nilai.array_elemen = (EnkiObject**)enki_alokasi(total_ukuran, mode_dinamis);
        if (mode_dinamis == 1) memset(obj->nilai.array_elemen, 0, total_ukuran);
    } else {
        obj->nilai.array_elemen = NULL;
    }
    return obj;
}

EnkiObject* ciptakan_objek_peta(int kapasitas, int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_OBJEK;
    obj->panjang = 0;
    if (kapasitas > 0) {
        size_t total_ukuran = kapasitas * sizeof(EnkiObject*);
        
        obj->nilai.objek_peta.kunci = (EnkiObject**)enki_alokasi(total_ukuran, mode_dinamis);
        if (mode_dinamis == 1) memset(obj->nilai.objek_peta.kunci, 0, total_ukuran);
        
        obj->nilai.objek_peta.konten = (EnkiObject**)enki_alokasi(total_ukuran, mode_dinamis);
        if (mode_dinamis == 1) memset(obj->nilai.objek_peta.konten, 0, total_ukuran);
    } else {
        obj->nilai.objek_peta.kunci = NULL;
        obj->nilai.objek_peta.konten = NULL;
    }
    return obj;
}

EnkiObject* ciptakan_kosong(int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_KOSONG;
    obj->panjang = 0;
    return obj;
}

// 📦 INI FUNGSI BLOB ANDA YANG SUDAH DIPERBARUI!
EnkiObject* ciptakan_blob(const unsigned char* data, size_t ukuran, int mode_dinamis) {
    EnkiObject* obj = (EnkiObject*)enki_alokasi(sizeof(EnkiObject), mode_dinamis);
    obj->tipe = ENKI_BLOB;
    obj->panjang = (int)ukuran; 
    
    if (data && ukuran > 0) {
        obj->nilai.blob.data = (unsigned char*)enki_alokasi(ukuran, mode_dinamis);
        memcpy(obj->nilai.blob.data, data, ukuran);
        obj->nilai.blob.ukuran = ukuran;
    } else {
        obj->nilai.blob.data = NULL;
        obj->nilai.blob.ukuran = 0;
    }
    return obj;
}

// 👯 INI FUNGSI DEEP COPY ANDA YANG SUDAH DIPERBARUI!
EnkiObject* ciptakan_salinan_objek(EnkiObject* sumber, int mode_dinamis) {
    if (!sumber) return ciptakan_kosong(mode_dinamis);

    if (sumber->tipe == ENKI_ANGKA) return ciptakan_angka(sumber->nilai.angka, mode_dinamis);
    else if (sumber->tipe == ENKI_TEKS) return ciptakan_teks(sumber->nilai.teks, mode_dinamis);
    else if (sumber->tipe == ENKI_ARRAY) {
        EnkiObject* salinan = ciptakan_array(sumber->panjang, mode_dinamis);
        salinan->panjang = sumber->panjang;
        for (int i = 0; i < sumber->panjang; i++) {
            salinan->nilai.array_elemen[i] = ciptakan_salinan_objek(sumber->nilai.array_elemen[i], mode_dinamis);
        }
        return salinan;
    } 
    else if (sumber->tipe == ENKI_BLOB) return ciptakan_blob(sumber->nilai.blob.data, sumber->nilai.blob.ukuran, mode_dinamis);
    else if (sumber->tipe == ENKI_OBJEK) {
        EnkiObject* salinan = ciptakan_objek_peta(sumber->panjang, mode_dinamis);
        salinan->panjang = sumber->panjang;
        for (int i = 0; i < sumber->panjang; i++) {
            salinan->nilai.objek_peta.kunci[i] = ciptakan_salinan_objek(sumber->nilai.objek_peta.kunci[i], mode_dinamis);
            salinan->nilai.objek_peta.konten[i] = ciptakan_salinan_objek(sumber->nilai.objek_peta.konten[i], mode_dinamis);
        }
        return salinan;
    }
    // 🟢 SUNTIKAN: PORTAL ADALAH PASS-BY-REFERENCE MUTLAK!
    else if (sumber->tipe == ENKI_PORTAL) {
        return sumber; // Kembalikan pointer asli!
    } // SATU-SATUNYA PASS BY REFERENCE YANG SAH UNTUK BAHASA PASS BY VALUE
    return ciptakan_kosong(mode_dinamis);
}

// ==========================================
// ☠️ PENGHANCUR DIMENSI (DESTRUCTOR)
// ==========================================

void hancurkan_objek(EnkiObject* obj, int mode_dinamis) {
    if (obj == NULL) return;

    if (obj->tipe == ENKI_TEKS && obj->nilai.teks != NULL) {
        enki_bebas(obj->nilai.teks, mode_dinamis);
    } 
    else if (obj->tipe == ENKI_ARRAY && obj->nilai.array_elemen != NULL) {
        for (int i = 0; i < obj->panjang; i++) {
            hancurkan_objek(obj->nilai.array_elemen[i], mode_dinamis);
        }
        enki_bebas(obj->nilai.array_elemen, mode_dinamis);
    }
    else if (obj->tipe == ENKI_OBJEK) {
        for (int i = 0; i < obj->panjang; i++) {
            if (obj->nilai.objek_peta.kunci) hancurkan_objek(obj->nilai.objek_peta.kunci[i], mode_dinamis);
            if (obj->nilai.objek_peta.konten) hancurkan_objek(obj->nilai.objek_peta.konten[i], mode_dinamis);
        }
        if (obj->nilai.objek_peta.kunci) enki_bebas(obj->nilai.objek_peta.kunci, mode_dinamis);
        if (obj->nilai.objek_peta.konten) enki_bebas(obj->nilai.objek_peta.konten, mode_dinamis);
    }
    else if (obj->tipe == ENKI_BLOB && obj->nilai.blob.data != NULL) {
        enki_bebas(obj->nilai.blob.data, mode_dinamis);
    }
    // 🟢 SUNTIKAN PENGHANCUR PORTAL
    else if (obj->tipe == ENKI_PORTAL) {
        for (int i = 0; i < obj->nilai.portal.jumlah; i++) {
            int idx = (obj->nilai.portal.kepala + i) % obj->nilai.portal.kapasitas;
            if (obj->nilai.portal.antrian[idx]) {
                hancurkan_objek(obj->nilai.portal.antrian[idx], mode_dinamis);
            }
        }
        enki_bebas(obj->nilai.portal.antrian, mode_dinamis);
        
        if (obj->nilai.portal.gembok) os_gembok_hancurkan(obj->nilai.portal.gembok, mode_dinamis);
        if (obj->nilai.portal.sinyal) os_sinyal_hancurkan(obj->nilai.portal.sinyal, mode_dinamis);
    }

    enki_bebas(obj, mode_dinamis);
}

// ==========================================
// 🖨️ UTILITAS TAMPILAN
// ==========================================
void cetak_objek(EnkiObject* obj) {
    if (!obj) return;
    
    if (obj->tipe == ENKI_ANGKA) {
        if (obj->nilai.angka == (int)obj->nilai.angka) printf("%d", (int)obj->nilai.angka);
        else printf("%g", obj->nilai.angka);
    } 
    else if (obj->tipe == ENKI_TEKS) {
        printf("%s", obj->nilai.teks);
    } 
    else if (obj->tipe == ENKI_ARRAY) {
        printf("[");
        for (int i = 0; i < obj->panjang; i++) {
            cetak_objek(obj->nilai.array_elemen[i]);
            if (i < obj->panjang - 1) printf(", ");
        }
        printf("]");
    }
    else if (obj->tipe == ENKI_OBJEK) {
        printf("{");
        for (int i = 0; i < obj->panjang; i++) {
            cetak_objek(obj->nilai.objek_peta.kunci[i]);
            printf(": ");
            cetak_objek(obj->nilai.objek_peta.konten[i]);
            if (i < obj->panjang - 1) printf(", ");
        }
        printf("}");
    }
    else if (obj->tipe == ENKI_KOSONG) {
        printf("(kosong)");
    }
}