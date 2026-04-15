#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "tui_renderer.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

struct termios terminal_lama;

// 🧠 SARAF PUSAT KOORDINAT LAYAR (Untuk Klik Mouse & Hover)
typedef struct {
    char id[128];
    int x, y, w, h;
    int indeks_logika;
    EnkiObject* node_asli; // 🟢 Menyimpan pointer langsung ke Objek UNUL!
} KotakInteraktif;

static KotakInteraktif daftar_kotak[1024];
static int jumlah_kotak = 0;
static int mouse_x = 0;
static int mouse_y = 0;

// 🟢 MESIN WAKTU MIKRO: Untuk melacak rentang mili-detik
long long dapatkan_waktu_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

// ========================================================
// 🎨 MESIN EKSTRAKSI SNUL (ANTI-HARDCODE)
// ========================================================
char* tui_ambil_gaya_teks(EnkiObject* gaya, const char* properti, char* nilai_bawaan) {
    if (!gaya || gaya->tipe != ENKI_OBJEK) return nilai_bawaan;
    for(int i=0; i<gaya->panjang; i++) {
        if(strcmp(gaya->nilai.objek_peta.kunci[i]->nilai.teks, properti) == 0) {
            return gaya->nilai.objek_peta.konten[i]->nilai.teks;
        }
    }
    return nilai_bawaan;
}

// Fungsi mengambil angka dari SNUL dengan pengaman Default
int tui_ambil_gaya_angka(EnkiObject* gaya, const char* properti, int nilai_bawaan) {
    char* hasil_teks = tui_ambil_gaya_teks(gaya, properti, NULL);
    if (hasil_teks != NULL) return atoi(hasil_teks);
    return nilai_bawaan;
}

// ========================================================
// 🔧 PENGATUR TERMINAL
// 🟢 SUNTIKAN: Terima angka milidetik
void tui_aktifkan_raw_mode(int timeout_ms) {
    tcgetattr(STDIN_FILENO, &terminal_lama);
    struct termios terminal_baru = terminal_lama;
    terminal_baru.c_lflag &= ~(ICANON | ECHO);
    
    if (timeout_ms > 0) {
        // Mode Game Loop (Berdetak)
        terminal_baru.c_cc[VMIN] = 0;  
        // VTIME menggunakan format desidetik (1/10 detik). Jadi 100ms = 1.
        int decidetik = timeout_ms / 100;
        terminal_baru.c_cc[VTIME] = decidetik > 0 ? decidetik : 1; 
    } else {
        // Mode UI Normal (Menunggu Input / Blocking)
        terminal_baru.c_cc[VMIN] = 1;  
        terminal_baru.c_cc[VTIME] = 0; 
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_baru);
}

void tui_matikan_raw_mode() { tcsetattr(STDIN_FILENO, TCSANOW, &terminal_lama); }
void tangkap_kiamat_sigint(int sig) {
    (void)sig;
    printf("\033[?1006l\033[?1000l\033[?1003l\033[?1049l\033[?25h");
    tui_matikan_raw_mode();
    printf("\n[SISTEM] Dibunuh secara paksa oleh Ctrl+C (SIGINT)\n");
    exit(0);
}
void tui_bersihkan_layar() { printf("\033[H\033[J"); }
void tui_pindah_kursor(int x, int y) { printf("\033[%d;%dH", y, x); }

