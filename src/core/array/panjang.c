#include "panjang.h"
#include <string.h>

int hitung_panjang_objek(EnkiObject* obj) {
    if (obj == NULL) return 0;

    switch (obj->tipe) {
        case ENKI_ARRAY:
        case ENKI_OBJEK:
            // ⚡ KECEPATAN CAHAYA: Tidak ada loop, langsung ambil variabel 'jumlah'
            return obj->jumlah;

        case ENKI_TEKS:
            // Untuk teks, kita tetap gunakan strlen
            if (obj->v_teks) return strlen(obj->v_teks);
            return 0;

        case ENKI_BLOB:
            return (int)obj->ukuran_biner;

        case ENKI_ANGKA:
            return 1; // Angka tunggal dianggap panjangnya 1

        default:
            return 0;
    }
}