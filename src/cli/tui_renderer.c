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
    terminal_baru.c_cc[VMIN] = 0;  
    terminal_baru.c_cc[VTIME] = 1; 
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
// ⌨️ PENYUNTIK TEKS (Word Wrap & Masking Terintegrasi)
// ========================================================
void tui_suntik_teks(EnkiObject* elemen, char c) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;
    
    EnkiObject* anak_anak = NULL;
    for(int i=0; i<elemen->panjang; i++) {
        if(strcmp(elemen->nilai.objek_peta.kunci[i]->nilai.teks, "anak_anak") == 0) {
            anak_anak = elemen->nilai.objek_peta.konten[i]; break;
        }
    }
    
    if (anak_anak && anak_anak->tipe == ENKI_ARRAY && anak_anak->panjang > 0) {
        EnkiObject* node_teks = anak_anak->nilai.array_elemen[0];
        if (node_teks && node_teks->tipe == ENKI_OBJEK) {
            for(int j=0; j<node_teks->panjang; j++) {
                if(strcmp(node_teks->nilai.objek_peta.kunci[j]->nilai.teks, "isi") == 0) {
                    EnkiObject* target = node_teks->nilai.objek_peta.konten[j];
                    char buffer[4096] = {0}; // Diperbesar untuk Areanulis
                    if(target->nilai.teks) strncpy(buffer, target->nilai.teks, 4095);

                    if (c == 127 || c == 8) { 
                        int len = strlen(buffer);
                        if (len > 0) buffer[len-1] = '\0';
                    } else if (c >= 32 && c <= 126) { 
                        int len = strlen(buffer);
                        if (len < 4094) { buffer[len] = c; buffer[len+1] = '\0'; }
                    }
                    
                    free(target->nilai.teks);
                    target->nilai.teks = strdup(buffer);
                    return;
                }
            }
        }
    }
}

// ========================================================
// 📝 MESIN PEMOTONG BARIS (Word Wrap)
// ========================================================
void tui_cetak_wrap(int x, int y, int w, const char* teks, int lantai_terminal) {
    int len = strlen(teks);
    int baris = 0;
    for (int i = 0; i < len; i += w) {
        if (y + baris >= 2 && y + baris <= lantai_terminal) {
            tui_pindah_kursor(x, y + baris);
            printf("%.*s", w, teks + i); // Cetak sesuai lebar w
        }
        baris++;
    }
}

void tui_gambar_kotak(int x, int y, int w, int h, const char* judul, EnkiObject* gaya, int lantai_terminal, int apakah_fokus) {
    tui_terapkan_warna(gaya); 
    if (apakah_fokus) printf("\033[7m"); 

    if (y >= 2 && y <= lantai_terminal) { tui_pindah_kursor(x, y); printf("╔"); for(int i=0; i<w-2; i++) printf("═"); printf("╗"); }
    for(int i=1; i<h-1; i++) {
        if (y+i >= 2 && y+i <= lantai_terminal) { tui_pindah_kursor(x, y+i); printf("║"); tui_pindah_kursor(x+w-1, y+i); printf("║"); }
    }
    if (y+h-1 >= 2 && y+h-1 <= lantai_terminal) { tui_pindah_kursor(x, y+h-1); printf("╚"); for(int i=0; i<w-2; i++) printf("═"); printf("╝"); }

    if(judul && y >= 2 && y <= lantai_terminal) { tui_pindah_kursor(x+2, y); printf("[ %s ]", judul); }
    printf("\033[0m"); 
}

