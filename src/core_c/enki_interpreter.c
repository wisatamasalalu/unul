#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>  // <--- UNTUK MENGINGAT HISTORY PANAH ATAS
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <regex.h>
#include <pthread.h>
#include "enki_interpreter.h"
#include "enki_scheduler.h"
#include "enki_network.h"
#include "enki_file_system.h"
#include "enki_os.h"
#include "../core/array/panjang.h"
#include "../core/array/ku.h"
#include "../core/array/ko.h"
#include "../core/enki_object.h"

// Helper pembongkar objek ke string untuk transmutasi
void objek_ke_string(EnkiObject* obj, char* buffer, size_t ukuran) {
    if (!obj) { buffer[0] = '\0'; return; }
    if (obj->tipe == ENKI_TEKS) snprintf(buffer, ukuran, "%s", obj->nilai.teks);
    else if (obj->tipe == ENKI_ANGKA) snprintf(buffer, ukuran, "%g", obj->nilai.angka);
    else buffer[0] = '\0';
}

// PENGUBAH \n MENJADI ENTER NYATA
void proses_escape_teks(char* teks) {
    char* tulis = teks;
    char* baca = teks;
    while (*baca) {
        if (*baca == '\\' && *(baca + 1) == 'n') {
            *tulis++ = '\n'; // Ubah jadi Enter beneran
            baca += 2;
        } else if (*baca == '\\' && *(baca + 1) == 't') {
            *tulis++ = '\t'; // Ubah jadi Tab beneran
            baca += 2;
        } else {
            *tulis++ = *baca++;
        }
    }
    *tulis = '\0';
}

// 🌌 FUNGSI HELPER: PEMBANGUN DIMENSI URANTIA (REKURSIF)
// Menggali objek terdalam dan membuatkan Mini RAM untuk masing-masing lapisan
void sihir_bangun_dimensi(KavlingMemori* kavling, EnkiObject* nilai, EnkiRAM* ram_pusat) {
    if (!nilai || nilai->tipe != ENKI_OBJEK) return;
    
    if (kavling->anak_anak) bebaskan_ram(kavling->anak_anak);
    kavling->anak_anak = ciptakan_ram_mini(ram_pusat);
    
    for (int j = 0; j < nilai->panjang; j++) {
        char* k = nilai->nilai.objek_peta.kunci[j]->nilai.teks;
        EnkiObject* v = ciptakan_salinan_objek(nilai->nilai.objek_peta.konten[j]);
        simpan_ke_ram(kavling->anak_anak, k, v);
        
        // 🟢 JIKA WUJUDNYA OBJEK LAGI, BANGUN DIMENSI DI DALAMNYA! (REKURSIF TINGKAT DEWA)
        if (v->tipe == ENKI_OBJEK) {
            for (int m = 0; m < kavling->anak_anak->jumlah; m++) {
                if (strcmp(kavling->anak_anak->kavling[m].nama, k) == 0) {
                    sihir_bangun_dimensi(&(kavling->anak_anak->kavling[m]), v, kavling->anak_anak);
                    break;
                }
            }
        }
    }
}

// =================================================================
// 🧊 SIHIR SEMAYAMKAN: MEMAHAT WUJUD KE DALAM GRIYA (.imah)
// Hierarki: Paradise -> Grand Universe -> Orvonton -> Nebadon -> Mortal
// =================================================================
void sihir_semayamkan_imah(EnkiObject* obj, FILE* fp, int level) {
    if (!obj) { fprintf(fp, "kosong"); return; }

    // Indentasi berdasarkan level (Visualisasi Dimensi Urantia)
    for (int i = 0; i < level; i++) fprintf(fp, "  ");

    if (obj->tipe == ENKI_ANGKA) {
        fprintf(fp, "%g", obj->nilai.angka);
    } else if (obj->tipe == ENKI_TEKS) {
        fprintf(fp, "\"%s\"", obj->nilai.teks);
    } else if (obj->tipe == ENKI_ARRAY) {
        fprintf(fp, "[\n");
        for (int i = 0; i < obj->panjang; i++) {
            sihir_semayamkan_imah(obj->nilai.array_elemen[i], fp, level + 1);
            if (i < obj->panjang - 1) fprintf(fp, ",\n");
        }
        fprintf(fp, "\n");
        for (int i = 0; i < level; i++) fprintf(fp, "  ");
        fprintf(fp, "]");
    } else if (obj->tipe == ENKI_OBJEK) {
        fprintf(fp, "{\n");
        for (int i = 0; i < obj->panjang; i++) {
            for (int j = 0; j < level + 1; j++) fprintf(fp, "  ");
            fprintf(fp, "\"%s\": ", obj->nilai.objek_peta.kunci[i]->nilai.teks);
            
            // Rekursif hingga ke Outer Space (Tanpa Batas)
            sihir_semayamkan_imah(obj->nilai.objek_peta.konten[i], fp, level + 1);
            if (i < obj->panjang - 1) fprintf(fp, ",\n");
        }
        fprintf(fp, "\n");
        for (int i = 0; i < level; i++) fprintf(fp, "  ");
        fprintf(fp, "}");
    }
}

// =================================================================
// DAPUR MESIN INTERPRETER (EKSEKUTOR C)
// Jantung LinuxDNC yang memompa Pohon Logika menjadi kenyataan!
// =================================================================

// Membangkitkan RAM Induk (OS Utama)
EnkiRAM inisialisasi_ram() {
    EnkiRAM ram;
    ram.kapasitas = 10;
    ram.jumlah = 0;
    ram.kavling = (KavlingMemori*)malloc(ram.kapasitas * sizeof(KavlingMemori));
    ram.butuh_anu_aktif = 0;
    ram.dalam_mode_coba = 0;
    ram.status_pulang = 0;
    ram.status_terus = 0;
    ram.status_henti = 0;
    ram.status_array_dinamis = 0; 
    ram.status_array_statis = 0;

    ram.induk = NULL;
    
    // --- SUNTIKAN UNLIMITED ---
    ram.nilai_kembalian = NULL;  
    ram.pesan_error_tabu = NULL; 
    return ram;
}

// =================================================================
// 🟢 SIHIR DETEKSI TYPO (ALGORITMA JARAK LEVENSHTEIN)
// =================================================================
int jarak_levenshtein(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    if(len1 > 255 || len2 > 255) return 999; 
    
    int matrix[256][256];
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            int del = matrix[i - 1][j] + 1;
            int ins = matrix[i][j - 1] + 1;
            int sub = matrix[i - 1][j - 1] + cost;
            int min = del < ins ? del : ins;
            matrix[i][j] = min < sub ? min : sub;
        }
    }
    return matrix[len1][len2];
}

char* cari_saran_typo(const char* nama_dicari, EnkiRAM* ram) {
    char* saran_terbaik = NULL;
    int jarak_terkecil = 3; // Maksimal salah 2 huruf agar dianggap typo
    
    for(int i = 0; i < ram->jumlah; i++) {
        int jarak = jarak_levenshtein(nama_dicari, ram->kavling[i].nama);
        if(jarak < jarak_terkecil) {
            jarak_terkecil = jarak;
            saran_terbaik = ram->kavling[i].nama;
        }
    }
    return saran_terbaik;
}
// =================================================================
// 🟢 SIHIR KIAMAT PRESISI (GCC-STYLE, SMART HINT, & HUKUM TABU)
// =================================================================
void pemicu_kiamat_presisi(ASTNode* node, EnkiRAM* ram, const char* pesan, const char* panduan_cerdas) {
    char buffer_error[2048];
    int baris = node->baris > 0 ? node->baris : 0;
    int kolom = node->kolom > 0 ? node->kolom : 0;
    
    snprintf(buffer_error, sizeof(buffer_error), 
             "🚨 [%s: Baris %d:%d] KIAMAT SINTAKSIS: %s\n", 
             node->nama_file ? node->nama_file : "skrip.unul", 
             baris, kolom, pesan);
             
    // 1. Cetak Visual ke Terminal
    printf("\n%s", buffer_error);
    if (node->nilai_teks) {
        printf("   |\n %d | ... %s ... \n   | ", baris, node->nilai_teks);
        for(int i = 0; i < kolom; i++) printf(" ");
        printf("^-- Bencana bermula di sini!\n\n");
    } else if (node->operator_math) {
        printf("   |\n %d | ... %s ... \n   | ", baris, node->operator_math);
        for(int i = 0; i < kolom; i++) printf(" ");
        printf("^-- Bencana bermula di sini!\n\n");
    } else {
        printf("\n");
    }

    // 2. Cetak Panduan Cerdas (Jika ada)
    if (panduan_cerdas) {
        printf("💡 PANDUAN CERDAS:\n%s\n\n", panduan_cerdas);
    }

    // 3. Abadikan secara diam-diam ke unul.diary
    FILE* file_diary = fopen("unul.diary", "a");
    if (file_diary) {
        fprintf(file_diary, "=== [WAKTU KIAMAT: %ld] ===\n", (long)time(NULL));
        fprintf(file_diary, "%s", buffer_error);
        if (node->nilai_teks) fprintf(file_diary, "   Kode Penyebab: %s\n", node->nilai_teks);
        if (panduan_cerdas) fprintf(file_diary, "   Saran: %s\n", panduan_cerdas);
        fprintf(file_diary, "=================================\n\n");
        fclose(file_diary);
    }

    // 4. 🔥 CEK PERISAI HUKUM TABU (Try-Catch / setjmp) 🔥
    if (ram && ram->dalam_mode_coba == 1) {
        // Bebaskan kotak error lama (jika ada) untuk mencegah memory leak
        if (ram->pesan_error_tabu) {
            free(ram->pesan_error_tabu);
        }
        
        // Cetak memori baru tanpa batasan ukuran
        ram->pesan_error_tabu = strdup(pesan);
        
        // Lontarkan program kembali ke blok 'coba' yang aman!
        longjmp(ram->titik_kembali, 1); 
    }

    // 5. JIKA TIDAK ADA PERISAI, MATIKAN OS/PROGRAM!
    exit(1); 
}

// Membangkitkan RAM Mini (Untuk Objek Bersarang / Fungsi)
EnkiRAM* ciptakan_ram_mini(EnkiRAM* induk) { // 🟢 DITAMBAH PARAMETER INDUK
    EnkiRAM* ram_baru = (EnkiRAM*)malloc(sizeof(EnkiRAM));
    ram_baru->kapasitas = 10;
    ram_baru->jumlah = 0;
    ram_baru->kavling = (KavlingMemori*)malloc(10 * sizeof(KavlingMemori));
    ram_baru->butuh_anu_aktif = 0;
    ram_baru->dalam_mode_coba = 0;
    ram_baru->status_pulang = 0;
    ram_baru->status_terus = 0;
    ram_baru->status_henti = 0;
    
    ram_baru->induk = induk; // 🟢 HUBUNGKAN KE INDUK (Untuk Scope Variabel Global)
    ram_baru->nilai_kembalian = NULL;
    ram_baru->pesan_error_tabu = NULL;
    return ram_baru;
}

// Menghancurkan RAM secara REKURSIF (Anti Kebocoran Memori)
void bebaskan_ram(EnkiRAM* ram) {
    if (!ram) return;
    for (int i = 0; i < ram->jumlah; i++) {
        if (ram->kavling[i].nama) free(ram->kavling[i].nama);
        
        // 🟢 HANCURKAN OBJEK (Mantra Baru)
        if (ram->kavling[i].objek) hancurkan_objek(ram->kavling[i].objek);
        
        if (ram->kavling[i].anak_anak) {
            bebaskan_ram(ram->kavling[i].anak_anak);
            free(ram->kavling[i].anak_anak);
        }
        
        // Bersihkan riwayat mesin waktu
        if (ram->kavling[i].riwayat) {
            for (int j = 0; j < ram->kavling[i].jumlah_riwayat; j++) {
                if (ram->kavling[i].riwayat[j].objek) hancurkan_objek(ram->kavling[i].riwayat[j].objek);
            }
            free(ram->kavling[i].riwayat);
        }
    }
    
    // Bebaskan wadah string unlimited
    if (ram->pesan_error_tabu) free(ram->pesan_error_tabu);
    
    // 🟢 Karena nilai_kembalian sekarang EnkiObject*, kita hancurkan dengan hancurkan_objek
    if (ram->nilai_kembalian) {
        hancurkan_objek(ram->nilai_kembalian);
    }
    
    if (ram->kavling) free(ram->kavling);
}

// Fungsi Internal: Menyimpan atau menimpa nilai di RAM
void simpan_ke_ram(EnkiRAM* ram, const char* nama, EnkiObject* nilai_objek) {
    if (!ram || !nama || !nilai_objek) return;
    
    // Cek apakah variabel sudah ada
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            // Hancurkan objek lama sebelum menimpa dengan yang baru
            if (ram->kavling[i].objek != NULL) hancurkan_objek(ram->kavling[i].objek);
            ram->kavling[i].objek = nilai_objek;
            return;
        }
    }
    
    // Jika belum ada, buat kavling baru
    if (ram->jumlah >= ram->kapasitas) {
        ram->kapasitas = (ram->kapasitas == 0) ? 16 : ram->kapasitas * 2;
        ram->kavling = realloc(ram->kavling, ram->kapasitas * sizeof(KavlingMemori));
    }
    
    ram->kavling[ram->jumlah].nama = strdup(nama);
    ram->kavling[ram->jumlah].objek = nilai_objek; // 🟢 Simpan Jantung Objek
    ram->kavling[ram->jumlah].tipe = TIPE_VARIABEL_SOFT;
    ram->kavling[ram->jumlah].anak_anak = NULL;
    ram->kavling[ram->jumlah].simpul_fungsi = NULL;
    ram->kavling[ram->jumlah].apakah_konstanta = 0;
    ram->kavling[ram->jumlah].riwayat = NULL;
    ram->kavling[ram->jumlah].jumlah_riwayat = 0;
    ram->kavling[ram->jumlah].kapasitas_riwayat = 0;
    ram->jumlah++;
}

// 🔥 SIHIR BALIKAN PARADOKS (Deep Copy Terproteksi)
EnkiRAM* salin_ram_rekursif(EnkiRAM* sumber) {
    if (!sumber) return NULL;
    EnkiRAM* baru = ciptakan_ram_mini(sumber->induk);
    
    baru->status_array_dinamis = sumber->status_array_dinamis;
    baru->status_array_statis = sumber->status_array_statis;
    
    for (int i = 0; i < sumber->jumlah; i++) {
        // 🟢 SUNTIKAN ANTI-PARADOKS: Gunakan Deep Copy!
        EnkiObject* obj_salinan = sumber->kavling[i].objek ? 
                                  ciptakan_salinan_objek(sumber->kavling[i].objek) : 
                                  ciptakan_kosong(); 
                                  
        simpan_ke_ram(baru, sumber->kavling[i].nama, obj_salinan);
        baru->kavling[baru->jumlah - 1].tipe = sumber->kavling[i].tipe;
        baru->kavling[baru->jumlah - 1].apakah_konstanta = sumber->kavling[i].apakah_konstanta;
        
        if (sumber->kavling[i].anak_anak) {
            baru->kavling[baru->jumlah - 1].anak_anak = salin_ram_rekursif(sumber->kavling[i].anak_anak);
        }
    }
    return baru;
}

// Fungsi Internal: Membaca nilai dari RAM
EnkiObject* baca_dari_ram(EnkiRAM* ram, const char* nama) {
    EnkiRAM* saat_ini = ram;
    while (saat_ini != NULL) {
        for (int i = 0; i < saat_ini->jumlah; i++) {
            if (strcmp(saat_ini->kavling[i].nama, nama) == 0) {
                return saat_ini->kavling[i].objek; // 🟢 Kembalikan Pointer Objek Asli
            }
        }
        saat_ini = saat_ini->induk; // Cek scope di atasnya (Global)
    }
    return NULL;
}

