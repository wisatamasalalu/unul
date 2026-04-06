#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_parser.h"

// --- 1. MANAJEMEN MEMORI POHON ---
Parser inisialisasi_parser(TokenArray tokens) {
    Parser p;
    p.tokens = tokens;
    p.kursor = 0; 
    return p;
}

// 🟢 2 FUNGSI INI KITA NAIKKAN KE ATAS (Sebelum buat_node)
Token token_sekarang(Parser* p) {
    if (p->kursor >= p->tokens.jumlah) return p->tokens.data[p->tokens.jumlah - 1];
    return p->tokens.data[p->kursor];
}

void maju(Parser* p) {
    if (p->kursor < p->tokens.jumlah) p->kursor++;
}

// 🟢 SIHIR KIAMAT SINTAKSIS (DENGAN PANDUAN CERDAS)
void kiamat_sintaksis(Parser* p, const char* pesan, const char* panduan_cerdas) {
    Token t = token_sekarang(p);
    int baris = t.baris > 0 ? t.baris : 0;
    int kolom = t.kolom > 0 ? t.kolom : 0;
    
    printf("\n🚨 [%s: Baris %d:%d] KIAMAT SINTAKSIS: %s\n", 
           t.nama_file ? t.nama_file : "skrip.unul", baris, kolom, pesan);
           
    if (t.isi) {
        printf("   |\n %d | ... %s ... \n   | ", baris, t.isi);
        for(int i = 0; i < kolom; i++) printf(" ");
        printf("^-- Bencana bermula di sini!\n\n");
    }

    if (panduan_cerdas) {
        printf("💡 PANDUAN CERDAS:\n%s\n\n", panduan_cerdas);
    }
    
    exit(1); 
}

// Fungsi Pemaksa Hukum Enlil
void harapkan_token(Parser* p, TokenJenis jenis_harapan, const char* pesan_error) {
    Token t = token_sekarang(p);
    
    if (t.jenis == jenis_harapan) {
        // Jika token sesuai harapan (misal: kurung tutup), makan dan lanjut!
        maju(p); 
    } else {
        // Jika tidak sesuai harapan, KIAMAT SINTAKSIS terjadi seketika!
        char pesan_detail[512];
        snprintf(pesan_detail, sizeof(pesan_detail), 
            "%s\n(Mendapatkan '%s' alih-alih token yang sah)", pesan_error, t.isi);
            
        kiamat_sintaksis(p, "Hukum Tata Bahasa Dilanggar!", pesan_detail);
    }
}

// --- 2. PENCIPTAAN NODE ---
ASTNode* buat_node(ASTJenis jenis, Parser* p) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->jenis = jenis;
    node->anak_anak = NULL;
    node->jumlah_anak = 0;
    node->kapasitas_anak = 0;
    node->nilai_teks = NULL;
    node->kiri = NULL;
    node->kanan = NULL;
    node->operator_math = NULL;
    node->syarat = NULL;
    node->blok_maka = NULL;
    node->blok_lain = NULL;
    node->pembanding = NULL;
    node->batas_loop = NULL;
    node->blok_siklus = NULL;
    node->blok_tebus = NULL;
    node->indeks_array = NULL;

    // 🟢 SUNTIKAN: Salin jejak Ruang & Waktu dari Token saat ini!
    Token t = token_sekarang(p);
    node->baris = t.baris;
    node->kolom = t.kolom;
    node->nama_file = t.nama_file ? strdup(t.nama_file) : NULL;

    return node;
}

// Menambahkan anak ke dalam blok PROGRAM (Mengurus alokasi dinamis pointer)
void tambah_anak(ASTNode* induk, ASTNode* anak) {
    if (induk->jumlah_anak >= induk->kapasitas_anak) {
        induk->kapasitas_anak = induk->kapasitas_anak == 0 ? 4 : induk->kapasitas_anak * 2;
        induk->anak_anak = (ASTNode**)realloc(induk->anak_anak, induk->kapasitas_anak * sizeof(ASTNode*));
    }
    induk->anak_anak[induk->jumlah_anak++] = anak;
}

// Fungsi gaib untuk menyisipkan argumen di urutan pertama (Khusus Pipa Aliran)
void sisip_anak_pertama(ASTNode* induk, ASTNode* anak) {
    if (induk->jumlah_anak >= induk->kapasitas_anak) {
        induk->kapasitas_anak = induk->kapasitas_anak == 0 ? 4 : induk->kapasitas_anak * 2;
        induk->anak_anak = (ASTNode**)realloc(induk->anak_anak, induk->kapasitas_anak * sizeof(ASTNode*));
    }
    // Geser semua anak yang sudah ada ke kanan 1 langkah
    for (int i = induk->jumlah_anak; i > 0; i--) {
        induk->anak_anak[i] = induk->anak_anak[i - 1];
    }
    // Letakkan yang baru di urutan paling awal (Index 0)
    induk->anak_anak[0] = anak;
    induk->jumlah_anak++;
}

// Wajib: Mengembalikan seluruh ranting pohon ke OS agar RAM tidak bocor!
void bebaskan_ast(ASTNode* node) {
    if (!node) return;
    if (node->nilai_teks) free(node->nilai_teks);
    if (node->kiri) bebaskan_ast(node->kiri);
    if (node->kanan) bebaskan_ast(node->kanan);
    if (node->pembanding) free(node->pembanding);
    if (node->syarat) bebaskan_ast(node->syarat);
    if (node->blok_maka) bebaskan_ast(node->blok_maka);
    if (node->blok_lain) bebaskan_ast(node->blok_lain);
    
    for (int i = 0; i < node->jumlah_anak; i++) {
        bebaskan_ast(node->anak_anak[i]);
    }
    if (node->anak_anak) free(node->anak_anak);
    if (node->batas_loop) bebaskan_ast(node->batas_loop);
    if (node->blok_siklus) bebaskan_ast(node->blok_siklus);
    if (node->blok_tebus) bebaskan_ast(node->blok_tebus);
    
    free(node);
}

// --- 3. LOGIKA PEMBEDAHAN (PARSING) ---
// Deklarasi Hierarki Matematika Mutlak
ASTNode* parse_struktur_objek(Parser* p);
ASTNode* parse_ekspresi(Parser* p);
ASTNode* parse_pipa(Parser* p);
ASTNode* parse_syarat_logika(Parser* p);
ASTNode* parse_penjumlahan(Parser* p);
ASTNode* parse_faktor(Parser* p);
ASTNode* parse_pangkat(Parser* p);
ASTNode* parse_nilai_dasar(Parser* p);

