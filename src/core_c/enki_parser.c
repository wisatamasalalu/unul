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

// Deklarasi Hierarki Matematika Mutlak
ASTNode* parse_ekspresi(Parser* p);
ASTNode* parse_faktor(Parser* p);
ASTNode* parse_nilai_dasar(Parser* p);

// ==========================================================
// TINGKAT 1: Menangkap Nilai Murni & Kurung ()
// ==========================================================
ASTNode* parse_nilai_dasar(Parser* p) {
    Token t = token_sekarang(p);
    ASTNode* simpul_kiri = NULL;

    // [MISI 1] PRIORITAS KURUNG MATEMATIKA
    if (t.jenis == TOKEN_KURUNG_B) {
        maju(p); // Lewati '('
        simpul_kiri = parse_ekspresi(p); // Selam dimensi dalam kurung!
        if (token_sekarang(p).jenis == TOKEN_KURUNG_T) {
            maju(p); // Lewati ')'
        } else {
            printf("🚨 Bencana Sintaksis: Kurung buka '(' kehilangan pasangan penutupnya ')'!\n");
            exit(1);
        }
        return simpul_kiri;
    }
    
    // 1. Tangkap Teks atau Angka Murni
    else if (t.jenis == TOKEN_TEKS || t.jenis == TOKEN_ANGKA) {
        simpul_kiri = buat_node(AST_LITERAL_TEKS);
        simpul_kiri->nilai_teks = strdup(t.isi);
        maju(p);
        return simpul_kiri;
    }

    // 2. Tangkap Identitas (Variabel, Fungsi, dengar)
    else if (t.jenis == TOKEN_IDENTITAS) {
        if (strcmp(t.isi, "dengar") == 0) {
            simpul_kiri = buat_node(AST_FUNGSI_DENGAR);
            maju(p); maju(p); maju(p); 
        } else {
            simpul_kiri = buat_node(AST_IDENTITAS);
            simpul_kiri->nilai_teks = strdup(t.isi);
            maju(p); 
            
            // Cek Panggilan Fungsi
            if (token_sekarang(p).jenis == TOKEN_KURUNG_B) {
                simpul_kiri->jenis = AST_PANGGILAN_FUNGSI;
                maju(p); 
                while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_T) {
                    ASTNode* arg = parse_ekspresi(p); 
                    if (arg) tambah_anak(simpul_kiri, arg); 
                    if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p);
                }
                maju(p); 
            }
            // Cek Akses Array
            else if (token_sekarang(p).jenis == TOKEN_KURUNG_S_B) {
                ASTNode* node_akses = buat_node(AST_AKSES_ARRAY);
                node_akses->kiri = simpul_kiri;
                maju(p);
                node_akses->indeks_array = parse_ekspresi(p);
                maju(p);
                simpul_kiri = node_akses;
            }
        }
        return simpul_kiri;
    }

    // 3. Tangkap Pembuatan Array Baru
    else if (t.jenis == TOKEN_KURUNG_S_B) {
        simpul_kiri = buat_node(AST_STRUKTUR_ARRAY);
        maju(p);
        while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_S_T) {
            ASTNode* elemen = parse_ekspresi(p); 
            if (elemen) tambah_anak(simpul_kiri, elemen); 
            if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p);
        }
        maju(p); 
        return simpul_kiri;
    }

    return NULL;
}

// ==========================================================
// TINGKAT 2: Kasta Kuat (*, /, :, //, %)
// ==========================================================
ASTNode* parse_faktor(Parser* p) {
    ASTNode* simpul_kiri = parse_nilai_dasar(p);
    
    // Hitung Kiri ke Kanan selama menemukan operator kuat
    while (token_sekarang(p).jenis == TOKEN_OPERATOR && 
          (strcmp(token_sekarang(p).isi, "*") == 0 || 
           strcmp(token_sekarang(p).isi, "/") == 0 ||
           strcmp(token_sekarang(p).isi, ":") == 0 ||
           strcmp(token_sekarang(p).isi, "//") == 0 ||
           strcmp(token_sekarang(p).isi, "%") == 0)) {
        
        Token t_op = token_sekarang(p);
        maju(p); 
        
        ASTNode* simpul_matematika = buat_node(AST_OPERASI_MATEMATIKA);
        simpul_matematika->operator_math = strdup(t_op.isi);
        simpul_matematika->kiri = simpul_kiri;
        simpul_matematika->kanan = parse_nilai_dasar(p); 
        
        simpul_kiri = simpul_matematika; 
    }
    return simpul_kiri;
}

