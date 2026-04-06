#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snul_parser.h"

typedef struct {
    SnulTokenArray tokens;
    int kursor;
} SnulParser;

static SnulToken t_sekarang(SnulParser* p) {
    if (p->kursor >= p->tokens.jumlah) return p->tokens.data[p->tokens.jumlah - 1];
    return p->tokens.data[p->kursor];
}

static void t_maju(SnulParser* p) {
    if (p->kursor < p->tokens.jumlah) p->kursor++;
}

// =======================================================
// 🟢 ASISTEN MEMORI (Sama seperti OTIM)
// =======================================================
static void o_simpan_ke_objek(EnkiObject* obj, const char* kunci, EnkiObject* nilai) {
    if (!obj || obj->tipe != ENKI_OBJEK) return;
    obj->panjang++;
    obj->nilai.objek_peta.kunci = realloc(obj->nilai.objek_peta.kunci, obj->panjang * sizeof(EnkiObject*));
    obj->nilai.objek_peta.konten = realloc(obj->nilai.objek_peta.konten, obj->panjang * sizeof(EnkiObject*));
    obj->nilai.objek_peta.kunci[obj->panjang - 1] = ciptakan_teks(kunci);
    obj->nilai.objek_peta.konten[obj->panjang - 1] = nilai;
}

EnkiObject* parse_snul(SnulTokenArray tokens) {
    SnulParser p = {tokens, 0};
    EnkiObject* root_gaya = ciptakan_objek_peta(0); // Kamus Besar Kosmetik

    while (t_sekarang(&p).jenis != TOKEN_SNUL_EOF) {
        
        // 1. Tangkap Selektor (Contoh: @wadah.utama)
        if (t_sekarang(&p).jenis == TOKEN_SNUL_SELECTOR) {
            char* nama_selektor = strdup(t_sekarang(&p).teks);
            t_maju(&p);

            // 2. Wajib ada kurung kurawal buka '{'
            if (t_sekarang(&p).jenis == TOKEN_SNUL_LBRACE) {
                t_maju(&p);
                
                EnkiObject* aturan_gaya = ciptakan_objek_peta(0); // Kamus kecil untuk properti

                // 3. Kumpulkan semua properti di dalam blok
                while (t_sekarang(&p).jenis != TOKEN_SNUL_RBRACE && t_sekarang(&p).jenis != TOKEN_SNUL_EOF) {
                    
                    if (t_sekarang(&p).jenis == TOKEN_SNUL_PROPERTI) {
                        char* nama_prop = strdup(t_sekarang(&p).teks);
                        t_maju(&p);

                        // Tangkap nilainya
                        if (t_sekarang(&p).jenis == TOKEN_SNUL_NILAI) {
                            o_simpan_ke_objek(aturan_gaya, nama_prop, ciptakan_teks(t_sekarang(&p).teks));
                            t_maju(&p);
                        } else {
                            // Jika kosong/error, isi teks kosong agar tidak segfault
                            o_simpan_ke_objek(aturan_gaya, nama_prop, ciptakan_teks(""));
                        }
                        free(nama_prop);
                    } else {
                        t_maju(&p); // Lewati token aneh di dalam blok
                    }
                }

                // 4. Tutup dengan kurung kurawal '}'
                if (t_sekarang(&p).jenis == TOKEN_SNUL_RBRACE) {
                    t_maju(&p);
                }
                
                // Simpan aturan ke Kamus Besar!
                o_simpan_ke_objek(root_gaya, nama_selektor, aturan_gaya);
            }
            free(nama_selektor);
        } else {
            t_maju(&p); // Lewati token liar di luar blok
        }
    }

    return root_gaya;
}