ASTNode* parse_struktur_objek(Parser* p) {
    ASTNode* node = buat_node(AST_STRUKTUR_OBJEK, p);
    maju(p); // Lewati '{'
    
    while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_K_T) {
        // 1. Tangkap Kunci
        ASTNode* kunci = parse_ekspresi(p); 
        tambah_anak(node, kunci);
        
        // 2. KIAMAT: Lupa Titik Dua (:)
        if (token_sekarang(p).jenis == TOKEN_TITIK_DUA) {
            maju(p);
        } else {
            // Tetap menggunakan pesan Anda yang legendaris!
            kiamat_sintaksis(p, "Wujud objek kehilangan titik dua ':' !", 
                "Dalam struktur objek {}, Anda harus memisahkan Kunci dan Nilai menggunakan titik dua.\n"
                "Contoh yang benar: takdir.soft dewa = {\"nama\": \"Enki\"}");
        }
        
        // 3. Tangkap Nilai
        ASTNode* konten = parse_ekspresi(p);
        tambah_anak(node, konten);
        
        if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p);
    }
    
    // 3. KIAMAT: Lupa Kurung Tutup (})
    // Ini dipicu oleh harapkan_token yang memanggil kiamat_sintaksis secara otomatis
    harapkan_token(p, TOKEN_KURUNG_K_T, 
        "Kehilangan kurung kurawal penutup '}' pada pembuatan Objek/Domain.\n"
        "💡 PANDUAN: Pastikan format Anda sesuai. Contoh: { \"nama\": \"Enki\" }");
        
    return node;
}

// ==========================================================
// TINGKAT 1: Menangkap Nilai Murni & Kurung ()
// ==========================================================
ASTNode* parse_nilai_dasar(Parser* p) {
    Token t = token_sekarang(p);
    ASTNode* simpul_kiri = NULL;

    // [MISI 1] PRIORITAS KURUNG MATEMATIKA & FUNGSI PANAH
    if (t.jenis == TOKEN_KURUNG_B) {
        
    //  PENANGKAP OPERATOR TERNARI (Ekspresi)
    if (t.jenis == TOKEN_KARMA && strcmp(t.isi, "jika") == 0) {
        maju(p); // Lewati 'jika'
        ASTNode* node_ternari = buat_node(AST_TERNARI, p);
        
        // 1. Tangkap syarat kondisinya
        node_ternari->syarat = parse_ekspresi(p);
        
        // 2. Wajib ada 'maka'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
        else kiamat_sintaksis(p, "Ternari cacat!", "Kehilangan 'maka' setelah kondisi pada operasi sebaris.");
        
        // 3. Tangkap nilai jika BENAR
        node_ternari->kiri = parse_ekspresi(p);
        
        // 4. Wajib ada 'lain'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "lain") == 0) maju(p);
        else kiamat_sintaksis(p, "Ternari cacat!", "Kehilangan 'lain' pada operasi sebaris. Ternari harus memiliki jalan alternatif.");
        
        // 5. Tangkap nilai jika SALAH
        node_ternari->kanan = parse_ekspresi(p);
        
        return node_ternari;
    }

        // Sihir Pengintaian (Lookahead)
        // Kita intip ke depan, apakah setelah kurung tutup ')' ada panah '=>' ?
        int is_panah = 0;
        int kursor_intip = p->kursor + 1;
        while (kursor_intip < p->tokens.jumlah && p->tokens.data[kursor_intip].jenis != TOKEN_KURUNG_T) kursor_intip++;
        if (kursor_intip + 1 < p->tokens.jumlah && p->tokens.data[kursor_intip + 1].jenis == TOKEN_PANAH) is_panah = 1;

        if (is_panah) {
            ASTNode* node_panah = buat_node(AST_FUNGSI_PANAH, p);
            maju(p); // Lewati '('
            // Tangkap parameter (x, y)
            while (token_sekarang(p).jenis != TOKEN_KURUNG_T) {
                if (token_sekarang(p).jenis == TOKEN_IDENTITAS) {
                    ASTNode* param = buat_node(AST_IDENTITAS, p);
                    param->nilai_teks = strdup(token_sekarang(p).isi);
                    tambah_anak(node_panah, param);
                    maju(p);
                }
                if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p);
            }
            maju(p); // Lewati ')'
            maju(p); // Lewati '=>'
            
            // Tangkap tubuh fungsinya (ekspresi)
            node_panah->kanan = parse_ekspresi(p);
            return node_panah;
        }

        // Jika bukan fungsi panah, berarti ini Kurung Matematika biasa!
        maju(p); // Lewati '('
        simpul_kiri = parse_ekspresi(p); 
        
        // 🟢 SUNTIKAN 4
        harapkan_token(p, TOKEN_KURUNG_T, "Operasi dalam kurung kehilangan kurung tutup ')'.\n💡 SOLUSI: Pastikan jumlah kurung buka dan tutup seimbang.");
        
        return simpul_kiri;
    }
    
    // 1. Tangkap Teks atau Angka Murni
    else if (t.jenis == TOKEN_TEKS || t.jenis == TOKEN_ANGKA) {
        simpul_kiri = buat_node(AST_LITERAL_TEKS, p);
        simpul_kiri->nilai_teks = strdup(t.isi);
        maju(p);
        return simpul_kiri;
    }

    // 2. Tangkap Identitas (Variabel, Fungsi)
    else if (t.jenis == TOKEN_IDENTITAS) {
        simpul_kiri = buat_node(AST_IDENTITAS, p);
        simpul_kiri->nilai_teks = strdup(t.isi);
        maju(p);

        // Cek Panggilan Fungsi Tunggal
        if (token_sekarang(p).jenis == TOKEN_KURUNG_B) {
            simpul_kiri->jenis = AST_PANGGILAN_FUNGSI;
            maju(p); // Lewati '('
            
            while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_T) {
                ASTNode* arg = parse_ekspresi(p); 
                if (arg) tambah_anak(simpul_kiri, arg); 
                if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p);
            }
            
            // 🟢 MENGEMBALIKAN PESAN ERROR & SMART HINT LAMA ANDA
            char pesan_fungsi[256];
            snprintf(pesan_fungsi, sizeof(pesan_fungsi), 
                "Pemanggilan fungsi '%s' kehilangan tanda kurung tutup ')'.\n"
                "💡 SOLUSI: Pastikan setiap fungsi ditutup dengan benar. Contoh: %s()", 
                simpul_kiri->nilai_teks, simpul_kiri->nilai_teks);
                
            harapkan_token(p, TOKEN_KURUNG_T, pesan_fungsi);
        }
        else {
            // 🔥 LOOP MULTIDIMENSI: Menggabungkan Titik (.) dan Kurung Siku ([]) 🔥
            // Sekarang mesin bisa membaca dewa.anak[1].cucu[0].kekuatan tanpa henti!
            while (token_sekarang(p).jenis == TOKEN_TITIK || token_sekarang(p).jenis == TOKEN_KURUNG_S_B) {
                
                // Jika itu TITIK (.)
                if (token_sekarang(p).jenis == TOKEN_TITIK) {
                    maju(p); // Lewati titik '.'
                    Token t_anak = token_sekarang(p);
                    if (t_anak.jenis == TOKEN_IDENTITAS) {
                        ASTNode* akses = buat_node(AST_AKSES_DOMAIN, p);
                        akses->kiri = simpul_kiri;              
                        akses->nilai_teks = strdup(t_anak.isi); 
                        simpul_kiri = akses;
                        maju(p);
                    } else {
                        // 🟢 SMART HINT BARU UNTUK TITIK GAIB
                        kiamat_sintaksis(p, "Sintaksis Cacat: Nama properti gaib setelah titik!", 
                            "Anda menulis tanda titik (.), tetapi tidak ada nama variabel setelahnya.\n"
                            "💡 CONTOH YANG BENAR: orvonton.nebadon");
                    }
                }
                
                // Jika itu KURUNG SIKU ([...])
                else if (token_sekarang(p).jenis == TOKEN_KURUNG_S_B) {
                    ASTNode* node_akses = buat_node(AST_AKSES_ARRAY, p);
                    node_akses->kiri = simpul_kiri;
                    maju(p); // Lewati '['
                    node_akses->indeks_array = parse_ekspresi(p);
                    
                    // 🟢 MENGEMBALIKAN SMART HINT LAMA ANDA
                    harapkan_token(p, TOKEN_KURUNG_S_T, "Akses elemen array kehilangan kurung siku penutup ']'.\n💡 CONTOH: nama_variabel[1]");
                    
                    simpul_kiri = node_akses;
                }
            }
        }
        
        return simpul_kiri;
    }

    // 3. Tangkap Pembuatan Array Baru
    else if (t.jenis == TOKEN_KURUNG_S_B) {
        simpul_kiri = buat_node(AST_STRUKTUR_ARRAY, p);
        maju(p);
        while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_S_T) {
            ASTNode* elemen = parse_ekspresi(p); 
            if (elemen) tambah_anak(simpul_kiri, elemen); 
            if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p);
        }
        // 🟢 SUNTIKAN SIKU (ARRAY LITERAL) YANG BENAR!
        harapkan_token(p, TOKEN_KURUNG_S_T, "Kehilangan kurung siku penutup ']' pada pembuatan Array.\n💡 SOLUSI: Pastikan daftar elemen Anda ditutup dengan benar."); 
        return simpul_kiri;
    }

    // --- SUNTIKAN BARU: Tangkap Pembuatan Objek JSON {} ---
    else if (t.jenis == TOKEN_KURUNG_K_B) {
        // Hapus logika AST_PASANGAN_KUNCI_NILAI yang lama,
        // Cukup panggil fungsi helper yang sudah Anda buat di atas:
        return parse_struktur_objek(p); 
    }

    // PENANGKAP OPERATOR TERNARI (Di letakkan paling bawah agar tidak tertelan)
    else if (t.jenis == TOKEN_KARMA && strcmp(t.isi, "jika") == 0) {
        maju(p); // Lewati 'jika'
        ASTNode* node_ternari = buat_node(AST_TERNARI, p);
        
        // 1. Tangkap syarat kondisinya (WAJIB PAKAI parse_syarat_logika!)
        node_ternari->syarat = parse_syarat_logika(p);
        
        // 2. Wajib ada 'maka'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
        else kiamat_sintaksis(p, "Ternari cacat!", "Kehilangan 'maka' setelah kondisi pada operasi sebaris.");
        
        // 3. Tangkap nilai jika BENAR
        node_ternari->kiri = parse_ekspresi(p);
        
        // 4. Wajib ada 'lain'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "lain") == 0) maju(p);
        else kiamat_sintaksis(p, "Ternari cacat!", "Kehilangan 'lain' pada operasi sebaris. Ternari harus memiliki jalan alternatif.");
        
        // 5. Tangkap nilai jika SALAH
        node_ternari->kanan = parse_ekspresi(p);
        
        return node_ternari;
    }

    return NULL;
}