// ========================================================
// 💅 FITUR WARNA LENGKAP & HEX!
// ========================================================
void tui_terapkan_warna(EnkiObject* gaya) {
    char* w = tui_ambil_gaya_teks(gaya, "warna_teks", NULL);
    if (!w) return;
    
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

// ========================================================
// ⌨️ PENYUNTIK TEKS & CENTANG KE DALAM DOM UNUL!
// ========================================================
void tui_suntik_teks(EnkiObject* elemen, char c) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;
    EnkiObject* anak_anak = NULL;
    for(int i=0; i<elemen->panjang; i++) {
        if(strcmp(elemen->nilai.objek_peta.kunci[i]->nilai.teks, "anak_anak") == 0) { anak_anak = elemen->nilai.objek_peta.konten[i]; break; }
    }
    if (anak_anak && anak_anak->tipe == ENKI_ARRAY && anak_anak->panjang > 0) {
        EnkiObject* node_teks = anak_anak->nilai.array_elemen[0];
        if (node_teks && node_teks->tipe == ENKI_OBJEK) {
            for(int j=0; j<node_teks->panjang; j++) {
                if(strcmp(node_teks->nilai.objek_peta.kunci[j]->nilai.teks, "isi") == 0) {
                    EnkiObject* target = node_teks->nilai.objek_peta.konten[j];
                    char buffer[4096] = {0};
                    if(target->nilai.teks) strncpy(buffer, target->nilai.teks, 4095);

                    if (c == 127 || c == 8) { 
                        int len = strlen(buffer); if (len > 0) buffer[len-1] = '\0';
                    } else if (c >= 32 && c <= 126) { 
                        int len = strlen(buffer); if (len < 4094) { buffer[len] = c; buffer[len+1] = '\0'; }
                    }
                    // 🟢 MENGGUNAKAN DEWA MEMORI ENKI
                    enki_bebas(target->nilai.teks, 1); 
                    target->nilai.teks = enki_salin_teks(buffer, 1); 
                    target->panjang = strlen(buffer);
                    return;
                }
            }
        }
    }
}

// 🟢 SIHIR BARU: Tulis status centang langsung ke Objek UI agar UNUL bisa membacanya!
void tui_toggle_centang(EnkiObject* elemen) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;
    EnkiObject* anak_anak = NULL;
    for(int i=0; i<elemen->panjang; i++) {
        if(strcmp(elemen->nilai.objek_peta.kunci[i]->nilai.teks, "anak_anak") == 0) { anak_anak = elemen->nilai.objek_peta.konten[i]; break; }
    }
    if (anak_anak && anak_anak->tipe == ENKI_ARRAY && anak_anak->panjang > 0) {
        EnkiObject* node_teks = anak_anak->nilai.array_elemen[0];
        for(int j=0; j<node_teks->panjang; j++) {
            if(strcmp(node_teks->nilai.objek_peta.kunci[j]->nilai.teks, "isi") == 0) {
                EnkiObject* target = node_teks->nilai.objek_peta.konten[j];
                char* isi_skrg = target->nilai.teks;
                char* isi_baru = (isi_skrg && strcmp(isi_skrg, "X") == 0) ? " " : "X"; // Toggle X dan Spasi
                
                enki_bebas(target->nilai.teks, 1);
                target->nilai.teks = enki_salin_teks(isi_baru, 1);
                target->panjang = strlen(isi_baru);
                return;
            }
        }
    }
}

// 🟢 MESIN CETAK CANGGIH (V3): Mendukung Enter (\n), Rata Tengah Mutlak, & Pembersihan Spasi!
void tui_cetak_wrap(int x, int y, int w, const char* teks, int lantai_terminal, const char* perataan) {
    if (!teks || w <= 0) return;
    
    int mode_rata = 0; 
    if (perataan) {
        if (strcmp(perataan, "tengah") == 0) mode_rata = 1;
        else if (strcmp(perataan, "kanan") == 0) mode_rata = 2;
    }

    int baris = 0;
    const char* ptr = teks;
    
    while (*ptr != '\0') {
        // 🟢 Abaikan spasi awal untuk Rata Tengah (agar ASCII Art simetris!)
        if (mode_rata == 1) {
            while (*ptr == ' ') ptr++; 
        }

        // Cari ujung baris (Enter atau habis memori)
        int len_baris = 0;
        while (ptr[len_baris] != '\0' && ptr[len_baris] != '\n' && len_baris < w) {
            len_baris++;
        }
        
        if (y + baris >= 2 && y + baris <= lantai_terminal) {
            int offset_x = x;
            if (mode_rata == 1) offset_x = x + (w - len_baris) / 2;
            else if (mode_rata == 2) offset_x = x + (w - len_baris);
            
            tui_pindah_kursor(offset_x, y + baris); 
            printf("%.*s", len_baris, ptr);
        }
        
        ptr += len_baris;
        if (*ptr == '\n') ptr++; 
        baris++;
    }
}

