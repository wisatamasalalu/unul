#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "enki_lexer.h"
#include "enki_parser.h"
#include "enki_interpreter.h" // <--- Sowan ke Jantung Eksekusi

// (Bagian baca_kitab dan nama_jenis_token tetap biarkan utuh di sini, saya persingkat agar muat)
char* baca_kitab(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) { printf("🚨 Bencana! Kitab '%s' tidak ditemukan.\n", path); exit(1); }
    fseek(file, 0, SEEK_END); long panjang = ftell(file); fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(panjang + 1);
    if (buffer) { fread(buffer, 1, panjang, file); buffer[panjang] = '\0'; }
    fclose(file); return buffer;
}

int main(int argc, char** argv) {
    // Bangkitkan benih acak berdasarkan waktu OS
    srand(time(NULL));
    if (argc < 2) {
        printf("🚨 Peringatan: Kamu lupa memasukkan kitab takdir!\n");
        return 1;
    }

    char* kode_sumber = baca_kitab(argv[1]);

    // 1. LEXER
    TokenArray token_list = enki_lexer(kode_sumber);

    // 2. PARSER
    Parser parser = inisialisasi_parser(token_list);
    ASTNode* pohon_utama = parse_program(&parser);
    
    // 3. INTERPRETER (Eksekusi Nyata)
    printf("\n=== EKSEKUSI NATIVE C (LINUXDNC) ===\n");
    
    // A. Siapkan Wadah RAM
    EnkiRAM ram = inisialisasi_ram(); 
    
    // B. Suntikkan Rahasia .anu ke dalam RAM yang baru dibuat
    // Gunakan '&' untuk mengirim ALAMAT memori variabel ram
    muat_anu(&ram); 
    
    // C. Jalankan Program
    eksekusi_program(pohon_utama, &ram);
    
    printf("====================================\n");

    // 4. KEMBALIKAN MEMORI
    bebaskan_ram(&ram);
    bebaskan_ast(pohon_utama);
    bebaskan_token_array(&token_list);
    free(kode_sumber);

    return 0;
}