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

// --- 2. LOGIKA EVALUASI NILAI ---
char* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram) {
    if (!node) return strdup(""); 
    
    // Jika Teks Asli ("Halo")
    if (node->jenis == AST_LITERAL_TEKS) {
        return strdup(node->nilai_teks);
    }
    
    // Jika Identitas / Variabel (nama_user) -> Panggil dari RAM!
    if (node->jenis == AST_IDENTITAS) {
        const char* memori = baca_dari_ram(ram, node->nilai_teks);
        if (memori) {
            return strdup(memori);
        } else {
            printf("🚨 KERNEL PANIC! Takdir '%s' belum diciptakan!\n", node->nilai_teks);
            exit(1);
        }
    }
    
    // Jika memanggil fungsi dengar()
    if (node->jenis == AST_FUNGSI_DENGAR) {
        char buffer[1024];
        printf("> "); // Tanda prompt
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Hapus enter (\n) di akhir input
            buffer[strcspn(buffer, "\n")] = '\0';
            return strdup(buffer);
        }
        return strdup("");
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