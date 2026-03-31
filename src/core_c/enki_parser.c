#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_parser.h"

// =================================================================
// DAPUR MESIN PARSER (POHON LOGIKA)
// Menyusun token-token acak menjadi Rantai Arsitektur (AST)
// =================================================================

// --- 1. MANAJEMEN MEMORI POHON ---
Parser inisialisasi_parser(TokenArray tokens) {
    Parser p;
    p.tokens = tokens;
    p.kursor = 0; // Mulai baca dari token ke-0
    return p;
}

// Menciptakan ranting kosong baru di memori (Minta RAM via malloc)
ASTNode* buat_node(ASTJenis jenis) {
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
    
    free(node);
}

// --- 2. FUNGSI BANTUAN KURSOR ---
Token token_sekarang(Parser* p) {
    if (p->kursor >= p->tokens.jumlah) return p->tokens.data[p->tokens.jumlah - 1];
    return p->tokens.data[p->kursor];
}

void maju(Parser* p) {
    if (p->kursor < p->tokens.jumlah) p->kursor++;
}

// --- 3. LOGIKA PEMBEDAHAN (PARSING) ---
// Deklarasi awal agar fungsi saling kenal
ASTNode* parse_ekspresi(Parser* p);

// Mengurai Nilai (Sisi Kanan) -> Teks, Variabel, atau Panggilan Fungsi
ASTNode* parse_ekspresi(Parser* p) {
    Token t = token_sekarang(p);
    ASTNode* simpul_kiri = NULL;
    
    // Tangkap Teks Literal, Angka, atau Identitas (Sisi Kiri)
    if (t.jenis == TOKEN_TEKS || t.jenis == TOKEN_ANGKA || (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "dengar") != 0)) {
        simpul_kiri = buat_node(t.jenis == TOKEN_TEKS ? AST_LITERAL_TEKS : (t.jenis == TOKEN_ANGKA ? AST_LITERAL_TEKS : AST_IDENTITAS));
        simpul_kiri->nilai_teks = strdup(t.isi);
        maju(p);
    } else if (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "dengar") == 0) {
        simpul_kiri = buat_node(AST_FUNGSI_DENGAR);
        maju(p); maju(p); maju(p); // Lewati 'dengar', '(', ')'
    }

    // Cek apakah setelahnya ada OPERATOR (misal: + atau -)
    Token t_next = token_sekarang(p);
    if (t_next.jenis == TOKEN_OPERATOR) {
        ASTNode* simpul_matematika = buat_node(AST_OPERASI_MATEMATIKA);
        simpul_matematika->operator_math = strdup(t_next.isi);
        simpul_matematika->kiri = simpul_kiri; // Kiri masuk ke operasi
        maju(p); // Lewati lambang '+'
        
        // Evaluasi sisi Kanan secara rekursif
        simpul_matematika->kanan = parse_ekspresi(p); 
        return simpul_matematika;
    }

    return simpul_kiri; // Jika tidak ada operator, kembalikan nilai tunggal
}

