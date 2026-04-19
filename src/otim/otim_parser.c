#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otim_parser.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

typedef struct {
    OtimTokenArray tokens;
    int kursor;
} OtimParser;

static OtimToken t_sekarang(OtimParser* p) {
    if (p->kursor >= p->tokens.jumlah) return p->tokens.data[p->tokens.jumlah - 1];
    return p->tokens.data[p->kursor];
}

static void t_maju(OtimParser* p) {
    if (p->kursor < p->tokens.jumlah) p->kursor++;
}

static void o_tambah_elemen_array(EnkiObject* arr, EnkiObject* elemen) {
    if (!arr || arr->tipe != ENKI_ARRAY) return;
    arr->panjang++;
    arr->nilai.array_elemen = (EnkiObject**)enki_realokasi(arr->nilai.array_elemen, (arr->panjang - 1) * sizeof(EnkiObject*), arr->panjang * sizeof(EnkiObject*), 1);
    arr->nilai.array_elemen[arr->panjang - 1] = elemen;
}

static void o_simpan_ke_objek(EnkiObject* obj, const char* kunci, EnkiObject* nilai) {
    if (!obj || obj->tipe != ENKI_OBJEK) return;
    obj->panjang++;
    obj->nilai.objek_peta.kunci = (EnkiObject**)enki_realokasi(obj->nilai.objek_peta.kunci, (obj->panjang - 1) * sizeof(EnkiObject*), obj->panjang * sizeof(EnkiObject*), 1);
    obj->nilai.objek_peta.konten = (EnkiObject**)enki_realokasi(obj->nilai.objek_peta.konten, (obj->panjang - 1) * sizeof(EnkiObject*), obj->panjang * sizeof(EnkiObject*), 1);
    obj->nilai.objek_peta.kunci[obj->panjang - 1] = ciptakan_teks(kunci, 1);
    obj->nilai.objek_peta.konten[obj->panjang - 1] = nilai;
}

static EnkiObject* parse_otim_elemen(OtimParser* p);

EnkiObject* parse_otim(OtimTokenArray tokens) {
    OtimParser p = {tokens, 0};
    
    if (t_sekarang(&p).jenis == TOKEN_OTIM_HEADER) { t_maju(&p); } 
    else { printf("🚨 KIAMAT VISUAL: Dokumen harus diawali '#!datang'!\n"); return NULL; }

    EnkiObject* root_ui = ciptakan_objek_peta(0, 1); 
    EnkiObject* array_anak = ciptakan_array(0, 1); 
    
    while (t_sekarang(&p).jenis != TOKEN_OTIM_FOOTER && t_sekarang(&p).jenis != TOKEN_OTIM_EOF) {
        EnkiObject* elemen = parse_otim_elemen(&p);
        if (elemen) o_tambah_elemen_array(array_anak, elemen);
        else t_maju(&p);
    }

    if (t_sekarang(&p).jenis == TOKEN_OTIM_FOOTER) t_maju(&p);

    o_simpan_ke_objek(root_ui, "jenis", ciptakan_teks("akar_dokumen", 1));
    o_simpan_ke_objek(root_ui, "anak_anak", array_anak);
    return root_ui;
}

static EnkiObject* parse_otim_elemen(OtimParser* p) {
    OtimToken t = t_sekarang(p);

    if (t.jenis == TOKEN_OTIM_TEKS) {
        EnkiObject* node_teks = ciptakan_objek_peta(0, 1);
        o_simpan_ke_objek(node_teks, "jenis", ciptakan_teks("teks", 1));
        o_simpan_ke_objek(node_teks, "isi", ciptakan_teks(t.isi_teks, 1));
        t_maju(p);
        return node_teks;
    }

    if (t.jenis == TOKEN_OTIM_TAG_BUKA) {
        EnkiObject* node_elemen = ciptakan_objek_peta(0, 1);
        o_simpan_ke_objek(node_elemen, "jenis", ciptakan_teks("tag", 1));
        o_simpan_ke_objek(node_elemen, "tag", ciptakan_teks(t.tag_nama, 1));
        
        if (t.tag_id) o_simpan_ke_objek(node_elemen, "id", ciptakan_teks(t.tag_id, 1));
        
        // Simpan Seluruh Teks Atribut ke dalam Objek (Siap diurai oleh Renderer!)
        if (t.atribut) o_simpan_ke_objek(node_elemen, "atribut", ciptakan_teks(t.atribut, 1));

        // 🟢 DNA KEMURNIAN: Lahirkan teks_input secara resmi agar Bridge UNUL menemukannya!
        if (strcmp(t.tag_nama, "masukan") == 0 || strcmp(t.tag_nama, "masukan_sandi") == 0 || strcmp(t.tag_nama, "areanulis") == 0) {
            o_simpan_ke_objek(node_elemen, "teks_input", ciptakan_teks("", 1));
        }

        EnkiObject* array_anak = ciptakan_array(0, 1); 
        char* nama_tag_buka = enki_salin_teks(t.tag_nama, 1);
        t_maju(p);

        while (t_sekarang(p).jenis != TOKEN_OTIM_FOOTER && t_sekarang(p).jenis != TOKEN_OTIM_EOF) {
            if (t_sekarang(p).jenis == TOKEN_OTIM_TAG_TUTUP) {
                if (strcmp(t_sekarang(p).tag_nama, nama_tag_buka) == 0) {
                    t_maju(p); break; 
                } else {
                    printf("🚨 KIAMAT VISUAL: <%s> ditutup dengan </%s>!\n", nama_tag_buka, t_sekarang(p).tag_nama);
                    enki_bebas(nama_tag_buka, 1);
                    return NULL;
                }
            }
            EnkiObject* anak = parse_otim_elemen(p);
            if (anak) o_tambah_elemen_array(array_anak, anak);
        }

        o_simpan_ke_objek(node_elemen, "anak_anak", array_anak);

        if (array_anak->panjang > 0) {
            EnkiObject* anak_pertama = array_anak->nilai.array_elemen[0];
            for (int j = 0; j < anak_pertama->panjang; j++) {
                if (strcmp(anak_pertama->nilai.objek_peta.kunci[j]->nilai.teks, "isi") == 0) {
                    o_simpan_ke_objek(node_elemen, "teks_dalam", ciptakan_teks(anak_pertama->nilai.objek_peta.konten[j]->nilai.teks, 1));
                    break;
                }
            }
        }

        enki_bebas(nama_tag_buka, 1);
        return node_elemen;
    }
    return NULL;
}