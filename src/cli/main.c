inisialisasi_kolam_memori();

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../core_c/enki_lexer.h"
#include "../core_c/enki_parser.h"
#include "../core_c/enki_interpreter.h"
#include "../core_c/enki_os.h"
#include "../core_c/enki_file_system.h"

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

// --- 2. MAIN PROGRAM ---

int main(int argc, char* argv[]) {
    EnkiRAM ram = inisialisasi_ram();
    muat_anu(&ram); 

    if (argc > 1) {
        if (strcmp(argv[1], "ayo") == 0 && argc > 2) {
            muat_ingatan(&ram);
            char* kode = baca_file_mentah(argv[2]);
            if (kode) { 
                jalankan_perintah(kode, &ram, argv[2]); 
                free(kode); 
            } else {
                printf("\n🚨 KIAMAT SISTEM: Kitab '%s' tidak ditemukan di alam semesta!\n", argv[2]);
                printf("💡 PANDUAN: Pastikan nama file dan jalur (path) sudah benar.\n\n");
                return 1;
            }
        } 
        // 🟢 FITUR KENANGAN (INGAT)
        else if (strcmp(argv[1], "ingat") == 0 && argc > 2) {
            if (strcmp(argv[2], "list") == 0) {
                FILE* f = fopen(".ingatan-unul", "r");
                if (!f) {
                    printf("💨 unul tidak menyimpan kenangan masa lalu apapun dalam pikirannya.\n");
                } else {
                    printf("🗂️ Inilah daftar kenangan unul:\n");
                    char path[1024];
                    int count = 0;
                    while (fgets(path, sizeof(path), f)) {
                        path[strcspn(path, "\n")] = 0; // Hapus enter
                        printf(" - %s\n", path);
                        count++;
                    }
                    fclose(f);
                    if (count == 0) {
                        printf("💨 unul tidak menyimpan kenangan masa lalu apapun dalam pikirannya.\n");
                    }
                }
            } else {
                // 🟢 CEK APAKAH FILE BENAR-BENAR ADA (ANTI-HALUSINASI)
                FILE* cek_file = fopen(argv[2], "r");
                if (cek_file) {
                    fclose(cek_file); // Tutup lagi, kita cuma numpang cek
                    
                    // Simpan ke ingatan karena filenya benar-benar nyata
                    FILE* f = fopen(".ingatan-unul", "a");
                    if (f) { fprintf(f, "%s\n", argv[2]); fclose(f); }
                    printf("💾 UNUL kenangan '%s' telah bersemayam dalam pikiran unul.\n", argv[2]);
                } else {
                    // Tolak dengan puitis jika filenya gaib
                    printf("💔 unul menolak mengingat ilusi! Berkas '%s' tidak wujud di dimensi ini.\n", argv[2]);
                }
            } 
        }
        // 🟢 FITUR AMNESIA (LUPA)
        else if (strcmp(argv[1], "lupa") == 0 && argc > 2) {
            if (strcmp(argv[2], "semua") == 0) {
                unlink(".ingatan-unul");
                printf("🧠 Segala kenangan masa lalu unul telah dilupakan.\n");
            } else {
                // Menghapus kenangan spesifik
                FILE* f = fopen(".ingatan-unul", "r");
                if (f) {
                    char temp_file[] = ".ingatan-unul.tmp";
                    FILE* ft = fopen(temp_file, "w");
                    char path[1024];
                    int found = 0;
                    
                    while (fgets(path, sizeof(path), f)) {
                        char clean_path[1024];
                        strcpy(clean_path, path);
                        clean_path[strcspn(clean_path, "\n")] = 0;
                        
                        if (strcmp(clean_path, argv[2]) == 0) {
                            found = 1; // Jangan ditulis ulang (Lupakan)
                        } else {
                            fprintf(ft, "%s", path);
                        }
                    }
                    fclose(f);
                    fclose(ft);
                    
                    remove(".ingatan-unul");
                    rename(temp_file, ".ingatan-unul");
                    
                    if (found) {
                        printf("🗑️ UNUL telah menghapus '%s' dari pikirannya.\n", argv[2]);
                    } else {
                        printf("💨 Kenangan '%s' memang tidak pernah ada di pikiran unul.\n", argv[2]);
                    }
                } else {
                    printf("💨 unul tidak menyimpan kenangan masa lalu apapun dalam pikirannya.\n");
                }
            }
        } 
        else {
            muat_ingatan(&ram);
            char* kode = baca_file_mentah(argv[1]);
            if (kode) { 
                jalankan_perintah(kode, &ram, argv[1]); 
                free(kode); 
            } else {
                printf("\n🚨 KIAMAT SISTEM: Kitab '%s' tidak ditemukan di alam semesta!\n", argv[1]);
                printf("💡 PANDUAN: Pastikan nama file skrip Anda benar dan Anda berada di folder yang tepat.\n\n");
                return 1;
            }
        }
    }
    else {
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