// ========================================================
// 🌳 TATA LETAK DINAMIS (DENGAN LEVEL & FITUR ADVANCED)
// ========================================================
int render_elemen_rekursif(EnkiObject* elemen, int x, int y, int w_default, EnkiObject* gaya_root, int lantai_terminal, int* counter_interaktif, int fokus_saat_ini, EnkiObject** elemen_fokus_ptr, int level) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return y;

    char* jenis = ""; char* tag_nama = ""; char* id_nama = ""; char* isi_teks = ""; char* tipe_atribut = "";
    EnkiObject* anak_anak = NULL;

    for(int i=0; i<elemen->panjang; i++) {
        char* k = elemen->nilai.objek_peta.kunci[i]->nilai.teks; EnkiObject* v = elemen->nilai.objek_peta.konten[i];
        if(strcmp(k, "jenis") == 0) jenis = v->nilai.teks;
        if(strcmp(k, "tag") == 0) tag_nama = v->nilai.teks;
        if(strcmp(k, "id") == 0) id_nama = v->nilai.teks;
        if(strcmp(k, "isi") == 0) isi_teks = v->nilai.teks;
        if(strcmp(k, "tipe") == 0) tipe_atribut = v->nilai.teks;
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

    int interaktif = (strcmp(tag_nama, "masukan") == 0 || strcmp(tag_nama, "areanulis") == 0 || strcmp(tag_nama, "tombol") == 0 || strcmp(tag_nama, "centang") == 0);
    int apakah_fokus = 0;

    if (interaktif) {
        if (*counter_interaktif == fokus_saat_ini) { apakah_fokus = 1; if (elemen_fokus_ptr) *elemen_fokus_ptr = elemen; }
        (*counter_interaktif)++; 
        y_anak = y + 1; 
    } else if (strcmp(tag_nama, "wadah") == 0) {
        y_anak = y + 1;
    }

    // PENGGAMBARAN TEKS DAN KONTEN
    tui_terapkan_warna(gaya_elemen);
    if(strcmp(tag_nama, "judul") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) { tui_pindah_kursor(x+2, y_anak); printf(">> %s <<", isi_teks); }
        y_anak++; 
    } 
    else if (strcmp(tag_nama, "butir") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) {
            char* simbol[] = {"•", "-", "○", "»"}; // 4 Level Bullets
            tui_pindah_kursor(x + (level * 2), y_anak);
            printf("%s %s", simbol[level % 4], isi_teks);
        }
        y_anak++;
    }
    else if (strcmp(tag_nama, "centang") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) {
            tui_pindah_kursor(x+2, y_anak);
            if (apakah_fokus) printf("\033[7m");
            // Placeholder sederhana, logika on/off nanti
            printf("[ ] %s", isi_teks); 
        }
        y_anak++;
    }
    else if (strcmp(tag_nama, "areanulis") == 0) {
        int w_area = lebar_aktual - 4;
        tui_cetak_wrap(x+2, y_anak, w_area, isi_teks, lantai_terminal);
        y_anak += (strlen(isi_teks) / w_area) + 1; // Tinggi bertambah sesuai baris!
    }
    else if(strcmp(jenis, "teks") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) { 
            tui_pindah_kursor(x+2, y_anak); 
            if (apakah_fokus) printf("\033[7m");
            
            // LOGIKA PASSWORD MASKING
            if (strcmp(tipe_atribut, "password") == 0) {
                for(int i=0; i < (int)strlen(isi_teks); i++) printf("*");
            } else {
                printf("%s", isi_teks); 
            }
        }
        y_anak++; 
    }
    printf("\033[0m"); 

    if(anak_anak && anak_anak->tipe == ENKI_ARRAY) {
        for(int i=0; i<anak_anak->panjang; i++) {
            // Oper level + 1 ke anak!
            y_anak = render_elemen_rekursif(anak_anak->nilai.array_elemen[i], x+2, y_anak, lebar_aktual-4, gaya_root, lantai_terminal, counter_interaktif, fokus_saat_ini, elemen_fokus_ptr, level + 1);
        }
    }

    if (interaktif || strcmp(tag_nama, "wadah") == 0) {
        int tinggi = (y_anak - y_awal) + 1; if (tinggi < 3) tinggi = 3;         
        tui_gambar_kotak(x, y_awal, lebar_aktual, tinggi, tag_nama, gaya_elemen, lantai_terminal, apakah_fokus);
        y_anak = y_awal + tinggi; 
    }
    return y_anak; 
}

char* tampilkan_tui(EnkiObject* ui_root, EnkiObject* gaya_root) {
    tui_aktifkan_raw_mode();
    printf("\033[?1049h\033[?25l\033[?1000h\033[?1006h"); 

    int berjalan = 1;
    int scroll_y = 0;
    int indeks_fokus = 0;
    int total_interaktif = 0;
    EnkiObject* elemen_fokus_saat_ini = NULL;
    char* id_yang_diklik = NULL; 

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
                // MULAI DENGAN LEVEL 0
                y_sekarang = render_elemen_rekursif(root_anak->nilai.array_elemen[i], 2, y_sekarang, 70, gaya_root, lantai_terminal, &counter_saat_ini, indeks_fokus, &elemen_fokus_saat_ini, 0);
            }
        }
        total_interaktif = counter_saat_ini; 

        tui_pindah_kursor(1, 1);
        printf("\033[1;37m[Mouse/Panah: Scroll | TAB: Pindah | Mengetik: Input | ESC: Keluar]\033[0m");
        fflush(stdout); 

        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 9) { 
                if (total_interaktif > 0) indeks_fokus = (indeks_fokus + 1) % total_interaktif;
            } 
            else if (c == '\033') { 
                char seq[4];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A') scroll_y++; 
                        else if (seq[1] == 'B') scroll_y--; 
                        else if (seq[1] == '<') { 
                            char mouse_buf[32] = {0}; int m_idx = 0; char m_c;
                            while (read(STDIN_FILENO, &m_c, 1) == 1) {
                                mouse_buf[m_idx++] = m_c; if (m_c == 'M' || m_c == 'm') break;
                            }
                            if (mouse_buf[m_idx-1] == 'M') { 
                                int cb, cx, cy;
                                if (sscanf(mouse_buf, "%d;%d;%d", &cb, &cx, &cy) == 3) {
                                    if (cb == 64) scroll_y++; else if (cb == 65) scroll_y--; 
                                    else if (cb == 0) { if (total_interaktif > 0) indeks_fokus = (indeks_fokus + 1) % total_interaktif; }
                                }
                            }
                        }
                    }
                } else {
                    berjalan = 0; 
                }
            }
            else {
                int adlh_input = 0; int adlh_tombol = 0;
                if (elemen_fokus_saat_ini) {
                    for(int i=0; i<elemen_fokus_saat_ini->panjang; i++) {
                        if(strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "tag") == 0) {
                            char* tg = elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks;
                            if(strcmp(tg, "masukan") == 0 || strcmp(tg, "areanulis") == 0) adlh_input = 1;
                            else if(strcmp(tg, "tombol") == 0 || strcmp(tg, "centang") == 0) adlh_tombol = 1;
                            break;
                        }
                    }
                }

                if (adlh_input) {
                    if (c != '\n' && c != '\r') tui_suntik_teks(elemen_fokus_saat_ini, c); 
                } 
                else if (adlh_tombol && (c == '\n' || c == '\r' || c == ' ')) {
                    for(int i=0; i<elemen_fokus_saat_ini->panjang; i++) {
                        if(strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "id") == 0) {
                            id_yang_diklik = elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks;
                            break;
                        }
                    }
                    berjalan = 0; 
                } 
            }
        }
    }

    printf("\033[?1006l\033[?1000l\033[?1049l\033[?25h"); 
    tui_matikan_raw_mode();
    return id_yang_diklik ? strdup(id_yang_diklik) : strdup("TUTUP_PAKSA");
}