// 🟢 BATAS DINAMIS
void tui_gambar_kotak(int x, int y, int w, int h, const char* judul, EnkiObject* gaya, int lantai_terminal, int apakah_fokus) {
    char* batas = tui_ambil_gaya_teks(gaya, "batas", "ganda");
    if (strcmp(batas, "polos") == 0) return; // Mode tanpa garis!

    tui_terapkan_warna(gaya); 
    if (apakah_fokus) printf("\033[7m"); 

    char *tl="╔", *tr="╗", *bl="╚", *br="╝", *h_line="═", *v_line="║";
    if (strcmp(batas, "tunggal") == 0) { tl="┌"; tr="┐"; bl="└"; br="┘"; h_line="─"; v_line="│"; }

    if (y >= 2 && y <= lantai_terminal) { tui_pindah_kursor(x, y); printf("%s", tl); for(int i=0; i<w-2; i++) printf("%s", h_line); printf("%s", tr); }
    for(int i=1; i<h-1; i++) { if (y+i >= 2 && y+i <= lantai_terminal) { tui_pindah_kursor(x, y+i); printf("%s", v_line); tui_pindah_kursor(x+w-1, y+i); printf("%s", v_line); } }
    if (y+h-1 >= 2 && y+h-1 <= lantai_terminal) { tui_pindah_kursor(x, y+h-1); printf("%s", bl); for(int i=0; i<w-2; i++) printf("%s", h_line); printf("%s", br); }
    
    // JUDUL HANYA MUNCUL JIKA ADA
    if(judul && y >= 2 && y <= lantai_terminal && strlen(judul) > 0) { tui_pindah_kursor(x+2, y); printf("[ %s ]", judul); }
    printf("\033[0m"); 
}

