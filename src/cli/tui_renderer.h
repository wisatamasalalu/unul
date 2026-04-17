#ifndef TUI_RENDERER_H
#define TUI_RENDERER_H

#include "../core/enki_object.h"

// Fungsi utama untuk menggambar Pohon OTIM ke Terminal
// Sekarang Pelukis menerima 2 referensi: Tulang (UI) dan Kosmetik (Gaya)
// Sebelumnya void, sekarang mengembalikan teks (ID tombol)
// 🟢 UBAH BARIS INI: Tambahkan int timeout_ms di paling kanan
EnkiObject* tampilkan_tui(EnkiObject* ui_root, EnkiObject* gaya_root, int timeout_ms);

// Fungsi pembantu untuk membersihkan layar dan mengatur warna
void tui_bersihkan_layar();
void tui_pindah_kursor(int x, int y);
void tui_set_warna(const char* warna_snul);

// ========================================================
// 🎨 SIHIR KANVAS MUTLAK (IMMEDIATE MODE DRAWING)
// ========================================================
void tui_layar_bersih();
void tui_layar_kursor(int x, int y);
void tui_layar_cetak(int x, int y, const char* teks, const char* warna);
void tui_layar_piksel(int x, int y, const char* wujud, const char* warna);

// Pintu Dimensi Layar Alternatif (Mode Vim/htop)
void tui_layar_alternatif_buka();
void tui_layar_alternatif_tutup();

// Saraf Penangkap Keyboard Real-Time (Non-Blocking)
char* tui_layar_baca_tombol(); 

#endif // TUI_RENDERER_H

// ========================================================