// ==========================================================
// TINGKAT 1.5: Kasta Pangkat (^) - Kasta Terkuat di Bawah Kurung
// ==========================================================
ASTNode* parse_pangkat(Parser* p) {
    ASTNode* simpul_kiri = parse_nilai_dasar(p); // Ambil angka/kurung murni
    
    // Jika melihat simbol pangkat '^'
    while (token_sekarang(p).jenis == TOKEN_OPERATOR && 
           strcmp(token_sekarang(p).isi, "^") == 0) {
        
        Token t_op = token_sekarang(p);
        maju(p); // Lewati simbol '^'
        
        ASTNode* simpul_matematika = buat_node(AST_OPERASI_MATEMATIKA, p);
        simpul_matematika->operator_math = strdup(t_op.isi);
        simpul_matematika->kiri = simpul_kiri;
        
        // Panggil dirinya sendiri (Rekursif) agar pangkat dihitung dari kanan-ke-kiri
        // Contoh: 2 ^ 3 ^ 2 = 2 ^ (3 ^ 2) = 512
        simpul_matematika->kanan = parse_pangkat(p); 
        
        simpul_kiri = simpul_matematika; 
    }
    return simpul_kiri;
}

// ==========================================================
// TINGKAT 2: Kasta Kuat (*, /, :, //, %)
// ==========================================================
ASTNode* parse_faktor(Parser* p) {
    ASTNode* simpul_kiri = parse_pangkat(p);
    
    // Hitung Kiri ke Kanan selama menemukan operator kuat
    while (token_sekarang(p).jenis == TOKEN_OPERATOR && 
          (strcmp(token_sekarang(p).isi, "*") == 0 || 
           strcmp(token_sekarang(p).isi, "/") == 0 ||
           strcmp(token_sekarang(p).isi, ":") == 0 ||
           strcmp(token_sekarang(p).isi, "//") == 0 ||
           strcmp(token_sekarang(p).isi, "%") == 0)) {
        
        Token t_op = token_sekarang(p);
        maju(p); 
        
        ASTNode* simpul_matematika = buat_node(AST_OPERASI_MATEMATIKA, p);
        simpul_matematika->operator_math = strdup(t_op.isi);
        simpul_matematika->kiri = simpul_kiri;
        simpul_matematika->kanan = parse_pangkat(p); 
        
        simpul_kiri = simpul_matematika; 
    }
    return simpul_kiri;
}

// ==========================================================
// TINGKAT 3: Kasta Lemah (+, -) 
// ==========================================================
ASTNode* parse_penjumlahan(Parser* p) { 
    ASTNode* simpul_kiri = parse_faktor(p); 
    
    while (token_sekarang(p).jenis == TOKEN_OPERATOR && 
          (strcmp(token_sekarang(p).isi, "+") == 0 || 
           strcmp(token_sekarang(p).isi, "-") == 0)) {
        
        Token t_op = token_sekarang(p);
        maju(p); 
        
        ASTNode* simpul_matematika = buat_node(AST_OPERASI_MATEMATIKA, p);
        simpul_matematika->operator_math = strdup(t_op.isi);
        simpul_matematika->kiri = simpul_kiri;
        simpul_matematika->kanan = parse_faktor(p); 
        simpul_kiri = simpul_matematika;
    }
    return simpul_kiri;
}