// ⏳ MESIN WAKTU: Menyimpan Salinan Dimensi Objek
void simpan_jejak_mesin_waktu(KavlingMemori* kavling) {
    if (kavling->jumlah_riwayat >= kavling->kapasitas_riwayat) {
        kavling->kapasitas_riwayat = (kavling->kapasitas_riwayat == 0) ? 4 : kavling->kapasitas_riwayat * 2;
        kavling->riwayat = realloc(kavling->riwayat, kavling->kapasitas_riwayat * sizeof(JejakMasaLalu));
    }
    JejakMasaLalu* jejak = &kavling->riwayat[kavling->jumlah_riwayat++];
    jejak->tipe = kavling->tipe;
    
    // 🟢 SUNTIKAN ANTI-PARADOKS: Ambil foto salinan murni (Deep Copy)
    jejak->objek = kavling->objek ? ciptakan_salinan_objek(kavling->objek) : NULL; 
    jejak->anak_anak = kavling->anak_anak ? salin_ram_rekursif(kavling->anak_anak) : NULL; 
}

void pulihkan_jejak_mesin_waktu(KavlingMemori* kavling) {
    if (kavling->jumlah_riwayat > 0) {
        JejakMasaLalu* jejak_terakhir = &kavling->riwayat[--kavling->jumlah_riwayat];
        kavling->tipe = jejak_terakhir->tipe;
        if (kavling->objek) hancurkan_objek(kavling->objek);
        kavling->objek = jejak_terakhir->objek; 
        kavling->anak_anak = jejak_terakhir->anak_anak;
    }
}

// Penjelajah Dimensi Objek Bersarang (Tak Terbatas)
KavlingMemori* cari_kavling_domain(ASTNode* node, EnkiRAM* ram) {
    if (!node || !ram) return NULL;
    
    // 1. Jika ini adalah Induk Utama (misal: dewa)
    if (node->jenis == AST_IDENTITAS) {
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) {
                return &(ram->kavling[i]); // Kembalikan alamat kavlingnya
            }
        }
        return NULL;
    }
    
    // 2. Jika ini adalah Akses Bersarang (misal: dewa.elemen)
    if (node->jenis == AST_AKSES_DOMAIN) {
        // Cari induknya terlebih dahulu (rekursif ke dalam)
        KavlingMemori* induk = cari_kavling_domain(node->kiri, ram);
        
        // Jika induknya ketemu dan wujudnya adalah OBJEK
        // 🟢 BARU: Kita periksa tipe di dalam jantung objeknya!
        if (induk && induk->objek && induk->objek->tipe == ENKI_OBJEK && induk->anak_anak) {
            // Cari properti anak di dalam RAM si Induk
            for (int i = 0; i < induk->anak_anak->jumlah; i++) {
                if (strcmp(induk->anak_anak->kavling[i].nama, node->nilai_teks) == 0) {
                    return &(induk->anak_anak->kavling[i]);
                }
            }
        }
    }
    return NULL;
}

// 🔥 SUNTIKAN MUTASI KEDALAMAN 🔥
// Menemukan atau menciptakan dimensi di dalam objek (Untuk Reassignment)
KavlingMemori* temukan_atau_ciptakan_kavling(ASTNode* node, EnkiRAM* ram) {
    if (!node || !ram) return NULL;

    // 1. Jika ini variabel utama (misal: entitas)
    if (node->jenis == AST_IDENTITAS) {
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) {
                return &(ram->kavling[i]);
            }
        }
        return NULL; // Induk Utama HARUS sudah ada via takdir.soft
    }

    // 2. Jika ini penembusan dimensi (misal: entitas.level1)
    if (node->jenis == AST_AKSES_DOMAIN) {
        KavlingMemori* induk = temukan_atau_ciptakan_kavling(node->kiri, ram);
        
        if (induk) {
            // JIKA INDUKNYA MASIH TEKS, HANCURKAN DAN PAKSA JADI OBJEK!
            // 🟢 JANTUNG BARU: Cek dan ubah menjadi Objek Peta (Dictionary)
            if (induk->objek == NULL || induk->objek->tipe != ENKI_OBJEK) {
                if (induk->objek) hancurkan_objek(induk->objek); // Buang wujud lamanya
                induk->objek = ciptakan_objek_peta(10);          // Bangkitkan wujud Objek
            }
            if (!induk->anak_anak) {
                induk->anak_anak = ciptakan_ram_mini(ram);       // 🟢 Masukkan 'ram' sebagai induk
            }

            EnkiRAM* ram_anak = induk->anak_anak;
            
            // Cari apakah properti anak ini sudah ada?
            for (int i = 0; i < ram_anak->jumlah; i++) {
                if (strcmp(ram_anak->kavling[i].nama, node->nilai_teks) == 0) {
                    return &(ram_anak->kavling[i]);
                }
            }

            // Jika belum ada, Ciptakan Jalan Dimensi Baru!
            simpan_ke_ram(ram_anak, node->nilai_teks, ciptakan_teks("[MUTASI]"));
            return &(ram_anak->kavling[ram_anak->jumlah - 1]);
        }
    }
    return NULL;
}

// 🔥 SUNTIKAN MASA LALU: Mencari Akar/Induk Utama (Ditaruh di sini!)
KavlingMemori* cari_induk_utama(ASTNode* node, EnkiRAM* ram) {
    if (!node || !ram) return NULL;
    if (node->jenis == AST_IDENTITAS) {
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) {
                return &(ram->kavling[i]);
            }
        }
        return NULL;
    }
    if (node->jenis == AST_AKSES_DOMAIN) {
        return cari_induk_utama(node->kiri, ram); // Menyelam terus sampai ke akar
    }
    return NULL;
}

// Fungsi Bantuan: Menghapus kutip ganda/tunggal di ujung teks
void bersihkan_kutip(char* teks) {
    if (!teks) return; // Pelindung Segfault Mutlak
    int len = strlen(teks);
    if (len >= 2 && (teks[0] == '"' || teks[0] == '\'') && (teks[len-1] == '"' || teks[len-1] == '\'')) {
        memmove(teks, teks + 1, len - 2);
        teks[len - 2] = '\0';
    }
}

// Fungsi Bantuan: Mengekstrak elemen ke-X dari teks array (contoh: "[A, B, C]")
char* ambil_elemen_array(const char* teks_array, int target_indeks) {
    if (!teks_array || teks_array[0] != '[') return strdup("");
    
    char* copy = strdup(teks_array + 1); // lewati '['
    char* akhir = strchr(copy, ']');
    if (akhir) *akhir = '\0'; // hapus ']'

    int indeks_sekarang = 0;
    char* token = strtok(copy, ","); // Potong berdasarkan koma
    char* hasil = strdup("");
    
    while (token != NULL) {
        if (indeks_sekarang == target_indeks) {
            while(*token == ' ') token++; // Hapus spasi di depan teks jika ada
            free(hasil);
            hasil = strdup(token);
            bersihkan_kutip(hasil); // Bersihkan kutipan jika itu teks
            break;
        }
        indeks_sekarang++;
        token = strtok(NULL, ",");
    }
    
    free(copy);
    return hasil;
}

// =================================================================
// 🟢 SIHIR SISIPAN TEKS (String Interpolation: "Halo {nama}")
// =================================================================
char* proses_sisipan_teks(const char* teks_asli, EnkiRAM* ram, ASTNode* node) {
    if (!teks_asli) return strdup("");
    
    // Jalan pintas: Jika tidak ada '{', langsung kembalikan teks aslinya (Sangat Cepat & Hemat CPU)
    if (strchr(teks_asli, '{') == NULL) {
        return strdup(teks_asli);
    }

    char buffer[4096] = {0}; // Batas aman: 4096 karakter per kalimat
    int buf_idx = 0;
    const char* p = teks_asli;

    while (*p != '\0') {
        if (*p == '{') {
            p++; // Lewati kurung kurawal buka '{'
            char nama_var[256] = {0};
            int var_idx = 0;

            // Menangkap nama variabel di dalamnya
            while (*p != '}' && *p != '\0' && var_idx < 255) {
                nama_var[var_idx++] = *p++;
            }

            if (*p == '}') {
                p++; // Lewati kurung kurawal tutup '}'
                
                // 🟢 SUNTIKAN CERDAS: Cek apakah isi kurungnya murni angka/koma (Pola Regex)
                int murni_angka_regex = 1;
                for(int i = 0; i < var_idx; i++) {
                    // Jika ada huruf biasa, berarti ini nama variabel, bukan regex
                    if (!isdigit(nama_var[i]) && nama_var[i] != ',') {
                        murni_angka_regex = 0; 
                        break;
                    }
                }

                // JIKA INI REGEX (contoh: {4} atau {1,3}), cetak ulang apa adanya!
                if (murni_angka_regex) {
                    buffer[buf_idx++] = '{';
                    strcpy(buffer + buf_idx, nama_var);
                    buf_idx += strlen(nama_var);
                    buffer[buf_idx++] = '}';
                } 
                // JIKA BUKAN REGEX (Berarti variabel beneran, misal {nama})
                else {
                    // 🟢 BARU: Baca objeknya, lalu terjemahkan ke teks!
                    EnkiObject* obj_nilai = baca_dari_ram(ram, nama_var);
                    
                    if (obj_nilai == NULL) {
                        // KIAMAT PRESISI JIKA VARIABEL GAIB
                        char pesan_error[512];
                        snprintf(pesan_error, sizeof(pesan_error), "Variabel sisipan '{%s}' belum diciptakan!", nama_var);
                        pemicu_kiamat_presisi(node, ram, pesan_error, 
                            "Anda mencoba menyisipkan variabel ke dalam teks, tetapi variabel tersebut tidak ada di RAM.\n"
                            "Pastikan tidak ada salah eja. Contoh yang sah:\n"
                            "takdir.soft nama = \"Enki\"\n"
                            "ketik(\"Halo {nama}\")");
                    } else {
                        // 🟢 Jika ADA nilainya, gabungkan ke dalam buffer teks!
                        char buf_temp[8192] = ""; 
                        objek_ke_string(obj_nilai, buf_temp, sizeof(buf_temp));
                        strcpy(buffer + buf_idx, buf_temp);
                        buf_idx += strlen(buf_temp);
                    }
                }
            } else {
                // Jika kurung tidak ditutup (misal teksnya aneh), cetak apa adanya
                buffer[buf_idx++] = '{';
                strcpy(buffer + buf_idx, nama_var);
                buf_idx += var_idx;
            }
        } else {
            // Salin huruf biasa
            buffer[buf_idx++] = *p++;
        }
    }
    
    return strdup(buffer);
}

// Pengumuman Fungsi (Forward Declaration)
int evaluasi_kondisi(ASTNode* kondisi, EnkiRAM* ram);

// =================================================================
// 🟢 CORONG TRANSMUTASI UNIVERSAL (Pembaca Multi-Basis)
// =================================================================
long baca_angka_universal(const char* teks) {
    if (!teks) return 0;
    
    // Jika Biner (0b... atau 0B...)
    if (strncmp(teks, "0b", 2) == 0 || strncmp(teks, "0B", 2) == 0) {
        return strtol(teks + 2, NULL, 2);
    }
    // Jika Oktal Eksplisit gaya modern (0o... atau 0O...)
    if (strncmp(teks, "0o", 2) == 0 || strncmp(teks, "0O", 2) == 0) {
        return strtol(teks + 2, NULL, 8);
    }
    
    // strtol basis 0 bawaan C akan otomatis membaca: "0x..." (Hex), "0..." (Oktal C), "1-9..." (Desimal)
    return strtol(teks, NULL, 0); 
}

// =================================================================
// 🟢 PEMBACA TERMINAL AMAN (Kapasitas 65KB + History + Panah Kursor)
// =================================================================
char* sihir_baca_terminal_aman(const char* prompt, int mode_rahasia) {
    int batas = 65536; // 64KB
    
    if (mode_rahasia) {
        // MODE BISIK (RAHASIA): Murni fgets, tanpa memori history!
        if (prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        char* buffer = malloc(batas);
        if (!buffer) return strdup("");
        if (fgets(buffer, batas, stdin) != NULL) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
            else if (len == (size_t)(batas - 1)) {
                printf("\n⚠️ PERINGATAN: Teks rahasia dipotong karena melebihi batas %d karakter.\n", batas - 1);
                int c; while ((c = getchar()) != '\n' && c != EOF);
            }
            return buffer;
        }
        free(buffer);
        return strdup("");
    } else {
        // MODE NORMAL (TANYA/DENGAR): Gunakan readline agar panah & history aktif!
        // readline() menggunakan malloc secara otomatis di Linux.
        char* input = readline(prompt ? prompt : "");
        if (input) {
            size_t len = strlen(input);
            if (len >= (size_t)(batas)) {
                printf("\n⚠️ PERINGATAN: Teks yang Anda masukkan terlalu panjang. Batas karakter adalah %d sehingga teks Anda terpotong.\n", batas - 1);
                input[batas - 1] = '\0'; // Potong paksa
                printf("   Akhir teks Anda: ...%s\n", input + (batas - 40));
            }
            if (*input) {
                add_history(input); // Simpan ke memori panah atas
            }
            return input; 
        }
        return strdup("");
    }
}

// =======================================================
// 🟢 SUNTIKAN UNIVERSAL: Membungkus Dimensi (Mini RAM) menjadi Objek
// =======================================================
EnkiObject* bungkus_dimensi_ke_objek(KavlingMemori* kav) {
    if (!kav) return ciptakan_kosong();
    
    // Jika dia punya Dimensi (anak_anak)
    if (kav->anak_anak && kav->anak_anak->jumlah > 0) {
        EnkiObject* obj_peta = ciptakan_objek_peta(kav->anak_anak->jumlah);
        obj_peta->panjang = kav->anak_anak->jumlah;
        
        for (int i = 0; i < kav->anak_anak->jumlah; i++) {
            KavlingMemori* anak = &(kav->anak_anak->kavling[i]);
            
            // Masukkan nama anak sebagai Kunci
            obj_peta->nilai.objek_peta.kunci[i] = ciptakan_teks(anak->nama);
            
            // Masukkan isi anak sebagai Konten (Rekursif jika anak punya dimensi lagi!)
            if (anak->anak_anak && anak->anak_anak->jumlah > 0) {
                obj_peta->nilai.objek_peta.konten[i] = bungkus_dimensi_ke_objek(anak); 
            } else {
                obj_peta->nilai.objek_peta.konten[i] = anak->objek ? ciptakan_salinan_objek(anak->objek) : ciptakan_kosong();
            }
        }
        return obj_peta;
    }
    
    // Jika tidak punya dimensi (variabel biasa), kembalikan wujud aslinya
    return kav->objek ? ciptakan_salinan_objek(kav->objek) : ciptakan_kosong();
}

