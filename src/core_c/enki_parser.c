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
    
    for (int i = 0; i < node->jumlah_anak; i++) {
        bebaskan_ast(node->anak_anak[i]);
    }
    if (node->anak_anak) free(node->anak_anak);
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
    
    // Jika itu "String Literal"
    if (t.jenis == TOKEN_TEKS) {
        ASTNode* node = buat_node(AST_LITERAL_TEKS);
        node->nilai_teks = strdup(t.isi);
        maju(p);
        return node;
    }
    
    // Jika itu Kata / Identitas
    if (t.jenis == TOKEN_IDENTITAS) {
        // Cek apakah ini pemanggilan fungsi dengar()
        if (strcmp(t.isi, "dengar") == 0) {
            ASTNode* node = buat_node(AST_FUNGSI_DENGAR);
            maju(p); // lewati kata 'dengar'
            maju(p); // lewati '('
            maju(p); // lewati ')'
            return node;
        } else {
            // Jika bukan, berarti ini nama variabel (misal: nama_user)
            ASTNode* node = buat_node(AST_IDENTITAS);
            node->nilai_teks = strdup(t.isi);
            maju(p);
            return node;
        }
    }
    
    return NULL;
}

// Mengurai Satu Baris Perintah Penuh (Statement)
ASTNode* parse_pernyataan(Parser* p) {
    Token t = token_sekarang(p);

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