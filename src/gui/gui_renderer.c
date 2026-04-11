#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

// 🟢 URUTAN INCLUDE MUTLAK
#include "../core/enki_object.h"  
#include "../snul/snul_lexer.h"   
#include "../snul/snul_parser.h"  
#include "gui_renderer.h"

#define MAKSIMAL_INPUT 20
static char daftar_id[MAKSIMAL_INPUT][64] = {0};
static char daftar_nilai[MAKSIMAL_INPUT][256] = {0};
static int total_input_terdaftar = 0;
static char id_fokus[64] = ""; 

// 🟢 MESIN BANTUAN HEX KE BYTE
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

// 🟢 MESIN WARNA KEMUTLAKAN (Murni, Bebas Bug)
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

// 🟢 PENCARI GAYA TERPUSAT (ANTI-SPASI & CASCADING)
static void terapkan_aturan_snul(EnkiObject* gaya, const char* target, Color* bg, Color* fg, int* bingkai, int* lebar) {
    if (!gaya || gaya->tipe != ENKI_OBJEK || !target) return;
    
    size_t target_len = strlen(target);
    
    for (int i = 0; i < gaya->panjang; i++) {
        char* kunci = gaya->nilai.objek_peta.kunci[i]->nilai.teks;
        
        // 🟢 Kunci Kesuksesan: strncmp mengabaikan spasi di belakang selektor SNUL!
        if (strncmp(kunci, target, target_len) == 0) {
            char char_setelahnya = kunci[target_len];
            if (char_setelahnya == '\0' || char_setelahnya == ' ' || char_setelahnya == '\n' || char_setelahnya == '\r') {
                
                EnkiObject* aturan = gaya->nilai.objek_peta.konten[i];
                for (int j = 0; j < aturan->panjang; j++) {
                    char* prop = aturan->nilai.objek_peta.kunci[j]->nilai.teks;
                    char* val = aturan->nilai.objek_peta.konten[j]->nilai.teks;
                    
                    // Bersihkan spasi dari properti
                    char prop_bersih[64] = {0};
                    sscanf(prop, " %s", prop_bersih); 

                    if (strcmp(prop_bersih, "warna_latar") == 0) *bg = heks_ke_warna(val, *bg);
                    else if (strcmp(prop_bersih, "warna_teks") == 0) *fg = heks_ke_warna(val, *fg);
                    else if (strcmp(prop_bersih, "bingkai") == 0 && strstr(val, "ganda") != NULL) *bingkai = 1;
                    else if (strcmp(prop_bersih, "lebar") == 0) {
                        if (strstr(val, "penuh") != NULL) *lebar = GetScreenWidth() - 80; 
                        else *lebar = atoi(val); 
                    }
                }
                break; 
            }
        }
    }
}