// Mengurai Satu Baris Perintah Penuh (Statement)
ASTNode* parse_pernyataan(Parser* p) {
    Token t = token_sekarang(p);

    // --- PENANGKAP TATA KRAMA (HEADER & KONTROL) ---
    if (t.jenis == TOKEN_HEADER) {
        ASTNode* node = buat_node(AST_DEKLARASI_DATANG);
        maju(p); // lewati 'datang'
        return node;
    }
    
    if (t.jenis == TOKEN_PRAGMA) {
        ASTNode* node = buat_node(AST_PRAGMA_MEMORI);
        node->nilai_teks = strdup(t.isi); // Simpan info apakah dinamis/statis
        maju(p); 
        return node;
    }
    
    if (t.jenis == TOKEN_KONTROL && strcmp(t.isi, "pergi") == 0) {
        ASTNode* node = buat_node(AST_PERINTAH_PERGI);
        maju(p); // lewati 'pergi'
        return node;
    }

    // 1. Apakah ini perintah KETIK? (ketik("Halo"))
    if (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "ketik") == 0) {
        ASTNode* node = buat_node(AST_PERINTAH_KETIK);
        maju(p); // lewati kata 'ketik'
        maju(p); // lewati '('
        
        node->kanan = parse_ekspresi(p); // Tangkap apapun yang ada di dalam kurung
        
        maju(p); // lewati ')'
        return node;
    }
    
    // 2. Apakah ini DEKLARASI? (takdir.soft nama = ...)
    if (t.jenis == TOKEN_TAKDIR) {
        ASTNode* node = buat_node(AST_DEKLARASI_TAKDIR);
        maju(p); // lewati 'takdir.soft'
        
        // Simpan nama variabel di simpul Kiri
        Token t_nama = token_sekarang(p);
        node->kiri = buat_node(AST_IDENTITAS);
        node->kiri->nilai_teks = strdup(t_nama.isi);
        maju(p); // lewati nama variabel
        
        maju(p); // lewati tanda '='
        
        // Simpan nilai di simpul Kanan
        node->kanan = parse_ekspresi(p);
        return node;
    }

    // 3. Apakah ini HUKUM KARMA? (jika ... maka ... lain ... putus)
    if (t.jenis == TOKEN_KARMA && strcmp(t.isi, "jika") == 0) {
        ASTNode* node = buat_node(AST_HUKUM_KARMA);
        maju(p); // lewati kata 'jika'

        // A. Tangkap Syarat
        node->syarat = buat_node(AST_KONDISI);
        node->syarat->kiri = parse_ekspresi(p); // Sisi kiri (misal: umur)
        
        Token t_pembanding = token_sekarang(p);
        if (t_pembanding.jenis == TOKEN_PEMBANDING) {
            node->syarat->pembanding = strdup(t_pembanding.isi);
            maju(p); // lewati simbol '=='
            node->syarat->kanan = parse_ekspresi(p); // Sisi kanan (misal: 10)
        }

        // B. Lewati kata 'maka'
        Token t_maka = token_sekarang(p);
        if (t_maka.jenis == TOKEN_KARMA && strcmp(t_maka.isi, "maka") == 0) maju(p);

        // C. Tangkap isi blok MAKA
        node->blok_maka = buat_node(AST_PROGRAM);
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
            node->blok_lain = buat_node(AST_PROGRAM);
            while (token_sekarang(p).jenis != TOKEN_EOF) {
                Token t_cek = token_sekarang(p);
                if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) break;
                ASTNode* stmt = parse_pernyataan(p);
                if (stmt) tambah_anak(node->blok_lain, stmt);
            }
        }

        // E. Lewati kata 'putus'
        Token t_putus = token_sekarang(p);
        if (t_putus.jenis == TOKEN_KARMA && strcmp(t_putus.isi, "putus") == 0) maju(p);

        return node;
    }

    // 4. Apakah ini HUKUM SIKLUS? (effort X kali maka ... putus)
    if (t.jenis == TOKEN_SIKLUS && strcmp(t.isi, "effort") == 0) {
        ASTNode* node = buat_node(AST_HUKUM_SIKLUS);
        maju(p); // lewati kata 'effort'

        // Tangkap jumlah (bisa angka "5" atau variabel "batas")
        node->batas_loop = parse_ekspresi(p);

        // Lewati 'kali' dan 'maka'
        Token t_kali = token_sekarang(p);
        if (t_kali.jenis == TOKEN_SIKLUS && strcmp(t_kali.isi, "kali") == 0) maju(p);
        Token t_maka = token_sekarang(p);
        if (t_maka.jenis == TOKEN_KARMA && strcmp(t_maka.isi, "maka") == 0) maju(p);

        // Tangkap isi blok perulangan
        node->blok_siklus = buat_node(AST_PROGRAM);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_siklus, stmt);
        }

        // Lewati kata 'putus'
        Token t_putus = token_sekarang(p);
        if (t_putus.jenis == TOKEN_KARMA && strcmp(t_putus.isi, "putus") == 0) maju(p);

        return node;
    }

    // Jika bukan apa-apa (misal karakter asing/sisa baris kosong), maju 1 langkah agar tidak infinite loop
    maju(p);
    return NULL;
}

// Pintu Masuk Parser: Membaca seluruh file hingga ketemu EOF
ASTNode* parse_program(Parser* p) {
    ASTNode* program = buat_node(AST_PROGRAM);
    
    while (token_sekarang(p).jenis != TOKEN_EOF) {
        ASTNode* pernyataan = parse_pernyataan(p);
        if (pernyataan != NULL) {
            tambah_anak(program, pernyataan);
        }
    }
    
    return program;
}