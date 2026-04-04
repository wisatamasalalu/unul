#ifndef ENKI_KO_H
#define ENKI_KO_H

#include "../enki_object.h"

// Fungsi untuk mengekstrak daftar konten (nilai) dari sebuah objek
// Mengembalikan EnkiObject baru bertipe ENKI_ARRAY
EnkiObject* ambil_konten_objek(EnkiObject* obj);

#endif