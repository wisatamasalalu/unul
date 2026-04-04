#ifndef ENKI_FILE_SYSTEM_H
#define ENKI_FILE_SYSTEM_H

// Sihir untuk mencari file berdasarkan pola (misal: *.unul)
// Menghasilkan teks panjang yang dipisahkan oleh koma
char* sihir_cari(const char* pola);

// Sihir untuk membaca isi file biner/teks
char* sihir_baca_file(const char* path);

// Sihir untuk menulis data ke dalam file (seperti > di Bash)
void sihir_tulis_file(const char* path, const char* konten);

#endif // ENKI_FILE_SYSTEM_H