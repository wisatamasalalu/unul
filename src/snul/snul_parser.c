#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snul_parser.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

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
// 🟢 ASISTEN MEMORI BERSIH
// =======================================================
static void o_simpan_ke_objek(EnkiObject* obj, const char* kunci, EnkiObject* nilai) {
    if (!obj || obj->tipe != ENKI_OBJEK) return;
    obj->panjang++;
    // 🟢 BERSIH
    obj->nilai.objek_peta.kunci = (EnkiObject**)enki_realokasi(obj->nilai.objek_peta.kunci, (obj->panjang - 1) * sizeof(EnkiObject*), obj->panjang * sizeof(EnkiObject*), 1);
    obj->nilai.objek_peta.konten = (EnkiObject**)enki_realokasi(obj->nilai.objek_peta.konten, (obj->panjang - 1) * sizeof(EnkiObject*), obj->panjang * sizeof(EnkiObject*), 1);
    obj->nilai.objek_peta.kunci[obj->panjang - 1] = ciptakan_teks(kunci, 1);
    obj->nilai.objek_peta.konten[obj->panjang - 1] = nilai;
}

EnkiObject* parse_snul(SnulTokenArray tokens) {
    SnulParser p = {tokens, 0};
    EnkiObject* root_gaya = ciptakan_objek_peta(0, 1); 

    while (t_sekarang(&p).jenis != TOKEN_SNUL_EOF) {
        
        // 1. Tangkap Selektor
        if (t_sekarang(&p).jenis == TOKEN_SNUL_SELECTOR) {
            // 🟢 BERSIH
            char* nama_selektor = enki_salin_teks(t_sekarang(&p).teks, 1);
            t_maju(&p);

            // 2. Wajib ada kurung kurawal buka '{'
            if (t_sekarang(&p).jenis == TOKEN_SNUL_LBRACE) {
                t_maju(&p);
                
                EnkiObject* aturan_gaya = ciptakan_objek_peta(0, 1); 

                // 3. Kumpulkan semua properti di dalam blok
                while (t_sekarang(&p).jenis != TOKEN_SNUL_RBRACE && t_sekarang(&p).jenis != TOKEN_SNUL_EOF) {
                    
                    if (t_sekarang(&p).jenis == TOKEN_SNUL_PROPERTI) {
                        // 🟢 BERSIH
                        char* nama_prop = enki_salin_teks(t_sekarang(&p).teks, 1);
                        t_maju(&p);

                        // Tangkap nilainya
                        if (t_sekarang(&p).jenis == TOKEN_SNUL_NILAI) {
                            o_simpan_ke_objek(aturan_gaya, nama_prop, ciptakan_teks(t_sekarang(&p).teks, 1));
                            t_maju(&p);
                        } else {
                            o_simpan_ke_objek(aturan_gaya, nama_prop, ciptakan_teks("", 1));
                        }
                        enki_bebas(nama_prop, 1); // 🟢 BERSIH
                    } else {
                        printf("🚨 KIAMAT VISUAL: Sintaks SNUL cacat di dalam blok '%s'!\n", nama_selektor);
                        enki_bebas(nama_selektor, 1); // 🟢 BERSIH
                        return NULL; 
                    }
                }

                // 4. Tutup dengan kurung kurawal '}'
                if (t_sekarang(&p).jenis == TOKEN_SNUL_RBRACE) {
                    t_maju(&p);
                }
                
                // Simpan aturan ke Kamus Besar!
                o_simpan_ke_objek(root_gaya, nama_selektor, aturan_gaya);
            }
            enki_bebas(nama_selektor, 1); // 🟢 BERSIH

        } else {
            // ❌ TANGKAP ERROR DI LUAR BLOK (Token siluman)
            printf("🚨 KIAMAT VISUAL: Sintaks SNUL tidak valid! Menemukan token liar di luar blok gaya!\n");
            return NULL; 
        }
    }

    return root_gaya;
}