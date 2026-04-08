#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otim_parser.h"

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

// =======================================================
// 🟢 ASISTEN MEMORI OTIM -> UNUL (Satu 'o' saja agar rapi)
// =======================================================
static void o_tambah_elemen_array(EnkiObject* arr, EnkiObject* elemen) {
    if (!arr || arr->tipe != ENKI_ARRAY) return;
    arr->panjang++;
    arr->nilai.array_elemen = realloc(arr->nilai.array_elemen, arr->panjang * sizeof(EnkiObject*));
    arr->nilai.array_elemen[arr->panjang - 1] = elemen;
}

static void o_simpan_ke_objek(EnkiObject* obj, const char* kunci, EnkiObject* nilai) {
    if (!obj || obj->tipe != ENKI_OBJEK) return;
    obj->panjang++;
    obj->nilai.objek_peta.kunci = realloc(obj->nilai.objek_peta.kunci, obj->panjang * sizeof(EnkiObject*));
    obj->nilai.objek_peta.konten = realloc(obj->nilai.objek_peta.konten, obj->panjang * sizeof(EnkiObject*));
    obj->nilai.objek_peta.kunci[obj->panjang - 1] = ciptakan_teks(kunci);
    obj->nilai.objek_peta.konten[obj->panjang - 1] = nilai;
}

static EnkiObject* parse_otim_elemen(OtimParser* p);

EnkiObject* parse_otim(OtimTokenArray tokens) {
    OtimParser p = {tokens, 0};
    
    if (t_sekarang(&p).jenis == TOKEN_OTIM_HEADER) {
        t_maju(&p);
    } else {
        printf("🚨 KIAMAT VISUAL: Dokumen .otim harus diawali dengan '#!datang'!\n");
        return NULL;
    }

    EnkiObject* root_ui = ciptakan_objek_peta(0); 
    EnkiObject* array_anak = ciptakan_array(0); // 🟢 SUDAH PAKAI (0)
    
    while (t_sekarang(&p).jenis != TOKEN_OTIM_FOOTER && t_sekarang(&p).jenis != TOKEN_OTIM_EOF) {
        EnkiObject* elemen = parse_otim_elemen(&p);
        if (elemen) {
            o_tambah_elemen_array(array_anak, elemen);
        } else {
            t_maju(&p);
        }
    }

    if (t_sekarang(&p).jenis == TOKEN_OTIM_FOOTER) {
        t_maju(&p);
    } else {
        printf("🚨 KIAMAT VISUAL: Dokumen .otim kehilangan penutup 'pergi!#'!\n");
    }

    o_simpan_ke_objek(root_ui, "jenis", ciptakan_teks("akar_dokumen"));
    o_simpan_ke_objek(root_ui, "anak_anak", array_anak);

    return root_ui;
}

static EnkiObject* parse_otim_elemen(OtimParser* p) {
    OtimToken t = t_sekarang(p);

    if (t.jenis == TOKEN_OTIM_TEKS) {
        EnkiObject* node_teks = ciptakan_objek_peta(0);
        o_simpan_ke_objek(node_teks, "jenis", ciptakan_teks("teks"));
        o_simpan_ke_objek(node_teks, "isi", ciptakan_teks(t.isi_teks));
        t_maju(p);
        return node_teks;
    }

    if (t.jenis == TOKEN_OTIM_TAG_BUKA) {
        EnkiObject* node_elemen = ciptakan_objek_peta(0);
        o_simpan_ke_objek(node_elemen, "jenis", ciptakan_teks("tag"));
        o_simpan_ke_objek(node_elemen, "tag", ciptakan_teks(t.tag_nama));
        
        if (t.tag_id) o_simpan_ke_objek(node_elemen, "id", ciptakan_teks(t.tag_id));
        if (t.atribut) o_simpan_ke_objek(node_elemen, "atribut", ciptakan_teks(t.atribut));

        EnkiObject* array_anak = ciptakan_array(0); // 🟢 SUDAH PAKAI (0)
        char* nama_tag_buka = strdup(t.tag_nama);
        t_maju(p);

        while (t_sekarang(p).jenis != TOKEN_OTIM_FOOTER && t_sekarang(p).jenis != TOKEN_OTIM_EOF) {
            if (t_sekarang(p).jenis == TOKEN_OTIM_TAG_TUTUP) {
                if (strcmp(t_sekarang(p).tag_nama, nama_tag_buka) == 0) {
                    t_maju(p);
                    break; 
                } else {
                    printf("🚨 KIAMAT VISUAL: Tag <%s> malah ditutup dengan </%s>!\n", nama_tag_buka, t_sekarang(p).tag_nama);
                    return NULL;
                }
            }
            EnkiObject* anak = parse_otim_elemen(p);
            if (anak) o_tambah_elemen_array(array_anak, anak);
        }

        o_simpan_ke_objek(node_elemen, "anak_anak", array_anak);
        free(nama_tag_buka);
        return node_elemen;
    }

    return NULL;
}