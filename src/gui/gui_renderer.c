#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

// =======================================================
// 🟢 SIHIR MEMBUNGKAM GCC: Hentikan cerewetnya untuk Raygui!
// =======================================================
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#pragma GCC diagnostic pop // Kembalikan aturan GCC seperti semula!
// =======================================================

// 🟢 URUTAN INCLUDE MUTLAK
#include "../core/enki_object.h"  
#include "../core/enki_memory.h"  
#include "../snul/snul_lexer.h"   
#include "../snul/snul_parser.h"
#include "../otim/otim_lexer.h"   
#include "../otim/otim_parser.h"  
#include "gui_renderer.h"

// 🟢 UPGRADE STANDAR 64KB UNTUK KOTAK MASUKAN
#define MAKSIMAL_INPUT 20
static char daftar_id[MAKSIMAL_INPUT][64] = {0};
static char daftar_nilai[MAKSIMAL_INPUT][65536] = {0}; 
static char daftar_label[MAKSIMAL_INPUT][256] = {0}; // 🟢 CACHE LABEL AMAN!
static bool daftar_fokus[MAKSIMAL_INPUT] = {false}; 
static int total_input_terdaftar = 0;

// Peta Tekstur untuk Cache Gambar agar memori GUI tidak bocor
#define MAKSIMAL_GAMBAR 10
static Texture2D cache_gambar[MAKSIMAL_GAMBAR];
static char cache_jalur_gambar[MAKSIMAL_GAMBAR][256] = {0};
static int total_gambar_dimuat = 0;

static unsigned char hex_ke_byte(char a, char b) {
    unsigned char val = 0;
    if (a >= '0' && a <= '9') val += (a - '0') * 16;
    else if (a >= 'a' && a <= 'f') val += (a - 'a' + 10) * 16;
    else if (a >= 'A' && a <= 'F') val += (a - 'A' + 10) * 16;
    if (b >= '0' && b <= '9') val += (b - '0');
    else if (b >= 'a' && b <= 'f') val += (b - 'a' + 10);
    else if (b >= 'A' && b <= 'F') val += (b - 'A' + 10);
    return val;
}

Color heks_ke_warna(const char* hex, Color warna_bawaan) {
    if (!hex) return warna_bawaan;
    const char* start = strchr(hex, '#');
    if (!start) return warna_bawaan;
    
    int len = 0;
    while (start[1+len] && ((start[1+len]>='0' && start[1+len]<='9') || 
                            (start[1+len]>='a' && start[1+len]<='f') || 
                            (start[1+len]>='A' && start[1+len]<='F'))) len++;
    if (len >= 8) {
        return (Color){ hex_ke_byte(start[1], start[2]), hex_ke_byte(start[3], start[4]), 
                        hex_ke_byte(start[5], start[6]), hex_ke_byte(start[7], start[8]) };
    } else if (len >= 6) {
        return (Color){ hex_ke_byte(start[1], start[2]), hex_ke_byte(start[3], start[4]), 
                        hex_ke_byte(start[5], start[6]), 255 };
    }
    return warna_bawaan;
}

// 🟢 ASISTEN TATA LETAK: Mengambil properti SNUL
static void terapkan_aturan_snul(EnkiObject* gaya, const char* target, Color* bg, Color* fg, int* bingkai, int* lebar, int* tinggi, int* ukuran_skala, char* susunan, int* ukuran_teks, int* padding_x, int* padding_y, int* gap_elemen) {
    if (!gaya || gaya->tipe != ENKI_OBJEK || !target) return;
    size_t target_len = strlen(target);
    
    for (int i = 0; i < gaya->panjang; i++) {
        char* kunci = gaya->nilai.objek_peta.kunci[i]->nilai.teks;
        if (strncmp(kunci, target, target_len) == 0) {
            char char_setelahnya = kunci[target_len];
            if (char_setelahnya == '\0' || char_setelahnya == ' ' || char_setelahnya == '\n' || char_setelahnya == '\r') {
                EnkiObject* aturan = gaya->nilai.objek_peta.konten[i];
                for (int j = 0; j < aturan->panjang; j++) {
                    char* prop = aturan->nilai.objek_peta.kunci[j]->nilai.teks;
                    char* val = aturan->nilai.objek_peta.konten[j]->nilai.teks;
                    
                    char prop_bersih[64] = {0};
                    sscanf(prop, " %s", prop_bersih);
                    
                    if (strcmp(prop_bersih, "warna_latar") == 0) *bg = heks_ke_warna(val, *bg);
                    else if (strcmp(prop_bersih, "warna_teks") == 0) *fg = heks_ke_warna(val, *fg);
                    else if (strcmp(prop_bersih, "bingkai") == 0 && strstr(val, "ganda") != NULL) *bingkai = 1;
                    else if (strcmp(prop_bersih, "lebar") == 0) {
                        if (strstr(val, "penuh") != NULL) *lebar = GetScreenWidth() - 80; 
                        else *lebar = atoi(val); 
                    }
                    else if (strcmp(prop_bersih, "tinggi") == 0) *tinggi = atoi(val);
                    else if (strcmp(prop_bersih, "ukuran") == 0) *ukuran_skala = atoi(val);
                    else if (strcmp(prop_bersih, "susunan") == 0) sscanf(val, " %s", susunan); 
                    else if (strcmp(prop_bersih, "ukuran_teks") == 0) *ukuran_teks = atoi(val); 
                    else if (strcmp(prop_bersih, "padding_x") == 0) { if (padding_x) *padding_x = atoi(val); } 
                    else if (strcmp(prop_bersih, "padding_y") == 0) { if (padding_y) *padding_y = atoi(val); } 
                    else if (strcmp(prop_bersih, "gap") == 0) { if (gap_elemen) *gap_elemen = atoi(val); }
                }
                break;
            }
        }
    }
}