// ========================================================
// 🏗️ MESIN RENDER (Dengan Flexbox & Bounding Box Saraf)
// ========================================================
int render_elemen_rekursif(EnkiObject* elemen, int x_parent, int y, int lebar_parent, int lebar_terminal, EnkiObject* gaya_root, int lantai_terminal, int* counter_interaktif, int fokus_saat_ini, EnkiObject** elemen_fokus_ptr, int level_daftar, int is_password, int is_wrap) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return y;

    char* jenis = ""; char* tag_nama = ""; char* id_nama = ""; char* isi_teks = ""; char* tipe_atribut = ""; char* teks_dalam = "";
    EnkiObject* anak_anak = NULL;

    for(int i=0; i<elemen->panjang; i++) {
        char* k = elemen->nilai.objek_peta.kunci[i]->nilai.teks; EnkiObject* v = elemen->nilai.objek_peta.konten[i];
        if(strcmp(k, "jenis") == 0) jenis = v->nilai.teks;
        else if(strcmp(k, "tag") == 0) tag_nama = v->nilai.teks;
        else if(strcmp(k, "id") == 0) id_nama = v->nilai.teks;
        else if(strcmp(k, "isi") == 0) isi_teks = v->nilai.teks;
        else if(strcmp(k, "teks_dalam") == 0) teks_dalam = v->nilai.teks; 
        else if(strcmp(k, "atribut") == 0) tipe_atribut = v->nilai.teks;
        else if(strcmp(k, "anak_anak") == 0) anak_anak = v;
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

    int lebar_aktual = tui_ambil_gaya_angka(gaya_elemen, "lebar", lebar_parent);
    int tinggi_minimal = tui_ambil_gaya_angka(gaya_elemen, "tinggi", 3);
    char* susunan = tui_ambil_gaya_teks(gaya_elemen, "susunan", "kiri");
    char* perataan = tui_ambil_gaya_teks(gaya_elemen, "perataan_teks", "kiri");

    int x_aktual = x_parent;
    if (strcmp(susunan, "tengah") == 0) x_aktual = x_parent + (lebar_parent / 2) - (lebar_aktual / 2);
    else if (strcmp(susunan, "kanan") == 0) x_aktual = x_parent + lebar_parent - lebar_aktual;

    int y_awal = y; int y_anak = y;        

    int interaktif = (strcmp(tag_nama, "masukan") == 0 || strcmp(tag_nama, "masukan_sandi") == 0 || strcmp(tag_nama, "areanulis") == 0 || strcmp(tag_nama, "tombol") == 0 || strcmp(tag_nama, "centang") == 0);
    int apakah_fokus = 0; int my_index = *counter_interaktif; 

    if (interaktif) {
        if (my_index == fokus_saat_ini) { apakah_fokus = 1; if (elemen_fokus_ptr) *elemen_fokus_ptr = elemen; }
        (*counter_interaktif)++; y_anak = y + 1; 
    } else if (strcmp(tag_nama, "wadah") == 0 || strcmp(tag_nama, "daftar") == 0) { y_anak = y + 1; }

    int next_is_pass = is_password || (strstr(tipe_atribut, "password") != NULL) || (strcmp(tag_nama, "masukan_sandi") == 0);
    int next_is_wrap = is_wrap || (strcmp(tag_nama, "areanulis") == 0);
    int next_level = level_daftar;
    if (strcmp(tag_nama, "daftar") == 0) next_level++; 

    tui_terapkan_warna(gaya_elemen);
    if(strcmp(tag_nama, "judul") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) { tui_pindah_kursor(x_aktual+2, y_anak); printf(">> %s <<", isi_teks); }
        y_anak++; 
    } 
    else if (strcmp(tag_nama, "butir") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) {
            char* simbol[] = {"-", "•", "○", "»"}; 
            int mod_level = next_level > 0 ? next_level - 1 : 0;
            tui_pindah_kursor(x_aktual + (mod_level * 2), y_anak);
            printf("%s ", simbol[mod_level % 4]);
            x_aktual += (mod_level * 2) + 2; 
        }
    }
    else if (strcmp(tag_nama, "centang") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) {
            tui_pindah_kursor(x_aktual+2, y_anak);
            if (apakah_fokus) printf("\033[7m");
            char tanda = (isi_teks && strcmp(isi_teks, "X") == 0) ? 'X' : ' ';
            printf("[%c] %s", tanda, teks_dalam); 
        }
        y_anak++;
    }
    else if (strcmp(tag_nama, "areanulis") == 0) {
        // 🟢 KOSONGKAN! Biarkan anak-anaknya yang berjenis 'teks' yang mencetak ke layar!
    }
    else if(strcmp(jenis, "teks") == 0) {
        if (y_anak >= 2 && y_anak <= lantai_terminal) { 
            tui_pindah_kursor(x_aktual+2, y_anak); 
            if (apakah_fokus) printf("\033[7m");
            
            if (next_is_pass) {
                for(size_t i=0; i<strlen(isi_teks); i++) printf("*");
            } else if (next_is_wrap) {
                int w_area = lebar_aktual - 4;
                if (w_area > 0) {
                    tui_cetak_wrap(x_aktual+2, y_anak, w_area, isi_teks, lantai_terminal, perataan);
                    y_anak += (strlen(isi_teks) / w_area);
                }
            } else {
                printf("%s", isi_teks); 
            }
        }
        y_anak++; 
    }
    printf("\033[0m"); 

    // 🟢 RENDER SEMUA ANAK SECARA ALAMI (Tanpa Perisai Blokir!)
    if(anak_anak && anak_anak->tipe == ENKI_ARRAY) {
        for(int i=0; i<anak_anak->panjang; i++) {
            y_anak = render_elemen_rekursif(anak_anak->nilai.array_elemen[i], x_aktual+2, y_anak, lebar_aktual-4, lebar_terminal, gaya_root, lantai_terminal, counter_interaktif, fokus_saat_ini, elemen_fokus_ptr, next_level, next_is_pass, next_is_wrap);
        }
    }
    
    if (interaktif || strcmp(tag_nama, "wadah") == 0) {
        int tinggi = (y_anak - y_awal) + 1; if (tinggi < tinggi_minimal) tinggi = tinggi_minimal; 
        
        if (interaktif && mouse_x >= x_aktual && mouse_x <= x_aktual + lebar_aktual && mouse_y >= y_awal && mouse_y <= y_awal + tinggi) {
            apakah_fokus = 1; 
        }

        tui_gambar_kotak(x_aktual, y_awal, lebar_aktual, tinggi, NULL, gaya_elemen, lantai_terminal, apakah_fokus);
        
        if (interaktif && jumlah_kotak < 1024 && strlen(id_nama) > 0) {
            strncpy(daftar_kotak[jumlah_kotak].id, id_nama, 127);
            daftar_kotak[jumlah_kotak].x = x_aktual;
            daftar_kotak[jumlah_kotak].y = y_awal;
            daftar_kotak[jumlah_kotak].w = lebar_aktual;
            daftar_kotak[jumlah_kotak].h = tinggi;
            daftar_kotak[jumlah_kotak].indeks_logika = my_index;
            daftar_kotak[jumlah_kotak].node_asli = elemen; 
            jumlah_kotak++;
        }

        y_anak = y_awal + tinggi; 
    }
    return y_anak; 
}

