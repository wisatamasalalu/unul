#include "enki_object.h"
#include <string.h>

// --- FUNGSI UNTUK MELAHIRKAN OBJEK BARU ---
EnkiObject* ciptakan_objek(TipeEnki tipe) {
    // 1. Alokasikan ruang di RAM sebesar struktur EnkiObject
    EnkiObject* obj = (EnkiObject*)malloc(sizeof(EnkiObject));
    
    if (obj == NULL) {
        printf("🚨 BENCANA MEMORI: Gagal melahirkan objek baru!\n");
        return NULL;
    }

    // 2. Tentukan identitasnya
    obj->tipe = tipe;
    
    // 3. Inisialisasi awal (Nol-kan semua agar tidak ada data sampah)
    obj->v_teks = NULL;
    obj->v_angka = 0.0;
    obj->kunci = NULL;
    obj->konten = NULL;
    obj->jumlah = 0;
    obj->ukuran_biner = 0;

    return obj;
}

// --- FUNGSI UNTUK MENGHAPUS OBJEK (PEMBERSIH DIMENSI) ---
void hancurkan_objek(EnkiObject* obj) {
    if (obj == NULL) return;

    // Jika ada teks, bebaskan
    if (obj->v_teks) free(obj->v_teks);

    // Jika ini Array/Objek, kita harus menghapus anak-anaknya juga (Rekursif)
    if (obj->jumlah > 0) {
        for (int i = 0; i < obj->jumlah; i++) {
            if (obj->kunci && obj->kunci[i]) hancurkan_objek(obj->kunci[i]);
            if (obj->konten && obj->konten[i]) hancurkan_objek(obj->konten[i]);
        }
        if (obj->kunci) free(obj->kunci);
        if (obj->konten) free(obj->konten);
    }

    free(obj);
}