void gambar_elemen_otim(EnkiObject* elemen, EnkiObject* gaya_bawaan, EnkiObject* gaya_dev, int* x_kursor, int* y_kursor, Vector2 mouse, bool klik_kiri, char* aksi_kembalian, Color warna_teks_turunan) {
    if (!elemen || elemen->tipe != ENKI_OBJEK) return;

    char* jenis = NULL; char* tag_nama = ""; char* tag_id = "";
    char* atribut = ""; char* teks_isi = ""; EnkiObject* anak_anak = NULL;

    for (int i = 0; i < elemen->panjang; i++) {
        char* k = elemen->nilai.objek_peta.kunci[i]->nilai.teks;
        EnkiObject* v = elemen->nilai.objek_peta.konten[i];
        if (strcmp(k, "jenis") == 0) jenis = v->nilai.teks;
        else if (strcmp(k, "tag") == 0) tag_nama = v->nilai.teks;
        else if (strcmp(k, "id") == 0) tag_id = v->nilai.teks;
        else if (strcmp(k, "atribut") == 0) atribut = v->nilai.teks;
        else if (strcmp(k, "teks_input") == 0) teks_isi = v->nilai.teks;
        else if (strcmp(k, "anak_anak") == 0) anak_anak = v;
    }
    if (!jenis) return;

    if (strcmp(jenis, "akar_dokumen") == 0 && anak_anak) {
        for (int i = 0; i < anak_anak->panjang; i++) {
            gambar_elemen_otim(anak_anak->nilai.array_elemen[i], gaya_bawaan, gaya_dev, x_kursor, y_kursor, mouse, klik_kiri, aksi_kembalian, warna_teks_turunan);
        }
        return;
    }

    char selektor[64] = "";
    if (strlen(atribut) > 0) sscanf(atribut, " %s", selektor); 

    // DEKLARASI GAYA DASAR
    Color warna_latar = BLANK; 
    Color warna_teks = warna_teks_turunan; 
    int bingkai_ganda = 0;
    int lebar_dinamis = 600; 

    // 🟢 EKSEKUSI CASCADING STYLESHEET (Mewarisi & Menimpa)
    char nama_tag_snul[64];
    snprintf(nama_tag_snul, sizeof(nama_tag_snul), "@%s", tag_nama); // Buat "@tombol"

    // Tahap 1: Serap Bawaan
    terapkan_aturan_snul(gaya_bawaan, "@*", &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis);
    terapkan_aturan_snul(gaya_bawaan, nama_tag_snul, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis);
    if (selektor[0] == '@') terapkan_aturan_snul(gaya_bawaan, selektor, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis);

    // Tahap 2: Timpa dengan Gaya Dev
    terapkan_aturan_snul(gaya_dev, "@*", &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis);
    terapkan_aturan_snul(gaya_dev, nama_tag_snul, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis);
    if (selektor[0] == '@') terapkan_aturan_snul(gaya_dev, selektor, &warna_latar, &warna_teks, &bingkai_ganda, &lebar_dinamis);


    // ================= GAMBAR KOMPONEN =================
    if (strcmp(tag_nama, "wadah") == 0 || strcmp(tag_nama, "wadah_dinamis") == 0) {
        int tinggi_dinamis = 40;
        if (anak_anak && anak_anak->tipe == ENKI_ARRAY) {
            for (int i = 0; i < anak_anak->panjang; i++) {
                tinggi_dinamis += 50; 
                EnkiObject* child = anak_anak->nilai.array_elemen[i];
                char* t_isi = ""; char* tg = "";
                for (int j = 0; j < child->panjang; j++) {
                    char* k = child->nilai.objek_peta.kunci[j]->nilai.teks;
                    if (strcmp(k, "teks_input") == 0) t_isi = child->nilai.objek_peta.konten[j]->nilai.teks;
                    if (strcmp(k, "tag") == 0) tg = child->nilai.objek_peta.konten[j]->nilai.teks;
                }
                if (strcmp(tg, "teks") == 0) {
                    for (int k = 0; t_isi[k] != '\0'; k++) {
                        if (t_isi[k] == '\n') tinggi_dinamis += 30; 
                        else if (t_isi[k] == '\\' && t_isi[k+1] == 'n') tinggi_dinamis += 30;
                    }
                }
            }
        }

        Rectangle area = { (float)*x_kursor, (float)*y_kursor, (float)lebar_dinamis, (float)tinggi_dinamis };
        if (warna_latar.a != 0) DrawRectangleRounded(area, 0.1f, 10, warna_latar); 
        if (bingkai_ganda) DrawRectangleRoundedLines(area, 0.1f, 10, warna_teks);

        int awal_y = *y_kursor;
        *x_kursor += 20; *y_kursor += 20; 
        
        if (anak_anak) {
            for (int i = 0; i < anak_anak->panjang; i++) {
                gambar_elemen_otim(anak_anak->nilai.array_elemen[i], gaya_bawaan, gaya_dev, x_kursor, y_kursor, mouse, klik_kiri, aksi_kembalian, warna_teks); 
            }
        }
        *y_kursor = awal_y + tinggi_dinamis + 20; 
        *x_kursor -= 20; 
    }

    else if (strcmp(jenis, "teks") == 0 || strcmp(tag_nama, "teks") == 0) {
        char* t = (strlen(teks_isi) > 0) ? teks_isi : atribut; 
        char* teks_copy = strdup(t);
        for (int i = 0; teks_copy[i] != '\0'; i++) {
            if (teks_copy[i] == '\\' && teks_copy[i+1] == 'n') { teks_copy[i] = ' '; teks_copy[i+1] = '\n'; }
            if ((unsigned char)teks_copy[i] < 32 && teks_copy[i] != '\n') teks_copy[i] = '?'; 
        }
        char* baris = strtok(teks_copy, "\n");
        while (baris != NULL) {
            while (*baris == ' ') baris++; 
            if (strlen(baris) > 0) DrawText(baris, *x_kursor, *y_kursor, 20, warna_teks); 
            *y_kursor += 30; 
            baris = strtok(NULL, "\n");
        }
        free(teks_copy);
        *y_kursor += 15; 
    }

    else if (strcmp(tag_nama, "masukan") == 0 || strcmp(tag_nama, "masukan_sandi") == 0) {
        // 1. GAMBAR LABEL TEKS (Gunakan warna_teks_turunan dari induk, misal Hijau)
        DrawText(atribut, *x_kursor, *y_kursor, 20, warna_teks_turunan);
        
        Rectangle area = { (float)*x_kursor + 200, (float)*y_kursor - 5, 250, 30 };
        char id_bersih[64]; sscanf(tag_id, " %s", id_bersih);
        int indeks_saya = -1;
        for (int i = 0; i < total_input_terdaftar; i++) {
            if (strcmp(daftar_id[i], id_bersih) == 0) { indeks_saya = i; break; }
        }
        if (indeks_saya == -1 && total_input_terdaftar < MAKSIMAL_INPUT) {
            indeks_saya = total_input_terdaftar;
            strcpy(daftar_id[indeks_saya], id_bersih);
            total_input_terdaftar++;
        }

        bool sedang_fokus = (strcmp(id_fokus, id_bersih) == 0);
        if (CheckCollisionPointRec(mouse, area)) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM); 
            if (klik_kiri) strcpy(id_fokus, id_bersih);
        } else if (sedang_fokus) {
            if (klik_kiri) strcpy(id_fokus, "");
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        if (sedang_fokus && indeks_saya != -1) {
            if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_V)) {
                const char* clipboard = GetClipboardText();
                if (clipboard != NULL) strncat(daftar_nilai[indeks_saya], clipboard, 255 - strlen(daftar_nilai[indeks_saya]));
            } else {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125) && (strlen(daftar_nilai[indeks_saya]) < 255)) {
                        int len = strlen(daftar_nilai[indeks_saya]);
                        daftar_nilai[indeks_saya][len] = (char)key;
                        daftar_nilai[indeks_saya][len+1] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    int len = strlen(daftar_nilai[indeks_saya]);
                    if (len > 0) daftar_nilai[indeks_saya][len-1] = '\0';
                }
            }
        }

        if (indeks_saya != -1) {
            int ada_isi = 0;
            for (int j = 0; j < elemen->panjang; j++) {
                if (strcmp(elemen->nilai.objek_peta.kunci[j]->nilai.teks, "teks_input") == 0) {
                    EnkiObject* v = elemen->nilai.objek_peta.konten[j];
                    if (v->nilai.teks) free(v->nilai.teks);
                    v->nilai.teks = strdup(daftar_nilai[indeks_saya]);
                    v->panjang = strlen(v->nilai.teks);
                    ada_isi = 1; break;
                }
            }
            if (ada_isi == 0) {
                elemen->panjang++;
                elemen->nilai.objek_peta.kunci = realloc(elemen->nilai.objek_peta.kunci, elemen->panjang * sizeof(EnkiObject*));
                elemen->nilai.objek_peta.konten = realloc(elemen->nilai.objek_peta.konten, elemen->panjang * sizeof(EnkiObject*));
                elemen->nilai.objek_peta.kunci[elemen->panjang - 1] = ciptakan_teks("teks_input");
                elemen->nilai.objek_peta.konten[elemen->panjang - 1] = ciptakan_teks(daftar_nilai[indeks_saya]);
            }
        }

        // 2. GAMBAR KOTAK (Default Putih, Fokus Abu-abu Terang)
        Color warna_latar_input = sedang_fokus ? LIGHTGRAY : WHITE;
        DrawRectangleRec(area, warna_latar_input); 
        
        // 3. GAMBAR BINGKAI (Fokus Biru, Default Hitam)
        if (sedang_fokus) DrawRectangleLinesEx(area, 2.0f, BLUE);
        else DrawRectangleLines((int)area.x, (int)area.y, (int)area.width, (int)area.height, BLACK);

        // 4. GAMBAR ISI TEKS/SANDI (Selalu Hitam)
        if (indeks_saya != -1) {
            if (strcmp(tag_nama, "masukan_sandi") == 0) {
                char bintang[256] = "";
                size_t panjang_sandi = strlen(daftar_nilai[indeks_saya]);
                for(size_t b=0; b<panjang_sandi; b++) bintang[b] = '*';
                bintang[panjang_sandi] = '\0';
                DrawText(bintang, area.x + 5, area.y + 5, 20, BLACK);
            } else {
                DrawText(daftar_nilai[indeks_saya], area.x + 5, area.y + 5, 20, BLACK);
            }
        }
        *y_kursor += 45;
    }

    else if (strcmp(tag_nama, "tombol") == 0) {
        Rectangle area = { (float)*x_kursor, (float)*y_kursor, 200, 40 };
        Color warna_final = warna_latar;
        if (CheckCollisionPointRec(mouse, area)) {
            warna_final = Fade(warna_latar, 0.7f); 
            if (klik_kiri) {
                warna_final = Fade(warna_latar, 0.4f); 
                char id_bersih[256]; sscanf(tag_id, " %s", id_bersih); 
                strcpy(aksi_kembalian, id_bersih);
            }
        }
        DrawRectangleRounded(area, 0.5f, 10, warna_final);
        int text_width = MeasureText(atribut, 20);
        
        // 🟢 FIXED: Paksa putih HANYA jika dev tidak memberi warna teks (Alpha 0)
        Color w_teks = (warna_teks.a == 0) ? WHITE : warna_teks;
        DrawText(atribut, *x_kursor + (100 - (text_width / 2)), *y_kursor + 10, 20, w_teks); 
        *y_kursor += 55;
    }
}