// ========================================================
// 🎮 EVENT LOOP TUI UTAMA
// ========================================================
EnkiObject* tampilkan_tui(EnkiObject* ui_root, EnkiObject* gaya_root, int timeout_ms) {
    
    // 🛡️ PERISAI ANTI CORE-DUMP: Pastikan ui_root benar-benar Objek DOM, bukan String Error!
    if (!ui_root || ui_root->tipe != ENKI_OBJEK) {
        printf("\033[H\033[J\n\n🚨 [FATAL ERROR TUI] 🚨\nUI yang diberikan bukan Objek DOM OTIM yang valid!\n");
        if (ui_root && ui_root->tipe == ENKI_TEKS) printf("Pesan Error: %s\n", ui_root->nilai.teks);
        printf("\nSistem dihentikan otomatis dalam 3 detik...\n");
        sleep(3);
        return ciptakan_teks("TUTUP_PAKSA", 1);
    }

    // 🟢 SUNTIKAN KECERDASAN: Baca Saklar "game_loop" dari Kosmetik SNUL!
    if (timeout_ms == 0 && ui_root->panjang > 0) {
        char* t_tag = tui_ambil_gaya_teks(ui_root, "tag", "wadah");
        char* t_id = tui_ambil_gaya_teks(ui_root, "id", "utama");
        char selektor_root[256]; snprintf(selektor_root, 256, "@%s.%s", t_tag, t_id);
        
        if (gaya_root && gaya_root->tipe == ENKI_OBJEK) {
            for(int i=0; i<gaya_root->panjang; i++) {
                if(strcmp(gaya_root->nilai.objek_peta.kunci[i]->nilai.teks, selektor_root) == 0) {
                    timeout_ms = tui_ambil_gaya_angka(gaya_root->nilai.objek_peta.konten[i], "game_loop", 0);
                    break;
                }
            }
        }
    }

    tui_aktifkan_raw_mode(timeout_ms);
    signal(SIGINT, tangkap_kiamat_sigint);

    // 🟢 AKTIFKAN MOUSE TRACKING PENUH
    printf("\033[?1049h\033[?25l\033[?1003h\033[?1006h"); 

    int berjalan = 1; int scroll_y = 0; int indeks_fokus = 0; int total_interaktif = 0;
    EnkiObject* elemen_fokus_saat_ini = NULL; char* id_yang_diklik = NULL; 

    long long waktu_klik_terakhir = 0;
    int indeks_klik_terakhir = -1;
    static char buffer_id_dinamis[256]; 

    while (berjalan) {
        struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); 
        int lantai_terminal = w.ws_row; 
        int lebar_terminal = w.ws_col;
        tui_bersihkan_layar();
        jumlah_kotak = 0; 

        EnkiObject* root_anak = NULL;
        for(int i=0; i<ui_root->panjang; i++) {
            if(strcmp(ui_root->nilai.objek_peta.kunci[i]->nilai.teks, "anak_anak") == 0) { root_anak = ui_root->nilai.objek_peta.konten[i]; break; }
        }

        int counter_saat_ini = 0; elemen_fokus_saat_ini = NULL;

        int lebar_root = 80; // Default
        if (ui_root->panjang > 0) {
            char* t_tag = tui_ambil_gaya_teks(ui_root, "tag", "wadah");
            char* t_id = tui_ambil_gaya_teks(ui_root, "id", "utama");
            char selektor_root[256];
            snprintf(selektor_root, 256, "@%s.%s", t_tag, t_id);
            for(int i=0; i<gaya_root->panjang; i++) {
                if(strcmp(gaya_root->nilai.objek_peta.kunci[i]->nilai.teks, selektor_root) == 0) {
                    lebar_root = tui_ambil_gaya_angka(gaya_root->nilai.objek_peta.konten[i], "lebar", 80);
                    break;
                }
            }
        }

        if(root_anak && root_anak->tipe == ENKI_ARRAY) {
            int y_sekarang = 2 + scroll_y; 
            for(int i=0; i<root_anak->panjang; i++) {
                y_sekarang = render_elemen_rekursif(root_anak->nilai.array_elemen[i], 2, y_sekarang, lebar_root, lebar_terminal, gaya_root, lantai_terminal, &counter_saat_ini, indeks_fokus, &elemen_fokus_saat_ini, 0, 0, 0);
            }
        }
        total_interaktif = counter_saat_ini; 

        tui_pindah_kursor(1, 1);
        printf("\033[1;37m[Mouse: Scroll/Klik/Hover | TAB: Pindah | SPASI: Centang | ENTER: Aksi | ESC: Keluar]\033[0m"); fflush(stdout);

        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            
            // 🟢 TANGKAP ENTER UNTUK GAME/AKSI UMUM
            if (c == '\n' || c == '\r') {
                if (elemen_fokus_saat_ini) {
                    int adlh_tombol = 0;
                    for(int i=0; i<elemen_fokus_saat_ini->panjang; i++) {
                        char* tg = elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks;
                        if(strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "tag") == 0 && strcmp(tg, "tombol") == 0) adlh_tombol = 1;
                        if (adlh_tombol && strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "id") == 0) {
                            id_yang_diklik = elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks;
                        }
                    }
                    if (adlh_tombol) berjalan = 0; 
                    else { id_yang_diklik = "ENTER"; berjalan = 0; } 
                } else {
                    id_yang_diklik = "ENTER"; berjalan = 0;
                }
            }
            else if (c == 9) { if (total_interaktif > 0) indeks_fokus = (indeks_fokus + 1) % total_interaktif; } 
            else if (c == '\033') { 
                char seq[4];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        
                        // 🟢 KECERDASAN GANDA: SCROLL (UI MODE) ATAU GERAK (GAME MODE)
                        if (seq[1] == 'A') { 
                            if (timeout_ms > 0) { id_yang_diklik = "PANAH_ATAS"; berjalan = 0; }
                            else { scroll_y++; } 
                        } 
                        else if (seq[1] == 'B') { 
                            if (timeout_ms > 0) { id_yang_diklik = "PANAH_BAWAH"; berjalan = 0; }
                            else { scroll_y--; } 
                        } 
                        
                        // 🟢 TANGKAP PANAH KANAN/KIRI (Selalu dikirim ke UNUL)
                        else if (seq[1] == 'C') { id_yang_diklik = "PANAH_KANAN"; berjalan = 0; }
                        else if (seq[1] == 'D') { id_yang_diklik = "PANAH_KIRI"; berjalan = 0; }
                        
                        // 🟢 TANGKAP LOGIKA MOUSE MENDALAM (SGR 1006)
                        else if (seq[1] == '<') { 
                            char mb[32] = {0}; int m_idx = 0; char m_c;
                            while (read(STDIN_FILENO, &m_c, 1) == 1) { mb[m_idx++] = m_c; if (m_c == 'M' || m_c == 'm') break; }
                            
                            int cb, cx, cy;
                            if (sscanf(mb, "%d;%d;%d", &cb, &cx, &cy) == 3) {
                                mouse_x = cx; mouse_y = cy; 

                                if (mb[m_idx-1] == 'M') { 
                                    if (cb == 64) scroll_y++; else if (cb == 65) scroll_y--; 
                                    else if (cb == 0 || cb == 2) { 
                                        for (int i = 0; i < jumlah_kotak; i++) {
                                            KotakInteraktif* ktk = &daftar_kotak[i];
                                            if (cx >= ktk->x && cx <= ktk->x + ktk->w && cy >= ktk->y && cy <= ktk->y + ktk->h) {
                                                indeks_fokus = ktk->indeks_logika; 
                                                
                                                if (cb == 0) { // Klik Kiri
                                                    long long waktu_sekarang = dapatkan_waktu_ms();
                                                    int adalah_double_click = 0;

                                                    if (indeks_fokus == indeks_klik_terakhir && (waktu_sekarang - waktu_klik_terakhir) < 300) {
                                                        adalah_double_click = 1;
                                                    }
                                                    
                                                    waktu_klik_terakhir = waktu_sekarang;
                                                    indeks_klik_terakhir = indeks_fokus;

                                                    if (adalah_double_click) {
                                                        snprintf(buffer_id_dinamis, sizeof(buffer_id_dinamis), "GANDA_%s", ktk->id);
                                                        id_yang_diklik = buffer_id_dinamis;
                                                        berjalan = 0; 
                                                    } 
                                                    else {
                                                        if (strstr(ktk->id, "btn_") != NULL || strstr(ktk->id, "tombol_") != NULL) {
                                                            id_yang_diklik = ktk->id; 
                                                            berjalan = 0; 
                                                        } else if (strstr(ktk->id, "centang") != NULL) {
                                                            tui_toggle_centang(ktk->node_asli); 
                                                        }
                                                    }
                                                }
                                                break; 
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else { berjalan = 0; } // 🟢 Keluar jika Timeout!
            }
            else {
                int adlh_input = 0; int adlh_tombol = 0; int adlh_centang = 0;
                if (elemen_fokus_saat_ini) {
                    for(int i=0; i<elemen_fokus_saat_ini->panjang; i++) {
                        if(strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "tag") == 0) {
                            char* tg = elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks;
                            if(strcmp(tg, "masukan") == 0 || strcmp(tg, "masukan_sandi") == 0 || strcmp(tg, "areanulis") == 0) adlh_input = 1;
                            else if(strcmp(tg, "tombol") == 0) adlh_tombol = 1;
                            else if(strcmp(tg, "centang") == 0) adlh_centang = 1; 
                            break;
                        }
                    }
                }
                
                if (adlh_centang && c == ' ') { tui_toggle_centang(elemen_fokus_saat_ini); }
                else if (adlh_input) { if (c != '\n' && c != '\r') tui_suntik_teks(elemen_fokus_saat_ini, c); } 
                else if (adlh_tombol && (c == '\n' || c == '\r' || c == ' ')) {
                    for(int i=0; i<elemen_fokus_saat_ini->panjang; i++) {
                        if(strcmp(elemen_fokus_saat_ini->nilai.objek_peta.kunci[i]->nilai.teks, "id") == 0) { id_yang_diklik = elemen_fokus_saat_ini->nilai.objek_peta.konten[i]->nilai.teks; break; }
                    }
                    berjalan = 0; 
                } 
            }
        } 
        else {
            // 🟢 GAME LOOP: Jika timeout_ms berlalu tanpa input, kirim "DETAK"!
            id_yang_diklik = "DETAK";
            berjalan = 0;
        }
    } 
    
    printf("\033[?1006l\033[?1003l\033[?1000l\033[?1049l\033[?25h"); 
    tui_matikan_raw_mode();
    
    return id_yang_diklik ? ciptakan_teks(id_yang_diklik, 1) : ciptakan_teks("TUTUP_PAKSA", 1);
}