// ==========================================================
// TINGKAT 3: Kasta Lemah (+, -) -> FUNGSI UTAMA
// ==========================================================
ASTNode* parse_ekspresi(Parser* p) {
    ASTNode* simpul_kiri = parse_faktor(p); // Panggil kasta kuat dulu!
    
    // Hitung Kiri ke Kanan selama menemukan + atau -
    while (token_sekarang(p).jenis == TOKEN_OPERATOR && 
          (strcmp(token_sekarang(p).isi, "+") == 0 || 
           strcmp(token_sekarang(p).isi, "-") == 0)) {
        
        Token t_op = token_sekarang(p);
        maju(p); 
        
        ASTNode* simpul_matematika = buat_node(AST_OPERASI_MATEMATIKA);
        simpul_matematika->operator_math = strdup(t_op.isi);
        simpul_matematika->kiri = simpul_kiri;
        simpul_matematika->kanan = parse_faktor(p); 
        
        simpul_kiri = simpul_matematika;
    }
    return simpul_kiri;
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
    
    // --- PENANGKAP KONTROL (pergi, henti, terus) ---
    if (t.jenis == TOKEN_KONTROL) {
        if (strcmp(t.isi, "pergi") == 0) {
            ASTNode* node = buat_node(AST_PERINTAH_PERGI);
            maju(p); return node;
        }
        else if (strcmp(t.isi, "henti") == 0) {
            // Kita buat node khusus untuk HENTI (Break) jika belum ada, 
            // atau sementara gunakan AST_TIDAK_DIKENAL dengan nilai "henti"
            ASTNode* node = buat_node(AST_TIDAK_DIKENAL); 
            node->nilai_teks = strdup("henti");
            maju(p); return node;
        }
        else if (strcmp(t.isi, "terus") == 0) {
            ASTNode* node = buat_node(AST_PERINTAH_TERUS);
            maju(p); return node;
        }
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

    // --- PENANGKAP PENCIPTAAN FUNGSI ---
    // Sintaksis: ciptakan fungsi nama(x, y) maka ... putus
    if (t.jenis == TOKEN_CIPTAKAN) {
        maju(p); // lewati 'ciptakan'
        if (token_sekarang(p).jenis == TOKEN_FUNGSI) maju(p); // lewati 'fungsi'

        ASTNode* node = buat_node(AST_DEKLARASI_FUNGSI);
        
        // Tangkap nama fungsi
        if (token_sekarang(p).jenis == TOKEN_IDENTITAS) {
            node->nilai_teks = strdup(token_sekarang(p).isi);
            maju(p);
        }

        // Tangkap Parameter: (x, y)
        if (token_sekarang(p).jenis == TOKEN_KURUNG_B) {
            maju(p); // lewati '('
            while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_T) {
                if (token_sekarang(p).jenis == TOKEN_IDENTITAS) {
                    ASTNode* param = buat_node(AST_IDENTITAS);
                    param->nilai_teks = strdup(token_sekarang(p).isi);
                    tambah_anak(node, param); // Simpan daftar parameter ke anak_anak
                    maju(p);
                }
                if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p); // lewati ','
            }
            maju(p); // lewati ')'
        }

        // Lewati 'maka'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

        // Tangkap isi blok fungsi (Tubuh Fungsi)
        node->blok_maka = buat_node(AST_PROGRAM);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            if (t_cek.jenis == TOKEN_KARMA && strcmp(t_cek.isi, "putus") == 0) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->blok_maka, stmt);
        }

        // Lewati 'putus'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "putus") == 0) maju(p);

        return node;
    }

    // --- PENANGKAP PERINTAH PULANG (RETURN) ---
    // Sintaksis: pulang "Sukses"
    if (t.jenis == TOKEN_PULANG) {
        ASTNode* node = buat_node(AST_PULANG);
        maju(p); // lewati 'pulang'
        
        // Tangkap nilai yang dikembalikan dan simpan di ranting kiri
        node->kiri = parse_ekspresi(p);
        
        return node;
    }

    // --- PENANGKAP PANGGILAN FUNGSI BERDIRI SENDIRI ---
    // Contoh: jika user menulis sapa_dunia() tanpa ditampung ke variabel
    if (t.jenis == TOKEN_IDENTITAS) {
        // Intip ke depan 1 langkah
        Token t_next = p->tokens.data[p->kursor + 1];
        if (t_next.jenis == TOKEN_KURUNG_B) {
            // Jika ada kurung, ini mutlak panggilan fungsi! Lempar ke parse_ekspresi
            return parse_ekspresi(p);
        }
    }

    // ATM DARI BLUEPRINT: elif token[0] == 'SOWAN':
    // (Jika Lexer Anda mendeteksi kata 'sowan' sebagai TOKEN_IDENTITAS)
    // --- PENANGKAP SIHIR SOWAN ---
    if (t.jenis == TOKEN_SOWAN) { 
        // printf("📡 [RADAR PARSER] Menangkap mantra SOWAN!\n");
        
        ASTNode* node = buat_node(AST_PERINTAH_SOWAN);
        maju(p); // lewati kata 'sowan'
        
        Token t_target = token_sekarang(p);
        if (t_target.jenis == TOKEN_TEKS) {
            node->nilai_teks = strdup(t_target.isi); 
            // printf("📡 [RADAR PARSER] Target dimensi: %s\n", node->nilai_teks);
            maju(p); // lewati nama file
        }
        return node;
    }

    // 5. Apakah ini HUKUM TABU? (coba maka ... tabu melanggar ... pasrah)
    if (t.jenis == TOKEN_COBA) {
        ASTNode* node = buat_node(AST_COBA_TABU);
        maju(p); // lewati kata 'coba'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

        // A. Tangkap isi blok COBA (simpan di node->kiri)
        node->kiri = buat_node(AST_PROGRAM);
        while (token_sekarang(p).jenis != TOKEN_EOF) {
            Token t_cek = token_sekarang(p);
            if (t_cek.jenis == TOKEN_TABU || t_cek.jenis == TOKEN_TEBUS || t_cek.jenis == TOKEN_PASRAH) break;
            ASTNode* stmt = parse_pernyataan(p);
            if (stmt) tambah_anak(node->kiri, stmt);
        }

        // B. Cek apakah ada blok TABU MELANGGAR
        if (token_sekarang(p).jenis == TOKEN_TABU) {
            maju(p); // lewati 'tabu'
            if (token_sekarang(p).jenis == TOKEN_MELANGGAR) {
                maju(p); // lewati 'melanggar'
                // Tangkap wadah error (misal: pesan_kiamat)
                if (token_sekarang(p).jenis == TOKEN_IDENTITAS) {
                    node->nilai_teks = strdup(token_sekarang(p).isi);
                    maju(p);
                }
            }
            if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

            // Tangkap isi blok TABU (simpan di node->kanan)
            node->kanan = buat_node(AST_PROGRAM);
            while (token_sekarang(p).jenis != TOKEN_EOF) {
                Token t_cek = token_sekarang(p);
                if (t_cek.jenis == TOKEN_TEBUS || t_cek.jenis == TOKEN_PASRAH) break;
                ASTNode* stmt = parse_pernyataan(p);
                if (stmt) tambah_anak(node->kanan, stmt);
            }
        }

        // (Opsional) Tambahkan penangkap untuk blok TEBUS jika diperlukan

        // C. Lewati kata 'pasrah'
        if (token_sekarang(p).jenis == TOKEN_PASRAH) maju(p);

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