// 🟢 ENGINE RENDER DOM UTAMA 
void gambar_elemen_otim(EnkiObject* elemen, EnkiObject* gaya_bawaan, EnkiObject* gaya_dev, int* x_kursor, int* y_kursor, int* tinggi_baris_maks, Vector2 mouse, bool klik_kiri, char* aksi_kembalian, Color warna_teks_turunan) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;
    char* jenis = NULL; char* tag_nama = ""; char* tag_id = "";
    char* atribut = ""; char* teks_isi = "";
    char* teks_dalam = ""; EnkiObject* anak_anak = NULL;

    for (int i = 0; i < elemen->panjang; i++) {
        char* k = elemen->nilai.objek_peta.kunci[i]->nilai.teks;
        EnkiObject* v = elemen->nilai.objek_peta.konten[i];
        if (strcmp(k, "jenis") == 0) jenis = v->nilai.teks;
        else if (strcmp(k, "tag") == 0) tag_nama = v->nilai.teks;
        else if (strcmp(k, "id") == 0) tag_id = v->nilai.teks;
        else if (strcmp(k, "atribut") == 0) atribut = v->nilai.teks;
        else if (strcmp(k, "teks_input") == 0) teks_isi = v->nilai.teks;
        else if (strcmp(k, "teks_dalam") == 0) teks_dalam = v->nilai.teks; 
        else if (strcmp(k, "anak_anak") == 0) anak_anak = v;
    }
    if (!jenis) return;

    if (strcmp(jenis, "akar_dokumen") == 0 && anak_anak) {
        int t_maks = 0;
        for (int i = 0; i < anak_anak->panjang; i++) {
            gambar_elemen_otim(anak_anak->nilai.array_elemen[i], gaya_bawaan, gaya_dev, x_kursor, y_kursor, &t_maks, mouse, klik_kiri, aksi_kembalian, warna_teks_turunan);
        }
        if (tinggi_baris_maks) *tinggi_baris_maks = t_maks; 
        return;
    }

    char selektor[64] = "";
    if (strlen(atribut) > 0) sscanf(atribut, " %s", selektor); 

    Color warna_latar = BLANK; 
    Color warna_teks = warna_teks_turunan; 
    int bingkai_ganda = 0;
    int lebar_dinamis = -1;
    int tinggi_dinamis = -1;
    int ukuran_skala = -1;
    char susunan_elemen[32] = "kolom"; 
    int font_size = 20; 
    int jarak_gap = 0; 

    char nama_tag_snul[64];
    snprintf(nama_tag_snul, sizeof(nama_tag_snul), "@%s", tag_nama); 

    terapkan_aturan_snul(gaya_bawaan, "@*", &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis, &tinggi_dinamis, &ukuran_skala, susunan_elemen, &font_size, NULL, NULL, &jarak_gap);
    terapkan_aturan_snul(gaya_bawaan, nama_tag_snul, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis, &tinggi_dinamis, &ukuran_skala, susunan_elemen, &font_size, NULL, NULL, &jarak_gap);
    if (selektor[0] == '@') terapkan_aturan_snul(gaya_bawaan, selektor, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis, &tinggi_dinamis, &ukuran_skala, susunan_elemen, &font_size, NULL, NULL, &jarak_gap);
    
    terapkan_aturan_snul(gaya_dev, "@*", &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis, &tinggi_dinamis, &ukuran_skala, susunan_elemen, &font_size, NULL, NULL, &jarak_gap);
    terapkan_aturan_snul(gaya_dev, nama_tag_snul, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis, &tinggi_dinamis, &ukuran_skala, susunan_elemen, &font_size, NULL, NULL, &jarak_gap);
    if (selektor[0] == '@') terapkan_aturan_snul(gaya_dev, selektor, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis, &tinggi_dinamis, &ukuran_skala, susunan_elemen, &font_size, NULL, NULL, &jarak_gap);

    int kotak_w = 0, kotak_h = 0;

    if (strcmp(tag_nama, "wadah") == 0 || strcmp(tag_nama, "wadah_dinamis") == 0) {
        int tinggi_kalkulasi = (tinggi_dinamis == -1) ? 40 : tinggi_dinamis;
        
        if (strcmp(tag_nama, "wadah_dinamis") == 0 && anak_anak && anak_anak->tipe == ENKI_ARRAY) {
            for (int i = 0; i < anak_anak->panjang; i++) {
                tinggi_kalkulasi += 50; 
                EnkiObject* child = anak_anak->nilai.array_elemen[i];
                char* t_isi = ""; char* tg = "";
                for (int j = 0; j < child->panjang; j++) {
                    char* k = child->nilai.objek_peta.kunci[j]->nilai.teks;
                    if (strcmp(k, "teks_input") == 0) t_isi = child->nilai.objek_peta.konten[j]->nilai.teks;
                    if (strcmp(k, "tag") == 0) tg = child->nilai.objek_peta.konten[j]->nilai.teks;
                }
                if (strcmp(tg, "teks") == 0) {
                    for (int k = 0; t_isi[k] != '\0'; k++) {
                        if (t_isi[k] == '\n') tinggi_kalkulasi += 30; 
                        else if (t_isi[k] == '\\' && t_isi[k+1] == 'n') tinggi_kalkulasi += 30;
                    }
                }
            }
        }

        int lebar_efektif = (lebar_dinamis == -1) ? 600 : lebar_dinamis;
        int x_anak = *x_kursor + 20;
        int y_anak = *y_kursor + 20;
        int t_baris_anak = 0;

        Rectangle area_bg = { (float)*x_kursor, (float)*y_kursor, (float)lebar_efektif, (float)tinggi_kalkulasi };
        if (warna_latar.a != 0) DrawRectangleRounded(area_bg, 0.1f, 10, warna_latar);
        
        if (anak_anak) {
            for (int i = 0; i < anak_anak->panjang; i++) {
                gambar_elemen_otim(anak_anak->nilai.array_elemen[i], gaya_bawaan, gaya_dev, &x_anak, &y_anak, &t_baris_anak, mouse, klik_kiri, aksi_kembalian, warna_teks);
            }
        }
        
        if (tinggi_dinamis == -1) {
            if (strcmp(susunan_elemen, "baris") == 0) tinggi_kalkulasi = t_baris_anak + 40; 
            else tinggi_kalkulasi = (y_anak - *y_kursor); 
        }
        
        Rectangle area_final = { (float)*x_kursor, (float)*y_kursor, (float)lebar_efektif, (float)tinggi_kalkulasi };
        if (bingkai_ganda) DrawRectangleRoundedLines(area_final, 0.1f, 10, warna_teks);

        kotak_w = lebar_efektif;
        kotak_h = tinggi_kalkulasi;
    }

    else if (strcmp(tag_nama, "gambar") == 0) {
        char sumber[256] = "";
        char* pos_src = strstr(atribut, "sumber=\"");
        if (pos_src) sscanf(pos_src + 8, "%[^\"]", sumber);
        else if ((pos_src = strstr(atribut, "sumber='"))) sscanf(pos_src + 8, "%[^']", sumber);

        if (strlen(sumber) > 0) {
            int id_cache = -1;
            for (int i = 0; i < total_gambar_dimuat; i++) {
                if (strcmp(cache_jalur_gambar[i], sumber) == 0) { id_cache = i; break; }
            }
            if (id_cache == -1 && total_gambar_dimuat < MAKSIMAL_GAMBAR) {
                cache_gambar[total_gambar_dimuat] = LoadTexture(sumber);
                strcpy(cache_jalur_gambar[total_gambar_dimuat], sumber);
                id_cache = total_gambar_dimuat;
                total_gambar_dimuat++;
            }
            if (id_cache != -1) {
                Texture2D tex = cache_gambar[id_cache];
                if (ukuran_skala != -1) {
                    float skala = (float)ukuran_skala / (float)tex.width;
                    DrawTextureEx(tex, (Vector2){*x_kursor, *y_kursor}, 0.0f, skala, WHITE);
                    kotak_w = (int)(tex.width * skala);
                    kotak_h = (int)(tex.height * skala);
                } else if (lebar_dinamis != -1 || tinggi_dinamis != -1) {
                    int final_w = (lebar_dinamis != -1) ? lebar_dinamis : tex.width;
                    int final_h = (tinggi_dinamis != -1) ? tinggi_dinamis : tex.height;
                    Rectangle sumber_rect = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
                    Rectangle target_rect = { (float)*x_kursor, (float)*y_kursor, (float)final_w, (float)final_h };
                    Vector2 pusat = { 0.0f, 0.0f };
                    DrawTexturePro(tex, sumber_rect, target_rect, pusat, 0.0f, WHITE);
                    kotak_w = final_w;
                    kotak_h = final_h;
                } else {
                    DrawTexture(tex, *x_kursor, *y_kursor, WHITE);
                    kotak_w = tex.width;
                    kotak_h = tex.height;
                }
            }
        }
    }

    else if (strcmp(jenis, "teks") == 0 || strcmp(tag_nama, "teks") == 0 || strcmp(tag_nama, "judul") == 0) {
        char* t = (strlen(teks_isi) > 0) ? teks_isi : teks_dalam; 
        if (strlen(t) == 0) { *y_kursor += 15; return; }
        
        char* teks_copy = enki_salin_teks(t, 1); 
        for (int i = 0; teks_copy[i] != '\0'; i++) {
            if (teks_copy[i] == '\\' && teks_copy[i+1] == 'n') { teks_copy[i] = ' '; teks_copy[i+1] = '\n'; }
            if ((unsigned char)teks_copy[i] < 32 && teks_copy[i] != '\n') teks_copy[i] = '?'; 
        }
        
        int ukuran_font = (strcmp(tag_nama, "judul") == 0) ? 36 : font_size; 
        int t_y = *y_kursor;
        
        char* baris = strtok(teks_copy, "\n");
        while (baris != NULL) {
            while (*baris == ' ') baris++; 
            if (strlen(baris) > 0) DrawText(baris, *x_kursor, t_y, ukuran_font, warna_teks); 
            t_y += ukuran_font + 10; 
            baris = strtok(NULL, "\n");
        }
        enki_bebas(teks_copy, 1);
        
        kotak_w = MeasureText(t, ukuran_font); 
        kotak_h = t_y - *y_kursor;
    }

    // =================================================================
    // 🟢 OVERRIDE MUTLAK: KITA BUAT TEXTBOX KITA SENDIRI!
    // =================================================================
    else if (strcmp(tag_nama, "masukan") == 0 || strcmp(tag_nama, "masukan_sandi") == 0) {
        int tinggi_kotak = (tinggi_dinamis == -1) ? (font_size + 20) : tinggi_dinamis; 
        int lebar_kotak = (lebar_dinamis == -1) ? 300 : lebar_dinamis; 
        Rectangle area = { (float)(*x_kursor + 250), (float)*y_kursor, (float)lebar_kotak, (float)tinggi_kotak };
        
        char id_bersih[64] = {0}; 
        sscanf(tag_id, " %s", id_bersih);
        int indeks_saya = -1;
        
        for (int i = 0; i < total_input_terdaftar; i++) {
            if (strcmp(daftar_id[i], id_bersih) == 0) { indeks_saya = i; break; }
        }
        if (indeks_saya == -1 && total_input_terdaftar < MAKSIMAL_INPUT) {
            indeks_saya = total_input_terdaftar;
            strcpy(daftar_id[indeks_saya], id_bersih);
            
            // 🟢 SIMPAN LABEL ASLI KE CACHE C (Agar teks_dalam aman diretas)
            if (strlen(teks_dalam) > 0) strcpy(daftar_label[indeks_saya], teks_dalam);
            else strcpy(daftar_label[indeks_saya], atribut);
            
            // 🟢 TARIK NILAI DARI DOM KE DALAM C
            for (int j = 0; j < elemen->panjang; j++) {
                if (strcmp(elemen->nilai.objek_peta.kunci[j]->nilai.teks, "teks_input") == 0) {
                    strncpy(daftar_nilai[indeks_saya], elemen->nilai.objek_peta.konten[j]->nilai.teks, 65000);
                    break;
                }
            }
            total_input_terdaftar++;
        }

        // 🟢 GAMBAR LABEL DARI CACHE, BUKAN DARI DOM
        DrawText(daftar_label[indeks_saya], *x_kursor, *y_kursor + (font_size/2), font_size, warna_teks_turunan); 

        if (indeks_saya != -1) {
            bool mode_sandi = (strcmp(tag_nama, "masukan_sandi") == 0);
            
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, area)) {
                    daftar_fokus[indeks_saya] = true; 
                    for (int j = 0; j < MAKSIMAL_INPUT; j++) {
                        if (j != indeks_saya) daftar_fokus[j] = false;
                    }
                } else {
                    daftar_fokus[indeks_saya] = false; 
                }
            }

            if (daftar_fokus[indeks_saya]) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125)) { 
                        int len = strlen(daftar_nilai[indeks_saya]);
                        if (len < 65000) {
                            daftar_nilai[indeks_saya][len] = (char)key;
                            daftar_nilai[indeks_saya][len+1] = '\0';
                        }
                    }
                    key = GetCharPressed();
                }
                
                if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                    int len = strlen(daftar_nilai[indeks_saya]);
                    if (len > 0) daftar_nilai[indeks_saya][len-1] = '\0';
                }

                bool ctrl_ditekan = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                if (ctrl_ditekan && IsKeyPressed(KEY_C)) {
                    SetClipboardText(daftar_nilai[indeks_saya]);
                }
                if (ctrl_ditekan && IsKeyPressed(KEY_V)) {
                    const char* teks_paste = GetClipboardText();
                    if (teks_paste != NULL) {
                        int len_sekarang = strlen(daftar_nilai[indeks_saya]);
                        int len_paste = strlen(teks_paste);
                        if (len_sekarang + len_paste < 65000) {
                            strcat(daftar_nilai[indeks_saya], teks_paste);
                        }
                    }
                }
            }

            GuiSetStyle(TEXTBOX, BASE_COLOR_NORMAL, ColorToInt(RAYWHITE));
            GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, daftar_fokus[indeks_saya] ? ColorToInt(RED) : ColorToInt(warna_teks));
            GuiTextBox(area, "", 65535, false);

            char teks_tampil[2048] = {0};
            if (mode_sandi) {
                size_t panjang_sandi = strlen(daftar_nilai[indeks_saya]);
                if (panjang_sandi > 120) panjang_sandi = 120; 
                for(size_t b=0; b<panjang_sandi; b++) teks_tampil[b] = '*';
                teks_tampil[panjang_sandi] = '\0';
            } else {
                strncpy(teks_tampil, daftar_nilai[indeks_saya], 2000);
            }
            DrawText(teks_tampil, area.x + 10, area.y + (area.height/2) - (font_size/2), font_size, BLACK);

            if (daftar_fokus[indeks_saya]) {
                static int frame_kursor = 0;
                frame_kursor++;
                if ((frame_kursor / 30) % 2 == 0) { 
                    int text_width = MeasureText(teks_tampil, font_size);
                    DrawRectangle(area.x + 12 + text_width, area.y + (area.height/2) - (font_size/2), 2, font_size, RED);
                }
            }

            // =====================================================================
            // 🟢 SINKRONISASI DOM UNUL (ANTI GC SWEEP - THE SHIELD HACK)
            // =====================================================================
            int ada_isi = 0;
            for (int j = 0; j < elemen->panjang; j++) {
                if (strcmp(elemen->nilai.objek_peta.kunci[j]->nilai.teks, "teks_input") == 0) {
                    EnkiObject* v = elemen->nilai.objek_peta.konten[j];
                    if (strcmp(v->nilai.teks, daftar_nilai[indeks_saya]) != 0) {
                        elemen->nilai.objek_peta.konten[j] = ciptakan_teks(daftar_nilai[indeks_saya], 1);
                    }
                    ada_isi = 1; break;
                }
            }
            
            if (ada_isi == 0) {
                EnkiObject** kunci_baru = (EnkiObject**)enki_alokasi((elemen->panjang + 1) * sizeof(EnkiObject*), 1);
                EnkiObject** konten_baru = (EnkiObject**)enki_alokasi((elemen->panjang + 1) * sizeof(EnkiObject*), 1);
                
                for(int j = 0; j < elemen->panjang; j++) {
                    kunci_baru[j] = elemen->nilai.objek_peta.kunci[j];
                    konten_baru[j] = elemen->nilai.objek_peta.konten[j];
                }
                
                // 🟢 PASANG PERISAI GC: Isi slot kosong dengan objek [0] yang sudah aman!
                kunci_baru[elemen->panjang] = elemen->nilai.objek_peta.kunci[0];
                konten_baru[elemen->panjang] = elemen->nilai.objek_peta.konten[0];
                
                // Pasang array ke DOM SEKARANG!
                elemen->nilai.objek_peta.kunci = kunci_baru;
                elemen->nilai.objek_peta.konten = konten_baru;
                elemen->panjang++; // DOM resmi bertambah!
                
                // 🟢 SEKARANG AMAN! Meskipun ciptakan_teks memicu GC, GC hanya akan melihat perisai kita!
                elemen->nilai.objek_peta.kunci[elemen->panjang - 1] = ciptakan_teks("teks_input", 1);
                elemen->nilai.objek_peta.konten[elemen->panjang - 1] = ciptakan_teks(daftar_nilai[indeks_saya], 1);
            }
            // =====================================================================
        }

        kotak_w = 250 + area.width; 
        kotak_h = area.height; 
    }

    // =================================================================
    // 🟢 REVOLUSI AREA NULIS (MULTI-LINE TEXTAREA DENGAN AUTO WRAP!)
    // =================================================================
    else if (strcmp(tag_nama, "areanulis") == 0) {
        int tinggi_kotak = (tinggi_dinamis == -1) ? 150 : tinggi_dinamis;
        int lebar_kotak = (lebar_dinamis == -1) ? 400 : lebar_dinamis; 
        Rectangle area = { (float)*x_kursor, (float)*y_kursor, (float)lebar_kotak, (float)tinggi_kotak };
        
        char id_bersih[64] = {0}; 
        sscanf(tag_id, " %s", id_bersih);
        int indeks_saya = -1;
        
        for (int i = 0; i < total_input_terdaftar; i++) {
            if (strcmp(daftar_id[i], id_bersih) == 0) { indeks_saya = i; break; }
        }
        if (indeks_saya == -1 && total_input_terdaftar < MAKSIMAL_INPUT) {
            indeks_saya = total_input_terdaftar;
            strcpy(daftar_id[indeks_saya], id_bersih);
            
            for (int j = 0; j < elemen->panjang; j++) {
                if (strcmp(elemen->nilai.objek_peta.kunci[j]->nilai.teks, "teks_input") == 0) {
                    strncpy(daftar_nilai[indeks_saya], elemen->nilai.objek_peta.konten[j]->nilai.teks, 65000);
                    break;
                }
            }
            total_input_terdaftar++;
        }

        if (indeks_saya != -1) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, area)) {
                    daftar_fokus[indeks_saya] = true; 
                    for (int j = 0; j < MAKSIMAL_INPUT; j++) {
                        if (j != indeks_saya) daftar_fokus[j] = false;
                    }
                } else {
                    daftar_fokus[indeks_saya] = false; 
                }
            }

            if (daftar_fokus[indeks_saya]) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125)) { 
                        int len = strlen(daftar_nilai[indeks_saya]);
                        if (len < 65000) {
                            daftar_nilai[indeks_saya][len] = (char)key;
                            daftar_nilai[indeks_saya][len+1] = '\0';
                        }
                    }
                    key = GetCharPressed();
                }
                
                if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                    int len = strlen(daftar_nilai[indeks_saya]);
                    if (len > 0) daftar_nilai[indeks_saya][len-1] = '\0';
                }

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                    int len = strlen(daftar_nilai[indeks_saya]);
                    if (len < 65000) {
                        strcat(daftar_nilai[indeks_saya], "\n");
                    }
                }
                
                bool ctrl_ditekan = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                if (ctrl_ditekan && IsKeyPressed(KEY_C)) SetClipboardText(daftar_nilai[indeks_saya]);
                if (ctrl_ditekan && IsKeyPressed(KEY_V)) {
                    const char* teks_paste = GetClipboardText();
                    if (teks_paste != NULL) {
                        int len_sekarang = strlen(daftar_nilai[indeks_saya]);
                        int len_paste = strlen(teks_paste);
                        if (len_sekarang + len_paste < 65000) strcat(daftar_nilai[indeks_saya], teks_paste);
                    }
                }
            }

            DrawRectangleRec(area, warna_latar.a == 0 ? RAYWHITE : warna_latar);
            DrawRectangleLinesEx(area, 2, daftar_fokus[indeks_saya] ? RED : warna_teks);

            BeginScissorMode(area.x, area.y, area.width, area.height);
            
            float teks_x = area.x + 5;
            float teks_y = area.y + 5;
            float spasi_w = MeasureText(" ", font_size);
            
            char* teks_salinan = enki_salin_teks(daftar_nilai[indeks_saya], 1);
            int panjang_teks = strlen(teks_salinan);
            char kata_sementara[256] = {0};
            int indeks_kata = 0;
            
            for (int i = 0; i <= panjang_teks; i++) {
                char c = teks_salinan[i];
                if (c == ' ' || c == '\n' || c == '\0') {
                    kata_sementara[indeks_kata] = '\0';
                    float lebar_kata = MeasureText(kata_sementara, font_size);
                    
                    if (teks_x + lebar_kata > area.x + area.width - 10) {
                        teks_x = area.x + 5;           
                        teks_y += (font_size + 5);     
                    }
                    
                    DrawText(kata_sementara, teks_x, teks_y, font_size, BLACK);
                    
                    teks_x += lebar_kata;
                    if (c == ' ') teks_x += spasi_w;
                    
                    if (c == '\n') {
                        teks_x = area.x + 5;
                        teks_y += (font_size + 5);
                    }
                    
                    indeks_kata = 0;
                    kata_sementara[0] = '\0';
                } else {
                    if (indeks_kata < 250) kata_sementara[indeks_kata++] = c;
                }
            }
            enki_bebas(teks_salinan, 1);

            if (daftar_fokus[indeks_saya]) {
                static int kedip = 0;
                kedip++;
                if ((kedip / 30) % 2 == 0) {
                    DrawLine(teks_x, teks_y, teks_x, teks_y + font_size, RED);
                }
            }
            
            EndScissorMode();

            // =====================================================================
            // 🟢 SINKRONISASI DOM UNUL (ANTI GC SWEEP - THE SHIELD HACK)
            // =====================================================================
            int ada_isi = 0;
            for (int j = 0; j < elemen->panjang; j++) {
                if (strcmp(elemen->nilai.objek_peta.kunci[j]->nilai.teks, "teks_input") == 0) {
                    EnkiObject* v = elemen->nilai.objek_peta.konten[j];
                    if (strcmp(v->nilai.teks, daftar_nilai[indeks_saya]) != 0) {
                        elemen->nilai.objek_peta.konten[j] = ciptakan_teks(daftar_nilai[indeks_saya], 1);
                    }
                    ada_isi = 1; break;
                }
            }
            
            if (ada_isi == 0) {
                EnkiObject** kunci_baru = (EnkiObject**)enki_alokasi((elemen->panjang + 1) * sizeof(EnkiObject*), 1);
                EnkiObject** konten_baru = (EnkiObject**)enki_alokasi((elemen->panjang + 1) * sizeof(EnkiObject*), 1);
                
                for(int j = 0; j < elemen->panjang; j++) {
                    kunci_baru[j] = elemen->nilai.objek_peta.kunci[j];
                    konten_baru[j] = elemen->nilai.objek_peta.konten[j];
                }
                
                // 🟢 PASANG PERISAI GC: Isi slot kosong dengan objek [0] yang sudah aman!
                kunci_baru[elemen->panjang] = elemen->nilai.objek_peta.kunci[0];
                konten_baru[elemen->panjang] = elemen->nilai.objek_peta.konten[0];
                
                // Pasang array ke DOM SEKARANG!
                elemen->nilai.objek_peta.kunci = kunci_baru;
                elemen->nilai.objek_peta.konten = konten_baru;
                elemen->panjang++; // DOM resmi bertambah!
                
                // 🟢 SEKARANG AMAN! Meskipun ciptakan_teks memicu GC, GC hanya akan melihat perisai kita!
                elemen->nilai.objek_peta.kunci[elemen->panjang - 1] = ciptakan_teks("teks_input", 1);
                elemen->nilai.objek_peta.konten[elemen->panjang - 1] = ciptakan_teks(daftar_nilai[indeks_saya], 1);
            }
            // =====================================================================
        }

        kotak_w = area.width; 
        kotak_h = area.height; 
    }
    
    // 🟢 ================= GAMBAR TOMBOL =================
    else if (strcmp(tag_nama, "tombol") == 0) {
        char* label_tombol = (strlen(teks_dalam) > 0) ? teks_dalam : atribut;
        int text_width = MeasureText(label_tombol, 20);
        kotak_w = (lebar_dinamis != -1) ? lebar_dinamis : (text_width + 40);
        kotak_h = (tinggi_dinamis != -1) ? tinggi_dinamis : 40;
        
        Rectangle area = { (float)*x_kursor, (float)*y_kursor, (float)kotak_w, (float)kotak_h };
        
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(warna_latar));
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(warna_teks));
        
        bool is_hovered = CheckCollisionPointRec(mouse, area);
        if (is_hovered) {
            Color h_bg = warna_latar; Color h_fg = warna_teks;
            int d_b, d_l, d_t, d_u; char d_s[32];
            int d_fs; 
            char selektor_hover[128]; snprintf(selektor_hover, sizeof(selektor_hover), "%s:hover", selektor);
            terapkan_aturan_snul(gaya_dev, selektor_hover, &h_bg, &h_fg, &d_b, &d_l, &d_t, &d_u, d_s, &d_fs, NULL, NULL, NULL); 
            char tag_hover[128]; snprintf(tag_hover, sizeof(tag_hover), "@%s:hover", tag_nama);
            terapkan_aturan_snul(gaya_dev, tag_hover, &h_bg, &h_fg, &d_b, &d_l, &d_t, &d_u, d_s, &d_fs, NULL, NULL, NULL);
            
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(h_bg));
            GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(h_fg));
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); 
        }

        if (GuiButton(area, label_tombol)) {
            char id_bersih[256]; sscanf(tag_id, " %s", id_bersih); 
            strcpy(aksi_kembalian, id_bersih);
            
            // 🟢 SIHIR PERBAIKAN MUTLAK: Bersihkan semua jejak input lama lintas dimensi!
            total_input_terdaftar = 0; 
            for(int k = 0; k < MAKSIMAL_INPUT; k++) {
                daftar_fokus[k] = false;
                memset(daftar_nilai[k], 0, sizeof(daftar_nilai[k])); 
                memset(daftar_label[k], 0, sizeof(daftar_label[k])); 
            }
        }
    }
    
    if (kotak_h > *tinggi_baris_maks) *tinggi_baris_maks = kotak_h;
    
    if (strcmp(tag_nama, "wadah") != 0 && strcmp(tag_nama, "wadah_dinamis") != 0) {
        *y_kursor += kotak_h + jarak_gap; 
    } else {
        *x_kursor += kotak_w + jarak_gap; 
    }
}

