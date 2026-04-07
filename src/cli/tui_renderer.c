#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "tui_renderer.h"

struct termios terminal_lama;

void tui_aktifkan_raw_mode() {
    tcgetattr(STDIN_FILENO, &terminal_lama);
    struct termios terminal_baru = terminal_lama;
    terminal_baru.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_baru);
}

void tui_matikan_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_lama);
}

void tui_bersihkan_layar() { 
    printf("\033[H\033[J"); 
}

void tui_pindah_kursor(int x, int y) { 
    printf("\033[%d;%dH", y, x); 
}

// ========================================================
// 💅 MESIN PEMBACA KOSMETIK (SNUL)
// ========================================================
void tui_terapkan_warna(EnkiObject* gaya) {
    if(!gaya || gaya->tipe != ENKI_OBJEK) return;
    for(int i=0; i<gaya->panjang; i++) {
        if(strcmp(gaya->nilai.objek_peta.kunci[i]->nilai.teks, "warna_teks") == 0) {
            char* w = gaya->nilai.objek_peta.konten[i]->nilai.teks;
            if (w[0] == '#') {
                int r, g, b;
                if (sscanf(w, "#%02x%02x%02x", &r, &g, &b) == 3) printf("\033[38;2;%d;%d;%dm", r, g, b); 
            } 
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
// ⌨️ MESIN PENYUNTIK TEKS (KEYLOGGER TO DOM)
// ========================================================
void tui_suntik_teks(EnkiObject* elemen, char c) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;
    for(int i=0; i<elemen->panjang; i++) {
        if(strcmp(elemen->nilai.objek_peta.kunci[i]->nilai.teks, "isi") == 0) {
            EnkiObject* target = elemen->nilai.objek_peta.konten[i];
            char buffer[1024] = {0};
            if(target->nilai.teks) strncpy(buffer, target->nilai.teks, 1023);

            if (c == 127 || c == 8) { 
                int len = strlen(buffer);
                if (len > 0) buffer[len-1] = '\0';
            } else if (c >= 32 && c <= 126) { 
                int len = strlen(buffer);
                if (len < 1022) { buffer[len] = c; buffer[len+1] = '\0'; }
            }
            
            free(target->nilai.teks);
            target->nilai.teks = strdup(buffer);
            return;
        }
    }
}

// ========================================================
// 🔲 MESIN PENGGAMBAR KOTAK (FOKUS & CLIPPING)
// ========================================================
void tui_gambar_kotak(int x, int y, int w, int h, const char* judul, EnkiObject* gaya, int lantai_terminal, int apakah_fokus) {
    tui_terapkan_warna(gaya); 
    if (apakah_fokus) printf("\033[7m"); // INVERT / SOROTAN!

    if (y >= 2 && y <= lantai_terminal) { tui_pindah_kursor(x, y); printf("╔"); for(int i=0; i<w-2; i++) printf("═"); printf("╗"); }
    for(int i=1; i<h-1; i++) {
        if (y+i >= 2 && y+i <= lantai_terminal) { tui_pindah_kursor(x, y+i); printf("║"); tui_pindah_kursor(x+w-1, y+i); printf("║"); }
    }
    if (y+h-1 >= 2 && y+h-1 <= lantai_terminal) { tui_pindah_kursor(x, y+h-1); printf("╚"); for(int i=0; i<w-2; i++) printf("═"); printf("╝"); }

    if(judul && y >= 2 && y <= lantai_terminal) { tui_pindah_kursor(x+2, y); printf("[ %s ]", judul); }
    
    printf("\033[0m"); 
}

// ========================================================
// 🌳 PENELUSUR DOM & TATA LETAK DINAMIS
// ========================================================
int render_elemen_rekursif(EnkiObject* elemen, int x, int y, int w_default, EnkiObject* gaya_root, int lantai_terminal, int* counter_interaktif, int fokus_saat_ini, EnkiObject** elemen_fokus_ptr) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return y;

    char* jenis = ""; char* tag_nama = ""; char* id_nama = ""; char* isi_teks = "";
    EnkiObject* anak_anak = NULL;

    for(int i=0; i<elemen->panjang; i++) {
        char* k = elemen->nilai.objek_peta.kunci[i]->nilai.teks; EnkiObject* v = elemen->nilai.objek_peta.konten[i];
        if(strcmp(k, "jenis") == 0) jenis = v->nilai.teks;
        if(strcmp(k, "tag") == 0) tag_nama = v->nilai.teks;
        if(strcmp(k, "id") == 0) id_nama = v->nilai.teks;
        if(strcmp(k, "isi") == 0) isi_teks = v->nilai.teks;
        if(strcmp(k, "anak_anak") == 0) anak_anak = v;
    }

    char selektor[256] = {0};
    if (strlen(tag_nama) > 0 && strlen(id_nama) > 0) snprintf(selektor, 256, "@%s.%s", tag_nama, id_nama);
    else if (strlen(tag_nama) > 0) snprintf(selektor, 256, "@%s", tag_nama);

    EnkiObject* gaya_elemen = NULL;
    if(gaya_root && gaya_root->tipe == ENKI_OBJEK) {
        for(int i=0; i<gaya_root->panjang; i++) {
            if(strcmp(gaya_root->nilai.objek_peta.kunci[i]->nilai.teks, selektor) == 0) { gaya_elemen = gaya_root->nilai.objek_peta.konten[i]; break; }
        }
    }

    int lebar_aktual = tui_ambil_lebar(gaya_elemen, w_default);
    int y_awal = y; int y_anak = y;        

    int apakah_interaktif = (strcmp(jenis, "tag") == 0 && (strcmp(tag_nama, "tombol") == 0 || strcmp(tag_nama, "masukan") == 0));
    int apakah_fokus = 0;

    if (apakah_interaktif) {
        if (*counter_interaktif == fokus_saat_ini) {
            apakah_fokus = 1;
            if (elemen_fokus_ptr) *elemen_fokus_ptr = elemen;
        }
        (*counter_interaktif)++; 
        y_anak = y + 1; 
    } else if (strcmp(jenis, "tag") == 0 && strcmp(tag_nama, "wadah") == 0) {
        y_anak = y + 1;
    }

    if(strcmp(jenis, "tag") == 0 && strcmp(tag_nama, "judul") == 0) {
        tui_terapkan_warna(gaya_elemen);
        if (y_anak >= 2 && y_anak <= lantai_terminal) { tui_pindah_kursor(x+2, y_anak); printf(">> %s <<", isi_teks); }
        printf("\033[0m"); y_anak++; 
    } else if(strcmp(jenis, "teks") == 0) {
        tui_terapkan_warna(gaya_elemen);
        if (y_anak >= 2 && y_anak <= lantai_terminal) { 
            tui_pindah_kursor(x+2, y_anak); 
            if (apakah_fokus) printf("\033[7m");
            printf("%s", isi_teks); 
        }
        printf("\033[0m"); y_anak++; 
    }

    if(anak_anak && anak_anak->tipe == ENKI_ARRAY) {
        for(int i=0; i<anak_anak->panjang; i++) {
            y_anak = render_elemen_rekursif(anak_anak->nilai.array_elemen[i], x+2, y_anak, lebar_aktual-4, gaya_root, lantai_terminal, counter_interaktif, fokus_saat_ini, elemen_fokus_ptr);
        }
    }

    if (apakah_interaktif || (strcmp(jenis, "tag") == 0 && strcmp(tag_nama, "wadah") == 0)) {
        int tinggi = (y_anak - y_awal) + 1; if (tinggi < 3) tinggi = 3;         
        char label[50] = {0};
        if (strcmp(tag_nama, "wadah") == 0) strcpy(label, "WADAH");
        else if (strcmp(tag_nama, "tombol") == 0) strcpy(label, "TOMBOL");
        else if (strcmp(tag_nama, "masukan") == 0) strcpy(label, "MASUKAN");

        tui_gambar_kotak(x, y_awal, lebar_aktual, tinggi, label, gaya_elemen, lantai_terminal, apakah_fokus);
        y_anak = y_awal + tinggi; 
    }
    return y_anak; 
}

void tampilkan_tui(EnkiObject* ui_root, EnkiObject* gaya_root) {
    tui_aktifkan_raw_mode();
    printf("\033[?1049h\033[?25l");

    int berjalan = 1;
    int scroll_y = 0;
    
    int indeks_fokus = 0;
    int total_interaktif = 0;
    EnkiObject* elemen_fokus_saat_ini = NULL;

    while (berjalan) {
        struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int lantai_terminal = w.ws_row; 

        tui_bersihkan_layar();

        EnkiObject* root_anak = NULL;
        for(int i=0; i<ui_root->panjang; i++) {
            if(strcmp(ui_root->nilai.objek_peta.kunci[i]->nilai.teks, "anak_anak") == 0) { root_anak = ui_root->nilai.objek_peta.konten[i]; break; }
        }

        int counter_saat_ini = 0; 
        elemen_fokus_saat_ini = NULL;

        if(root_anak && root_anak->tipe == ENKI_ARRAY) {
            int y_sekarang = 2 + scroll_y; 
            for(int i=0; i<root_anak->panjang; i++) {
                y_sekarang = render_elemen_rekursif(root_anak->nilai.array_elemen[i], 2, y_sekarang, 70, gaya_root, lantai_terminal, &counter_saat_ini, indeks_fokus, &elemen_fokus_saat_ini);
            }
        }
        total_interaktif = counter_saat_ini; 

        tui_pindah_kursor(1, 1);
        printf("\033[1;37m[Panah Atas/Bawah: Scroll | TAB: Pindah Fokus | Mengetik: Input Data | 'Q': Keluar]\033[0m");
        fflush(stdout); 

        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 9) { // TAB
                if (total_interaktif > 0) indeks_fokus = (indeks_fokus + 1) % total_interaktif;
            } 
            else if (c == '\033') { 
                char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A') scroll_y++; 
                        if (seq[1] == 'B') scroll_y--; 
                    }
                }
            }
            else {
                int adalah_masukan = 0;
                if (elemen_fokus_saat_ini) {
                    for(int i=0; i<elemen_fokus_saat_ini->panjang; i++) {
                        if(strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "tag") == 0 && 
                           strcmp(elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks, "masukan") == 0) {
                            adalah_masukan = 1; break;
                        }
                    }
                }

                if (adalah_masukan) {
                    tui_suntik_teks(elemen_fokus_saat_ini, c); 
                } else if (c == 'q' || c == 'Q') { 
                    berjalan = 0;
                }
            }
        }
    }

    printf("\033[?1049l\033[?25h"); tui_matikan_raw_mode();
}