#ifndef TUI_RENDERER_H
#define TUI_RENDERER_H

#include "../core/enki_object.h"

// Fungsi utama untuk menggambar Pohon OTIM ke Terminal
// Sekarang Pelukis menerima 2 referensi: Tulang (UI) dan Kosmetik (Gaya)
void tampilkan_tui(EnkiObject* ui_root, EnkiObject* gaya_root);

// Fungsi pembantu untuk membersihkan layar dan mengatur warna
void tui_bersihkan_layar();
void tui_pindah_kursor(int x, int y);
void tui_set_warna(const char* warna_snul);

#endif