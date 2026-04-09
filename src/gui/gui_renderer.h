#ifndef GUI_RENDERER_H
#define GUI_RENDERER_H

#include "../core/enki_object.h"

// Fungsi untuk menjalankan Raylib, merender UI, dan mengembalikan ID tombol yang diklik.
// Fungsi ini juga akan menyuntikkan teks yang diketik user ke dalam objek ui_root!
char* tampilkan_gui_raylib(EnkiObject* ui_root, EnkiObject* gaya_root);

#endif