#include "panjang.h"
#include <string.h>

int hitung_panjang_objek(EnkiObject* obj) {
    if (obj == NULL) return 0;

    switch (obj->tipe) {
        case ENKI_ARRAY:
        case ENKI_OBJEK:
            // ⚡ KECEPATAN CAHAYA: Tidak ada loop, langsung ambil variabel 'panjang'
            return obj->panjang;

        case ENKI_TEKS:
            // Untuk teks, panjang sudah dihitung saat diciptakan, tapi kita pastikan
            if (obj->nilai.teks) return obj->panjang;
            return 0;

        case ENKI_BLOB:
            return (int)obj->nilai.blob.ukuran;

        case ENKI_ANGKA:
            return 1; // Angka tunggal dianggap panjangnya 1

        default:
            return 0;
    }
}