// --- 2. LOGIKA EVALUASI NILAI ---
EnkiObject* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram) {
    if (!node) return ciptakan_kosong();
    
    // Evaluasi Operator Ternari Sebaris
    if (node->jenis == AST_TERNARI) {
        int sah = evaluasi_kondisi(node->syarat, ram);
        if (sah) {
            return evaluasi_ekspresi(node->kiri, ram);
        } else {
            return evaluasi_ekspresi(node->kanan, ram);
        }
    }

    // 1. Literal Teks / Angka (Kini mendukung Deteksi Angka Murni!)
    if (node->jenis == AST_LITERAL_TEKS) {
        char* teks_mentah = node->nilai_teks;
        if (!teks_mentah) return ciptakan_kosong();

        // 🟢 SIHIR DETEKSI: Jika tidak diawali kutip dan diawali angka/minus, ini ANGKA!
        if (teks_mentah[0] != '"' && teks_mentah[0] != '\'' && 
           (isdigit(teks_mentah[0]) || (teks_mentah[0] == '-' && isdigit(teks_mentah[1])))) {
            return ciptakan_angka(atof(teks_mentah)); // 🪄 Ubah string "2.0" jadi double 2.0
        }

        // Jika berwujud teks (pakai kutip), lakukan prosedur normal
        char* teks_bersih = strdup(teks_mentah);
        bersihkan_kutip(teks_bersih);

        // 🟢 SUNTIKAN ENTER: Ubah \n menjadi Enter nyata!
        proses_escape_teks(teks_bersih);
        
        // B. Masukkan teks yang sudah bersih ke dalam mesin sisipan
        char* teks_final = proses_sisipan_teks(teks_bersih, ram, node);
        
        // C. Bersihkan memori sementara agar tidak bocor
        free(teks_bersih);
        
        EnkiObject* hasil_teks = ciptakan_teks(teks_final);
        free(teks_final);
        return hasil_teks;
    }
    
    // Evaluasi Pendaftaran Fungsi Panah
    if (node->jenis == AST_FUNGSI_PANAH) {
        static int anon_counter = 0;
        char nama_anon[64];
        snprintf(nama_anon, sizeof(nama_anon), "<anonim_%d>", anon_counter++); // Buat nama gaib

        // Sulap tubuh fungsi panah agar memiliki perintah 'pulang' otomatis
        if (!node->blok_maka) {
            node->blok_maka = (ASTNode*)malloc(sizeof(ASTNode));
            memset(node->blok_maka, 0, sizeof(ASTNode));
            node->blok_maka->jenis = AST_PROGRAM;
            
            ASTNode* pulang = (ASTNode*)malloc(sizeof(ASTNode));
            memset(pulang, 0, sizeof(ASTNode));
            pulang->jenis = AST_PULANG;
            pulang->kiri = node->kanan; // Pulangkan nilai hasil ekspresi kanan
            
            tambah_anak(node->blok_maka, pulang);
        }

        simpan_ke_ram(ram, nama_anon, ciptakan_teks("<fungsi_panah>"));
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, nama_anon) == 0) {
                ram->kavling[i].simpul_fungsi = node; // Tanam cetak biru ke RAM
                break;
            }
        }
        return ciptakan_teks(nama_anon); // Kembalikan nama gaibnya ke pemanggil
    }

    // 🟢 SUNTIKAN BARU: Evaluasi Operator Ternari Sebaris
    if (node->jenis == AST_TERNARI) {
        // Cek secara gaib, apakah kondisinya sah?
        int sah = evaluasi_kondisi(node->syarat, ram);
        
        // Jika sah, evaluasi dan kembalikan nilai di sebelah kiri
        if (sah) {
            return evaluasi_ekspresi(node->kiri, ram);
        } 
        // Jika batal, evaluasi dan kembalikan nilai di sebelah kanan
        else {
            return evaluasi_ekspresi(node->kanan, ram);
        }
    }

    // =======================================================
    // 2. PEMANGGILAN VARIABEL BIASA (AST_IDENTITAS)
    // =======================================================
    if (node->jenis == AST_IDENTITAS) {
        KavlingMemori* kavling = NULL;
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) {
                kavling = &(ram->kavling[i]);
                break;
            }
        }
        
        // JIKA VARIABEL GAGAL DITEMUKAN
        if (!kavling) {
            char pesan_typo[512];
            
            // Coba cari apakah ada variabel yang namanya mirip (Typo)
            char* saran = cari_saran_typo(node->nilai_teks, ram); 
            
            if (saran) {
                // JIKA KETEMU KEMIRIPAN (Misal: apell -> apel)
                snprintf(pesan_typo, sizeof(pesan_typo), "Variabel '%s' tidak ditemukan. Apakah maksud Anda '%s'?", node->nilai_teks, saran);
                pemicu_kiamat_presisi(node, ram, pesan_typo, "Pastikan Anda mengetik nama variabel dengan benar tanpa salah eja.");
            } else {
                // JIKA SAMA SEKALI TIDAK ADA YANG MIRIP (Mungkin lupa tanda kutip!)
                snprintf(pesan_typo, sizeof(pesan_typo), "Variabel '%s' belum diciptakan di realita ini. Coba periksa kembali apakah anda menuliskannya kurang/kelebihan tanda kutip.", node->nilai_teks);
                
                // 🟢 PESAN ERROR LENGKAP KEMBALI!
                pemicu_kiamat_presisi(node, ram, pesan_typo, 
                    "1. Jika Anda bermaksud mencetak teks/kalimat, pastikan Anda mengapitnya dengan tanda kutip ganda (\").\n"
                    "   Contoh: ketik(\"Halo Dunia\")\n"
                    "2. Jika ini adalah variabel, pastikan Anda sudah menciptakannya dengan 'takdir.soft' atau 'takdir.hard' terlebih dahulu.");
            }
            return ciptakan_kosong(); 
        }

        // 🟢 PEMBUNGKUS UNIVERSAL: Jadikan kavling sebagai objek nyata!
        return bungkus_dimensi_ke_objek(kavling); 
    }

    // =======================================================
    // 3. PEMANGGILAN DOMAIN BERTINGKAT (AST_AKSES_DOMAIN)
    // =======================================================
    else if (node->jenis == AST_AKSES_DOMAIN) {
        // JALUR 1: Coba cari dari Kavling / Mini RAM (Paling Aman untuk Mutasi)
        KavlingMemori* target = cari_kavling_domain(node, ram);
        if (target) {
            // 🟢 PEMBUNGKUS UNIVERSAL: Jadikan kavling sebagai objek nyata!
            return bungkus_dimensi_ke_objek(target); 
        }
        
        // 🟢 JALUR 2 (ALTERNATIF): Coba baca sebagai wujud Objek murni
        // Ini menyelamatkan pemanggilan menyilang seperti array[1].nama
        EnkiObject* induk = evaluasi_ekspresi(node->kiri, ram);
        if (induk && induk->tipe == ENKI_OBJEK) {
            char* nama_kunci = node->nilai_teks;
            for (int i = 0; i < induk->panjang; i++) {
                EnkiObject* k = induk->nilai.objek_peta.kunci[i];
                if (k && k->tipe == ENKI_TEKS && strcmp(k->nilai.teks, nama_kunci) == 0) {
                    EnkiObject* v = induk->nilai.objek_peta.konten[i];
                    EnkiObject* hasil = ciptakan_kosong();
                    if (v->tipe == ENKI_ANGKA) hasil = ciptakan_angka(v->nilai.angka);
                    else if (v->tipe == ENKI_TEKS) hasil = ciptakan_teks(v->nilai.teks);
                    else if (v->tipe == ENKI_OBJEK || v->tipe == ENKI_ARRAY) hasil = ciptakan_salinan_objek(v);
                    
                    hancurkan_objek(induk);
                    return hasil; // Selamat!
                }
            }
        }
        
        // Bersihkan memori jika gagal
        if (induk) hancurkan_objek(induk);
        
        // 🟢 SINTESIS PESAN ERROR: Menggabungkan yang lama dengan panduan evolusi baru!
        pemicu_kiamat_presisi(node, ram, "Domain bersarang atau properti tidak ditemukan!", 
            "Domain bersarang adalah fitur untuk memanggil isi objek menggunakan tanda titik (.).\n"
            "Mesin UNUL kini mendukung akses lintas dimensi tanpa batas (contoh: bos.gaji atau planet[1].nama).\n\n"
            "Penyebab Kiamat ini:\n"
            "1. Variabel induk belum diciptakan di RAM, ATAU\n"
            "2. Variabel induk bukan berwujud objek { }, ATAU\n"
            "3. Properti yang Anda cari tidak ada di dalam objek tersebut, ATAU\n"
            "4. Anda mencoba menembus indeks array yang kosong atau salah sebelum tanda titik.\n\n"
            "💡 SOLUSI: Pastikan struktur array ata database atau kode Anda benar. Gunakan ketik() untuk memeriksa wujud asli dari variabel induk Anda.");
            
        return ciptakan_kosong();
    }

    // 4. Operasi Matematika & Penugasan
    if (node->jenis == AST_OPERASI_MATEMATIKA) {
        
        // ==========================================================
        // 🔥 SUNTIKAN MUTASI BRUTAL (TANDA '=')
        // ==========================================================
        if (node->operator_math && strcmp(node->operator_math, "=") == 0) {
            // 0. EVALUASI SISI KANAN DULU!
            EnkiObject* hasil_masa_depan = evaluasi_ekspresi(node->kanan, ram);

            // 0.5 DESTRUCTURING ARRAY (Pembedahan Objek)
            if (node->kiri->jenis == AST_STRUKTUR_ARRAY) {
                if (hasil_masa_depan && hasil_masa_depan->tipe == ENKI_ARRAY) {
                    for (int idx = 0; idx < node->kiri->jumlah_anak && idx < hasil_masa_depan->panjang; idx++) {
                        ASTNode* var_kiri = node->kiri->anak_anak[idx];
                        if (var_kiri->jenis == AST_IDENTITAS) {
                            simpan_ke_ram(ram, var_kiri->nilai_teks, hasil_masa_depan->nilai.array_elemen[idx]); // PINJAM SEMENTARA
                        }
                    }
                } else {
                    pemicu_kiamat_presisi(node, ram, "Gagal membedah Array!", "Sisi kanan harus berwujud murni Array [...].");
                }
                return ciptakan_kosong(); 
            }

            // 1. Dapatkan Induk Utama DULU!
            KavlingMemori* induk = cari_induk_utama(node->kiri, ram); 
            
            // 2. MESIN WAKTU: Simpan masa lalu DI INDUK UTAMA (SEBELUM RAM DIMUTASI)
            if (induk) {
                simpan_jejak_mesin_waktu(induk);
            }

            // 3. BARU KITA CARI/CIPTAKAN TARGET
            KavlingMemori* target = temukan_atau_ciptakan_kavling(node->kiri, ram);
            
            if (target && induk) {
                // SUNTIKAN HUKUM: Cek apakah target dikunci
                // SUNTIKAN HUKUM: Cek apakah target dikunci
                if (target->apakah_konstanta) {
                    char pesan[256];
                    char hint[512];
                    snprintf(pesan, sizeof(pesan), "Hukum Takdir Dilanggar: Variabel '%s' tidak bisa diubah!", target->nama);
                    snprintf(hint, sizeof(hint), "Variabel '%s' diciptakan menggunakan 'takdir.hard' sehingga abadi.\nSilakan gunakan 'takdir.soft' saat menciptakannya jika Anda ingin nilainya bisa dimutasi.", target->nama);
                    
                    pemicu_kiamat_presisi(node, ram, pesan, hint);
                    return ciptakan_kosong(); 
                }

                // Hancurkan kenangan lama masa kini (Dari Target)
                if (target->objek) {
                    hancurkan_objek(target->objek);
                }

                // CEK APAKAH INI SIHIR BALIKAN()
                if (node->kanan->jenis == AST_PANGGILAN_FUNGSI && strcmp(node->kanan->nilai_teks, "balikan") == 0) {
                    KavlingMemori* sumber = temukan_atau_ciptakan_kavling(node->kanan->anak_anak[0], ram);
                    if (sumber) {
                        target->tipe = sumber->tipe;
                        target->objek = sumber->objek; // PINJAM
                        // 🟢 BARU: Kita cek tipe dari jantung objeknya!
                        if (sumber->objek && sumber->objek->tipe == ENKI_OBJEK && sumber->anak_anak) {
                            target->anak_anak = salin_ram_rekursif(sumber->anak_anak);
                        }
                        return ciptakan_kosong();
                    }
                }
                
                // JIKA DITIMPA BIASA
                target->tipe = TIPE_VARIABEL_SOFT; // Bisa jadi HARD jika diubah nanti
                target->objek = hasil_masa_depan; // 🟢 LANGSUNG SIMPAN POINTER OBJEKNYA!

                // 🟢 Jika ini Fungsi Panah, salin cetak birunya!
                if (hasil_masa_depan && hasil_masa_depan->tipe == ENKI_TEKS && strncmp(hasil_masa_depan->nilai.teks, "<anonim_", 8) == 0) {
                    for(int i = 0; i < ram->jumlah; i++) {
                        if (strcmp(ram->kavling[i].nama, target->nama) == 0) {
                            for(int j = 0; j < ram->jumlah; j++) {
                                if (strcmp(ram->kavling[j].nama, hasil_masa_depan->nilai.teks) == 0) {
                                    ram->kavling[i].simpul_fungsi = ram->kavling[j].simpul_fungsi;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            } else {
                pemicu_kiamat_presisi(node, ram, "Gagal memutasi memori. Variabel belum diciptakan!",
                    "Gunakan 'takdir.soft' atau 'takdir.hard' terlebih dahulu.");
                if (hasil_masa_depan) hancurkan_objek(hasil_masa_depan);
            }
            return ciptakan_kosong();
        }
        // ==========================================================

        // JIKA BUKAN PENUGASAN '=' (Melainkan +, -, *, /)
        double hasil_akhir = 0; 
        EnkiObject* obj_kiri = evaluasi_ekspresi(node->kiri, ram);
        EnkiObject* obj_kanan = evaluasi_ekspresi(node->kanan, ram);

        // 🟢 SIHIR TRANSMUTASI: Ambil angka, jika teks coba ubah jadi angka!
        double angka_kiri = 0;
        if (obj_kiri) {
            if (obj_kiri->tipe == ENKI_ANGKA) angka_kiri = obj_kiri->nilai.angka;
            else if (obj_kiri->tipe == ENKI_TEKS) angka_kiri = atof(obj_kiri->nilai.teks);
        }

        double angka_kanan = 0;
        if (obj_kanan) {
            if (obj_kanan->tipe == ENKI_ANGKA) angka_kanan = obj_kanan->nilai.angka;
            else if (obj_kanan->tipe == ENKI_TEKS) angka_kanan = atof(obj_kanan->nilai.teks);
        }

        if (node->operator_math) {
            if (strcmp(node->operator_math, "+") == 0) {
                // SUNTIKAN PENGGABUNGAN TEKS!
                if ((obj_kiri && obj_kiri->tipe == ENKI_TEKS) || (obj_kanan && obj_kanan->tipe == ENKI_TEKS)) {
                    char buf_kiri[1024] = ""; char buf_kanan[1024] = "";
                    if (obj_kiri && obj_kiri->tipe == ENKI_TEKS) strcpy(buf_kiri, obj_kiri->nilai.teks);
                    else if (obj_kiri && obj_kiri->tipe == ENKI_ANGKA) snprintf(buf_kiri, 1024, "%g", obj_kiri->nilai.angka);
                    if (obj_kanan && obj_kanan->tipe == ENKI_TEKS) strcpy(buf_kanan, obj_kanan->nilai.teks);
                    else if (obj_kanan && obj_kanan->tipe == ENKI_ANGKA) snprintf(buf_kanan, 1024, "%g", obj_kanan->nilai.angka);
                    
                    char gabungan[4096]; snprintf(gabungan, 4096, "%s%s", buf_kiri, buf_kanan);
                    if (obj_kiri) hancurkan_objek(obj_kiri);
                    if (obj_kanan) hancurkan_objek(obj_kanan);
                    return ciptakan_teks(gabungan);
                }
                hasil_akhir = angka_kiri + angka_kanan;
            }
            else if (strcmp(node->operator_math, "-") == 0) hasil_akhir = angka_kiri - angka_kanan;
            else if (strcmp(node->operator_math, "*") == 0) hasil_akhir = angka_kiri * angka_kanan;
            else if (strcmp(node->operator_math, "/") == 0 || strcmp(node->operator_math, ":") == 0) {
                if (fabs(angka_kanan) < 0.0000001) pemicu_kiamat_presisi(node, ram, "Kehancuran Dimensi!", "Pembagian dengan nol (0) dilarang oleh Hukum Enlil!");
                else hasil_akhir = angka_kiri / angka_kanan;
            }
            else if (strcmp(node->operator_math, "^") == 0) hasil_akhir = pow(angka_kiri, angka_kanan);
            else if (strcmp(node->operator_math, "//") == 0) {
                if (fabs(angka_kanan) < 0.0000001) pemicu_kiamat_presisi(node, ram, "Kehancuran Dimensi!", "Pembagian dengan nol (0) dilarang oleh Hukum Enlil!");
                else hasil_akhir = floor(angka_kiri / angka_kanan);
            }
            else if (strcmp(node->operator_math, "%") == 0) {
                if (fabs(angka_kanan) < 0.0000001) pemicu_kiamat_presisi(node, ram, "Kehancuran Dimensi!", "Pembagian dengan nol (0) dilarang oleh Hukum Enlil!");
                else hasil_akhir = fmod(angka_kiri, angka_kanan);
            }
        }
        
        if (obj_kiri) hancurkan_objek(obj_kiri);
        if (obj_kanan) hancurkan_objek(obj_kanan);
        return ciptakan_angka(hasil_akhir);
    }

    // 5. EVALUASI PEMBUATAN ARRAY LITERAl ( misal: [1, "dua", 3.5] )
    else if (node->jenis == AST_STRUKTUR_ARRAY) {
        EnkiObject* obj_array = ciptakan_array(node->jumlah_anak);
        obj_array->panjang = node->jumlah_anak;
        
        for (int i = 0; i < node->jumlah_anak; i++) {
            EnkiObject* elemen = evaluasi_ekspresi(node->anak_anak[i], ram);
            if (elemen) {
                // Simpan ke dalam array C murni!
                if (elemen->tipe == ENKI_TEKS) obj_array->nilai.array_elemen[i] = ciptakan_teks(elemen->nilai.teks);
                else if (elemen->tipe == ENKI_ANGKA) obj_array->nilai.array_elemen[i] = ciptakan_angka(elemen->nilai.angka);
                
                // 🟢 SUNTIKAN MUTLAK: Izinkan Objek dan Array hidup di dalam Array!
                else if (elemen->tipe == ENKI_OBJEK || elemen->tipe == ENKI_ARRAY) {
                    obj_array->nilai.array_elemen[i] = ciptakan_salinan_objek(elemen);
                } 
                else obj_array->nilai.array_elemen[i] = ciptakan_kosong();
                
                hancurkan_objek(elemen); 
            } else {
                obj_array->nilai.array_elemen[i] = ciptakan_kosong();
            }
        }
        return obj_array;
    }

    // 🌌 EVALUASI OBJEK URANTIA (Sistem Anak Selang-Seling)
    else if (node->jenis == AST_STRUKTUR_OBJEK) {
        int jumlah_pasangan = node->jumlah_anak / 2;
        EnkiObject* obj_peta = ciptakan_objek_peta(jumlah_pasangan);
        obj_peta->panjang = jumlah_pasangan;
        
        for (int i = 0; i < jumlah_pasangan; i++) {
            EnkiObject* obj_kunci = evaluasi_ekspresi(node->anak_anak[i * 2], ram);
            EnkiObject* obj_konten = evaluasi_ekspresi(node->anak_anak[i * 2 + 1], ram);
            
            if (obj_kunci && obj_kunci->tipe == ENKI_TEKS) {
                obj_peta->nilai.objek_peta.kunci[i] = ciptakan_teks(obj_kunci->nilai.teks);
                obj_peta->nilai.objek_peta.konten[i] = ciptakan_salinan_objek(obj_konten);
            }
            
            if (obj_kunci) hancurkan_objek(obj_kunci);
            if (obj_konten) hancurkan_objek(obj_konten);
        }
        return obj_peta;
    }

    // 6. AKSES ELEMEN ARRAY / OBJEK JSON ( data[1] atau dewa["nama"] )
    else if (node->jenis == AST_AKSES_ARRAY) {
        
        // 🟢 SUNTIKAN PERBAIKAN: Evaluasi seluruh silsilah dimensinya!
        EnkiObject* obj_target = evaluasi_ekspresi(node->kiri, ram);
        
        if (!obj_target || obj_target->tipe == ENKI_KOSONG) {
            char pesan_err[1024];
            snprintf(pesan_err, sizeof(pesan_err), "Induk Array/Objek Gaib!");
            pemicu_kiamat_presisi(node, ram, pesan_err, "Variabel induk yang Anda coba akses dengan kurung siku [...] tidak ada atau bernilai kosong.");
            if (obj_target) hancurkan_objek(obj_target);
            return ciptakan_kosong();
        }

        EnkiObject* obj_indeks = evaluasi_ekspresi(node->indeks_array, ram);
        EnkiObject* hasil = ciptakan_kosong();

        // --- 🟢 TAHAP 1: SIHIR TRANSMUTASI (Soft Typing) ---
        double indeks_angka = -1;
        int indeks_valid_angka = 0;

        if (obj_indeks) {
            if (obj_indeks->tipe == ENKI_ANGKA) {
                indeks_angka = obj_indeks->nilai.angka;
                indeks_valid_angka = 1;
            } else if (obj_indeks->tipe == ENKI_TEKS) {
                char* endptr;
                indeks_angka = strtod(obj_indeks->nilai.teks, &endptr);
                if (endptr != obj_indeks->nilai.teks) indeks_valid_angka = 1; 
            }
        }

        // --- 🟢 TAHAP 2: EKSEKUSI BERDASARKAN WUJUD DATA ---

        // A. JIKA TARGET ADALAH ARRAY
        if (obj_target->tipe == ENKI_ARRAY) {
            if (!indeks_valid_angka) {
                pemicu_kiamat_presisi(node, ram, "Indeks Array Cacat!", 
                    "Array hanya bisa dipanggil menggunakan angka (Contoh: data[1]).\n"
                    "Anda mencoba memasukkan kunci teks yang bukan angka.");
            } else {
                int indeks_unul = (int)indeks_angka;
                int indeks_c = indeks_unul - 1; // 1-based ke 0-based
                
                if (indeks_c >= 0 && indeks_c < obj_target->panjang) {
                    EnkiObject* elemen = obj_target->nilai.array_elemen[indeks_c];
                    if (elemen->tipe == ENKI_ANGKA) hasil = ciptakan_angka(elemen->nilai.angka);
                    else if (elemen->tipe == ENKI_TEKS) hasil = ciptakan_teks(elemen->nilai.teks);
                    // 🟢 MENDUKUNG ARRAY DI DALAM ARRAY / OBJEK DI DALAM ARRAY
                    else if (elemen->tipe == ENKI_OBJEK || elemen->tipe == ENKI_ARRAY) hasil = ciptakan_salinan_objek(elemen);
                } else {
                    char pesan_kiamat[1024]; 
                    snprintf(pesan_kiamat, sizeof(pesan_kiamat), "Indeks %d Keluar Batas!", indeks_unul);
                    char hint[2048]; 
                    snprintf(hint, sizeof(hint), 
                        "Array hanya memiliki %d elemen. Anda tidak bisa memanggil urutan ke-%d.\n"
                        "💡 Tips: UNUL menghitung urutan mulai dari angka 1.", 
                        obj_target->panjang, indeks_unul);
                    pemicu_kiamat_presisi(node, ram, pesan_kiamat, hint);
                }
            }
        } 
        // B. JIKA TARGET ADALAH OBJEK (Kamus / Dictionary / JSON)
        else if (obj_target->tipe == ENKI_OBJEK) {
            if (!obj_indeks || obj_indeks->tipe != ENKI_TEKS) {
                 pemicu_kiamat_presisi(node, ram, "Kunci Objek Cacat!", 
                    "Objek Kamus (Dictionary) hanya bisa dipanggil menggunakan teks kutip.\n"
                    "Contoh: dewa[\"nama\"]");
            } else {
                char kunci_str[512] = "";
                objek_ke_string(obj_indeks, kunci_str, sizeof(kunci_str)); 
                
                int ketemu = 0;
                for (int i = 0; i < obj_target->panjang; i++) {
                    EnkiObject* k = obj_target->nilai.objek_peta.kunci[i];
                    if (k && k->tipe == ENKI_TEKS && strcmp(k->nilai.teks, kunci_str) == 0) {
                        EnkiObject* elemen = obj_target->nilai.objek_peta.konten[i];
                        if (elemen->tipe == ENKI_ANGKA) hasil = ciptakan_angka(elemen->nilai.angka);
                        else if (elemen->tipe == ENKI_TEKS) hasil = ciptakan_teks(elemen->nilai.teks);
                        // 🟢 MENDUKUNG OBJEK DI DALAM OBJEK (Nested)
                        else if (elemen->tipe == ENKI_OBJEK || elemen->tipe == ENKI_ARRAY) hasil = ciptakan_salinan_objek(elemen);
                        ketemu = 1; break;
                    }
                }
                if (!ketemu) {
                    char pesan_kiamat[1024]; 
                    snprintf(pesan_kiamat, sizeof(pesan_kiamat), "Kunci '%s' Tidak Ditemukan!", kunci_str);
                    pemicu_kiamat_presisi(node, ram, pesan_kiamat, 
                        "Objek tersebut tidak memiliki data dengan nama kunci yang Anda minta.\n"
                        "Gunakan 'periksa(objek)' untuk melihat daftar kunci.");
                }
            }
        }
        else {
            pemicu_kiamat_presisi(node, ram, "Pelanggaran Dimensi Data!", 
                "Anda menggunakan kurung siku [...] pada variabel yang BUKAN Array atau Objek (Kamus).\n"
                "Variabel ini mungkin hanya berisi teks tunggal atau angka.");
        }

        // 🔥 WAJIB BEBASKAN: Karena evaluasi_ekspresi merakit objek baru
        if (obj_target) hancurkan_objek(obj_target);
        if (obj_indeks) hancurkan_objek(obj_indeks);
        
        return hasil;
    }

    // --- PENANGKAP PANGGILAN FUNGSI ---
    if (node->jenis == AST_PANGGILAN_FUNGSI) {
        
        // 🟢 SUNTIKAN: Izinkan ketik() menerima Pipa Aliran!
        // 🟢 SUNTIKAN: Izinkan ketik() menerima Pipa Aliran!
        if (strcmp(node->nilai_teks, "ketik") == 0) {
            if (node->jumlah_anak > 0 && node->anak_anak[0] != NULL) {
                EnkiObject* obj_hasil = evaluasi_ekspresi(node->anak_anak[0], ram);
                    
                if (obj_hasil && obj_hasil->tipe == ENKI_TEKS && obj_hasil->nilai.teks) {
                    printf("%s\n", obj_hasil->nilai.teks);
                } else if (obj_hasil && obj_hasil->tipe == ENKI_ANGKA) {
                    printf("%g\n", obj_hasil->nilai.angka);
                }
                if (obj_hasil) hancurkan_objek(obj_hasil);
            } else {
                printf("\n"); 
            }
            return ciptakan_kosong();
        }
        
        // =======================================================
        // 1. [TABLET OF DESTINIES] CEK FUNGSI BAWAAN (NATIVE) DULU!
        // =======================================================
        
        // A. Fungsi acak(min, max)
        if (strcmp(node->nilai_teks, "acak") == 0) {
            EnkiObject* str_min = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* str_max = evaluasi_ekspresi(node->anak_anak[1], ram);
            int min = (str_min && str_min->tipe == ENKI_ANGKA) ? (int)str_min->nilai.angka : 0;
            int max = (str_max && str_max->tipe == ENKI_ANGKA) ? (int)str_max->nilai.angka : 0;
            if (str_min) hancurkan_objek(str_min); 
            if (str_max) hancurkan_objek(str_max);
            
            int hasil = min + (rand() % (max - min + 1));
            return ciptakan_angka((double)hasil);
        }
        
        // B. Fungsi panjang(teks) -> KINI MENDUKUNG ARRAY (panjang.c)
        if (strcmp(node->nilai_teks, "panjang") == 0) {
            EnkiObject* isi = evaluasi_ekspresi(node->anak_anak[0], ram);
            // 🟢 Panggil fungsi Mahakarya core/array/panjang.c Anda!
            int panjang_karakter = hitung_panjang_objek(isi); 
            if (isi) hancurkan_objek(isi);
            return ciptakan_angka((double)panjang_karakter);
        }

        // C. Fungsi waktu_sekarang()
        if (strcmp(node->nilai_teks, "waktu_sekarang") == 0) {
            return ciptakan_angka((double)time(NULL));
        }

        // =======================================================
        // 🟢 SUNTIKAN: ARRAY NON-DUALISME (ko & ku)
        // =======================================================
        
        // 1. Fungsi ko(objek) -> Mengambil Nilai / Konten
        if (strcmp(node->nilai_teks, "ko") == 0) {
            if (ram->status_array_dinamis == 0) {
                pemicu_kiamat_presisi(node, ram, "Sihir Array Tertidur!", "Gunakan 'untuk array.dinamis'.");
                return ciptakan_kosong();
            }

            // 🟢 TAHAP 1: Cari Dimensi/Mini RAM dulu!
            KavlingMemori* kav = cari_kavling_domain(node->anak_anak[0], ram);
            if (kav && kav->anak_anak) {
                EnkiRAM* mini = kav->anak_anak;
                EnkiObject* arr = ciptakan_array(mini->jumlah);
                arr->panjang = mini->jumlah;
                for (int i = 0; i < mini->jumlah; i++) {
                    arr->nilai.array_elemen[i] = ciptakan_salinan_objek(mini->kavling[i].objek);
                }
                return arr;
            }

            // 🟢 TAHAP 2: Jika bukan dimensi, baru ambil dari Object Data biasa
            EnkiObject* target = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* hasil = ambil_konten_objek(target);
            if (target) hancurkan_objek(target);
            return hasil;
        }

        // 2. Fungsi ku(objek) -> Mengambil Kunci
        if (strcmp(node->nilai_teks, "ku") == 0) {
            if (ram->status_array_statis == 0) {
                pemicu_kiamat_presisi(node, ram, "Sihir Array Tertidur!", "Gunakan 'untuk array.statis'.");
                return ciptakan_kosong();
            }

            // 🟢 TAHAP 1: Cari Kunci dari Dimensi/Mini RAM!
            KavlingMemori* kav = cari_kavling_domain(node->anak_anak[0], ram);
            if (kav && kav->anak_anak) {
                EnkiRAM* mini = kav->anak_anak;
                EnkiObject* arr = ciptakan_array(mini->jumlah);
                arr->panjang = mini->jumlah;
                for (int i = 0; i < mini->jumlah; i++) {
                    arr->nilai.array_elemen[i] = ciptakan_teks(mini->kavling[i].nama);
                }
                return arr;
            }

            // 🟢 TAHAP 2: Fallback ke data objek murni
            EnkiObject* target = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* hasil = ambil_kunci_objek(target);
            if (target) hancurkan_objek(target);
            return hasil;
        }
        
        // D. Fungsi huruf_besar(teks) dan huruf_kecil(teks)
        if (strcmp(node->nilai_teks, "huruf_besar") == 0 || strcmp(node->nilai_teks, "huruf_kecil") == 0) {
            // 🟢 PERISAI ANTI-SEGFAULT: Cek apakah argumennya ada!
            if (node->jumlah_anak == 0 || node->anak_anak[0] == NULL) {
                pemicu_kiamat_presisi(node, ram, "Fungsi kehilangan objek sasaran!", "Fungsi huruf_besar/kecil membutuhkan teks di dalamnya, atau dialiri data dari pipa '|>'.");
                return ciptakan_kosong();
            }
            
            EnkiObject* obj_teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            
            if (obj_teks && obj_teks->tipe == ENKI_TEKS && obj_teks->nilai.teks) {
                // 🟢 UNLIMITED SIZE: Salin teks sebesar ukuran aslinya!
                char* teks = strdup(obj_teks->nilai.teks); 
                int is_upper = (strcmp(node->nilai_teks, "huruf_besar") == 0);
                
                for(int i = 0; teks[i]; i++) {
                    teks[i] = is_upper ? toupper(teks[i]) : tolower(teks[i]);
                }
                
                hancurkan_objek(obj_teks);
                EnkiObject* hasil = ciptakan_teks(teks);
                free(teks); // Buang salinannya karena sudah dibungkus objek baru
                return hasil; 
            }
            if (obj_teks) hancurkan_objek(obj_teks);
            return ciptakan_kosong();
        }

        // =======================================================
        // E. TRANSMUTASI MATRIKS 5x5 MUTLAK (Universal Cross-Conversion)
        // =======================================================
        
        if (strcmp(node->nilai_teks, "ke_hex") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi ke_hex() butuh angka!", "Contoh: ke_hex(255) atau ke_hex(\"0b1111\")"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            char teks_arg[256]; objek_ke_string(arg, teks_arg, 256);
            long angka = baca_angka_universal(teks_arg); 
            if (arg) hancurkan_objek(arg);
            
            char buffer[32]; snprintf(buffer, sizeof(buffer), "0x%lX", angka);
            return ciptakan_teks(buffer);
        }
        else if (strcmp(node->nilai_teks, "ke_oktal") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi ke_oktal() butuh angka!", "Contoh: ke_oktal(8)"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            char teks_arg[256]; objek_ke_string(arg, teks_arg, 256);
            long angka = baca_angka_universal(teks_arg); 
            if (arg) hancurkan_objek(arg);
            
            char buffer[32]; snprintf(buffer, sizeof(buffer), "0o%lo", angka);
            return ciptakan_teks(buffer);
        }
        else if (strcmp(node->nilai_teks, "ke_desimal") == 0 || strcmp(node->nilai_teks, "ke_angka") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi ke_desimal() butuh argumen!", "Contoh: ke_desimal(\"0xFF\")"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            char teks_arg[256]; objek_ke_string(arg, teks_arg, 256);
            long angka = baca_angka_universal(teks_arg); 
            if (arg) hancurkan_objek(arg);
            
            return ciptakan_angka((double)angka); // 🟢 Langsung jadi angka murni!
        }
        else if (strcmp(node->nilai_teks, "ke_biner") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi ke_biner() butuh angka!", "Contoh: ke_biner(10)"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            char teks_arg[256]; objek_ke_string(arg, teks_arg, 256);
            long angka = baca_angka_universal(teks_arg); 
            if (arg) hancurkan_objek(arg);

            char buffer[128]; buffer[0] = '0'; buffer[1] = 'b'; int index = 2;
            if (angka == 0) { buffer[2] = '0'; buffer[3] = '\0'; } 
            else {
                long temp = angka; int bits[64]; int bit_count = 0;
                while(temp > 0) { bits[bit_count++] = temp % 2; temp /= 2; }
                for(int i = bit_count - 1; i >= 0; i--) buffer[index++] = bits[i] + '0';
                buffer[index] = '\0';
            }
            return ciptakan_teks(buffer);
        }

        // --- TRANSMUTASI: BULATKAN ---
        else if (strcmp(node->nilai_teks, "bulatkan") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi bulatkan() butuh angka!", "Contoh: bulatkan(3.14)"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            double angka = (arg && arg->tipe == ENKI_ANGKA) ? arg->nilai.angka : 0;
            if (arg) hancurkan_objek(arg);

            // Karena kita menggunakan tipe data ENKI_ANGKA murni, 
            // kita langsung me-return angkanya saja yang sudah di-round!
            return ciptakan_angka(round(angka)); 
        }

        // F. TRANSMUTASI ASCII <-> ANGKA
        else if (strcmp(node->nilai_teks, "ke_ascii") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi ke_ascii() butuh karakter!", "Contoh: ke_ascii(\"A\")"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            double nilai_ascii = 0;
            if (arg && arg->tipe == ENKI_TEKS && arg->nilai.teks[0] != '\0') {
                nilai_ascii = (double)(arg->nilai.teks[0]);
            }
            if (arg) hancurkan_objek(arg);
            return ciptakan_angka(nilai_ascii);
        }
        else if (strcmp(node->nilai_teks, "dari_ascii") == 0 || strcmp(node->nilai_teks, "ke_karakter") == 0) {
            if (node->jumlah_anak < 1) { pemicu_kiamat_presisi(node, ram, "Fungsi dari_ascii() butuh kode!", "Contoh: dari_ascii(65) atau dari_ascii(\"0x41\")"); return ciptakan_kosong(); }
            EnkiObject* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            long ascii_val = (arg && arg->tipe == ENKI_ANGKA) ? (long)arg->nilai.angka : 0;
            if (arg) hancurkan_objek(arg);
            
            char buffer[2]; buffer[0] = (char)ascii_val; buffer[1] = '\0';
            return ciptakan_teks(buffer);
        }
        
        // --- TRANSMUTASI: COCOK POLA (TRUE REGEX) ---
        else if (strcmp(node->nilai_teks, "cocok") == 0) {
            // Pastikan argumen yang diberikan minimal 2 (teks dan pola)
            if (node->jumlah_anak < 2) {
                pemicu_kiamat_presisi(node, ram, "Pelanggaran Argumen: fungsi 'cocok' butuh 2 parameter (teks, pola_regex)!", "Contoh penggunaan: cocok(kalimat, \"^[A-Z]\")");
                return ciptakan_angka(0);
            }

            EnkiObject* obj_teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* obj_pola = evaluasi_ekspresi(node->anak_anak[1], ram);
            
            char teks[8192]=""; objek_ke_string(obj_teks, teks, sizeof(teks));
            char pola[1024]=""; objek_ke_string(obj_pola, pola, sizeof(pola));

            regex_t mesin_regex;
            int hasil_akhir = 0; // Default: 0 (Palsu / Tidak Cocok)

            // 1. Kompilasi Pola Regex (Gunakan REG_EXTENDED agar mirip standar modern)
            int status_kompilasi = regcomp(&mesin_regex, pola, REG_EXTENDED);
            
            if (status_kompilasi == 0) {
                // 2. Jika pola sah, jalankan eksekusi pencocokan
                int status_cocok = regexec(&mesin_regex, teks, 0, NULL, 0);
                
                if (status_cocok == 0) {
                    hasil_akhir = 1; // Sah! Cocok mutlak!
                }
                
                // 3. Bersihkan memori mesin regex C
                regfree(&mesin_regex); 
            } else {
                pemicu_kiamat_presisi(node, ram, "Sintaks pola Regex yang diberikan tidak valid!", "Pastikan pola regex yang Anda tulis mematuhi standar POSIX Extended Regular Expressions.");
            }

            if (obj_teks) hancurkan_objek(obj_teks); 
            if (obj_pola) hancurkan_objek(obj_pola);
            return ciptakan_angka((double)hasil_akhir);
        }

        // G. SIHIR TERTINGGI: evaluasi(teks) - RECURSIVE DYNAMIC EVALUATION
        if (strcmp(node->nilai_teks, "evaluasi") == 0) {
            EnkiObject* obj_kode = evaluasi_ekspresi(node->anak_anak[0], ram);
            char teks_kode[8192]=""; objek_ke_string(obj_kode, teks_kode, sizeof(teks_kode));
            
            // 1. Panggil Pemindai (Lexer) khusus untuk teks ini
            TokenArray token_eval = enki_lexer(teks_kode, "<evaluasi_dinamis>");
            
            // 2. Panggil Pohon Logika (Parser)
            Parser parser_eval = inisialisasi_parser(token_eval);
            
            // 3. Baca hanya sebagai Ekspresi (bukan program utuh)
            ASTNode* ast_eval = parse_ekspresi(&parser_eval);
            
            // 4. Eksekusi hasilnya!
            EnkiObject* hasil_eval = evaluasi_ekspresi(ast_eval, ram);
            
            // 5. Bersihkan sampah dimensi
            bebaskan_ast(ast_eval);
            bebaskan_token_array(&token_eval);
            if (obj_kode) hancurkan_objek(obj_kode);
            
            return hasil_eval;
        }

        // --- TERMINAL I/O: DENGAR (Statis, History Aktif) ---
        if (strcmp(node->nilai_teks, "dengar") == 0) {
            if (node->jumlah_anak > 0) {
                pemicu_kiamat_presisi(node, ram, "Fungsi dengar() tidak menerima teks panduan!", 
                    "Jika Anda ingin menampilkan pesan sebelum meminta input, gunakan fungsi tanya().\n"
                    "Contoh: tanya(\"Siapa namamu? \")");
                return ciptakan_kosong();
            }
            // Kirim "> " langsung ke readline agar panah kiri tidak tembus batas
            char* input = sihir_baca_terminal_aman("> ", 0); 
            EnkiObject* hasil = ciptakan_teks(input); 
            free(input); 
            return hasil;
        }

        // --- TERMINAL I/O: TANYA (Dinamis, History Aktif) ---
        else if (strcmp(node->nilai_teks, "tanya") == 0) {
            if (node->jumlah_anak < 1) {
                pemicu_kiamat_presisi(node, ram, "Fungsi tanya() butuh pesan panduan!", 
                    "Anda tidak memberikan teks pertanyaan.\n"
                    "Contoh: tanya(\"Masukkan umur Anda: \")\n"
                    "Atau gunakan dengar() jika Anda tidak butuh teks.");
                return ciptakan_kosong();
            }
            EnkiObject* obj_pesan = evaluasi_ekspresi(node->anak_anak[0], ram);
            char pesan[1024]=""; objek_ke_string(obj_pesan, pesan, sizeof(pesan));
            
            char* input = sihir_baca_terminal_aman(pesan, 0);
            
            if (obj_pesan) hancurkan_objek(obj_pesan);
            EnkiObject* hasil = ciptakan_teks(input); 
            free(input); 
            return hasil;
        }

        // --- TERMINAL I/O: BISIK (Rahasia / Password) ---
        else if (strcmp(node->nilai_teks, "bisik") == 0) {
            char pesan[1024]=""; EnkiObject* obj_pesan = NULL;
            if (node->jumlah_anak > 0 && node->anak_anak[0] != NULL) { 
                obj_pesan = evaluasi_ekspresi(node->anak_anak[0], ram);
                objek_ke_string(obj_pesan, pesan, sizeof(pesan));
            }
            
            struct termios terminal_lama, terminal_baru;
            tcgetattr(STDIN_FILENO, &terminal_lama);
            terminal_baru = terminal_lama;
            terminal_baru.c_lflag &= ~(ECHO); // Matikan layar
            tcsetattr(STDIN_FILENO, TCSANOW, &terminal_baru);
            
            // Mode 1: Aktifkan mode rahasia (fgets tanpa history)
            char* hasil_bisik = sihir_baca_terminal_aman(strlen(pesan)>0?pesan:NULL, 1);
            
            tcsetattr(STDIN_FILENO, TCSANOW, &terminal_lama); // Nyalakan layar
            printf("\n"); 
            
            if (obj_pesan) hancurkan_objek(obj_pesan);
            EnkiObject* hasil = ciptakan_teks(hasil_bisik); 
            free(hasil_bisik); 
            return hasil;
        }

        // --- SIHIR JARINGAN (UNUL PACKAGE MANAGER) ---
        if (strcmp(node->nilai_teks, "ambil") == 0) {
            if (node->jumlah_anak < 1) {
                pemicu_kiamat_presisi(node, ram, "Fungsi ambil() butuh URL!", "Anda tidak memberikan URL tempat paket/file berada.");
                return ciptakan_kosong();
            }
            EnkiObject* obj_url = evaluasi_ekspresi(node->anak_anak[0], ram);
            char url[2048]=""; objek_ke_string(obj_url, url, sizeof(url));
            
            char* hasil_mentah = sihir_ambil(url);
            
            if (obj_url) hancurkan_objek(obj_url);
            EnkiObject* hasil = ciptakan_teks(hasil_mentah); 
            free(hasil_mentah); 
            return hasil;
        }
        else if (strcmp(node->nilai_teks, "setor") == 0) {
            if (node->jumlah_anak < 2) {
                pemicu_kiamat_presisi(node, ram, "Fungsi setor() butuh URL dan Data!", "Anda tidak memberikan URL atau data yang akan dikirim.\nContoh: setor(\"http://api.com/data\", \"{\\\"nama\\\":\\\"UNUL\\\"}\")");
                return ciptakan_kosong();
            }
            EnkiObject* obj_url = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* obj_data = evaluasi_ekspresi(node->anak_anak[1], ram);
            
            char url[2048]=""; objek_ke_string(obj_url, url, sizeof(url));
            char data[8192]=""; objek_ke_string(obj_data, data, sizeof(data));
            
            // 🟢 Nanti pastikan di enki_network.c Anda punya fungsi sihir_setor(url, data)
            char* hasil_mentah = sihir_setor(url, data); 
            
            if (obj_url) hancurkan_objek(obj_url);
            if (obj_data) hancurkan_objek(obj_data);
            
            EnkiObject* hasil = ciptakan_teks(hasil_mentah); 
            free(hasil_mentah); 
            return hasil;
        }


        // =======================================================
        // G. SIHIR META-PROGRAMMING: JALANKAN PROGRAM UTUH
        // =======================================================
        else if (strcmp(node->nilai_teks, "jalankan") == 0) {
            // DETEKSI PELANGGARAN
            if (node->jumlah_anak < 1) {
                pemicu_kiamat_presisi(node, ram, "Fungsi jalankan() butuh kode!", 
                    "Anda tidak memberikan teks kode yang akan dieksekusi.\n"
                    "Contoh: jalankan(\"ketik(\\\"Halo\\\")\") atau jalankan(baca(\"file.unul\"))");
                return ciptakan_kosong();
            }
            EnkiObject* obj_kode = evaluasi_ekspresi(node->anak_anak[0], ram);
            // 🟢 UNLIMITED SIZE! Langsung pakai pointer aslinya!
            const char* teks_kode = (obj_kode && obj_kode->tipe == ENKI_TEKS) ? obj_kode->nilai.teks : "";
            
            TokenArray token_run = enki_lexer((char*)teks_kode, "<dimensi_jalankan>");
            Parser parser_run = inisialisasi_parser(token_run);
            ASTNode* ast_run = parse_program(&parser_run);
            
            eksekusi_program(ast_run, ram); // Menjalankan langsung ke RAM
            
            // 🔥 KOREKSI MUTLAK: JANGAN DIBEBASKAN JUGA!
            // bebaskan_ast(ast_run);
            // bebaskan_token_array(&token_run);
            
            if (obj_kode) hancurkan_objek(obj_kode);
            return ciptakan_kosong();
        }

        // =======================================================
        // H. MANIPULASI TEKS (ARRAY SPLITTER)
        // =======================================================
        else if (strcmp(node->nilai_teks, "pecah_teks") == 0) {
            if (node->jumlah_anak < 2) {
                pemicu_kiamat_presisi(node, ram, "Fungsi pecah_teks() butuh 2 argumen!", "Contoh: pecah_teks(\"a,b\", \",\")");
                return ciptakan_kosong();
            }
            EnkiObject* obj_teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* obj_pemisah = evaluasi_ekspresi(node->anak_anak[1], ram);
            // 🟢 UNLIMITED SIZE!
            const char* teks_asli = (obj_teks && obj_teks->tipe == ENKI_TEKS) ? obj_teks->nilai.teks : "";
            char pemisah[256]=""; objek_ke_string(obj_pemisah, pemisah, sizeof(pemisah));
            if (strcmp(pemisah, "\\n") == 0 || strcmp(pemisah, "\\r\\n") == 0) strcpy(pemisah, "\n");
            
            // Kita duplikasi secara dinamis seukuran aslinya, bukan pakai 8192 lagi!
            char* teks_copy = strdup(teks_asli); 
            char* teks_copy2 = strdup(teks_asli); // Untuk pass kedua
            
            int jumlah = 0; 
            char* t = strtok(teks_copy, pemisah); 
            while(t) { jumlah++; t = strtok(NULL, pemisah); }
            
            EnkiObject* array_hasil = ciptakan_array(jumlah); 
            array_hasil->panjang = jumlah;
            
            int idx = 0; 
            char* token = strtok(teks_copy2, pemisah);
            while (token != NULL) { 
                array_hasil->nilai.array_elemen[idx++] = ciptakan_teks(token); 
                token = strtok(NULL, pemisah); 
            }
            
            free(teks_copy); free(teks_copy2); // Buang duplikat
            if (obj_teks) hancurkan_objek(obj_teks);
            if (obj_pemisah) hancurkan_objek(obj_pemisah);
            return array_hasil;
        }

        // =======================================================
        // I. PENGAMBIL DATA ARRAY SEMENTARA
        // =======================================================
        else if (strcmp(node->nilai_teks, "ambil_array") == 0) {
            if (node->jumlah_anak < 2) {
                pemicu_kiamat_presisi(node, ram, "Fungsi ambil_array() butuh 2 argumen!", "Contoh: ambil_array(data, 1)");
                return ciptakan_kosong();
            }
            EnkiObject* obj_array = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* obj_index = evaluasi_ekspresi(node->anak_anak[1], ram);
            
            int target_index = (obj_index && obj_index->tipe == ENKI_ANGKA) ? (int)obj_index->nilai.angka : -1;
            EnkiObject* hasil = ciptakan_kosong();
            
            if (obj_array && obj_array->tipe == ENKI_ARRAY) {
                int idx_c = target_index - 1; // UNUL memakai 1-based index (misal: ambil_array(data, 1))
                if (idx_c >= 0 && idx_c < obj_array->panjang) {
                    // Karena ini mengambil, kita salin nilainya agar tidak rusak saat aslinya dibebaskan
                    EnkiObject* elemen = obj_array->nilai.array_elemen[idx_c];
                    if (elemen->tipe == ENKI_TEKS) hasil = ciptakan_teks(elemen->nilai.teks);
                    else if (elemen->tipe == ENKI_ANGKA) hasil = ciptakan_angka(elemen->nilai.angka);
                }
            } else {
                 pemicu_kiamat_presisi(node, ram, "Argumen bukan Array!", "Fungsi ambil_array() mengharuskan argumen pertama adalah Array.");
            }
            
            if (obj_array) hancurkan_objek(obj_array); 
            if (obj_index) hancurkan_objek(obj_index);
            return hasil;
        }

        // --- SIHIR SISTEM FILE (BLOB/GLOB) ---
        if (strcmp(node->nilai_teks, "cari") == 0) {
            if (node->jumlah_anak < 1) {
                pemicu_kiamat_presisi(node, ram, "Fungsi cari() butuh peta pencarian!", 
                    "Anda tidak memberikan pola file yang ingin dicari.\n"
                    "Contoh penggunaan yang sah: cari(\"*.unul\") atau cari(\"tests/*\")");
                return ciptakan_kosong(); 
            }
            EnkiObject* obj_pola = evaluasi_ekspresi(node->anak_anak[0], ram);
            char pola_mentah[2048]=""; objek_ke_string(obj_pola, pola_mentah, sizeof(pola_mentah));
            char* pola_final = ekspansi_jalur(pola_mentah); // 🌍 SIHIR EKSPANSI
            
            char* hasil_mentah = sihir_cari(pola_final);
            
            if (obj_pola) hancurkan_objek(obj_pola); 
            free(pola_final);
            
            EnkiObject* hasil = ciptakan_teks(hasil_mentah); 
            free(hasil_mentah); 
            return hasil;
        }

        if (strcmp(node->nilai_teks, "baca") == 0) {
            if (node->jumlah_anak < 1) {
                pemicu_kiamat_presisi(node, ram, "Fungsi baca() butuh nama file!", 
                    "Anda tidak memberikan jalur (path) file yang ingin dibaca.\n"
                    "Contoh penggunaan yang sah: baca(\"konfigurasi.txt\")");
                return ciptakan_kosong(); 
            }
            EnkiObject* obj_path = evaluasi_ekspresi(node->anak_anak[0], ram);
            char path_mentah[2048]=""; objek_ke_string(obj_path, path_mentah, sizeof(path_mentah));
            char* path_final = ekspansi_jalur(path_mentah); // 🌍 SIHIR EKSPANSI
            
            char* hasil_mentah = sihir_baca_file(path_final);
            
            if (obj_path) hancurkan_objek(obj_path); 
            free(path_final);
            
            EnkiObject* hasil = ciptakan_teks(hasil_mentah); 
            free(hasil_mentah); 
            return hasil;
        }

        if (strcmp(node->nilai_teks, "tulis") == 0) {
            if (node->jumlah_anak < 2) {
                pemicu_kiamat_presisi(node, ram, "Fungsi tulis() butuh target dan isi!", "Contoh: tulis(\"output.txt\", data)");
                return ciptakan_kosong(); 
            }
            EnkiObject* obj_path = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* obj_konten = evaluasi_ekspresi(node->anak_anak[1], ram);
            
            char path_mentah[2048]=""; objek_ke_string(obj_path, path_mentah, sizeof(path_mentah));
            char* path_final = ekspansi_jalur(path_mentah); 
            
            // 🟢 UNLIMITED SIZE! Langsung ambil pointernya jika itu teks!
            char buf_angka[256]="";
            const char* konten_final = "";
            if (obj_konten && obj_konten->tipe == ENKI_TEKS) konten_final = obj_konten->nilai.teks;
            else if (obj_konten && obj_konten->tipe == ENKI_ANGKA) {
                snprintf(buf_angka, sizeof(buf_angka), "%g", obj_konten->nilai.angka);
                konten_final = buf_angka;
            }
            
            sihir_tulis_file(path_final, konten_final);
            
            if (obj_path) hancurkan_objek(obj_path); 
            if (obj_konten) hancurkan_objek(obj_konten);
            free(path_final); 
            return ciptakan_kosong(); 
        }

        // 🕋 SEMAYAMKAN: Mengunci data ke dalam Griya (.imah)
        if (strcmp(node->nilai_teks, "ekspor") == 0) {
            if (node->jumlah_anak < 2) {
                pemicu_kiamat_presisi(node, ram, "Penanaman Data Gagal: Butuh objek dan nama Database!", "Contoh: ekspor(dewa, \"surga.imah\")");
                return ciptakan_kosong();
            }
            EnkiObject* obj_target = evaluasi_ekspresi(node->anak_anak[0], ram);
            EnkiObject* obj_path = evaluasi_ekspresi(node->anak_anak[1], ram);
            
            char path_mentah[2048]=""; objek_ke_string(obj_path, path_mentah, sizeof(path_mentah));
            char* path_final = ekspansi_jalur(path_mentah); 
            
            FILE* fp = fopen(path_final, "w");
            if (fp) {
                fprintf(fp, "^^ IMAH: DATABASE DIMENSI URANTIA\n"); // Header Identitas
                sihir_semayamkan_imah(obj_target, fp, 0);
                fclose(fp);
            } else {
                pemicu_kiamat_presisi(node, ram, "Gagal memahat dimensi!", "Pastikan jalur file dapat diakses oleh OS.");
            }
            
            if (obj_target) hancurkan_objek(obj_target);
            if (obj_path) hancurkan_objek(obj_path);
            free(path_final);
            return ciptakan_kosong();
        }

        // ✨ BANGKITKAN: Memanggil data dari alam .imah ke RAM
        if (strcmp(node->nilai_teks, "impor") == 0) {
            if (node->jumlah_anak < 1) {
                pemicu_kiamat_presisi(node, ram, "Impor data dari .imah gagal: Database mana yang ingin Anda buka?", "Contoh: takdir.soft data = bangkitkan(\"surga.imah\")");
                return ciptakan_kosong();
            }
            EnkiObject* obj_path = evaluasi_ekspresi(node->anak_anak[0], ram);
            char path_mentah[2048]=""; objek_ke_string(obj_path, path_mentah, sizeof(path_mentah));
            char* path_final = ekspansi_jalur(path_mentah); 
            
            char* teks_imah = sihir_baca_file(path_final);
            EnkiObject* hasil_bangkit = ciptakan_kosong();

            if (teks_imah && strlen(teks_imah) > 0) {
                // Lewati header jika ada
                char* data_murni = strstr(teks_imah, "{");
                if (!data_murni) data_murni = strstr(teks_imah, "[");
                if (!data_murni) data_murni = teks_imah;

                // 🪄 SIHIR EVALUASI: Karena format .imah adalah ekspresi UNUL yang sah!
                TokenArray token_eval = enki_lexer(data_murni, "<dimensi_bangkit>");
                Parser parser_eval = inisialisasi_parser(token_eval);
                ASTNode* ast_eval = parse_ekspresi(&parser_eval);
                
                hasil_bangkit = evaluasi_ekspresi(ast_eval, ram);
                
                bebaskan_ast(ast_eval);
                bebaskan_token_array(&token_eval);
            }
            
            if (teks_imah) free(teks_imah);
            free(path_final);
            if (obj_path) hancurkan_objek(obj_path);
            
            return hasil_bangkit;
        }

        // =======================================================
        // 2. JIKA BUKAN FUNGSI BAWAAN, CARI DI RAM (FUNGSI KUSTOM)
        // =======================================================
        
        ASTNode* func_node = NULL;
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) {
                func_node = ram->kavling[i].simpul_fungsi;
                break;
            }
        }
        
        if (func_node == NULL) {
            char pesan_error[512];
            snprintf(pesan_error, sizeof(pesan_error), "Fungsi gaib '%s' tidak ditemukan!", node->nilai_teks);
            pemicu_kiamat_presisi(node, ram, pesan_error, 
                "Anda mencoba memanggil fungsi yang belum pernah diciptakan ke alam semesta (RAM).\n"
                "Pastikan Anda sudah mendeklarasikannya dengan 'ciptakan fungsi ...' atau periksa ejaan Anda.");
            return ciptakan_kosong(); 
        }

        EnkiRAM ram_lokal = inisialisasi_ram();
        ram_lokal.butuh_anu_aktif = ram->butuh_anu_aktif;
        
        // 🟢 WARISKAN SAKLAR KE DALAM FUNGSI!
        ram_lokal.status_array_dinamis = ram->status_array_dinamis;
        ram_lokal.status_array_statis = ram->status_array_statis;

        // Salin seluruh isi semesta RAM induk ke RAM lokal fungsi
        for (int i = 0; i < ram->jumlah; i++) {
            // 🟢 WAJIB DEEP COPY! Jika tidak, RAM global ikut hancur saat fungsi selesai!
            EnkiObject* obj_salinan = ram->kavling[i].objek ? ciptakan_salinan_objek(ram->kavling[i].objek) : ciptakan_kosong();
            simpan_ke_ram(&ram_lokal, ram->kavling[i].nama, obj_salinan);
            ram_lokal.kavling[ram_lokal.jumlah - 1].simpul_fungsi = ram->kavling[i].simpul_fungsi;
        }

        // Pemetakan Parameter
        for (int i = 0; i < func_node->jumlah_anak; i++) { 
            ASTNode* param = func_node->anak_anak[i];
            char* nama_param = "";
            EnkiObject* nilai_akhir = NULL;

            int punya_bawaan = (param->operator_math && strcmp(param->operator_math, "=") == 0);
            
            if (punya_bawaan) {
                nama_param = param->kiri->nilai_teks; 
            } else {
                nama_param = param->nilai_teks; 
            }

            if (i < node->jumlah_anak) { 
                nilai_akhir = evaluasi_ekspresi(node->anak_anak[i], ram);
            } else if (punya_bawaan) {
                nilai_akhir = evaluasi_ekspresi(param->kanan, &ram_lokal); 
            } else {
                pemicu_kiamat_presisi(node, ram, "Kekurangan Argumen!", 
                    "Fungsi ini membutuhkan lebih banyak argumen, dan tidak ada parameter bawaan yang bisa menyelamatkan.");
                nilai_akhir = ciptakan_kosong();
            }

            if (nilai_akhir) {
                simpan_ke_ram(&ram_lokal, nama_param, nilai_akhir);
                // 🟢 Tidak perlu free(nilai_akhir) karena pointer objek kini dimiliki oleh RAM
            }
        }

        // Eksekusi tubuh fungsi
        eksekusi_program(func_node->blok_maka, &ram_lokal);
        
        // Penangkapan hasil kepulangan (return)
        EnkiObject* hasil_akhir = ciptakan_kosong();
        if (ram_lokal.status_pulang == 1 && ram_lokal.nilai_kembalian) {
            hancurkan_objek(hasil_akhir); // Buang yang kosong
            hasil_akhir = ram_lokal.nilai_kembalian;
            // Lepaskan kepemilikan dari ram_lokal agar tidak ikut musnah saat dibebaskan!
            ram_lokal.nilai_kembalian = NULL; 
        }
        
        bebaskan_ram(&ram_lokal);
        return hasil_akhir;
    }

    return ciptakan_kosong(); // Default return yang HALAL (berada di akhir evaluasi_ekspresi)
}

// Fungsi Internal: Mengevaluasi Syarat Hukum Karma & Logika
int evaluasi_kondisi(ASTNode* kondisi, EnkiRAM* ram) {
    if (!kondisi) return 0;

    // --- SUNTIKAN BARU: JIKA INI ADALAH LOGIKA MAJEMUK (dan / atau) ---
    if (kondisi->jenis == AST_OPERASI_LOGIKA) {
        int hasil_kiri = evaluasi_kondisi(kondisi->kiri, ram);
        
        // Logika Short-Circuit ala C untuk optimasi performa!
        if (strcmp(kondisi->pembanding, "dan") == 0) {
            if (hasil_kiri == 0) return 0; // Kiri salah, pasti salah semua!
            return evaluasi_kondisi(kondisi->kanan, ram);
        } 
        else if (strcmp(kondisi->pembanding, "atau") == 0) {
            if (hasil_kiri == 1) return 1; // Kiri benar, pasti benar semua!
            return evaluasi_kondisi(kondisi->kanan, ram);
        }
    }

    // --- LOGIKA KONDISI DASAR (A == B, dst) ---
    if (!kondisi->pembanding) return 0;
    
    // 🟢 Ambil objek murni, bukan string!
    EnkiObject* obj_kiri = evaluasi_ekspresi(kondisi->kiri, ram);
    EnkiObject* obj_kanan = evaluasi_ekspresi(kondisi->kanan, ram);
    int hasil_sah = 0;

    // 🟢 Ekstrak Angka untuk perbandingan matematis
    // 🟢 Ekstrak Angka dengan Transmutasi Otomatis
    double num_kiri = 0;
    if (obj_kiri) {
        if (obj_kiri->tipe == ENKI_ANGKA) num_kiri = obj_kiri->nilai.angka;
        else if (obj_kiri->tipe == ENKI_TEKS) num_kiri = atof(obj_kiri->nilai.teks);
    }

    double num_kanan = 0;
    if (obj_kanan) {
        if (obj_kanan->tipe == ENKI_ANGKA) num_kanan = obj_kanan->nilai.angka;
        else if (obj_kanan->tipe == ENKI_TEKS) num_kanan = atof(obj_kanan->nilai.teks);
    }
    
    // 🟢 Ekstrak String untuk perbandingan kesetaraan Teks
    char teks_kiri[2048] = ""; 
    char teks_kanan[2048] = ""; 
    objek_ke_string(obj_kiri, teks_kiri, sizeof(teks_kiri));
    objek_ke_string(obj_kanan, teks_kanan, sizeof(teks_kanan));

    if (strcmp(kondisi->pembanding, "==") == 0) hasil_sah = (strcmp(teks_kiri, teks_kanan) == 0);
    else if (strcmp(kondisi->pembanding, "!=") == 0) hasil_sah = (strcmp(teks_kiri, teks_kanan) != 0);
    else if (strcmp(kondisi->pembanding, ">") == 0) hasil_sah = (num_kiri > num_kanan);
    else if (strcmp(kondisi->pembanding, "<") == 0) hasil_sah = (num_kiri < num_kanan);
    else if (strcmp(kondisi->pembanding, ">=") == 0) hasil_sah = (num_kiri >= num_kanan);
    else if (strcmp(kondisi->pembanding, "<=") == 0) hasil_sah = (num_kiri <= num_kanan);

    // --- SUNTIKAN RADAR DEBUG ---
    // printf("[RADAR LOGIKA] Kiri: '%s' (%g) | Pembanding: '%s' | Kanan: '%s' (%g) | Hasil: %d\n", teks_kiri, num_kiri, kondisi->pembanding, teks_kanan, num_kanan, hasil_sah);

    // 🟢 Buang objek sementara
    if (obj_kiri) hancurkan_objek(obj_kiri); 
    if (obj_kanan) hancurkan_objek(obj_kanan);
    
    return hasil_sah; 
}

// FUNGSI PENCATAT BUKU HARIAN (DIARY LOGGING)
void pemicu_kernel_panic(EnkiRAM* ram, const char* pesan) {
    const char* mode_debug = "0";
    if (ram->butuh_anu_aktif == 1) {
        EnkiObject* obj_val = baca_dari_ram(ram, "MODE_DEBUG");
        if (obj_val && obj_val->tipe == ENKI_TEKS) mode_debug = obj_val->nilai.teks;
    }

    FILE *log = fopen("unul.diary", "a");
    if (log) {
        time_t t = time(NULL); struct tm *tm = localtime(&t);
        fprintf(log, "=== [TABU DILANGGAR] ===\n");
        fprintf(log, "Waktu Kejadian : %04d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        fprintf(log, "Pesan Alam     : %s\n", pesan);
        
        if (strcmp(mode_debug, "1") == 0) {
            fprintf(log, "Status         : MODE DEBUG AKTIF\nIsi RAM Saat Ini (%d Kavling Terisi):\n", ram->jumlah);
            for (int i = 0; i < ram->jumlah; i++) {
                const char* sifat = ram->kavling[i].apakah_konstanta ? "HARD" : "SOFT";
                if (ram->kavling[i].objek && ram->kavling[i].objek->tipe == ENKI_OBJEK) {
                    int jumlah_anak = ram->kavling[i].anak_anak ? ram->kavling[i].anak_anak->jumlah : 0;
                    fprintf(log, "  -> [%s] %s = { Wujud Objek: %d Properti Dalam }\n", sifat, ram->kavling[i].nama, jumlah_anak);
                } else if (ram->kavling[i].simpul_fungsi != NULL) {
                    fprintf(log, "  -> [FUNGSI] %s = <cetak_biru_instruksi>\n", ram->kavling[i].nama);
                } else {
                    char temp_val[1024] = ""; objek_ke_string(ram->kavling[i].objek, temp_val, sizeof(temp_val));
                    fprintf(log, "  -> [%s] %s = \"%s\"\n", sifat, ram->kavling[i].nama, temp_val);
                }
            }
        } else {
            fprintf(log, "Status         : MODE PRODUKSI (Minimal Log)\nSaran          : Aktifkan MODE_DEBUG=1 di file .anu\n");
        }
        fprintf(log, "========================\n\n"); fclose(log);
    }
    if (strcmp(mode_debug, "1") == 0) printf("[DEBUG-ANOMALI] Kernel Panic terdeteksi: %s\n", pesan);

    if (ram->dalam_mode_coba == 1) {
        if (ram->pesan_error_tabu) free(ram->pesan_error_tabu);
        ram->pesan_error_tabu = strdup(pesan);
        longjmp(ram->titik_kembali, 1); 
    }
    printf("🚨 KERNEL PANIC! %s\n", pesan); exit(1);
}

// Fungsi bantuan: Menghapus spasi (seperti .strip() di Python)
char* trim_spasi(char* str) {
    while(*str == ' ' || *str == '\t') str++;
    if(*str == 0) return str;
    char* ujung = str + strlen(str) - 1;
    while(ujung > str && (*ujung == ' ' || *ujung == '\t' || *ujung == '\r' || *ujung == '\n')) ujung--;
    ujung[1] = '\0';
    return str;
}

// MESIN PEMUAT RAHASIA (.anu)
void muat_anu(EnkiRAM* ram) {
    FILE *file = fopen(".anu", "r"); if (!file) return;
    simpan_ke_ram(ram, "__STATUS_ANU__", ciptakan_teks("ADA"));

    char baris[512];
    while (fgets(baris, sizeof(baris), file)) {
        char* teks = trim_spasi(baris);
        if (strlen(teks) == 0 || teks[0] == '#' || (teks[0] == '^' && teks[1] == '^')) continue;
        char* pemisah = strchr(teks, '=');
        if (pemisah) {
            *pemisah = '\0'; char* kunci = trim_spasi(teks); char* nilai = trim_spasi(pemisah + 1);
            bersihkan_kutip(nilai); simpan_ke_ram(ram, kunci, ciptakan_teks(nilai));
        }
    }
    fclose(file); printf("🛡️ [SISTEM] Kitab rahasia .anu berhasil merasuk ke memori!\n");
}

// ========================================================
// 🟢 SIHIR UTAS GAIB (GOROUTINES / MULTITHREADING)
// ========================================================
typedef struct {
    ASTNode* simpul_panggilan;
    EnkiRAM* ram_paralel;
} KapsulUtas;

// 🟢 SUNTIKAN ANTI-AMNESIA: Menyalin seluruh isi RAM termasuk fungsi!
EnkiRAM* salin_ram_untuk_utas(EnkiRAM* sumber) {
    EnkiRAM* baru = ciptakan_ram_mini(NULL); 
    baru->butuh_anu_aktif = sumber->butuh_anu_aktif;
    
    // 🟢 BAWA SAKLAR ARRAY KE DUNIA PARALEL!
    baru->status_array_dinamis = sumber->status_array_dinamis;
    baru->status_array_statis = sumber->status_array_statis;

    for (int i = 0; i < sumber->jumlah; i++) {
        EnkiObject* obj_aman = sumber->kavling[i].objek ? sumber->kavling[i].objek : ciptakan_kosong();
        simpan_ke_ram(baru, sumber->kavling[i].nama, obj_aman);
        baru->kavling[baru->jumlah - 1].simpul_fungsi = sumber->kavling[i].simpul_fungsi;
        baru->kavling[baru->jumlah - 1].tipe = sumber->kavling[i].tipe;
        baru->kavling[baru->jumlah - 1].apakah_konstanta = sumber->kavling[i].apakah_konstanta;
    }
    return baru;
}

void* pelari_utas_gaib(void* arg) {
    KapsulUtas* kapsul = (KapsulUtas*)arg;
    
    // 🟢 Ganti char* menjadi EnkiObject*
    EnkiObject* hasil = evaluasi_ekspresi(kapsul->simpul_panggilan, kapsul->ram_paralel);
    if (hasil) hancurkan_objek(hasil); // 🟢 Ganti free jadi hancurkan_objek
    
    bebaskan_ram(kapsul->ram_paralel);
    free(kapsul->ram_paralel);
    free(kapsul);
    return NULL;
}

// --- 3. EKSEKUSI NODE (MENJALANKAN PERINTAH) ---
void eksekusi_node(ASTNode* node, EnkiRAM* ram) {
    if (!node) return;
    
    // 1. Eksekusi KETIK (Output)
    if (node->jenis == AST_PERINTAH_KETIK) {
        EnkiObject* obj_hasil = evaluasi_ekspresi(node->kanan, ram);
        
        if (obj_hasil) {
            if (obj_hasil->tipe == ENKI_TEKS && obj_hasil->nilai.teks) {
                printf("%s\n", obj_hasil->nilai.teks);
            } 
            else if (obj_hasil->tipe == ENKI_ANGKA) {
                printf("%g\n", obj_hasil->nilai.angka);
            } 
            // 🟢 TAMBAHAN BARU UNTUK MENCETAK ARRAY / OBJEK
            else if (obj_hasil->tipe == ENKI_ARRAY || obj_hasil->tipe == ENKI_OBJEK) {
                cetak_objek(obj_hasil); 
                printf("\n");
            }
            else {
                printf("\n"); 
            }
            hancurkan_objek(obj_hasil); 
        } else {
            printf("\n");
        }
    }
    
    // 2. DEKLARASI TAKDIR (takdir.soft x = y ATAU takdir.soft dewa = {})
    else if (node->jenis == AST_DEKLARASI_TAKDIR) {
        char* nama = node->kiri ? node->kiri->nilai_teks : node->nilai_teks;
        if (!nama) return;

        int adalah_hard = (node->nilai_teks && strcmp(node->nilai_teks, "takdir.hard") == 0);

        // =======================================================
        // 🔥 MESIN WAKTU: Simpan masa lalu sebelum variabel ditimpa
        // =======================================================
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, nama) == 0) {
                simpan_jejak_mesin_waktu(&(ram->kavling[i])); 
                break;
            }
        }

        // =======================================================
        // 🔮 EVALUASI UNIVERSAL (Mengurus {}, angka, balikan, impor, dll)
        // =======================================================
        EnkiObject* nilai = evaluasi_ekspresi(node->kanan, ram);
        
        if (nilai) {
            simpan_ke_ram(ram, nama, nilai);
            
            // Cari kavling yang baru saja disimpan untuk mengatur properti tambahannya
            for (int i = 0; i < ram->jumlah; i++) {
                if (strcmp(ram->kavling[i].nama, nama) == 0) {
                    
                    // 1. Terapkan Hukum Konstanta (takdir.hard)
                    ram->kavling[i].apakah_konstanta = adalah_hard;

                    // 2. Jika ini Fungsi Panah, wariskan cetak birunya
                    if (nilai->tipe == ENKI_TEKS && strncmp(nilai->nilai.teks, "<anonim_", 8) == 0) {
                        for(int j = 0; j < ram->jumlah; j++) {
                            if (strcmp(ram->kavling[j].nama, nilai->nilai.teks) == 0) {
                                ram->kavling[i].simpul_fungsi = ram->kavling[j].simpul_fungsi;
                                break;
                            }
                        }
                    }

                    // 3. 🌌 SINKRONISASI DIMENSI URANTIA (Pembuatan Mini RAM)
                    // Jika nilai yang masuk adalah Objek, bangun dimensinya!
                    if (nilai->tipe == ENKI_OBJEK) {
                        sihir_bangun_dimensi(&(ram->kavling[i]), nilai, ram);
                    }
                    
                    break; // Selesai mengurus kavling ini
                }
            }
        }
    }

    // 3. Eksekusi HUKUM KARMA (Percabangan)
    else if (node->jenis == AST_HUKUM_KARMA) {
        int sah = evaluasi_kondisi(node->syarat, ram);
        if (sah) eksekusi_program(node->blok_maka, ram);
        else if (node->blok_lain) eksekusi_program(node->blok_lain, ram);
    }

    // Eksekusi Hukum Kecuali (Unless) ---
    else if (node->jenis == AST_KECUALI) {
        int sah = evaluasi_kondisi(node->syarat, ram);
        if (!sah) eksekusi_program(node->blok_maka, ram);
        else if (node->blok_lain) eksekusi_program(node->blok_lain, ram);
    }
    
    // Eksekusi PERCOCOKAN POLA (Switch-Case) ---
    else if (node->jenis == AST_COCOKKAN) {
        EnkiObject* obj_target = evaluasi_ekspresi(node->kiri, ram);
        char nilai_target[2048] = ""; objek_ke_string(obj_target, nilai_target, sizeof(nilai_target));
        int sudah_cocok = 0;

        for (int i = 0; i < node->jumlah_anak; i++) {
            ASTNode* kasus = node->anak_anak[i];
            EnkiObject* obj_kasus = evaluasi_ekspresi(kasus->kiri, ram);
            char nilai_kasus[2048] = ""; objek_ke_string(obj_kasus, nilai_kasus, sizeof(nilai_kasus));
            
            if (strcmp(nilai_target, nilai_kasus) == 0) {
                eksekusi_program(kasus->blok_maka, ram);
                sudah_cocok = 1;
                if (obj_kasus) hancurkan_objek(obj_kasus);
                break; 
            }
            if (obj_kasus) hancurkan_objek(obj_kasus);
        }
        if (!sudah_cocok && node->blok_lain) eksekusi_program(node->blok_lain, ram);
        if (obj_target) hancurkan_objek(obj_target);
    }

    // 4. Eksekusi KONTROL & TATA KRAMA
    else if (node->jenis == AST_PERINTAH_PERGI) exit(0); 
    else if (node->jenis == AST_PERINTAH_TERUS) { ram->status_terus = 1; return; }
    else if (node->jenis == AST_PERINTAH_HENTI) { ram->status_henti = 1; return; }
    else if (node->jenis == AST_DEKLARASI_DATANG) {} // [Opsional] Bisa diam saja

    // --- EVALUASI PRAGMA (ATURAN MESIN & IMPORT MODUL) ---
    else if (node->jenis == AST_PRAGMA_MEMORI) {
        if (node->nilai_teks == NULL) return; // Keamanan Ekstra
        
        if (strstr(node->nilai_teks, "array.dinamis") != NULL) {
            ram->status_array_dinamis = 1; // 🟢 Aktifkan sihir .ko
        } 
        else if (strstr(node->nilai_teks, "array.statis") != NULL) {
            ram->status_array_statis = 1;  // 🟢 Aktifkan sihir .ku
        }
        else if (strcmp(node->nilai_teks, "butuh .anu") == 0) {
            ram->butuh_anu_aktif = 1; 
            EnkiObject* status = baca_dari_ram(ram, "__STATUS_ANU__");
            if (status == NULL) pemicu_kernel_panic(ram, "Kitab ini mewajibkan file rahasia '.anu', tapi tidak ditemukan!");
        }
        // Jangan di-return di sini, biarkan jalan ke bawah jika perlu
    }

    // --- 5. HUKUM SIKLUS (Looping: effort X kali maka) ---
    // (BLOK INI SUDAH ANDA TANGANI DI PESAN SEBELUMNYA. KITA GUNAKAN YANG SUDAH JADI)
    else if (node->jenis == AST_HUKUM_SIKLUS) {
        if (!node->batas_loop) {
            pemicu_kiamat_presisi(node, ram, "Hukum Siklus cacat!", "Anda tidak memberikan jumlah perulangan.\nContoh yang sah: effort 5 kali maka");
            return;
        }

        EnkiObject* obj_batas = evaluasi_ekspresi(node->batas_loop, ram);
        if (!obj_batas || (obj_batas->tipe == ENKI_TEKS && (!obj_batas->nilai.teks || strlen(obj_batas->nilai.teks) == 0))) {
            pemicu_kiamat_presisi(node, ram, "Batas perulangan Gaib!", "Nilai batas perulangan tidak boleh kosong.");
            if (obj_batas) hancurkan_objek(obj_batas);
            return;
        }

        int batas = 0;
        if (obj_batas->tipe == ENKI_ANGKA) batas = (int)obj_batas->nilai.angka;
        else if (obj_batas->tipe == ENKI_TEKS) batas = atoi(obj_batas->nilai.teks);
        
        if (batas <= 0) {
            pemicu_kiamat_presisi(node, ram, "Batas siklus tidak masuk akal!", "Jumlah 'kali' pada effort harus berupa angka mutlak lebih dari 0."); 
            if (obj_batas) hancurkan_objek(obj_batas);
            return;
        }
        if (obj_batas) hancurkan_objek(obj_batas);

        for (int i = 1; i <= batas; i++) {
            int ketemu = 0;
            for (int j = 0; j < ram->jumlah; j++) {
                if (strcmp(ram->kavling[j].nama, "putaran") == 0) {
                    if (ram->kavling[j].objek) hancurkan_objek(ram->kavling[j].objek);
                    ram->kavling[j].objek = ciptakan_angka((double)i);
                    ketemu = 1; break;
                }
            }
            if (!ketemu) { 
                if (ram->jumlah >= ram->kapasitas) { ram->kapasitas *= 2; ram->kavling = realloc(ram->kavling, ram->kapasitas * sizeof(KavlingMemori)); }
                ram->kavling[ram->jumlah].nama = strdup("putaran");
                ram->kavling[ram->jumlah].tipe = TIPE_VARIABEL_SOFT;
                ram->kavling[ram->jumlah].objek = ciptakan_angka((double)i);
                ram->kavling[ram->jumlah].anak_anak = NULL;
                ram->jumlah++;
            }

            eksekusi_program(node->blok_siklus, ram);
            if (ram->status_henti) { ram->status_henti = 0; break; }
            if (ram->status_terus) { ram->status_terus = 0; continue; }
            if (ram->status_pulang) { break; }
        }
    }

    // --- EKSEKUSI UTAS GAIB (PARALEL) ---
    else if (node->jenis == AST_UTAS) {
        if (node->kiri && node->kiri->jenis == AST_PANGGILAN_FUNGSI) {
            pthread_t id_utas;
            KapsulUtas* kapsul = (KapsulUtas*)malloc(sizeof(KapsulUtas));
            kapsul->simpul_panggilan = node->kiri;
            kapsul->ram_paralel = salin_ram_untuk_utas(ram); 
            
            pthread_create(&id_utas, NULL, pelari_utas_gaib, kapsul);
            pthread_detach(id_utas); 
        } else {
            pemicu_kiamat_presisi(node, ram, "Sihir utas cacat!", "Sihir 'utas' atau 'gaib' hanya bisa digunakan untuk memanggil fungsi.");
        }
    }

    // --- EKSEKUSI JADWAL & EFFORT ---
    else if (node->jenis == AST_JADWAL) eksekusi_jadwal_gaib(node, ram);
    else if (node->jenis == AST_EFFORT_WAKTU) eksekusi_effort_gaib(node, ram);

    // --- 6. SOWAN ---
    // (BLOK INI SUDAH ANDA TANGANI DI PESAN SEBELUMNYA. KITA GUNAKAN YANG SUDAH JADI)
    else if (node->jenis == AST_PERINTAH_SOWAN) {
        EnkiObject* obj_target = evaluasi_ekspresi(node->kiri, ram);
        const char* target_mentah = "";
        if (obj_target && obj_target->tipe == ENKI_TEKS && obj_target->nilai.teks) target_mentah = obj_target->nilai.teks;
        
        char* target_ekspansi = ekspansi_jalur(target_mentah);
        char target_file[1024]; strncpy(target_file, target_ekspansi, sizeof(target_file) - 1); target_file[sizeof(target_file) - 1] = '\0';
        if (obj_target) hancurkan_objek(obj_target);
        
        const char* titik = strrchr(target_file, '.');
        if (titik && strcmp(titik, ".unll") != 0) {
            char pesan_err[2048]; snprintf(pesan_err, sizeof(pesan_err), "Pustaka '%s' DITOLAK!", target_file);
            char panduan[2048]; snprintf(panduan, sizeof(panduan), "Sesuai Hukum Dasar UNUL, perintah 'sowan' HANYA untuk pustaka (.unll).\nEkstensi '%s' tidak diizinkan di jalur ini.", titik);
            pemicu_kiamat_presisi(node, ram, pesan_err, panduan);
            free(target_ekspansi); return;
        }

        if (!titik) strncat(target_file, ".unll", sizeof(target_file) - strlen(target_file) - 1);

        char jalur_ditemukan[2048] = ""; FILE* file = NULL;
        if (target_ekspansi[0] == '/' || target_ekspansi[0] == '\\' || target_mentah[0] == '~') {
            snprintf(jalur_ditemukan, sizeof(jalur_ditemukan), "%s", target_file); file = fopen(jalur_ditemukan, "r");
        }
        if (!file) {
            char* daftar_radar[] = {"./", "./lib/", "/usr/lib/unul/", "/opt/unul/lib/", NULL};
            for (int i = 0; daftar_radar[i] != NULL; i++) {
                snprintf(jalur_ditemukan, sizeof(jalur_ditemukan), "%s%s", daftar_radar[i], target_file);
                file = fopen(jalur_ditemukan, "r"); if (file) break;
            }
        }
        if (!file) {
            char* markas_user = dapatkan_jalur_markas_user(); 
            if (markas_user) {
                snprintf(jalur_ditemukan, sizeof(jalur_ditemukan), "%s%c.unul%clib%c%s", markas_user, PEMISAH_JALUR, PEMISAH_JALUR, PEMISAH_JALUR, target_file);
                file = fopen(jalur_ditemukan, "r");
                if (!file) {
                    snprintf(jalur_ditemukan, sizeof(jalur_ditemukan), "%s%c.local%clib%cunul%c%s", markas_user, PEMISAH_JALUR, PEMISAH_JALUR, PEMISAH_JALUR, PEMISAH_JALUR, target_file);
                    file = fopen(jalur_ditemukan, "r");
                }
            }
        }

        if (!file) {
            char pesan_gaib[2048]; snprintf(pesan_gaib, sizeof(pesan_gaib), "Pustaka '%s' tidak ditemukan di alam manapun!", target_file);
            pemicu_kiamat_presisi(node, ram, pesan_gaib, "Radar UNUL sudah menyisir lokasi sistem dan lokal. Hasil: NIHIL.");
            free(target_ekspansi); return;
        }

        fseek(file, 0, SEEK_END); long fsize = ftell(file); fseek(file, 0, SEEK_SET);
        char *kode_sowan = malloc(fsize + 1); fread(kode_sowan, 1, fsize, file); kode_sowan[fsize] = '\0'; fclose(file);
        
        TokenArray tk_sowan = enki_lexer(kode_sowan, jalur_ditemukan);
        Parser p_sowan = inisialisasi_parser(tk_sowan);
        ASTNode* ast_sowan = parse_program(&p_sowan);
        eksekusi_program(ast_sowan, ram);
        free(target_ekspansi);
    }

    // --- 8. PEMBEBASAN MEMORI (pasrah) ---
    // (BLOK INI SUDAH ANDA TANGANI, KITA MASUKKAN VERSI TERBARU)
    else if (node->jenis == AST_PERINTAH_PASRAH) {
        char* target_nama = NULL;
        if (node->kiri && node->kiri->jenis == AST_IDENTITAS) target_nama = node->kiri->nilai_teks;
        else if (node->nilai_teks) target_nama = node->nilai_teks;

        if (target_nama) {
            for (int i = 0; i < ram->jumlah; i++) {
                if (strcmp(ram->kavling[i].nama, target_nama) == 0) {
                    if (ram->kavling[i].apakah_konstanta) {
                        char pesan[256]; snprintf(pesan, sizeof(pesan), "Pelanggaran Memori: Variabel '%s' abadi, tidak bisa di-pasrah-kan!", target_nama);
                        pemicu_kiamat_presisi(node, ram, pesan, "Variabel 'hard' menolak ketiadaan.");
                        return;
                    }
                    if (ram->kavling[i].objek) hancurkan_objek(ram->kavling[i].objek);
                    if (ram->kavling[i].anak_anak) { bebaskan_ram(ram->kavling[i].anak_anak); free(ram->kavling[i].anak_anak); ram->kavling[i].anak_anak = NULL; }
                    ram->kavling[i].tipe = TIPE_VARIABEL_SOFT;
                    ram->kavling[i].objek = ciptakan_kosong();
                    break;
                }
            }
        }
    }

    // --- 10. SIHIR JEDA (KENDALI WAKTU) ---
    else if (node->jenis == AST_PERINTAH_JEDA) {
        EnkiObject* obj_waktu = evaluasi_ekspresi(node->kiri, ram);
        if (obj_waktu) {
            double nilai = 0; long long ms = 0; 
            if (obj_waktu->tipe == ENKI_ANGKA) { nilai = obj_waktu->nilai.angka; ms = (long long)nilai; }
            else if (obj_waktu->tipe == ENKI_TEKS && obj_waktu->nilai.teks) {
                nilai = atof(obj_waktu->nilai.teks);
                if (strstr(obj_waktu->nilai.teks, "s") || strstr(obj_waktu->nilai.teks, "d")) ms = (long long)(nilai * 1000.0);
                else if (strstr(obj_waktu->nilai.teks, "m")) ms = (long long)(nilai * 60.0 * 1000.0);
                else if (strstr(obj_waktu->nilai.teks, "h") || strstr(obj_waktu->nilai.teks, "j")) ms = (long long)(nilai * 3600.0 * 1000.0);
                else ms = (long long)nilai;
            }
            usleep(ms * 1000); 
            hancurkan_objek(obj_waktu);
        }
    }

    // --- 9. SIHIR BALIKAN (MESIN WAKTU: VERSI DIMENSI) ---
    else if (node->jenis == AST_PERINTAH_BALIKAN) {
        KavlingMemori* akar_utama = NULL; // Akar (bos) yang memegang riwayat
        
        if (node->kiri) { 
            // Kita hanya butuh Akar Utama untuk memutar balik waktu seluruh objek
            akar_utama = cari_induk_utama(node->kiri, ram); 
        } 
        else if (node->nilai_teks) {
            // Logika pencarian variabel biasa (akar langsung)
            for (int i = 0; i < ram->jumlah; i++) {
                if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) { 
                    akar_utama = &(ram->kavling[i]); 
                    break; 
                }
            }
        }
        
        // 🌀 PROSES PEMULIHAN 🌀
        if (akar_utama && akar_utama->jumlah_riwayat > 0) {
            akar_utama->jumlah_riwayat--; 
            JejakMasaLalu* masa_lalu = &akar_utama->riwayat[akar_utama->jumlah_riwayat];
            
            // 1. Hancurkan wujud masa kini (Akar & Seluruh Anak-anaknya)
            if (akar_utama->objek) hancurkan_objek(akar_utama->objek);
            if (akar_utama->anak_anak) { 
                bebaskan_ram(akar_utama->anak_anak); 
                free(akar_utama->anak_anak); 
                akar_utama->anak_anak = NULL;
            }
            
            // 2. 🟢 PULIHKAN: Reinkarnasi dari masa lalu (Deep Copy)
            akar_utama->tipe = masa_lalu->tipe; 
            akar_utama->objek = masa_lalu->objek ? ciptakan_salinan_objek(masa_lalu->objek) : NULL; 
            akar_utama->anak_anak = masa_lalu->anak_anak ? salin_ram_rekursif(masa_lalu->anak_anak) : NULL;
        }
    }

    // 7. HUKUM TABU (Try-Catch / setjmp)
    else if (node->jenis == AST_COBA_TABU) {
        int status_coba_lama = ram->dalam_mode_coba;
        jmp_buf titik_kembali_lama; memcpy(titik_kembali_lama, ram->titik_kembali, sizeof(jmp_buf));

        ram->dalam_mode_coba = 1;
        int lontaran = setjmp(ram->titik_kembali);
        
        if (lontaran == 0) {
            if (node->kiri) eksekusi_program(node->kiri, ram); 
        } else {
            ram->dalam_mode_coba = status_coba_lama; 
            if (node->nilai_teks != NULL) {
                if (ram->pesan_error_tabu) { simpan_ke_ram(ram, node->nilai_teks, ciptakan_teks(ram->pesan_error_tabu)); free(ram->pesan_error_tabu); ram->pesan_error_tabu = NULL; } 
                else simpan_ke_ram(ram, node->nilai_teks, ciptakan_teks("Kiamat Tak Dikenal"));
            }
            if (node->kanan) eksekusi_program(node->kanan, ram);
        }

        if (node->blok_tebus) { ram->dalam_mode_coba = status_coba_lama; eksekusi_program(node->blok_tebus, ram); }
        ram->dalam_mode_coba = status_coba_lama; memcpy(ram->titik_kembali, titik_kembali_lama, sizeof(jmp_buf));
    }

    // --- PENANGKAP PENCIPTAAN FUNGSI ---
    else if (node->jenis == AST_DEKLARASI_FUNGSI) {
        simpan_ke_ram(ram, node->nilai_teks, ciptakan_teks("<fungsi_kustom>"));
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) { ram->kavling[i].simpul_fungsi = node; break; }
        }
        return;
    }

    // --- PENANGKAP PERINTAH PULANG (RETURN) ---
    else if (node->jenis == AST_PULANG) {
        if (ram->nilai_kembalian) { hancurkan_objek(ram->nilai_kembalian); ram->nilai_kembalian = NULL; }
        if (node->kiri) {
            EnkiObject* hasil = evaluasi_ekspresi(node->kiri, ram);
            if (hasil) ram->nilai_kembalian = hasil; 
        } else {
            ram->nilai_kembalian = ciptakan_kosong();
        }
        ram->status_pulang = 1; return;
    }

    // EKSEKUTOR MUTASI BEBAS
    else if (node->jenis == AST_OPERASI_MATEMATIKA || node->jenis == AST_PANGGILAN_FUNGSI) {
        EnkiObject* hasil_mutasi = evaluasi_ekspresi(node, ram);
        if (hasil_mutasi) hancurkan_objek(hasil_mutasi); 
    }
}

// --- 4. EKSEKUSI PROGRAM UTAMA ---
void eksekusi_program(ASTNode* program, EnkiRAM* ram) {
    if (!program || program->jenis != AST_PROGRAM) return;
    
    for (int i = 0; i < program->jumlah_anak; i++) {
        eksekusi_node(program->anak_anak[i], ram);

        // --- REM DARURAT ---
        if (ram->status_pulang == 1) break; 
        
        // JIKA ALARM TERUS MENYALA, hentikan eksekusi baris bawahnya,
        // lalu kembalikan kendali ke pemanggil (Hukum Siklus)
        if (ram->status_terus == 1) break; 
        if (ram->status_henti == 1) break;
    }
}