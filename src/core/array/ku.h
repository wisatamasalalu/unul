#ifndef ENKI_KU_H
#define ENKI_KU_H

#include "../enki_object.h"

// Fungsi untuk mengekstrak daftar kunci dari sebuah objek
// Mengembalikan EnkiObject baru bertipe ENKI_ARRAY
EnkiObject* ambil_kunci_objek(EnkiObject* obj);

#endif