#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_parser.h"

void kiamat_sintaksis(const char* pesan) {
    printf("🚨 KIAMAT SINTAKSIS: %s\n", pesan);
    
    // Abadikan kematian sebelum mati (sesuai tradisi Enki)
    FILE* file_diary = fopen("unul.diary", "a");
    if (file_diary) {
        fprintf(file_diary, "=== [KIAMAT SINTAKSIS] ===\n");
        fprintf(file_diary, "Pesan Alam     : %s\n", pesan);
        fprintf(file_diary, "Status         : GAGAL MERAKIT POHON LOGIKA (PARSER TERHENTI)\n");
        fprintf(file_diary, "========================\n\n");
        fclose(file_diary);
    }
    exit(1); // Tetap exit(1) karena kode tidak mungkin bisa dieksekusi jika cacat ketik
}

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
    node->blok_tebus = NULL;
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
    if (node->blok_tebus) bebaskan_ast(node->blok_tebus);
    
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
ASTNode* parse_penjumlahan(Parser* p);
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
            kiamat_sintaksis("Blok 'coba' kehilangan penutup 'pasrah'!");
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
            
            // 🔥 SUNTIKAN BARU: Cek Akses Domain Bersarang (Titik) 🔥
            // Memungkinkan pembacaan dewa.nama atau bahkan dewa.nama.depan
            while (token_sekarang(p).jenis == TOKEN_TITIK) {
                maju(p); // Lewati titik '.'
                Token t_anak = token_sekarang(p);
                if (t_anak.jenis == TOKEN_IDENTITAS) {
                    ASTNode* akses = buat_node(AST_AKSES_DOMAIN);
                    akses->kiri = simpul_kiri;              // Induknya (misal: dewa)
                    akses->nilai_teks = strdup(t_anak.isi); // Anaknya (misal: nama)
                    simpul_kiri = akses;
                    maju(p);
                }
            }
            
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

    // --- SUNTIKAN BARU: Tangkap Pembuatan Objek JSON {} ---
    else if (t.jenis == TOKEN_KURUNG_K_B) {
        simpul_kiri = buat_node(AST_STRUKTUR_OBJEK);
        maju(p); // Lewati '{'
        
        while (token_sekarang(p).jenis != TOKEN_EOF && token_sekarang(p).jenis != TOKEN_KURUNG_K_T) {
            Token t_kunci = token_sekarang(p);
            
            // Kunci objek bisa berupa TEKS ("nama") atau IDENTITAS (nama)
            if (t_kunci.jenis == TOKEN_TEKS || t_kunci.jenis == TOKEN_IDENTITAS) {
                ASTNode* pasangan = buat_node(AST_PASANGAN_KUNCI_NILAI);
                pasangan->pembanding = strdup(t_kunci.isi); // Pinjam 'pembanding' untuk simpan nama kunci
                maju(p); // Lewati kunci
                
                if (token_sekarang(p).jenis == TOKEN_TITIK_DUA) {
                    maju(p); // Lewati ':'
                } else {
                    kiamat_sintaksis("Wujud objek kehilangan titik dua ':' !");
                }
                
                pasangan->kiri = parse_ekspresi(p); // Tangkap nilainya
                tambah_anak(simpul_kiri, pasangan);
            }
            
            if (token_sekarang(p).jenis == TOKEN_KOMA) maju(p); // Lewati koma antar properti
        }
        maju(p); // Lewati '}'
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
// TINGKAT 3: Kasta Lemah (+, -) 
// ==========================================================
ASTNode* parse_penjumlahan(Parser* p) { 
    ASTNode* simpul_kiri = parse_faktor(p); 
    
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

// ==========================================================
// TINGKAT 4: Kasta Mutasi / Penugasan Ulang (=) -> FUNGSI UTAMA
// ==========================================================
ASTNode* parse_ekspresi(Parser* p) {
    ASTNode* simpul_kiri = parse_penjumlahan(p);
    
    // Jika menemukan tanda '=' setelah variabel / akses domain
    if (token_sekarang(p).jenis == TOKEN_ASSIGN || 
       (token_sekarang(p).jenis == TOKEN_OPERATOR && strcmp(token_sekarang(p).isi, "=") == 0)) {
        
        maju(p); // Lewati '=' (kita tidak butuh menyimpan token t_op lagi)
        
        ASTNode* simpul_mutasi = buat_node(AST_OPERASI_MATEMATIKA);
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
        simpul_kiri = buat_node(AST_OPERASI_BUKAN);
        maju(p); // lewati kata 'bukan'
        
        // Tangkap kondisi setelah kata 'bukan' (misal: x == 2)
        ASTNode* kondisi = buat_node(AST_KONDISI);
        kondisi->kiri = parse_ekspresi(p);
        if (token_sekarang(p).jenis == TOKEN_PEMBANDING) {
            kondisi->pembanding = strdup(token_sekarang(p).isi);
            maju(p);
            kondisi->kanan = parse_ekspresi(p);
        }
        simpul_kiri->kiri = kondisi; // Masukkan kondisi ke dalam pelukan 'bukan'
    } else {
        // 2. KONDISI NORMAL TANPA BUKAN
        simpul_kiri = buat_node(AST_KONDISI);
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
        
        ASTNode* simpul_logika = buat_node(AST_OPERASI_LOGIKA);
        simpul_logika->pembanding = strdup(t_logika.isi); // Simpan "dan" / "atau"
        simpul_logika->kiri = simpul_kiri; // Kiri adalah kondisi sebelumnya
        
        // Kanan adalah kondisi berikutnya (misal: uang > 100)
        ASTNode* kondisi_kanan = buat_node(AST_KONDISI);
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
            ASTNode* node = buat_node(AST_PERINTAH_HENTI); 
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
        node->nilai_teks = strdup(t.isi);
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
        node->syarat = parse_syarat_logika(p);

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


    // --- SIHIR BALIKAN ---
    if (t.jenis == TOKEN_IDENTITAS && strcmp(t.isi, "balikan") == 0) {
        ASTNode* node = buat_node(AST_PERINTAH_BALIKAN);
        maju(p); // lewati kata 'balikan'
        node->kiri = parse_ekspresi(p); // Ambil target yang mau dibalikkan
        return node;
    }

    // --- PENANGKAP EKSPRESI BERDIRI SENDIRI (MUTASI & FUNGSI) ---
    // Mengizinkan: `entitas = {}`, `dewa.nama = "Enki"`, atau `sapa()`
    if (t.jenis == TOKEN_IDENTITAS) {
        return parse_ekspresi(p);
    }

    // --- PENANGKAP PELEPASAN MEMORI BERDIRI SENDIRI (PASRAH VARIABEL) ---
    if (t.jenis == TOKEN_PASRAH) {
        ASTNode* node = buat_node(AST_PERINTAH_PASRAH);
        maju(p); // lewati kata 'pasrah'
        
        node->kiri = parse_ekspresi(p); // Ambil target variabel
        return node;
    }

    // ATM DARI BLUEPRINT: elif token[0] == 'SOWAN':
    // (Jika Lexer Anda mendeteksi kata 'sowan' sebagai TOKEN_IDENTITAS)
    // --- PENANGKAP SIHIR SOWAN ---
    if (t.jenis == TOKEN_SOWAN) { 
        ASTNode* node = buat_node(AST_PERINTAH_SOWAN);
        maju(p); // lewati kata 'sowan'
        
        Token t_target = token_sekarang(p);
        if (t_target.jenis == TOKEN_IDENTITAS) {
            // KEAJAIBAN KHUSUS SOWAN: Jika diketik tanpa kutip (misal: sowan matematika)
            // Paksa mesin membacanya sebagai teks literal agar tidak memicu Kernel Panic
            node->kiri = buat_node(AST_LITERAL_TEKS);
            node->kiri->nilai_teks = strdup(t_target.isi);
            maju(p);
        } else {
            // Jika menggunakan kutip ("matematika") atau ekspresi lain
            node->kiri = parse_ekspresi(p); 
        }
        
        return node;
    }

    // 5. Apakah ini HUKUM TABU? (coba maka ... tabu melanggar e maka ... tebus maka ... pasrah)
    if (t.jenis == TOKEN_COBA) {
        ASTNode* node = buat_node(AST_COBA_TABU);
        maju(p); // lewati kata 'coba'
        if (token_sekarang(p).jenis == TOKEN_KARMA && strcmp(token_sekarang(p).isi, "maka") == 0) maju(p);

        // A. Tangkap isi blok COBA (TRY)
        node->kiri = buat_node(AST_PROGRAM);
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
            node->kanan = buat_node(AST_PROGRAM);
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
            node->blok_tebus = buat_node(AST_PROGRAM);
            while (token_sekarang(p).jenis != TOKEN_EOF) {
                Token t_cek = token_sekarang(p);
                // Berhenti jika bertemu penutup pasrah
                if (t_cek.jenis == TOKEN_PASRAH) break;
                ASTNode* stmt = parse_pernyataan(p);
                if (stmt) tambah_anak(node->blok_tebus, stmt);
            }
        }

        // D. Lewati kata 'pasrah' sebagai PENUTUP MUTLAK blok Hukum Tabu
        if (token_sekarang(p).jenis == TOKEN_PASRAH) {
            maju(p); 
        } else {
            printf("🚨 Bencana Sintaksis: Blok 'coba' kehilangan penutup 'pasrah'!\n");
            kiamat_sintaksis("Blok 'coba' kehilangan penutup 'pasrah'!");
        }

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