#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "enki_lexer.h"
#include "enki_parser.h"
#include "enki_interpreter.h"
#include "enki_os.h"

// --- 1. DEKLARASI PEMBANTU (Agar tidak implicit declaration) ---

char* baca_file_mentah(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    char* buffer = malloc(sz + 1);
    if (buffer) {
        fread(buffer, 1, sz, f);
        buffer[sz] = '\0';
    }
    fclose(f);
    return buffer;
}

void jalankan_perintah(const char* kode, EnkiRAM* ram, const char* label) {
    TokenArray tokens = enki_lexer((char*)kode, (char*)label);
    Parser p = inisialisasi_parser(tokens);
    ASTNode* program = parse_program(&p);
    eksekusi_program(program, ram);
    bebaskan_ast(program);
    bebaskan_token_array(&tokens);
}

int apakah_ekstensi(const char* path, const char* ext) {
    const char* titik = strrchr(path, '.');
    if (!titik || titik == path) return 0;
    return (strcmp(titik, ext) == 0);
}

char* buat_nama_variabel(const char* path) {
    char* nama = strdup(path);
    for (int i = 0; nama[i]; i++) {
        if (!isalnum(nama[i])) nama[i] = '_';
    }
    return nama;
}

void suntik_data_ke_ram(const char* path, EnkiRAM* ram) {
    char* isi = baca_file_mentah(path);
    if (!isi) return;

    char* nama_var = buat_nama_variabel(path);
    
    // 🟢 SUNTIKAN JANTUNG: Bungkus teks mentah menjadi Objek Dewa!
    EnkiObject* obj_isi = ciptakan_teks(isi);
    
    // Simpan ke RAM menggunakan objek yang baru diciptakan
    simpan_ke_ram(ram, nama_var, obj_isi);
    
    free(nama_var);
    free(isi); // Aman dibebaskan karena ciptakan_teks sudah menduplikasi isinya
}

void muat_ingatan(EnkiRAM* ram) {
    FILE* f = fopen(".ingatan-unul", "r");
    if (!f) return;
    char path[1024];
    while (fgets(path, sizeof(path), f)) {
        path[strcspn(path, "\n")] = 0;
        if (apakah_ekstensi(path, ".unll")) {
            char* kode = baca_file_mentah(path);
            if (kode) { jalankan_perintah(kode, ram, path); free(kode); }
        } else {
            suntik_data_ke_ram(path, ram);
        }
    }
    fclose(f);
}

// --- 2. MAIN PROGRAM ---

int main(int argc, char* argv[]) {
    EnkiRAM ram = inisialisasi_ram();
    muat_anu(&ram); 

    if (argc > 1) {
        if (strcmp(argv[1], "ayo") == 0 && argc > 2) {
            muat_ingatan(&ram);
            char* kode = baca_file_mentah(argv[2]);
            if (kode) { jalankan_perintah(kode, &ram, argv[2]); free(kode); }
        } else if (strcmp(argv[1], "ingat") == 0 && argc > 2) {
            FILE* f = fopen(".ingatan-unul", "a");
            if (f) { fprintf(f, "%s\n", argv[2]); fclose(f); }
            printf("💾 Diingat: '%s' kini merasuk ke latar belakang.\n", argv[2]);
        } else if (strcmp(argv[1], "lupa") == 0 && argc > 2 && strcmp(argv[2], "semua") == 0) {
            unlink(".ingatan-unul");
            printf("🧠 Memori latar belakang telah dibersihkan.\n");
        } else {
            muat_ingatan(&ram);
            char* kode = baca_file_mentah(argv[1]);
            if (kode) { jalankan_perintah(kode, &ram, argv[1]); free(kode); }
        }
    } else {
        // 🌍 AKTIFKAN RADAR DIMENSI
        cetak_info_dimensi();
        printf("🦅 OS LinuxDNC - UNUL CLI Interaktif 🦅\n");
        muat_ingatan(&ram);
        while (1) {
            char* input = readline("unul> ");
            if (!input) break;
            if (strlen(input) > 0) {
                add_history(input);
                // --- CEK PINTU KELUAR (Multi-Alias) ---
                if (strcmp(input, "pergi") == 0 || 
                    strcmp(input, "exit") == 0 || 
                    strcmp(input, "keluar") == 0 || 
                    strcmp(input, "out") == 0 ||
                    strcmp(input, "quit") == 0) {
                    
                    free(input);
                    printf("Sampai jumpa di dimensi lain, Arsitek! 🦅✨\n");
                    break;
                }
                jalankan_perintah(input, &ram, "<unul-cli>");
            }
            free(input);
        }
    }
    return 0;
}