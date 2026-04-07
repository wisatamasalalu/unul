#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "tui_renderer.h"

// Variabel global penyimpan memori terminal lama Anda
struct termios terminal_lama;

void tui_aktifkan_raw_mode() {
    tcgetattr(STDIN_FILENO, &terminal_lama);
    struct termios terminal_baru = terminal_lama;
    
    // Matikan ECHO (jangan tampilkan huruf yang diketik)
    // Matikan ICANON (baca per-tombol langsung, bukan per-baris/Enter)
    terminal_baru.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_baru);
}

void tui_matikan_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_lama);
}

// void tui_bersihkan_layar() { 
    // Masuk ke Layar Alternatif, Pindah ke ujung kiri atas, lalu bersihkan!
   // printf("\033[?1049h\033[H\033[J"); 
// }

void tui_bersihkan_layar() { 
    // HANYA hapus layar dan kembalikan kursor ke (1,1)
    printf("\033[H\033[J"); 
}
void tui_pindah_kursor(int x, int y) { printf("\033[%d;%dH", y, x); }

// ========================================================
// 💅 MESIN PEMBACA KOSMETIK (SNUL EXTRACTOR - MENDUKUNG HEX TRUE COLOR!)
// ========================================================
void tui_terapkan_warna(EnkiObject* gaya) {
    if(!gaya || gaya->tipe != ENKI_OBJEK) return;
    for(int i=0; i<gaya->panjang; i++) {
        if(strcmp(gaya->nilai.objek_peta.kunci[i]->nilai.teks, "warna_teks") == 0) {
            char* w = gaya->nilai.objek_peta.konten[i]->nilai.teks;
            
            // 1. Deteksi Sihir Hexadecimal (Contoh: #FF55AA)
            if (w[0] == '#') {
                int r, g, b;
                // Membedah string hex menjadi 3 angka desimal RGB
                if (sscanf(w, "#%02x%02x%02x", &r, &g, &b) == 3) {
                    printf("\033[38;2;%d;%d;%dm", r, g, b); // ANSI True Color!
                }
            } 
            // 2. Fallback Gaya Kuno (Teks Biasa)
            else if(strcmp(w, "merah") == 0) printf("\033[1;31m");
            else if(strcmp(w, "hijau") == 0) printf("\033[1;32m");
            else if(strcmp(w, "kuning") == 0) printf("\033[1;33m");
            else if(strcmp(w, "biru") == 0) printf("\033[1;34m");
            else if(strcmp(w, "cyan") == 0) printf("\033[1;36m");
        }
    }
}

int tui_ambil_lebar(EnkiObject* gaya, int lebar_default) {
    if(!gaya || gaya->tipe != ENKI_OBJEK) return lebar_default;
    for(int i=0; i<gaya->panjang; i++) {
        if(strcmp(gaya->nilai.objek_peta.kunci[i]->nilai.teks, "lebar") == 0) {
            return atoi(gaya->nilai.objek_peta.konten[i]->nilai.teks);
        }
    }
    return lebar_default;
}

// ========================================================
// 🔲 MESIN PENGGAMBAR KOTAK ASCII (DENGAN CLIPPING!)
// ========================================================
void tui_gambar_kotak(int x, int y, int w, int h, const char* judul, EnkiObject* gaya) {
    tui_terapkan_warna(gaya); 

    // Garis Atas (Hanya gambar jika melewati baris 2, agar tidak menabrak instruksi)
    if (y >= 2) {
        tui_pindah_kursor(x, y);
        printf("╔"); for(int i=0; i<w-2; i++) printf("═"); printf("╗");
    }

    // Dinding Samping
    for(int i=1; i<h-1; i++) {
        if (y+i >= 2) { // Batas anti-tabrakan atas layar
            tui_pindah_kursor(x, y+i);     printf("║");
            tui_pindah_kursor(x+w-1, y+i); printf("║");
        }
    }

    // Garis Bawah
    if (y+h-1 >= 2) {
        tui_pindah_kursor(x, y+h-1);
        printf("╚"); for(int i=0; i<w-2; i++) printf("═"); printf("╝");
    }

    // Judul
    if(judul && y >= 2) {
        tui_pindah_kursor(x+2, y);
        printf("[ %s ]", judul);
    }
    
    printf("\033[0m"); // Reset warna
}