// ==========================================================
// TINGKAT 3.5: Kasta Pipa Aliran (|>) -> "Sihir Gula Sintaksis"
// ==========================================================
ASTNode* parse_pipa(Parser* p) {
    ASTNode* simpul_kiri = parse_penjumlahan(p); 
    
    while (token_sekarang(p).jenis == TOKEN_PIPA) {
        maju(p); // Lewati '|>'
        
        // Tangkap fungsi tujuan di kanannya
        ASTNode* simpul_kanan = parse_penjumlahan(p); 
        
        // Pipa wajib dialirkan ke dalam sebuah Fungsi!
        if (simpul_kanan == NULL || simpul_kanan->jenis != AST_PANGGILAN_FUNGSI) { // <--- TAMBAH == NULL
            kiamat_sintaksis(p, "Tujuan Pipa Aliran '|>' wajib berupa fungsi!", 
                "Pipa aliran digunakan untuk memasukkan data ke dalam fungsi.\n"
                "Contoh yang sah:\n"
                "\"halo\" |> huruf_besar() |> ketik()");
        }
        
        // Sihir terjadi di sini: Masukkan KIRI ke dalam fungsi KANAN secara diam-diam!
        sisip_anak_pertama(simpul_kanan, simpul_kiri);
        
        simpul_kiri = simpul_kanan; // Jadikan hasil ini sebagai "kiri" untuk pipa selanjutnya
    }
    return simpul_kiri;
}

// ==========================================================
// TINGKAT 4: Kasta Mutasi / Penugasan Ulang (=) -> FUNGSI UTAMA
// ==========================================================
ASTNode* parse_ekspresi(Parser* p) {
    ASTNode* simpul_kiri = parse_pipa(p);
    
    // Jika menemukan tanda '=' setelah variabel / akses domain
    if (token_sekarang(p).jenis == TOKEN_ASSIGN || 
       (token_sekarang(p).jenis == TOKEN_OPERATOR && strcmp(token_sekarang(p).isi, "=") == 0)) {
        
        maju(p); // Lewati '=' (kita tidak butuh menyimpan token t_op lagi)
        
        ASTNode* simpul_mutasi = buat_node(AST_OPERASI_MATEMATIKA, p);
        simpul_mutasi->operator_math = strdup("=");
        simpul_mutasi->kiri = simpul_kiri; // Target (misal: entitas.level1)
        simpul_mutasi->kanan = parse_ekspresi(p); // Nilai yang baru (misal: {})
        
        return simpul_mutasi;
    }
    return simpul_kiri;
}

// ==========================================================
// [FASE 1] PENANGKAP LOGIKA MAJEMUK (dan, atau, bukan)
// ==========================================================
ASTNode* parse_syarat_logika(Parser* p) {
    ASTNode* simpul_kiri = NULL;
    
    // 1. PENANGKAP SIHIR 'bukan' DI AWAL SYARAT
    if (token_sekarang(p).jenis == TOKEN_LOGIKA && strcmp(token_sekarang(p).isi, "bukan") == 0) {
        simpul_kiri = buat_node(AST_OPERASI_BUKAN, p);
        maju(p); // lewati kata 'bukan'
        
        // Tangkap kondisi setelah kata 'bukan' (misal: x == 2)
        ASTNode* kondisi = buat_node(AST_KONDISI, p);
        kondisi->kiri = parse_ekspresi(p);
        if (token_sekarang(p).jenis == TOKEN_PEMBANDING) {
            kondisi->pembanding = strdup(token_sekarang(p).isi);
            maju(p);
            kondisi->kanan = parse_ekspresi(p);
        }
        simpul_kiri->kiri = kondisi; // Masukkan kondisi ke dalam pelukan 'bukan'
    } else {
        // 2. KONDISI NORMAL TANPA BUKAN
        simpul_kiri = buat_node(AST_KONDISI, p);
        simpul_kiri->kiri = parse_ekspresi(p); 
        
        if (token_sekarang(p).jenis == TOKEN_PEMBANDING) {
            simpul_kiri->pembanding = strdup(token_sekarang(p).isi);
            maju(p); // lewati '==' atau '>'
            simpul_kiri->kanan = parse_ekspresi(p);
        }
    }
 
    // 3. Cek apakah ada kata 'dan' atau 'atau' setelahnya!
    while (token_sekarang(p).jenis == TOKEN_LOGIKA && 
          (strcmp(token_sekarang(p).isi, "dan") == 0 || strcmp(token_sekarang(p).isi, "atau") == 0)) {
        
        Token t_logika = token_sekarang(p);
        maju(p); // Lewati kata 'dan' / 'atau'
        
        ASTNode* simpul_logika = buat_node(AST_OPERASI_LOGIKA, p);
        simpul_logika->pembanding = strdup(t_logika.isi); // Simpan "dan" / "atau"
        simpul_logika->kiri = simpul_kiri; // Kiri adalah kondisi sebelumnya
        
        // Kanan adalah kondisi berikutnya (misal: uang > 100)
        ASTNode* kondisi_kanan = buat_node(AST_KONDISI, p);
        kondisi_kanan->kiri = parse_ekspresi(p);
        if (token_sekarang(p).jenis == TOKEN_PEMBANDING) {
            kondisi_kanan->pembanding = strdup(token_sekarang(p).isi);
            maju(p);
            kondisi_kanan->kanan = parse_ekspresi(p);
        }
        simpul_logika->kanan = kondisi_kanan;
        
        simpul_kiri = simpul_logika; // Timpa untuk mendukung rantai panjang (A dan B atau C)
    }
    
    return simpul_kiri;
}