const char* SNUL_BAWAAN_MESIN = 
"@tombol {\n"
"    warna_latar: #87CEEBF0;\n" 
"    warna_teks: #FFFFFF;\n"
"}\n"
"@wadah {\n"
"    warna_latar: #D3D3D380;\n" 
"    susunan: kolom;\n"
"}\n"
"@masukan {\n"
"    warna_latar: #FFFFFF;\n"
"    warna_teks: #000000;\n"
"}\n"
"@masukan_sandi {\n"
"    warna_latar: #FFFFFF;\n"
"    warna_teks: #000000;\n"
"}\n";

char* tampilkan_gui_raylib(EnkiObject* ui_root, EnkiObject* gaya_root) {
    // 🟢 BERSIHKAN INGATAN JASAD C SETIAP KALI HALAMAN BARU DIMUAT!
    total_input_terdaftar = 0;
    for(int k=0; k < MAKSIMAL_INPUT; k++) {
        daftar_fokus[k] = false;
        daftar_nilai[k][0] = '\0'; 
        daftar_label[k][0] = '\0'; 
    }
    if (!IsWindowReady()) { 
        SetConfigFlags(FLAG_WINDOW_RESIZABLE); 
        InitWindow(800, 600, "OS Urantia - Dimensi Native GUI");
        SetTargetFPS(60); 
        GuiLoadStyleDefault(); 
    }
    
    char aksi_kembalian[256] = "";
    SnulTokenArray tokens_bawaan = snul_lexer(SNUL_BAWAAN_MESIN);
    EnkiObject* gaya_bawaan_ast = parse_snul(tokens_bawaan);

    static float scroll_y = 0;
    static int tinggi_konten_terakhir = 600; 

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        bool klik_kiri = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); 
        
        scroll_y += GetMouseWheelMove() * 40.0f; 
        if (IsKeyDown(KEY_UP)) scroll_y += 15.0f;
        if (IsKeyDown(KEY_DOWN)) scroll_y -= 15.0f;
        if (scroll_y > 0) scroll_y = 0;          

        int max_scroll = GetScreenHeight() - tinggi_konten_terakhir - 50;
        if (max_scroll > 0) max_scroll = 0; 
        if (scroll_y < max_scroll) scroll_y = max_scroll;

        int pad_x = 0; 
        int pad_y = 0; 

        BeginDrawing();
        Color bgColor = RAYWHITE;
        int d_b, d_l, d_t, d_u, d_fs; char d_s[32]; Color d_f;
        
        terapkan_aturan_snul(gaya_root, "@wadah_utama", &bgColor, &d_f, &d_b, &d_l, &d_t, &d_u, d_s, &d_fs, &pad_x, &pad_y, NULL);
        ClearBackground(bgColor);

        int x_mulai = pad_x;
        int y_mulai = pad_y + (int)scroll_y; 
        int tinggi_maks_akar = 0;
        
        if (ui_root && ui_root->tipe == ENKI_OBJEK) {
            gambar_elemen_otim(ui_root, gaya_bawaan_ast, gaya_root, &x_mulai, &y_mulai, &tinggi_maks_akar, mouse, klik_kiri, aksi_kembalian, BLACK);
        }
        
        tinggi_konten_terakhir = tinggi_maks_akar + pad_y;

        if (tinggi_konten_terakhir > GetScreenHeight()) {
            int s_height = (GetScreenHeight() * GetScreenHeight()) / tinggi_konten_terakhir;
            if (s_height < 30) s_height = 30; 
            
            float proporsi = scroll_y / (float)max_scroll;
            int s_y = (int)(proporsi * (GetScreenHeight() - s_height));
            
            Rectangle scroll_rect = { GetScreenWidth() - 15, s_y, 10, s_height };
            
            static bool sedang_ditarik = false;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, scroll_rect)) {
                sedang_ditarik = true; 
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                sedang_ditarik = false; 
            }
            
            if (sedang_ditarik) {
                float drag_proporsi = (mouse.y - (s_height / 2.0f)) / (GetScreenHeight() - s_height);
                if (drag_proporsi < 0.0f) drag_proporsi = 0.0f;
                if (drag_proporsi > 1.0f) drag_proporsi = 1.0f;
                
                scroll_y = drag_proporsi * max_scroll; 
                s_y = (int)(drag_proporsi * (GetScreenHeight() - s_height));
                scroll_rect.y = s_y;
            }

            DrawRectangleRounded(scroll_rect, 0.5f, 10, sedang_ditarik ? BLACK : Fade(DARKGRAY, 0.5f));
            
            if (!sedang_ditarik && CheckCollisionPointRec(mouse, scroll_rect)) {
                DrawRectangleRounded(scroll_rect, 0.5f, 10, Fade(BLACK, 0.8f));
            }
        }
        
        EndDrawing();

        if (strlen(aksi_kembalian) > 0) {
            scroll_y = 0; 
            break; 
        }
    }

    if (WindowShouldClose() && strlen(aksi_kembalian) == 0) { 
        strcpy(aksi_kembalian, "TUTUP_PAKSA"); 
        CloseWindow(); 
    } 
    
    if (gaya_bawaan_ast) hancurkan_objek(gaya_bawaan_ast, 1);
    bebaskan_snul_token(&tokens_bawaan);

    SetMouseCursor(MOUSE_CURSOR_DEFAULT); 

    return enki_salin_teks(aksi_kembalian, 1); 
}