// ========================================================
// 🌳 PENELUSUR DOM (DOCUMENT OBJECT MODEL)
// ========================================================
void render_elemen_rekursif(EnkiObject* elemen, int x, int y, int w_default, EnkiObject* gaya_root) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;

    char* jenis = ""; char* tag_nama = ""; char* id_nama = ""; char* isi_teks = "";
    EnkiObject* anak_anak = NULL;

    // 1. Bongkar Properti OTIM
    for(int i=0; i<elemen->panjang; i++) {
        char* kunci = elemen->nilai.objek_peta.kunci[i]->nilai.teks;
        EnkiObject* nilai = elemen->nilai.objek_peta.konten[i];

        if(strcmp(kunci, "jenis") == 0) jenis = nilai->nilai.teks;
        if(strcmp(kunci, "tag") == 0) tag_nama = nilai->nilai.teks;
        if(strcmp(kunci, "id") == 0) id_nama = nilai->nilai.teks;
        if(strcmp(kunci, "isi") == 0) isi_teks = nilai->nilai.teks;
        if(strcmp(kunci, "anak_anak") == 0) anak_anak = nilai;
    }

    // 2. Buat Jembatan Selektor (Misal: @wadah.utama)
    char selektor[256] = {0};
    if (strlen(tag_nama) > 0 && strlen(id_nama) > 0) snprintf(selektor, 256, "@%s.%s", tag_nama, id_nama);
    else if (strlen(tag_nama) > 0) snprintf(selektor, 256, "@%s", tag_nama);

    // 3. Cari Gaya di SNUL
    EnkiObject* gaya_elemen = NULL;
    if(gaya_root && gaya_root->tipe == ENKI_OBJEK) {
        for(int i=0; i<gaya_root->panjang; i++) {
            if(strcmp(gaya_root->nilai.objek_peta.kunci[i]->nilai.teks, selektor) == 0) {
                gaya_elemen = gaya_root->nilai.objek_peta.konten[i]; break;
            }
        }
    }

    // 4. Proses Penggambaran
    int lebar_aktual = tui_ambil_lebar(gaya_elemen, w_default);

    if(strcmp(jenis, "tag") == 0) {
        if(strcmp(tag_nama, "wadah") == 0) {
            tui_gambar_kotak(x, y, lebar_aktual, 6, "WADAH", gaya_elemen);
        } else if(strcmp(tag_nama, "tombol") == 0) {
            tui_gambar_kotak(x, y, lebar_aktual, 3, "TOMBOL", gaya_elemen);
        } else if(strcmp(tag_nama, "judul") == 0) {
            tui_terapkan_warna(gaya_elemen);
            tui_pindah_kursor(x+2, y+1);
            printf(">> %s <<", isi_teks);
            printf("\033[0m");
        }
    } else if(strcmp(jenis, "teks") == 0) {
        tui_terapkan_warna(gaya_elemen);
        tui_pindah_kursor(x+2, y+1);
        printf("%s", isi_teks);
        printf("\033[0m");
    }

    // 5. Gambar Anak-anaknya (Rekursif)
    if(anak_anak && anak_anak->tipe == ENKI_ARRAY) {
        for(int i=0; i<anak_anak->panjang; i++) {
            render_elemen_rekursif(anak_anak->nilai.array_elemen[i], x+2, y+2+(i*3), lebar_aktual-4, gaya_root);
        }
    }
}

void tampilkan_tui(EnkiObject* ui_root, EnkiObject* gaya_root) {
    // 1. MASUK DIMENSI TUI!
    tui_aktifkan_raw_mode();
    printf("\033[?1049h"); // Buka layar alternatif
    printf("\033[?25l");   // Sembunyikan kursor berkedip bawaan terminal

    int berjalan = 1;
    int scroll_y = 0; // Ini adalah nyawa dari fitur scroll Anda!

    // ==========================================
    // 🌀 EVENT LOOP (JANTUNG SISTEM OPERASI TUI)
    // ==========================================
    while (berjalan) {
        tui_bersihkan_layar();

        // Cari Akar Anak-anak OTIM
        EnkiObject* root_anak = NULL;
        for(int i=0; i<ui_root->panjang; i++) {
            if(strcmp(ui_root->nilai.objek_peta.kunci[i]->nilai.teks, "anak_anak") == 0) {
                root_anak = ui_root->nilai.objek_peta.konten[i]; break;
            }
        }

        // Gambar UI dengan ditambah Offset Scroll!
        if(root_anak && root_anak->tipe == ENKI_ARRAY) {
            for(int i=0; i<root_anak->panjang; i++) {
                // Perhatikan y = 2 + (i*8) + scroll_y (agar bisa naik/turun)
                render_elemen_rekursif(root_anak->nilai.array_elemen[i], 2, 2 + (i*8) + scroll_y, 50, gaya_root);
            }
        }

        // Tampilkan Instruksi di sudut layar tetap
        tui_pindah_kursor(1, 1);
        printf("\033[1;37m[Panah Atas/Bawah: Scroll | Tombol 'q': Keluar]\033[0m");

        // 2. TANGKAP INPUT KEYBOARD REAL-TIME!
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'q' || c == 'Q') { // Tombol Keluar
                berjalan = 0;
            } 
            else if (c == '\033') { // Mendeteksi Kode ANSI Panah (Escape Sequence)
                char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A') scroll_y++; // Panah Atas (UI turun)
                        if (seq[1] == 'B') scroll_y--; // Panah Bawah (UI naik)
                    }
                }
            }
        }
    }

    // 3. KEMBALI KE DUNIA NYATA
    printf("\033[?1049l"); // Tutup layar alternatif
    printf("\033[?25h");   // Munculkan kursor berkedip lagi
    tui_matikan_raw_mode();
}