// Mengurai Satu Baris Perintah Penuh (Statement)
ASTNode* parse_pernyataan(Parser* p) {
    Token t = token_sekarang(p);

    // --- PENANGKAP TATA KRAMA (HEADER & KONTROL) ---
    if (t.jenis == TOKEN_HEADER) {
        ASTNode* node = buat_node(AST_DEKLARASI_DATANG, p);
        maju(p); // lewati 'datang'
        return node;
    }
    
    if (t.jenis == TOKEN_PRAGMA) {
        ASTNode* node = buat_node(AST_PRAGMA_MEMORI, p);
        node->nilai_teks = strdup(t.isi); // Simpan info apakah dinamis/statis
        maju(p); 
        return node;
    }
    
    // PENANGKAP SIHIR PERCOCOKAN POLA (SWITCH-CASE) ---
    if (t.jenis == TOKEN_COCOKKAN) {
        ASTNode* node = buat_node(AST_COCOKKAN, p);
        maju(p); // lewati 'cocokkan'

        // 1. Tangkap target yang mau dicocokkan (misal: status_server)
        node->kiri = parse_ekspresi(p);

        // 2. Pastikan ada 'maka' pembuka
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
        else kiamat_sintaksis(p, "Kehilangan 'maka'!", "Gunakan 'maka' setelah target cocokkan.");

        // 3. Baca isi seluruh percocokan
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            
            // A. Jika menemukan KASUS
            if (t_cek.jenis == TOKEN_KASUS) {
                maju(p); // lewati 'kasus'
                ASTNode* node_kasus = buat_node(AST_KASUS, p);
                
                node_kasus->kiri = parse_ekspresi(p); // Nilai kasus (misal: "Menyala")
                if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
                
                // Tangkap perintah di dalam kasus
                node_kasus->blok_maka = buat_node(AST_PROGRAM, p);
                while (token_sekarang(p).jenis != TOKEN_EOF) {
                    Token t_dalam = token_sekarang(p);
                    if (t_dalam.jenis == TOKEN_KARMA && strcmp(t_dalam.isi, "putus") == 0) {
                        maju(p); // lewati 'putus' untuk kasus ini
                        break;
                    }
                    ASTNode* stmt = parse_pernyataan(p);
                    if (stmt) tambah_anak(node_kasus->blok_maka, stmt);
                }
                tambah_anak(node, node_kasus); // Masukkan kasus ke wadah anak_anak
            } 
            // B. Jika menemukan LAIN (Default Case)
            else if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "lain") == 0) {
                maju(p); // lewati 'lain'
                if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
                
                node->blok_lain = buat_node(AST_PROGRAM, p);
                while (token_sekarang(p).jenis != TOKEN_EOF) {
                    Token t_dalam = token_sekarang(p);
                    if (t_dalam.jenis == TOKEN_KARMA && strcmp(t_dalam.isi, "putus") == 0) {
                        maju(p); // lewati 'putus' untuk blok lain
                        break;
                    }
                    ASTNode* stmt = parse_pernyataan(p);
                    if (stmt) tambah_anak(node->blok_lain, stmt);
                }
            }
            // C. Jika menemukan PUTUS (Penutup Mutlak)
            else if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) {
                maju(p); // lewati 'putus' utama
                break;
            } else {
                kiamat_sintaksis(p, "Struktur cocokkan cacat!", "Harap gunakan 'kasus [nilai] maka', 'lain maka', dan akhiri setiap blok dengan 'putus'.");
            }
        }
        return node;
    }

    // --- PENANGKAP KONTROL (pergi, henti, terus) ---
    if (t.jenis == TOKEN_KONTROL) {
        if (strcmp(t.isi, "pergi") == 0) {
            ASTNode* node = buat_node(AST_PERINTAH_PERGI, p);
            maju(p); return node;
        }
        else if (strcmp(t.isi, "henti") == 0) {
            ASTNode* node = buat_node(AST_PERINTAH_HENTI, p); 
            maju(p); return node;
        }

        else if (strcmp(t.isi, "terus") == 0) {
            ASTNode* node = buat_node(AST_PERINTAH_TERUS, p);
            maju(p); return node;
        }
    }

    // 1. Apakah ini perintah KETIK? (ketik("Halo"))
    if (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "ketik") == 0) {
        ASTNode* node = buat_node(AST_PERINTAH_KETIK, p);
        maju(p); // lewati kata 'ketik'
        // 🟢 SUNTIKAN 1 (Buka)
        harapkan_token(p, TOKEN_KURUNG_B, "Fungsi 'ketik' membutuhkan tanda '(' untuk memulai argumen.");
        
        node->kanan = parse_ekspresi(p); // Tangkap apapun yang ada di dalam kurung
        
        // 🟢 SUNTIKAN 1 (Tutup)
        harapkan_token(p, TOKEN_KURUNG_T, "Fungsi 'ketik' kehilangan tanda kurung tutup ')'.\n💡 SOLUSI: Pastikan Anda menutup fungsi dengan benar.");
        return node;
    }
    
    // 2. Apakah ini DEKLARASI? (takdir.soft nama = ...)
    if (t.jenis == TOKEN_TAKDIR) {
        ASTNode* node = buat_node(AST_DEKLARASI_TAKDIR, p);
        node->nilai_teks = strdup(t.isi);
        maju(p); // lewati 'takdir.soft'
        
        // Simpan nama variabel di simpul Kiri
        Token t_nama = token_sekarang(p);
        node->kiri = buat_node(AST_IDENTITAS, p);
        node->kiri->nilai_teks = strdup(t_nama.isi);
        maju(p); // lewati nama variabel
        
        maju(p); // lewati tanda '='
        
        // Simpan nilai di simpul Kanan
        node->kanan = parse_ekspresi(p);
        return node;
    }

    // 3. Apakah ini HUKUM KARMA? (jika ... maka ... lain ... putus)
    if (t.jenis == TOKEN_KARMA && strcmp(t.isi, "jika") == 0) {
        ASTNode* node = buat_node(AST_HUKUM_KARMA, p);
        maju(p); // lewati kata 'jika'

        // A. Tangkap Syarat
        node->syarat = parse_syarat_logika(p);

        // B. Lewati kata 'maka'
        Token t_maka = token_sekarang(p);
        if (t_maka.jenis == TOKEN_KARMA && strcmp(t_maka.isi, "maka") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Struktur percabangan kehilangan gerbang 'maka'!", 
                             "Sepertinya Anda melewatkan kata kunci 'maka'.\n"
                             "Silakan tambahkan setelah kondisi 'jika'.\n"
                             "Contoh: jika x == 1 maka");
        }

        // C. Tangkap isi blok MAKA
        node->blok_maka = buat_node(AST_PROGRAM, p);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            if (t_cek.jenis == TOKEN_KARMA && (strcmp(t_cek.isi, "lain") == 0 || strcmp(t_cek.isi, "putus") == 0)) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_maka, stmt);
        }

        // D. Cek apakah ada blok LAIN
        Token t_lain = token_sekarang(p);
        if (t_lain.jenis == TOKEN_KARMA && strcmp(t_lain.isi, "lain") == 0) {
            maju(p); // lewati kata 'lain'
            // 🟢 HUKUM MUTLAK DISIPLIN MILITER!
            if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) {
                maju(p); // lewati 'maka'
            } else {
                kiamat_sintaksis(p, "Pelanggaran Tata Bahasa Blok!", "Kata 'lain' sebagai pembuka blok perintah WAJIB diikuti oleh kata 'maka'.");
            }
            node->blok_lain = buat_node(AST_PROGRAM, p);
            while (token_sekarang(p).jenis != TOKEN_EOF) {
                Token t_cek = token_sekarang(p);
                if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) break;
                ASTNode* stmt = parse_pernyataan(p);
                if (stmt) tambah_anak(node->blok_lain, stmt);
            }
        }

        // E. WAJIB ADA KATA 'PUTUS' (HUKUM DISIPLIN MUTLAK)
        Token t_putus = token_sekarang(p);
        if (t_putus.jenis == TOKEN_KARMA && strcmp(t_putus.isi, "putus") == 0) {
            maju(p); // lewati 'putus' dengan selamat
        } else {
            kiamat_sintaksis(p, "Hukum Karma kehilangan ujung ('putus')!", 
                             "Sebuah blok 'jika' atau 'lain' wajib ditutup dengan kata kunci 'putus'.\n"
                             "Jika tidak, mesin akan menganggap seluruh sisa file berada di dalam blok ini.");
        }

        return node;
    }

    // --- PENANGKAP HUKUM KECUALI (UNLESS) ---
    if (t.jenis == TOKEN_KARMA && strcmp(t.isi, "kecuali") == 0) {
        ASTNode* node = buat_node(AST_KECUALI, p);
        maju(p); // lewati 'kecuali'
        
        node->syarat = parse_syarat_logika(p);
        
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
        else kiamat_sintaksis(p, "Hukum Kecuali cacat!", "Kehilangan kata 'maka' setelah syarat.");
        
        node->blok_maka = buat_node(AST_PROGRAM, p);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_dalam = token_sekarang(p);
            if (t_dalam.jenis == TOKEN_KARMA && strcmp(t_dalam.isi, "lain") == 0) {
                maju(p); // lewati 'lain'
                if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) {
                    maju(p);
                } else {
                    kiamat_sintaksis(p, "Pelanggaran Tata Bahasa Blok!", "Kata 'lain' sebagai pembuka blok perintah WAJIB diikuti oleh kata 'maka'.");
                }
                node->blok_lain = buat_node(AST_PROGRAM, p);
                continue;
            }
            if (t_dalam.jenis == TOKEN_KARMA && strcmp(t_dalam.isi, "putus") == 0) {
                maju(p); // lewati 'putus'
                break;
            }
            
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) {
                if (node->blok_lain) tambah_anak(node->blok_lain, stmt);
                else tambah_anak(node->blok_maka, stmt);
            }
        }
        return node;
    }

    // --- PENANGKAP SIKLUS SELAMA (WHILE LOOP) ---
    if (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "siklus") == 0) {
        maju(p); // Lewati 'siklus'
        
        if (token_sekarang(p).jenis == TOKEN_IDENTITAS && strcmp(token_sekarang(p).isi, "selama") == 0) maju(p); // lewati 'selama'
        else kiamat_sintaksis(p, "Siklus cacat!", "Gunakan 'siklus selama [kondisi] maka'.");

        ASTNode* node = buat_node(AST_HUKUM_SIKLUS, p);
        
        // 1. Tangkap syarat kondisinya
        node->syarat = parse_syarat_logika(p);
        
        // 2. Wajib ada 'maka' (Ini sama fungsinya seperti '{' di C)
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);
        else kiamat_sintaksis(p, "Siklus cacat!", "Kehilangan 'maka' setelah syarat.");
        
        // 3. Tangkap isi ruangan siklus (Aman menelan anak-anak di dalamnya)
        node->blok_siklus = buat_node(AST_PROGRAM, p);
        int ketemu_putus = 0; // 🟢 ALARM PENANDA

        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            
            // 4. Inilah gerbang penutupnya, sama seperti '}' di C!
            if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) {
                maju(p); // lewati 'putus' khusus milik SIKLUS
                ketemu_putus = 1; // 🟢 ALARM DIMATIKAN KARENA AMAN
                break;
            }
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_siklus, stmt);
        }

        // 🟢 EKSEKUSI ALARM KIAMAT
        if (!ketemu_putus) {
            kiamat_sintaksis(p, "Hukum Siklus kehilangan ujung ('putus')!", 
                             "Sebuah blok siklus wajib ditutup dengan kata kunci 'putus'.");
        }
        return node;
    }

    // 4. Apakah ini HUKUM SIKLUS? (effort X kali maka ... putus)
    // --- PENANGKAP SIKLUS EFFORT (DENGAN KIAMAT PRESISI) ---
    if (t.jenis == TOKEN_SIKLUS && strcmp(t.isi, "effort") == 0) {
        maju(p); // lewati kata 'effort'
        
        // 1. Tangkap tolok ukurnya (bisa angka atau teks interval)
        ASTNode* batas = parse_ekspresi(p); 
        
        // 🚨 KIAMAT SINTAKSIS JIKA KOSONG!
        if (batas == NULL) {
            kiamat_sintaksis(p, "Siklus 'effort' kehilangan tolok ukur!", 
                "UNUL tidak bisa melakukan effort tanpa batas yang jelas.\n"
                "Untuk siklus angka : effort 5 kali maka ...\n"
                "Untuk siklus waktu : effort \"2s\" maka ... (gunakan tanda kutip untuk s/m/h)\n"
                "Untuk milidetik    : effort 300 maka ... (tanpa satuan dan tanpa kutip)");
            return NULL;
        }

        ASTNode* node = NULL;
        
        // 2. Cek apakah ini siklus angka (pakai 'kali') atau waktu (tanpa 'kali')
        if (token_sekarang(p).jenis == TOKEN_SIKLUS && strcmp(token_sekarang(p).isi, "kali") == 0) {
            maju(p); // lewati 'kali'
            node = buat_node(AST_HUKUM_SIKLUS, p); // Catatan: pastikan namanya sesuai dengan enum Anda (AST_HUKUM_SIKLUS)
        } else {
            node = buat_node(AST_EFFORT_WAKTU, p);
        }
        
        node->batas_loop = batas;
        
        // 3. Wajib ada 'maka'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Pelanggaran Tata Bahasa Blok!", "Siklus 'effort' WAJIB dibuka dengan kata 'maka'.");
            return NULL;
        }
        
        // 4. Tangkap isi ruangan
        node->blok_siklus = buat_node(AST_PROGRAM, p);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "putus") == 0) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_siklus, stmt);
        }
        
        // 5. Wajib ada 'putus'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "putus") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Siklus tak berujung!", "Blok perintah WAJIB diakhiri dengan kata kunci 'putus'.");
            return NULL;
        }
        
        return node;
    }

    // --- PENANGKAP JADWAL (CRON-JOB) DENGAN KIAMAT PRESISI ---
    if (t.jenis == TOKEN_JADWAL) {
        maju(p); // lewati kata 'jadwal'
        
        // 1. Tangkap waktu target (misal: "15:30:00")
        ASTNode* waktu_target = parse_ekspresi(p); 
        
        // 🚨 KIAMAT SINTAKSIS JIKA WAKTU KOSONG!
        if (waktu_target == NULL) {
            kiamat_sintaksis(p, "Perintah 'jadwal' kehilangan target waktu!", 
                "Anda harus menentukan kapan jadwal ini dieksekusi.\n"
                "Contoh penggunaan yang sah:\n"
                "jadwal \"15:30:00\" maka\n"
                "   ketik(\"Waktu tiba!\")\n"
                "putus");
            return NULL;
        }

        ASTNode* node = buat_node(AST_JADWAL, p);
        node->kiri = waktu_target;
        
        // 2. Wajib ada 'maka'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Pelanggaran Tata Bahasa Blok!", "Perintah 'jadwal' WAJIB dibuka dengan kata 'maka'.");
            return NULL;
        }
        
        // 3. Tangkap isi ruangan
        node->blok_maka = buat_node(AST_PROGRAM, p);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "putus") == 0) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_maka, stmt);
        }
        
        // 4. Wajib ada 'putus'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "putus") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Jadwal tak berujung!", "Blok perintah WAJIB diakhiri dengan kata kunci 'putus'.");
            return NULL;
        }
        
        return node;
    }

    // --- PENANGKAP PENCIPTAAN FUNGSI ---
    // Sintaksis: ciptakan fungsi nama(x, y) atau nama(x = 10) maka ... putus
    if (t.jenis == TOKEN_CIPTAKAN) {
        maju(p); // lewati 'ciptakan'
        if (token_sekarang(p).jenis == TOKEN_FUNGSI) maju(p); // lewati 'fungsi'

        ASTNode* node = buat_node(AST_DEKLARASI_FUNGSI, p);
        
        // Tangkap nama fungsi
        if (token_sekarang(p).jenis == TOKEN_IDENTITAS) {
            node->nilai_teks = strdup(token_sekarang(p).isi);
            maju(p);
        }

        // Tangkap Parameter
        if (token_sekarang(p).jenis == TOKEN_KURUNG_B) {
            maju(p); // lewati '('
            while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_T) {
                ASTNode* param = parse_ekspresi(p); 
                tambah_anak(node, param);
                if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p); // lewati ','
            }
            
            char pesan_param[256];
            snprintf(pesan_param, sizeof(pesan_param), 
                "Deklarasi fungsi '%s' kehilangan tanda kurung tutup ')'.\n"
                "💡 SOLUSI: Pastikan parameter fungsi ditutup sebelum kata 'maka'.", 
                node->nilai_teks ? node->nilai_teks : "kustom");
                
            harapkan_token(p, TOKEN_KURUNG_T, pesan_param);
        }

        // 🟢 SUNTIKAN RANJAU 1: WAJIB ADA 'MAKA'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Deklarasi fungsi kehilangan gerbang 'maka'!", 
                             "Untuk menciptakan fungsi, Anda wajib menggunakan kata 'maka' sebelum isinya.\n"
                             "Contoh: ciptakan fungsi sapa(nama) maka");
        }

        // Tangkap isi blok fungsi (Tubuh Fungsi)
        node->blok_maka = buat_node(AST_PROGRAM, p);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_maka, stmt);
        }

        // 🟢 SUNTIKAN RANJAU 2: WAJIB ADA 'PUTUS'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "putus") == 0) {
            maju(p);
        } else {
            kiamat_sintaksis(p, "Tubuh fungsi tidak memiliki ujung ('putus')!", 
                             "Sebuah fungsi yang diciptakan wajib diakhiri dengan kata kunci 'putus'.\n"
                             "Jika tidak, mesin akan menelan seluruh sisa program Anda ke dalam fungsi ini.");
        }

        return node;
    }

    // --- PENANGKAP PERINTAH PULANG (RETURN) ---
    // Sintaksis: pulang "Sukses"
    if (t.jenis == TOKEN_PULANG) {
        ASTNode* node = buat_node(AST_PULANG, p);
        maju(p); // lewati 'pulang'
        
        // Tangkap nilai yang dikembalikan dan simpan di ranting kiri
        node->kiri = parse_ekspresi(p);
        
        return node;
    }


   // --- SIHIR BALIKAN ---
    if (strcmp(t.isi, "balikan") == 0) { 
        ASTNode* node = buat_node(AST_PERINTAH_BALIKAN, p);
        maju(p); // lewati kata 'balikan'
        node->kiri = parse_ekspresi(p); // Ambil target yang mau dibalikkan
        return node;
    }


    // --- PENANGKAP SIHIR JEDA (WAKTU) ---
    // (Catatan Penting: Untuk format bersatuan seperti 1s atau 1d, gunakan tanda kutip "1s" agar mesin memandangnya sebagai kesatuan parameter wujud waktu, bukan terpisah sebagai 1 dan s).
    if (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "jeda") == 0) {
        ASTNode* node = buat_node(AST_PERINTAH_JEDA, p);
        maju(p); // lewati kata 'jeda'
        
        // Tangkap ekspresi durasinya (Bisa angka 600, atau teks "1s")
        node->kiri = parse_ekspresi(p); 
        return node;
    }

    // --- PENANGKAP EKSPRESI BERDIRI SENDIRI (MUTASI & FUNGSI & PIPA) ---
    // Mengizinkan: `entitas = {}`, `dewa.nama = "Enki"`, atau `sapa()`
    if (t.jenis == TOKEN_IDENTITAS || t.jenis == TOKEN_TEKS || t.jenis == TOKEN_ANGKA || 
        t.jenis == TOKEN_KURUNG_B || t.jenis == TOKEN_KURUNG_S_B || t.jenis == TOKEN_KURUNG_K_B) {
        return parse_ekspresi(p);
    }

    // --- PENANGKAP PELEPASAN MEMORI BERDIRI SENDIRI (PASRAH VARIABEL) ---
    if (t.jenis == TOKEN_PASRAH) {
        ASTNode* node = buat_node(AST_PERINTAH_PASRAH, p);
        maju(p); // lewati kata 'pasrah'
        
        node->kiri = parse_ekspresi(p); // Ambil target variabel
        return node;
    }

    // ATM DARI BLUEPRINT: elif token[0] == 'SOWAN':
    // --- PENANGKAP SIHIR SOWAN ---
    if (t.jenis == TOKEN_SOWAN) { 
        ASTNode* node = buat_node(AST_PERINTAH_SOWAN, p);
        maju(p); // lewati kata 'sowan'
        
        Token t_target = token_sekarang(p);
        if (t_target.jenis == TOKEN_IDENTITAS) {
            // KEAJAIBAN KHUSUS SOWAN: Jika diketik tanpa kutip (misal: sowan matematika)
            // Paksa mesin membacanya sebagai teks literal agar tidak memicu Kernel Panic
            node->kiri = buat_node(AST_LITERAL_TEKS, p);
            node->kiri->nilai_teks = strdup(t_target.isi);
            maju(p);
        } else {
            // Jika menggunakan kutip ("matematika") atau ekspresi lain
            node->kiri = parse_ekspresi(p); 
        }
        
        return node;
    }

    // --- PENANGKAP UTAS GAIB (MULTITHREADING) ---
    if (t.jenis == TOKEN_UTAS) {
        ASTNode* node = buat_node(AST_UTAS, p);
        maju(p); // lewati kata 'utas' atau 'gaib'
        
        node->kiri = parse_ekspresi(p); // Tangkap panggilan fungsinya, misal: hitung_berat()
        return node;
    }

    // 5. Apakah ini HUKUM TABU? (coba maka ... tabu melanggar e maka ... tebus maka ... pasrah)
    if (t.jenis == TOKEN_COBA) {
        ASTNode* node = buat_node(AST_COBA_TABU, p);
        maju(p); // lewati kata 'coba'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

        // A. Tangkap isi blok COBA (TRY)
        node->kiri = buat_node(AST_PROGRAM, p);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            // Berhenti jika mulai masuk blok tabu, tebus, atau pasrah
            if (t_cek.jenis == TOKEN_TABU || t_cek.jenis == TOKEN_TEBUS || t_cek.jenis == TOKEN_PASRAH) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->kiri, stmt);
        }

        // B. Cek apakah ada blok TABU MELANGGAR (EXCEPT)
        if (token_sekarang(p).jenis == TOKEN_TABU) {
            maju(p); // lewati 'tabu'
            if (token_sekarang(p).jenis == TOKEN_MELANGGAR) {
                maju(p); // lewati 'melanggar'
                // Tangkap wadah error (misal nama variabel errornya)
                if (token_sekarang(p).jenis == TOKEN_IDENTITAS) {
                    node->nilai_teks = strdup(token_sekarang(p).isi);
                    maju(p);
                }
            }
            if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

            // Tangkap isi blok TABU
            node->kanan = buat_node(AST_PROGRAM, p);
            while (token_sekarang(p).jenis != TOKEN_EOF) {
                Token t_cek = token_sekarang(p);
                // Berhenti jika mulai masuk blok tebus atau pasrah
                if (t_cek.jenis == TOKEN_TEBUS || t_cek.jenis == TOKEN_PASRAH) break;
                ASTNode* stmt = parse_pernyataan(p);
                if (stmt) tambah_anak(node->kanan, stmt);
            }
        }

        // C. Cek apakah ada blok TEBUS (FINALLY)
        if (token_sekarang(p).jenis == TOKEN_TEBUS) {
            maju(p); // lewati 'tebus'
            if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

            // Tangkap isi blok TEBUS
            node->blok_tebus = buat_node(AST_PROGRAM, p);
            while (token_sekarang(p).jenis != TOKEN_EOF) {
                Token t_cek = token_sekarang(p);
                // Berhenti jika bertemu penutup pasrah
                if (t_cek.jenis == TOKEN_PASRAH) break;
                ASTNode* stmt = parse_pernyataan(p);
                if (stmt) tambah_anak(node->blok_tebus, stmt);
            }
        }

        // 🟢 Validasi Logika Try-Catch Mutlak
        if (node->kanan == NULL && node->blok_tebus == NULL) {
            kiamat_sintaksis(p, "Blok 'coba' tidak berguna karena tidak memiliki 'tabu' ataupun 'tebus'!", 
                "Hukum Tabu tidak lengkap! Sebuah blok 'coba' harus diikuti oleh minimal satu blok 'tabu' (untuk menangkap error) atau blok 'tebus' (untuk eksekusi akhir).\n"
                "Contoh yang benar:\ncoba maka\n   ...\ntabu melanggar e maka\n   ...\npasrah");
        }

        // D. Lewati kata 'pasrah' sebagai PENUTUP MUTLAK blok Hukum Tabu
        if (token_sekarang(p).jenis == TOKEN_PASRAH) {
            maju(p); 
        } else {
            kiamat_sintaksis(p, "Blok 'coba' kehilangan penutup 'pasrah'!", 
                "Sistem Hukum Tabu wajib diakhiri dengan kata kunci 'pasrah'.\n"
                "Struktur yang sah:\ncoba maka\n   ...\ntabu melanggar e maka\n   ...\npasrah");
        }

        return node;
    }

    if (t.jenis == TOKEN_TABU || t.jenis == TOKEN_TEBUS || t.jenis == TOKEN_PASRAH) {
        kiamat_sintaksis(p, "Kata kunci Hukum Tabu ditemukan berdiri sendiri!", 
            "Kata 'tabu', 'tebus', dan 'pasrah' tidak bisa digunakan sembarangan.\n"
            "Mereka adalah bagian dari Hukum Tabu dan harus selalu diawali dengan blok 'coba'.\n"
            "Struktur yang sah:\ncoba maka\n   ...\ntabu melanggar e maka\n   ...\npasrah");
    }
    
    if (t.jenis == TOKEN_KARMA && (strcmp(t.isi, "maka") == 0 || strcmp(t.isi, "lain") == 0)) {
        kiamat_sintaksis(p, "Kata kunci percabangan ditemukan tanpa induk 'jika'!", 
            "Kata 'maka' dan 'lain' hanya bisa digunakan sebagai pasangan dari perintah 'jika'.");
    }

    // ... (blok if untuk TOKEN_TAKDIR, TOKEN_KARMA, dll) ...

    // 🟢 SUNTIKAN KIAMAT MUTLAK (CATCH-ALL) 🟢
    // Jika kode sampai di titik ini, berarti token sama sekali tidak dikenali 
    // atau posisinya melanggar Hukum Enlil (misal: takdir*)
    char pesan_kiamat[256];
    snprintf(pesan_kiamat, sizeof(pesan_kiamat), "Sintaksis tersesat pada token: '%s'", t.isi);
        
    kiamat_sintaksis(p, pesan_kiamat, 
        "Mesin tidak memahami maksud dari ejaan ini.\n"
        "💡 PENYEBAB MUNGKIN:\n"
        "1. Ada karakter ilegal (seperti %s, *, @, atau °) di luar tanda kutip teks.\n"
        "2. Anda salah mengetik Hukum Takdir, Siklus, atau Karma.\n"
        "Mesin dihentikan untuk mencegah kerusakan dimensi!");
            
    // WAJIB: Hentikan program agar tidak infinite loop!
    exit(1); 
    return NULL;
}    

// Pintu Masuk Parser: Membaca seluruh file hingga ketemu EOF
ASTNode* parse_program(Parser* p) {
    ASTNode* program = buat_node(AST_PROGRAM, p);
    
    while (token_sekarang(p).jenis != TOKEN_EOF) {
        ASTNode* pernyataan = parse_pernyataan(p);
        if (pernyataan != NULL) {
            tambah_anak(program, pernyataan);
        }
    }
    
    return program;
}