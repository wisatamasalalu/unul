#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enki_interpreter.h"

// =================================================================
// DAPUR MESIN INTERPRETER (EKSEKUTOR C)
// Jantung LinuxDNC yang memompa Pohon Logika menjadi kenyataan!
// =================================================================

// --- 1. MANAJEMEN RAM UTAMA (ENKI RAM) ---
EnkiRAM inisialisasi_ram() {
    EnkiRAM ram;
    ram.kapasitas = 64; // Ruang awal 64 variabel
    ram.jumlah = 0;
    ram.kavling = (KavlingMemori*)malloc(ram.kapasitas * sizeof(KavlingMemori));
    return ram;
}

void bebaskan_ram(EnkiRAM* ram) {
    for (int i = 0; i < ram->jumlah; i++) {
        if (ram->kavling[i].nama) free(ram->kavling[i].nama);
        if (ram->kavling[i].nilai_teks) free(ram->kavling[i].nilai_teks);
    }
    free(ram->kavling);
    ram->kavling = NULL;
    ram->jumlah = 0;
    ram->kapasitas = 0;
}

// Fungsi Internal: Menyimpan atau menimpa nilai di RAM
void simpan_ke_ram(EnkiRAM* ram, const char* nama, const char* nilai) {
    // 1. Cari apakah sudah ada di RAM (Timpa nilai lama)
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            free(ram->kavling[i].nilai_teks); // Buang ingatan lama
            ram->kavling[i].nilai_teks = strdup(nilai); // Masukkan ingatan baru
            return;
        }
    }
    
    // 2. Jika belum ada, buat kavling baru
    if (ram->jumlah >= ram->kapasitas) {
        ram->kapasitas *= 2;
        ram->kavling = (KavlingMemori*)realloc(ram->kavling, ram->kapasitas * sizeof(KavlingMemori));
    }
    
    ram->kavling[ram->jumlah].nama = strdup(nama);
    ram->kavling[ram->jumlah].nilai_teks = strdup(nilai);
    ram->jumlah++;
}

// Fungsi Internal: Membaca nilai dari RAM
const char* baca_dari_ram(EnkiRAM* ram, const char* nama) {
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            return ram->kavling[i].nilai_teks;
        }
    }
    return NULL; 
}

// Fungsi Bantuan: Menghapus kutip ganda/tunggal di ujung teks (seperti .strip() di Python)
void bersihkan_kutip(char* teks) {
    int len = strlen(teks);
    if (len >= 2 && (teks[0] == '"' || teks[0] == '\'') && (teks[len-1] == '"' || teks[len-1] == '\'')) {
        memmove(teks, teks + 1, len - 2);
        teks[len - 2] = '\0';
    }
}

// --- 2. LOGIKA EVALUASI NILAI ---
char* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram) {
    if (!node) return strdup(""); 
    
    if (node->jenis == AST_LITERAL_TEKS) {
        char* teks_bersih = strdup(node->nilai_teks);
        bersihkan_kutip(teks_bersih); // Hapus kutip sebelum masuk memori!
        return teks_bersih;
    }
    
    if (node->jenis == AST_IDENTITAS) {
        const char* memori = baca_dari_ram(ram, node->nilai_teks);
        if (memori) return strdup(memori);
        printf("🚨 KERNEL PANIC! Takdir '%s' belum diciptakan!\n", node->nilai_teks);
        exit(1);
    }
    
    if (node->jenis == AST_FUNGSI_DENGAR) {
        char buffer[1024] = {0};
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = '\0';
            return strdup(buffer);
        }
        return strdup("");
    }

    // --- KEAJAIBAN BARU: OPERASI MATEMATIKA & GABUNG TEKS ---
    if (node->jenis == AST_OPERASI_MATEMATIKA) {
        char* hasil_kiri = evaluasi_ekspresi(node->kiri, ram);
        char* hasil_kanan = evaluasi_ekspresi(node->kanan, ram);
        
        // C kaku! Kita harus pesan memori bersih dan kosong untuk hasilnya
        char* hasil_akhir = (char*)malloc(1024);
        memset(hasil_akhir, 0, 1024);

        if (node->operator_math && strcmp(node->operator_math, "+") == 0) {
            double angka_kiri = atof(hasil_kiri);
            double angka_kanan = atof(hasil_kanan);

            // Deteksi cerdas: Apakah ini teks murni?
            if ((angka_kiri == 0 && strcmp(hasil_kiri, "0") != 0) || 
                (angka_kanan == 0 && strcmp(hasil_kanan, "0") != 0)) {
                // Ada teks, gabungkan string!
                snprintf(hasil_akhir, 1024, "%s%s", hasil_kiri, hasil_kanan);
            } else {
                // Keduanya angka murni, jumlahkan!
                snprintf(hasil_akhir, 1024, "%g", angka_kiri + angka_kanan);
            }
        }
        
        free(hasil_kiri); free(hasil_kanan);
        return hasil_akhir;
    }
    
    return strdup("");
}

// --- 3. EKSEKUSI NODE (MENJALANKAN PERINTAH) ---
void eksekusi_node(ASTNode* node, EnkiRAM* ram) {
    if (!node) return;
    
    // 1. Eksekusi KETIK (Output)
    if (node->jenis == AST_PERINTAH_KETIK) {
        char* hasil = evaluasi_ekspresi(node->kanan, ram);
        printf("%s\n", hasil);
        free(hasil); // Selalu bersihkan memori sementara
    }
    
    // 2. Eksekusi DEKLARASI TAKDIR (Input ke RAM)
    else if (node->jenis == AST_DEKLARASI_TAKDIR) {
        char* nama_variabel = node->kiri->nilai_teks;
        char* hasil_kanan = evaluasi_ekspresi(node->kanan, ram);
        
        simpan_ke_ram(ram, nama_variabel, hasil_kanan);
        free(hasil_kanan);
    }
}

// --- 4. EKSEKUSI PROGRAM UTAMA ---
void eksekusi_program(ASTNode* program, EnkiRAM* ram) {
    if (!program || program->jenis != AST_PROGRAM) return;
    
    for (int i = 0; i < program->jumlah_anak; i++) {
        eksekusi_node(program->anak_anak[i], ram);
    }
}