// 🟢 GAYA BAWAAN MESIN (Lengkap dengan masukan_sandi)
const char* SNUL_BAWAAN_MESIN = 
"@tombol {\n"
"    warna_latar: #87CEEBF0;\n" 
"    warna_teks: #FFFFFF;\n"
"}\n"
"@wadah {\n"
"    warna_latar: #D3D3D380;\n" 
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
    if (!IsWindowReady()) { InitWindow(800, 600, "OS Urantia - Dimensi Native GUI"); SetTargetFPS(60); }
    char aksi_kembalian[256] = "";
    Vector2 mouse = GetMousePosition();
    bool klik_kiri = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Parsing Bawaan
    SnulTokenArray tokens_bawaan = snul_lexer(SNUL_BAWAAN_MESIN);
    EnkiObject* gaya_bawaan_ast = parse_snul(tokens_bawaan);

    static float scroll_y = 0;
    scroll_y += GetMouseWheelMove() * 40.0f; 
    if (scroll_y > 0) scroll_y = 0;          

    int x_mulai = 40;
    int y_mulai = 40 + (int)scroll_y; 

    BeginDrawing();
    ClearBackground(RAYWHITE); 
    
    if (ui_root && ui_root->tipe == ENKI_OBJEK) {
        // 🟢 FIXED: HANYA GAMBAR 1 KALI
        gambar_elemen_otim(ui_root, gaya_bawaan_ast, gaya_root, &x_mulai, &y_mulai, mouse, klik_kiri, aksi_kembalian, BLACK);
    }
    
    EndDrawing();

    if (WindowShouldClose()) { strcpy(aksi_kembalian, "TUTUP_PAKSA"); CloseWindow(); }
    if (strlen(aksi_kembalian) > 0) scroll_y = 0; 
    
    if (gaya_bawaan_ast) hancurkan_objek(gaya_bawaan_ast);
    bebaskan_snul_token(&tokens_bawaan);

    return strdup(aksi_kembalian);
}