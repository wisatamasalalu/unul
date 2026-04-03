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
#include "enki_interpreter.h"

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
    
    // --- SUNTIKAN UNLIMITED ---
    ram.nilai_kembalian = NULL;  
    ram.pesan_error_tabu = NULL; 
    return ram;
}

// Membangkitkan RAM Mini (Untuk Objek Bersarang)
EnkiRAM* ciptakan_ram_mini() {
    EnkiRAM* ram_baru = (EnkiRAM*)malloc(sizeof(EnkiRAM));
    ram_baru->kapasitas = 10;
    ram_baru->jumlah = 0;
    ram_baru->kavling = (KavlingMemori*)malloc(10 * sizeof(KavlingMemori));
    ram_baru->butuh_anu_aktif = 0;
    ram_baru->dalam_mode_coba = 0;
    ram_baru->status_pulang = 0;
    ram_baru->status_terus = 0;
    ram_baru->status_henti = 0;
    
    ram_baru->nilai_kembalian = NULL;
    ram_baru->pesan_error_tabu = NULL;
    return ram_baru;
}

// Menghancurkan RAM secara REKURSIF (Anti Kebocoran Memori)
void bebaskan_ram(EnkiRAM* ram) {
    if (!ram) return;
    for (int i = 0; i < ram->jumlah; i++) {
        if (ram->kavling[i].nama) free(ram->kavling[i].nama);
        
        // Hancurkan teks biasa
        if (ram->kavling[i].tipe == TIPE_TEKS && ram->kavling[i].nilai_teks) {
            free(ram->kavling[i].nilai_teks);
        }
        
        // 🔥 Hancurkan Dimensi Objek Bersarang! 🔥
        if (ram->kavling[i].tipe == TIPE_OBJEK && ram->kavling[i].anak_anak) {
            bebaskan_ram(ram->kavling[i].anak_anak); // Panggil diri sendiri untuk bersihkan isinya
            free(ram->kavling[i].anak_anak);         // Hancurkan pointer RAM-nya
        }
    }
    
    // Bebaskan wadah string unlimited
    if (ram->pesan_error_tabu) free(ram->pesan_error_tabu);
    if (ram->nilai_kembalian) free(ram->nilai_kembalian);
    
    if (ram->kavling) free(ram->kavling);
    // (JANGAN memanggil free(ram) di sini karena struct awal bukan pointer yang di-malloc)
}

// Fungsi Internal: Menyimpan atau menimpa nilai di RAM
void simpan_ke_ram(EnkiRAM* ram, const char* nama, const char* nilai) {
    // 1. Cari apakah sudah ada di RAM (Cek Hukum Takdir)
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            
            // 🔥 CEK HUKUM TAKDIR MUTLAK 🔥
            if (ram->kavling[i].apakah_konstanta) {
                char pesan[256];
                snprintf(pesan, sizeof(pesan), "Pelanggaran Hukum Takdir: Variabel 'hard' (%s) tidak boleh diubah!", nama);
                pemicu_kernel_panic(ram, pesan);
                return;
            }

            // (Lanjutkan proses timpa nilai jika bukan konstanta...)
            if (ram->kavling[i].tipe == TIPE_TEKS && ram->kavling[i].nilai_teks) {
                free(ram->kavling[i].nilai_teks);
            } 
            else if (ram->kavling[i].tipe == TIPE_OBJEK && ram->kavling[i].anak_anak) {
                // Jika variabel ini asalnya objek, hancurkan dimensi anaknya!
                bebaskan_ram(ram->kavling[i].anak_anak);
                free(ram->kavling[i].anak_anak);
                ram->kavling[i].anak_anak = NULL;
            }

            ram->kavling[i].tipe = TIPE_TEKS;
            ram->kavling[i].nilai_teks = strdup(nilai); 
            return;
        }
    }
    
    // 2. Jika belum ada, buat kavling baru
    if (ram->jumlah >= ram->kapasitas) {
        ram->kapasitas *= 2;
        ram->kavling = (KavlingMemori*)realloc(ram->kavling, ram->kapasitas * sizeof(KavlingMemori));
    }
    
    // Inisialisasi anatomi kavling baru dengan bersih
    ram->kavling[ram->jumlah].nama = strdup(nama);
    ram->kavling[ram->jumlah].tipe = TIPE_TEKS;
    ram->kavling[ram->jumlah].nilai_teks = strdup(nilai);
    ram->kavling[ram->jumlah].anak_anak = NULL;            // Wajib NULL agar tidak dianggap objek
    ram->kavling[ram->jumlah].simpul_fungsi = NULL;
    ram->kavling[ram->jumlah].apakah_konstanta = 0; // 🔥 Default awal adalah soft (0)

    // ========================================================
    // 🔥 LETAKKAN DI SINI: INISIALISASI KETIADAAN MASA LALU 🔥
    // ========================================================
    ram->kavling[ram->jumlah].riwayat = NULL;
    ram->kavling[ram->jumlah].jumlah_riwayat = 0;
    ram->kavling[ram->jumlah].kapasitas_riwayat = 0;

    ram->jumlah++;
}

// 🔥 SIHIR BALIKAN (DEEP COPY MESIN WAKTU) 🔥
EnkiRAM* salin_ram_rekursif(EnkiRAM* sumber) {
    if (!sumber) return NULL;
    
    // 1. Ciptakan dimensi bayangan yang kosong
    EnkiRAM* baru = ciptakan_ram_mini();
    
    // 2. Fotokopi seluruh kavling di dimensi sumber
    for (int i = 0; i < sumber->jumlah; i++) {
        // 🔥 PELINDUNG SEGFAULT: Objek tidak memiliki nilai_teks (NULL).
        // Kita berikan "KOSONG" agar strdup() di simpan_ke_ram tidak meledak!
        const char* nilai_aman = sumber->kavling[i].nilai_teks ? sumber->kavling[i].nilai_teks : "KOSONG";
        simpan_ke_ram(baru, sumber->kavling[i].nama, nilai_aman);
        
        // Kunci status takdir dan tipe wujudnya agar persis sama
        baru->kavling[i].tipe = sumber->kavling[i].tipe;
        baru->kavling[i].apakah_konstanta = sumber->kavling[i].apakah_konstanta;
        
        // 3. JIKA IA OBJEK: Selam dan salin RAM di dalamnya secara rekursif!
        if (sumber->kavling[i].tipe == TIPE_OBJEK && sumber->kavling[i].anak_anak) {
            baru->kavling[i].anak_anak = salin_ram_rekursif(sumber->kavling[i].anak_anak);
        }
    }
    return baru;
}

// Fungsi Internal: Membaca nilai dari RAM
const char* baca_dari_ram(EnkiRAM* ram, const char* nama) {
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            
            // 🔥 SUNTIKAN BARU: Jika yang dipanggil adalah Objek Utuh
            if (ram->kavling[i].tipe == TIPE_OBJEK) {
                return "[Wujud Objek / Domain Bersarang]";
            }
            
            // Jika teks biasa
            return ram->kavling[i].nilai_teks;
        }
    }
    return NULL; 
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
        if (induk && induk->tipe == TIPE_OBJEK && induk->anak_anak) {
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
            if (induk->tipe == TIPE_TEKS) {
                if (induk->nilai_teks) { free(induk->nilai_teks); induk->nilai_teks = NULL; }
                induk->tipe = TIPE_OBJEK;
                induk->anak_anak = ciptakan_ram_mini();
            }

            EnkiRAM* ram_anak = induk->anak_anak;
            
            // Cari apakah properti anak ini sudah ada?
            for (int i = 0; i < ram_anak->jumlah; i++) {
                if (strcmp(ram_anak->kavling[i].nama, node->nilai_teks) == 0) {
                    return &(ram_anak->kavling[i]);
                }
            }

            // Jika belum ada, Ciptakan Jalan Dimensi Baru!
            simpan_ke_ram(ram_anak, node->nilai_teks, "[MUTASI]"); 
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

// --- 2. LOGIKA EVALUASI NILAI ---
// --- 2. LOGIKA EVALUASI NILAI ---
char* evaluasi_ekspresi(ASTNode* node, EnkiRAM* ram) {
    if (!node) return strdup(""); 
    
    // 1. Literal Teks/Angka
    if (node->jenis == AST_LITERAL_TEKS) {
        char* teks_bersih = strdup(node->nilai_teks);
        bersihkan_kutip(teks_bersih);
        return teks_bersih;
    }
    
    // 2. Identitas (Variabel)
    if (node->jenis == AST_IDENTITAS) {
        const char* memori = baca_dari_ram(ram, node->nilai_teks);
        if (memori) return strdup(memori);
        
        char pesan_error[256];
        snprintf(pesan_error, sizeof(pesan_error), "Takdir '%s' belum diciptakan!", node->nilai_teks);
        
        // Panggil dengan ram dan pesan
        pemicu_kernel_panic(ram, pesan_error); 
        return strdup(""); 
    }

    // 3.1 EVALUASI AKSES DOMAIN (TITIK) ---
    else if (node->jenis == AST_AKSES_DOMAIN) {
        KavlingMemori* target = cari_kavling_domain(node, ram);
        
        if (target) {
            // Jika yang dipanggil ternyata objek lagi (misal: ketik(dewa.senjata) dan senjata itu objek)
            if (target->tipe == TIPE_OBJEK) {
                return strdup("[Wujud Objek / Domain Bersarang]");
            }
            // Jika teks biasa
            if (target->nilai_teks) {
                return strdup(target->nilai_teks);
            }
        }
        
        // Jika properti gaib / tidak ada
        pemicu_kernel_panic(ram, "Domain bersarang atau properti tidak ditemukan!");
        return strdup("");
    }
    
    // 3. Fungsi Dengar (Input)
    if (node->jenis == AST_FUNGSI_DENGAR) {
        char* input = readline("> ");
        if (input) {
            if (*input) add_history(input); // Ingat ketikan ini agar panah atas berfungsi
            return input; // readline menggunakan malloc, jadi aman langsung direturn
        }
        return strdup("");
    }

    // 4. Operasi Matematika & Penugasan
    if (node->jenis == AST_OPERASI_MATEMATIKA) {
        
        // ==========================================================
        // 🔥 SUNTIKAN MUTASI BRUTAL (TANDA '=') HARUS DI SINI! 🔥
        // Mencegat mesin sebelum ia mencoba membaca sisi kiri (L-Value)
        // ==========================================================
        if (node->operator_math && strcmp(node->operator_math, "=") == 0) {
            char* hasil_akhir = (char*)malloc(1024);
            memset(hasil_akhir, 0, 1024);

            // Cari target dan JUGA INDUKNYA!
            KavlingMemori* target = temukan_atau_ciptakan_kavling(node->kiri, ram);
            KavlingMemori* induk = cari_induk_utama(node->kiri, ram); 
            
            if (target && induk) {
                // =======================================================
                // 🔥 MESIN WAKTU: Simpan masa lalu DI INDUK UTAMA!
                // =======================================================
                if (induk->jumlah_riwayat >= induk->kapasitas_riwayat) {
                    induk->kapasitas_riwayat = induk->kapasitas_riwayat == 0 ? 4 : induk->kapasitas_riwayat * 2;
                    induk->riwayat = (JejakMasaLalu*)realloc(induk->riwayat, induk->kapasitas_riwayat * sizeof(JejakMasaLalu));
                }
                JejakMasaLalu* jejak = &induk->riwayat[induk->jumlah_riwayat++];
                jejak->tipe = induk->tipe;
                jejak->nilai_teks = induk->nilai_teks ? strdup(induk->nilai_teks) : NULL;
                
                // Salin seluruh wujud semesta dari Induk ini!
                jejak->anak_anak = (induk->tipe == TIPE_OBJEK && induk->anak_anak) ? salin_ram_rekursif(induk->anak_anak) : NULL;

                // =======================================================
                // 🔥 SUNTIKAN HUKUM: Cek apakah target dikunci
                if (target->apakah_konstanta) {
                    char pesan[256];
                    snprintf(pesan, sizeof(pesan), "🚨 KIAMAT! Pelanggaran Hukum Takdir: Variabel 'hard' (%s) tidak bisa diubah!", target->nama);
                    pemicu_kernel_panic(ram, pesan);
                    return strdup(""); 
                }

                // Hancurkan kenangan lama masa kini (Dari Target)
                if (target->tipe == TIPE_TEKS && target->nilai_teks) {
                    free(target->nilai_teks);
                    target->nilai_teks = NULL;
                } else if (target->tipe == TIPE_OBJEK && target->anak_anak) {
                    bebaskan_ram(target->anak_anak);
                    free(target->anak_anak);
                    target->anak_anak = NULL;
                }

                // =======================================================
                // 🔥 CEK APAKAH INI SIHIR BALIKAN() via Penugasan ( x = balikan(y) )
                // =======================================================
                if (node->kanan->jenis == AST_PANGGILAN_FUNGSI && strcmp(node->kanan->nilai_teks, "balikan") == 0) {
                    KavlingMemori* sumber = temukan_atau_ciptakan_kavling(node->kanan->anak_anak[0], ram);
                    if (sumber) {
                        target->tipe = sumber->tipe;
                        target->nilai_teks = strdup(sumber->nilai_teks ? sumber->nilai_teks : "KOSONG");
                        
                        // Fotokopi dimensinya secara brutal!
                        if (sumber->tipe == TIPE_OBJEK && sumber->anak_anak) {
                            target->anak_anak = salin_ram_rekursif(sumber->anak_anak);
                        }
                        return strdup(target->nilai_teks ? target->nilai_teks : "[Wujud Objek]");
                    }
                }
                
                // ... (Biarkan kode di bawahnya A. JIKA DIA DITIMPA MENJADI OBJEK {} tetap seperti aslinya)

                // A. JIKA DIA DITIMPA MENJADI OBJEK {}
                if (node->kanan && node->kanan->jenis == AST_STRUKTUR_OBJEK) {
                    target->tipe = TIPE_OBJEK;
                    target->anak_anak = ciptakan_ram_mini();
                    
                    for (int i = 0; i < node->kanan->jumlah_anak; i++) {
                        ASTNode* pasangan = node->kanan->anak_anak[i];
                        if (!pasangan || !pasangan->pembanding) continue;

                        char* kunci = strdup(pasangan->pembanding);
                        bersihkan_kutip(kunci);
                        
                        char* nilai_anak = evaluasi_ekspresi(pasangan->kiri, ram);
                        if (nilai_anak) {
                            simpan_ke_ram(target->anak_anak, kunci, nilai_anak);
                            free(nilai_anak);
                        }
                        free(kunci);
                    }
                    snprintf(hasil_akhir, 1024, "[Wujud Objek / Domain Bersarang]");
                } 
                // B. JIKA DIA DITIMPA MENJADI TEKS/ANGKA BIASA
                else {
                    // Evaluasi nilai kanan SAJA!
                    char* nilai_baru = evaluasi_ekspresi(node->kanan, ram);
                    target->tipe = TIPE_TEKS;
                    target->nilai_teks = nilai_baru ? strdup(nilai_baru) : strdup("");
                    snprintf(hasil_akhir, 1024, "%s", target->nilai_teks);
                    if (nilai_baru) free(nilai_baru);
                }
            } else {
                pemicu_kernel_panic(ram, "Gagal memutasi memori. Variabel induk belum ditakdirkan!");
            }
            return hasil_akhir; // Langsung pulang!
        }
        // ==========================================================

        // JIKA BUKAN PENUGASAN '=' (Melainkan +, -, *, /)
        // Baru kita evaluasi kedua sisinya sebagai angka/teks!
        char* hasil_kiri = evaluasi_ekspresi(node->kiri, ram);
        char* hasil_kanan = evaluasi_ekspresi(node->kanan, ram);
        char* hasil_akhir = (char*)malloc(1024);
        memset(hasil_akhir, 0, 1024);

        // Konversi angka HANYA dilakukan JIKA operatornya bukan '='
        if (node->operator_math) {
            double angka_kiri = atof(hasil_kiri);
            double angka_kanan = atof(hasil_kanan);

            // 1. TAMBAH (+) : Punya sihir penggabungan teks
            if (strcmp(node->operator_math, "+") == 0) {
                if ((angka_kiri == 0 && strcmp(hasil_kiri, "0") != 0) || 
                    (angka_kanan == 0 && strcmp(hasil_kanan, "0") != 0)) {
                    snprintf(hasil_akhir, 1024, "%s%s", hasil_kiri, hasil_kanan);
                } else {
                    snprintf(hasil_akhir, 1024, "%g", angka_kiri + angka_kanan);
                }
            }
            // 2. KURANG (-) : Tolak teks (Validasi Tipe Data)
            else if (strcmp(node->operator_math, "-") == 0) {
                if ((angka_kiri == 0 && strcmp(hasil_kiri, "0") != 0) || 
                    (angka_kanan == 0 && strcmp(hasil_kanan, "0") != 0)) {
                    pemicu_kernel_panic(ram, "Pelanggaran Tipe: Operasi kurang (-) tidak berlaku untuk kata/teks!");
                } else {
                    snprintf(hasil_akhir, 1024, "%g", angka_kiri - angka_kanan);
                }
            }
            // 3. KALI (*)
            else if (strcmp(node->operator_math, "*") == 0) {
                snprintf(hasil_akhir, 1024, "%g", angka_kiri * angka_kanan);
            }
            // 4. BAGI PRESISI (/ atau :)
            else if (strcmp(node->operator_math, "/") == 0 || strcmp(node->operator_math, ":") == 0) {
                // --- HUKUM ENLIL (Cegah Pembagian 0) ---
                if (angka_kanan == 0) {
                    pemicu_kernel_panic(ram, "🚨 KERNEL PANIC! Kehancuran Dimensi: Pembagian dengan nol (0) dilarang oleh Hukum Enlil!");
                } else {
                    snprintf(hasil_akhir, 1024, "%g", angka_kiri / angka_kanan);
                }
            }
            // 5. BAGI BULAT/FLOOR (//)
            else if (strcmp(node->operator_math, "//") == 0) {
                if (angka_kanan == 0) {
                    pemicu_kernel_panic(ram, "Kehancuran Dimensi: Pembagian dengan nol (0) dilarang oleh Hukum Enlil!");
                } else {
                    snprintf(hasil_akhir, 1024, "%.0f", floor(angka_kiri / angka_kanan));
                }
            }
            // 6. MODULUS/SISA BAGI (%)
            else if (strcmp(node->operator_math, "%") == 0) {
                if (angka_kanan == 0) {
                    pemicu_kernel_panic(ram, "Kehancuran Dimensi: Modulus dengan nol (0) dilarang oleh Hukum Enlil!");
                } else {
                    snprintf(hasil_akhir, 1024, "%g", fmod(angka_kiri, angka_kanan));
                }
            }
        }
        free(hasil_kiri); free(hasil_kanan);
        return hasil_akhir;
    }

    // 5. Struktur Array [a, b, c]
    if (node->jenis == AST_STRUKTUR_ARRAY) {
        char buffer[2048] = "[";
        for (int i = 0; i < node->jumlah_anak; i++) {
            char* isi_elemen = evaluasi_ekspresi(node->anak_anak[i], ram);
            strncat(buffer, isi_elemen, 2048 - strlen(buffer) - 1);
            free(isi_elemen);
            if (i < node->jumlah_anak - 1) strncat(buffer, ", ", 2048 - strlen(buffer) - 1);
        }
        strncat(buffer, "]", 2048 - strlen(buffer) - 1);
        return strdup(buffer);
    }

    // 6. Akses Array (daftar[0])
    if (node->jenis == AST_AKSES_ARRAY) {
        const char* mem_arr = baca_dari_ram(ram, node->kiri->nilai_teks);
        if (!mem_arr) {
            char p[256];
            snprintf(p, sizeof(p), "Array '%s' belum diciptakan!", node->kiri->nilai_teks);
            pemicu_kernel_panic(ram, p);
        }
        char* teks_indeks = evaluasi_ekspresi(node->indeks_array, ram);
        int indeks = atoi(teks_indeks);
        free(teks_indeks);
        return ambil_elemen_array(mem_arr, indeks);
    }

    // --- PENANGKAP PANGGILAN FUNGSI ---
    if (node->jenis == AST_PANGGILAN_FUNGSI) {
        
        // =======================================================
        // 1. [TABLET OF DESTINIES] CEK FUNGSI BAWAAN (NATIVE) DULU!
        // =======================================================
        
        // A. Fungsi acak(min, max)
        if (strcmp(node->nilai_teks, "acak") == 0) {
            char* str_min = evaluasi_ekspresi(node->anak_anak[0], ram);
            char* str_max = evaluasi_ekspresi(node->anak_anak[1], ram);
            int min = atoi(str_min);
            int max = atoi(str_max);
            free(str_min); free(str_max);
            
            int hasil = min + (rand() % (max - min + 1));
            
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d", hasil);
            return strdup(buffer);
        }
        
        // B. Fungsi panjang(teks)
        if (strcmp(node->nilai_teks, "panjang") == 0) {
            char* isi_teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            int panjang_karakter = strlen(isi_teks);
            free(isi_teks);
            
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d", panjang_karakter);
            return strdup(buffer);
        }

        // C. Fungsi waktu_sekarang()
        if (strcmp(node->nilai_teks, "waktu_sekarang") == 0) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%ld", (long)time(NULL));
            return strdup(buffer);
        }

        // D. Fungsi huruf_besar(teks) dan huruf_kecil(teks)
        if (strcmp(node->nilai_teks, "huruf_besar") == 0 || strcmp(node->nilai_teks, "huruf_kecil") == 0) {
            char* teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            int is_upper = (strcmp(node->nilai_teks, "huruf_besar") == 0);
            for(int i = 0; teks[i]; i++) {
                teks[i] = is_upper ? toupper(teks[i]) : tolower(teks[i]);
            }
            return teks; // Teks sudah di-malloc, kita langsung kembalikan
        }

        // E. Transmutasi Basis Matriks (ke_hex, ke_oktal)
        if (strcmp(node->nilai_teks, "ke_hex") == 0) {
            char* angka_str = evaluasi_ekspresi(node->anak_anak[0], ram);
            int angka = atoi(angka_str); free(angka_str);
            char buffer[32]; snprintf(buffer, sizeof(buffer), "0x%X", angka);
            return strdup(buffer);
        }

        // --- TRANSMUTASI: KE BINER (0b...) ---
        else if (strcmp(node->nilai_teks, "ke_biner") == 0) {
            char* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            
            // 🔥 SIHIR PEMBACA HEX/DESIMAL OTOMATIS 🔥
            // Menggunakan strtol basis 0 agar bisa menelan "15" maupun "0xFF"
            long angka = 0;
            if (strncmp(arg, "0x", 2) == 0 || strncmp(arg, "0X", 2) == 0) {
                angka = strtol(arg, NULL, 16);
            } else {
                angka = strtol(arg, NULL, 10);
            }
            free(arg);

            char buffer[128];
            buffer[0] = '0';
            buffer[1] = 'b';
            int index = 2;
            
            if (angka == 0) {
                buffer[2] = '0';
                buffer[3] = '\0';
            } else {
                long temp = angka;
                int bits[64];
                int bit_count = 0;
                while(temp > 0) {
                    bits[bit_count++] = temp % 2;
                    temp /= 2;
                }
                for(int i = bit_count - 1; i >= 0; i--) {
                    buffer[index++] = bits[i] + '0';
                }
                buffer[index] = '\0';
            }
            return strdup(buffer);
        }

        // --- TRANSMUTASI: KE DESIMAL (Dari Hex/Biner ke Angka Biasa) ---
        else if (strcmp(node->nilai_teks, "ke_desimal") == 0) {
            char* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            long angka = 0;
            
            // Jika diawali 0b, baca sebagai biner (basis 2)
            if (strncmp(arg, "0b", 2) == 0 || strncmp(arg, "0B", 2) == 0) {
                angka = strtol(arg + 2, NULL, 2);
            } else {
                // Basis 0 otomatis membaca "0x..." sebagai heksadesimal
                angka = strtol(arg, NULL, 0); 
            }
            free(arg);

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%ld", angka);
            return strdup(buffer);
        }

        // --- TRANSMUTASI: KE KARAKTER (Sandi Angka -> Huruf) ---
        else if (strcmp(node->nilai_teks, "ke_karakter") == 0) {
            char* arg = evaluasi_ekspresi(node->anak_anak[0], ram);
            int ascii_val = atoi(arg);
            free(arg);
            
            char buffer[2];
            buffer[0] = (char)ascii_val;
            buffer[1] = '\0';
            return strdup(buffer);
        }

        // --- TRANSMUTASI: BULATKAN ---
        else if (strcmp(node->nilai_teks, "bulatkan") == 0) {
            char* arg_angka = evaluasi_ekspresi(node->anak_anak[0], ram);
            double angka = atof(arg_angka);
            free(arg_angka);

            int presisi = 0;
            // Gunakan jumlah_anak, BUKAN jumlah_argumen
            if (node->jumlah_anak > 1) {
                char* arg_digit = evaluasi_ekspresi(node->anak_anak[1], ram);
                presisi = atoi(arg_digit);
                free(arg_digit);
            }

            char buffer[64];
            if (presisi <= 0) {
                snprintf(buffer, sizeof(buffer), "%.0f", round(angka));
            } else {
                char format[16];
                snprintf(format, sizeof(format), "%%.%df", presisi); 
                snprintf(buffer, sizeof(buffer), format, angka);
            }
            return strdup(buffer);
        }

        if (strcmp(node->nilai_teks, "ke_oktal") == 0) {
            char* angka_str = evaluasi_ekspresi(node->anak_anak[0], ram);
            int angka = atoi(angka_str); free(angka_str);
            char buffer[32]; snprintf(buffer, sizeof(buffer), "0o%o", angka);
            return strdup(buffer);
        }

        // F. Transmutasi ASCII (ke_ascii, dari_ascii)
        if (strcmp(node->nilai_teks, "ke_ascii") == 0) {
            char* teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d", teks[0] != '\0' ? teks[0] : 0);
            free(teks);
            return strdup(buffer);
        }
        if (strcmp(node->nilai_teks, "dari_ascii") == 0 || strcmp(node->nilai_teks, "ke_karakter") == 0) {
            char* angka_str = evaluasi_ekspresi(node->anak_anak[0], ram);
            int kode = atoi(angka_str); free(angka_str);
            char buffer[2] = {(char)kode, '\0'};
            return strdup(buffer);
        }

        // --- TRANSMUTASI: COCOK POLA (TRUE REGEX) ---
        else if (strcmp(node->nilai_teks, "cocok") == 0) {
            // Pastikan argumen yang diberikan minimal 2 (teks dan pola)
            if (node->jumlah_anak < 2) {
                pemicu_kernel_panic(ram, "Pelanggaran Argumen: fungsi 'cocok_pola' butuh 2 parameter (teks, pola_regex)!");
                return strdup("0");
            }

            char* teks = evaluasi_ekspresi(node->anak_anak[0], ram);
            char* pola = evaluasi_ekspresi(node->anak_anak[1], ram);

            regex_t mesin_regex;
            char* hasil_akhir = "0"; // Default: 0 (Palsu / Tidak Cocok)

            // 1. Kompilasi Pola Regex (Gunakan REG_EXTENDED agar mirip standar modern)
            int status_kompilasi = regcomp(&mesin_regex, pola, REG_EXTENDED);
            
            if (status_kompilasi == 0) {
                // 2. Jika pola sah, jalankan eksekusi pencocokan
                int status_cocok = regexec(&mesin_regex, teks, 0, NULL, 0);
                
                if (status_cocok == 0) {
                    hasil_akhir = "1"; // Sah! Cocok mutlak!
                }
                
                // 3. Bersihkan memori mesin regex C
                regfree(&mesin_regex); 
            } else {
                pemicu_kernel_panic(ram, "🚨 KERNEL PANIC! Sintaks pola Regex yang diberikan tidak valid!");
            }

            free(teks);
            free(pola);
            return strdup(hasil_akhir);
        }

        // G. SIHIR TERTINGGI: evaluasi(teks) - RECURSIVE DYNAMIC EVALUATION
        if (strcmp(node->nilai_teks, "evaluasi") == 0) {
            char* teks_kode = evaluasi_ekspresi(node->anak_anak[0], ram);
            
            // 1. Panggil Pemindai (Lexer) khusus untuk teks ini
            TokenArray token_eval = enki_lexer(teks_kode);
            
            // 2. Panggil Pohon Logika (Parser)
            Parser parser_eval = inisialisasi_parser(token_eval);
            
            // 3. Baca hanya sebagai Ekspresi (bukan program utuh)
            ASTNode* ast_eval = parse_ekspresi(&parser_eval);
            
            // 4. Eksekusi hasilnya!
            char* hasil_eval = evaluasi_ekspresi(ast_eval, ram);
            
            // 5. Bersihkan sampah dimensi
            bebaskan_ast(ast_eval);
            bebaskan_token_array(&token_eval);
            free(teks_kode);
            
            return hasil_eval;
        }

        // H. Fungsi tanya(teks) - Meminta input dengan pesan kustom
        if (strcmp(node->nilai_teks, "tanya") == 0) {
            char* teks_prompt = evaluasi_ekspresi(node->anak_anak[0], ram);
            char* input = readline(teks_prompt);
            free(teks_prompt); // Bebaskan teks prompt dari RAM
            
            if (input) {
                if (*input) add_history(input); // Ingat ketikan
                return input;
            }
            return strdup("");
        }

        // --- SUNTIKAN BARU: TABLET OF DESTINIES: FUNGSI BISIK ---
    else if (node->jenis == AST_IDENTITAS && strcmp(node->nilai_teks, "bisik") == 0) {
        // 1. Jika ada argumen teks (seperti tanya), cetak teksnya dulu
        if (node->kiri) { 
            char* pesan = evaluasi_ekspresi(node->kiri, ram);
            printf("%s", pesan);
            fflush(stdout); // Paksa terminal mencetak tanpa menunggu Enter
            free(pesan);
        }
        
        char buffer[256];
        struct termios terminal_lama, terminal_baru;
        
        // 2. MATIKAN ECHO TERMINAL (Sihir Gaib)
        tcgetattr(STDIN_FILENO, &terminal_lama);
        terminal_baru = terminal_lama;
        terminal_baru.c_lflag &= ~(ECHO); // Hapus bendera ECHO
        tcsetattr(STDIN_FILENO, TCSANOW, &terminal_baru);
        
        // 3. Baca masukan dari keyboard
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0; // Bersihkan \n
        }
        
        // 4. NYALAKAN LAGI ECHO (Sangat penting agar terminal Linux Anda tidak rusak!)
        tcsetattr(STDIN_FILENO, TCSANOW, &terminal_lama);
        printf("\n"); // Turun baris secara manual karena user menekan enter secara gaib
        
        return strdup(buffer);
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
        
        // Jika di RAM juga tidak ada, BARU KITA PANIK!
        if (func_node == NULL) {
            char pesan_err[128];
            snprintf(pesan_err, sizeof(pesan_err), "Fungsi gaib '%s' tidak ditemukan!", node->nilai_teks);
            pemicu_kernel_panic(ram, pesan_err);
        }

        // 3. MENCIPTAKAN RAM SEMENTARA (LOCAL SCOPE DINAMIS)
        EnkiRAM ram_lokal = inisialisasi_ram();
        ram_lokal.butuh_anu_aktif = ram->butuh_anu_aktif;
        
        // Kloning memori global
        for (int i = 0; i < ram->jumlah; i++) {
            simpan_ke_ram(&ram_lokal, ram->kavling[i].nama, ram->kavling[i].nilai_teks);
            ram_lokal.kavling[ram_lokal.jumlah - 1].simpul_fungsi = ram->kavling[i].simpul_fungsi;
        }

        // 4. TRANSFER NILAI PARAMETER KE RAM LOKAL
        for (int i = 0; i < func_node->jumlah_anak && i < node->jumlah_anak; i++) {
            char* nilai_argumen = evaluasi_ekspresi(node->anak_anak[i], ram);
            simpan_ke_ram(&ram_lokal, func_node->anak_anak[i]->nilai_teks, nilai_argumen);
            free(nilai_argumen);
        }

        // 5. JALANKAN MESIN!
        eksekusi_program(func_node->blok_maka, &ram_lokal);

        // 6. AMBIL HASIL KEMBALIAN
        char* hasil_akhir = strdup(ram_lokal.status_pulang == 1 ? ram_lokal.nilai_kembalian : "KOSONG");

        // 7. HANCURKAN RAM SEMENTARA
        bebaskan_ram(&ram_lokal);

        return hasil_akhir;
    }

    return strdup(""); // Default return yang HALAL (berada di dalam fungsi)
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
    
    char* kiri = evaluasi_ekspresi(kondisi->kiri, ram);
    char* kanan = evaluasi_ekspresi(kondisi->kanan, ram);
    int hasil_sah = 0;

    double num_kiri = atof(kiri); double num_kanan = atof(kanan);

    if (strcmp(kondisi->pembanding, "==") == 0) hasil_sah = (strcmp(kiri, kanan) == 0);
    else if (strcmp(kondisi->pembanding, "!=") == 0) hasil_sah = (strcmp(kiri, kanan) != 0);
    else if (strcmp(kondisi->pembanding, ">") == 0) hasil_sah = (num_kiri > num_kanan);
    else if (strcmp(kondisi->pembanding, "<") == 0) hasil_sah = (num_kiri < num_kanan);
    else if (strcmp(kondisi->pembanding, ">=") == 0) hasil_sah = (num_kiri >= num_kanan);
    else if (strcmp(kondisi->pembanding, "<=") == 0) hasil_sah = (num_kiri <= num_kanan);

    // --- SUNTIKAN RADAR DEBUG ---
    // printf("[RADAR LOGIKA] Kiri: '%s' (%g) | Pembanding: '%s' | Kanan: '%s' (%g) | Hasil: %d\n", kiri, num_kiri, kondisi->pembanding, kanan, num_kanan, hasil_sah);

    free(kiri); free(kanan);
    return hasil_sah; 
}

// FUNGSI PENCATAT BUKU HARIAN (DIARY LOGGING)
void pemicu_kernel_panic(EnkiRAM* ram, const char* pesan) {
    // 1. Tentukan Mode Debug Terlebih Dahulu
    const char* mode_debug = "0";
    if (ram->butuh_anu_aktif == 1) {
        const char* val = baca_dari_ram(ram, "MODE_DEBUG");
        if (val) mode_debug = val;
    }

    // 2. Catat ke unul.diary dengan Waktu dan Detail RAM
    FILE *log = fopen("unul.diary", "a");
    if (log) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        
        fprintf(log, "=== [TABU DILANGGAR] ===\n");
        fprintf(log, "Waktu Kejadian : %04d-%02d-%02d %02d:%02d:%02d\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        fprintf(log, "Pesan Alam     : %s\n", pesan);
        
        // Catat isi memori jika sedang mode Debug (Untuk analisis post-mortem)
        if (strcmp(mode_debug, "1") == 0) {
            fprintf(log, "Status         : MODE DEBUG AKTIF\n");
            fprintf(log, "Isi RAM Saat Ini (%d variabel):\n", ram->jumlah);
            for (int i = 0; i < ram->jumlah; i++) {
                fprintf(log, "  -> %s = %s\n", ram->kavling[i].nama, ram->kavling[i].nilai_teks);
            }
        } else {
            fprintf(log, "Status         : MODE PRODUKSI (Minimal Log)\n");
        }
        
        fprintf(log, "========================\n\n");
        fclose(log);
    }

    // 3. Tampilkan log tambahan di terminal jika .anu (MODE_DEBUG=1) aktif
    if (strcmp(mode_debug, "1") == 0) {
        printf("[DEBUG-ANOMALI] Kernel Panic terdeteksi: %s\n", pesan);
    }

    // 4. CEK PERISAI HUKUM TABU (Try-Catch / setjmp)
    if (ram->dalam_mode_coba == 1) {
        // Bebaskan kotak error lama (jika ada) untuk mencegah memory leak
        if (ram->pesan_error_tabu) {
            free(ram->pesan_error_tabu);
        }
        
        // Cetak memori baru tanpa batasan ukuran
        ram->pesan_error_tabu = strdup(pesan);
        
        longjmp(ram->titik_kembali, 1); 
    }

    // 5. JIKA TIDAK ADA PERISAI, MATIKAN OS/PROGRAM!
    printf("🚨 KERNEL PANIC! %s\n", pesan);
    exit(1);
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
    FILE *file = fopen(".anu", "r");
    if (!file) {
        return; // Jika tidak ada, diam saja.
    }

    // --- TINGGALKAN JEJAK GAIB ---
    simpan_ke_ram(ram, "__STATUS_ANU__", "ADA");

    char baris[512];
    while (fgets(baris, sizeof(baris), file)) {
        char* teks = trim_spasi(baris);
        
        if (strlen(teks) == 0 || teks[0] == '#' || (teks[0] == '^' && teks[1] == '^')) continue;

        char* pemisah = strchr(teks, '=');
        if (pemisah) {
            *pemisah = '\0'; 
            char* kunci = trim_spasi(teks);
            char* nilai = trim_spasi(pemisah + 1);
            bersihkan_kutip(nilai); 
            simpan_ke_ram(ram, kunci, nilai);
        }
    }
    fclose(file);
    printf("🛡️ [SISTEM] Kitab rahasia .anu berhasil merasuk ke memori!\n");
}

// --- 3. EKSEKUSI NODE (MENJALANKAN PERINTAH) ---
void eksekusi_node(ASTNode* node, EnkiRAM* ram) {
    if (!node) return;
    
    // 1. Eksekusi KETIK (Output)
    if (node->jenis == AST_PERINTAH_KETIK) {
        char* hasil = evaluasi_ekspresi(node->kanan, ram);
        printf("%s\n", hasil);
        free(hasil); // Selalu bersihkan memori sementara
    }
    
    // 2. DEKLARASI TAKDIR (takdir.soft x = y ATAU takdir.soft dewa = {})
    else if (node->jenis == AST_DEKLARASI_TAKDIR) {
        char* nama = node->kiri->nilai_teks;
        int adalah_hard = (node->nilai_teks && strcmp(node->nilai_teks, "takdir.hard") == 0); // 🔥 Cek status hard

        // ... (Logika simpan untuk Objek atau Teks) ...
        
        // 🔥 SUNTIKAN: Kunci kavling di RAM setelah disimpan
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, nama) == 0) {
                ram->kavling[i].apakah_konstanta = adalah_hard;
                break;
            }
        }
        
        // 🔥 JIKA NILAINYA ADALAH WUJUD OBJEK {}
        if (node->kanan && node->kanan->jenis == AST_STRUKTUR_OBJEK) {
            simpan_ke_ram(ram, nama, "[Proses Penciptaan Objek]"); 
            
            // Penguncian Indeks Statis (Mencegah Dangling Pointer saat realloc)
            int id_target = -1;
            for (int i = 0; i < ram->jumlah; i++) {
                if (strcmp(ram->kavling[i].nama, nama) == 0) {
                    id_target = i;
                    break;
                }
            }
            
            if (id_target != -1) {
                ram->kavling[id_target].tipe = TIPE_OBJEK;               
                ram->kavling[id_target].anak_anak = ciptakan_ram_mini(); 
                
                EnkiRAM* ram_anak = ram->kavling[id_target].anak_anak;

                // Masukkan semua pasangan "kunci": nilai ke dalam RAM anak
                for (int i = 0; i < node->kanan->jumlah_anak; i++) {
                    ASTNode* pasangan = node->kanan->anak_anak[i];
                    if (!pasangan || !pasangan->pembanding) continue; // Pelindung Segfault

                    char* kunci = strdup(pasangan->pembanding); 
                    bersihkan_kutip(kunci); // Melepas kutipan gaya JSON
                    
                    char* nilai_anak = evaluasi_ekspresi(pasangan->kiri, ram);
                    if (nilai_anak) {
                        simpan_ke_ram(ram_anak, kunci, nilai_anak);
                        free(nilai_anak);
                    }
                    free(kunci);
                }
            }
        } 

        // 🔥 SUNTIKAN: JIKA INI ADALAH FUNGSI BALIKAN()
        else if (node->kanan && node->kanan->jenis == AST_PANGGILAN_FUNGSI && strcmp(node->kanan->nilai_teks, "balikan") == 0) {
            // Ambil nama variabel sumber dari argumen pertama balikan(sumber)
            KavlingMemori* sumber = temukan_atau_ciptakan_kavling(node->kanan->anak_anak[0], ram);
            
            simpan_ke_ram(ram, nama, sumber ? sumber->nilai_teks : "KOSONG");
            
            // Kunci target dan masukkan dimensi fotokopiannya
            for (int i = 0; i < ram->jumlah; i++) {
                if (strcmp(ram->kavling[i].nama, nama) == 0) {
                    ram->kavling[i].apakah_konstanta = adalah_hard;
                    if (sumber && sumber->tipe == TIPE_OBJEK) {
                        ram->kavling[i].tipe = TIPE_OBJEK;
                        ram->kavling[i].anak_anak = salin_ram_rekursif(sumber->anak_anak);
                    }
                    break;
                }
            }
        }

        // 💧 JIKA NILAINYA ADALAH TEKS/ANGKA BIASA
        else {
            char* nilai = evaluasi_ekspresi(node->kanan, ram);
            if (nilai) {
                simpan_ke_ram(ram, nama, nilai);
                
                // 🔥 SUNTIKAN: Set status hard jika keywordnya adalah 'takdir.hard'
                if (node->nilai_teks && strcmp(node->nilai_teks, "takdir.hard") == 0) {
                    for (int i = 0; i < ram->jumlah; i++) {
                        if (strcmp(ram->kavling[i].nama, nama) == 0) {
                            ram->kavling[i].apakah_konstanta = 1;
                            break;
                        }
                    }
                }
                free(nilai);
            }
        }
    }

    // 3. Eksekusi HUKUM KARMA (Percabangan)
    else if (node->jenis == AST_HUKUM_KARMA) {
        int sah = evaluasi_kondisi(node->syarat, ram);
        
        if (sah) {
            // Jika syarat terpenuhi, jalankan blok MAKA
            eksekusi_program(node->blok_maka, ram);
        } else if (node->blok_lain) {
            // Jika gagal, dan ada blok LAIN, jalankan blok LAIN
            eksekusi_program(node->blok_lain, ram);
        }
    }

    // 4. Eksekusi KONTROL & TATA KRAMA
    else if (node->jenis == AST_PERINTAH_PERGI) {
        // PERGI = Membunuh proses OS secara langsung dengan damai (Return Code 0)
        exit(0); 
    }

    // Terus = continue di bahasa pemrograman umum
    else if (node->jenis == AST_PERINTAH_TERUS) {
        ram->status_terus = 1; // Nyalakan alarm lompat!
        return;
    }

    // Henti = break di bahasa pemrograman umum
    else if (node->jenis == AST_PERINTAH_HENTI) {
        ram->status_henti = 1; // Nyalakan alarm hancurkan putaran!
        return;
    }

    else if (node->jenis == AST_DEKLARASI_DATANG) {
        // [Opsional] Bisa diam saja, atau cetak log rahasia
        // printf("[SISTEM] Pintu masuk alam semesta dibuka...\n");
    }
    // --- EVALUASI PRAGMA (ATURAN MESIN) ---
    else if (node->jenis == AST_PRAGMA_MEMORI) {
        if (strcmp(node->nilai_teks, "butuh .anu") == 0) {
            // Aktifkan flag pengawasan
            ram->butuh_anu_aktif = 1; 

            // Cek apakah file .anu benar-benar sudah masuk RAM
            const char* status = baca_dari_ram(ram, "__STATUS_ANU__");
            if (status == NULL) {
                pemicu_kernel_panic(ram, "Kitab ini mewajibkan file rahasia '.anu', tapi tidak ditemukan!");
            }
        }
    }

    // 5. Eksekusi HUKUM SIKLUS (Perulangan)
    else if (node->jenis == AST_HUKUM_SIKLUS) {
        char* nilai_batas = evaluasi_ekspresi(node->batas_loop, ram);
        int batas = atoi(nilai_batas);
        free(nilai_batas);

        for (int i = 0; i < batas; i++) {
            ram->status_terus = 0; // Matikan alarm sebelum putaran dimulai
            
            // Eksekusi isi blok
            eksekusi_program(node->blok_siklus, ram);
            
            // Jika ada sinyal HENTI (Break), hancurkan loop di sini (Opsional)
            if (ram->status_henti == 1) {
                ram->status_henti = 0;
                break;
            }
        }
    }

    // 6. Sowan (Pemanggilan Dimensi / Import)
    else if (node->jenis == AST_PERINTAH_SOWAN) {
        // 1. Dapatkan nama file mentah (entah dari teks "..." atau dari variabel)
        char* target_mentah = evaluasi_ekspresi(node->kiri, ram);
        char target_file[512];
        
        // [SIHIR IMBUHAN OTOMATIS] Jika user mengetik 'sowan aljabar', otomatis jadi 'aljabar.unll'
        if (strstr(target_mentah, ".") == NULL) {
            snprintf(target_file, sizeof(target_file), "%s.unll", target_mentah);
        } else {
            strncpy(target_file, target_mentah, sizeof(target_file));
        }

        // Tambahkan Log ke diary saat ada pustaka yang dirasukkan (Optional Debug)
        if (ram->butuh_anu_aktif) {
            FILE *log = fopen("unul.diary", "a");
            if (log) {
                fprintf(log, "[INFO] Memanggil kasta '%s' ke dalam memori.\n", target_file);
                fclose(log);
            }
        }
        
        // 2. RADAR LOKAL: Cari di folder tempat user berada saat ini
        FILE *file = fopen(target_file, "r");
        
        // 3. RADAR SISTEM: Jika di lokal tidak ada, cari di Pustaka OS LinuxDNC!
        if (!file) {
            char path_sistem[1024];
            
            // Simulasi 1: Folder 'lib/' yang berdampingan dengan program (untuk masa development)
            snprintf(path_sistem, sizeof(path_sistem), "lib/%s", target_file);
            file = fopen(path_sistem, "r");
            
            // Simulasi 2: Folder absolut OS (nanti jika sudah rilis di Arch via yay/apt)
            if (!file) {
                snprintf(path_sistem, sizeof(path_sistem), "/usr/lib/unul/%s", target_file);
                file = fopen(path_sistem, "r");
            }
            
            // 4. KIAMAT: Jika di seluruh sistem tidak ada, hancurkan program!
            if (!file) {
                char pesan_kiamat[1024];
                snprintf(pesan_kiamat, sizeof(pesan_kiamat), "Bencana Sowan! Kitab pustaka '%s' tidak ditemukan di lokal maupun direktori sistem OS.", target_file);
                pemicu_kernel_panic(ram, pesan_kiamat);
                free(target_mentah);
                return;
            }
        }
        
        // Baca seluruh isi pustaka ke dalam memori
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *kode_sowan = malloc(fsize + 1);
        fread(kode_sowan, 1, fsize, file);
        kode_sowan[fsize] = '\0';
        fclose(file);
        
        // [SIHIR REKURSIF] Panggil Lexer dan Parser untuk mengeksekusi isi pustaka ini!
        TokenArray token_list_sowan = enki_lexer(kode_sowan); 
        Parser parser_sowan = inisialisasi_parser(token_list_sowan); 
        ASTNode* ast_sowan = parse_program(&parser_sowan);
        
        // Eksekusi Pustaka! (Fungsi-fungsinya akan tersimpan abadi di dalam EnkiRAM)
        eksekusi_program(ast_sowan, ram); 
        
        // Bersihkan sampah memori
        // bebaskan_ast(ast_sowan); -> DIMATIKAN AGAR FUNGSI TIDAK HILANG (Mencegah SegFault)
        bebaskan_token_array(&token_list_sowan);
        free(kode_sowan);
        
        // KITA MEMBERSIHKAN TARGET MENTAH (Bukan target_file karena target_file bukan hasil malloc)
        free(target_mentah); 
    }

    // --- 8. PEMBEBASAN MEMORI (pasrah) ---
    else if (node->jenis == AST_PERINTAH_PASRAH) {
        // Nama variabel bisa berada di kiri (jika formatnya 'pasrah x')
        char* target_nama = NULL;
        if (node->kiri && node->kiri->jenis == AST_IDENTITAS) {
            target_nama = node->kiri->nilai_teks;
        } else if (node->nilai_teks) {
            target_nama = node->nilai_teks;
        }

        if (target_nama) {
            for (int i = 0; i < ram->jumlah; i++) {
                if (strcmp(ram->kavling[i].nama, target_nama) == 0) {
                    
                    // 🔥 CEK HUKUM: Variabel 'hard' abadi, tidak bisa dipasrahkan!
                    if (ram->kavling[i].apakah_konstanta) {
                        char pesan[256];
                        snprintf(pesan, sizeof(pesan), "🚨 KIAMAT! Variabel 'hard' (%s) abadi, tidak bisa di-pasrah-kan!", target_nama);
                        pemicu_kernel_panic(ram, pesan);
                        return;
                    }

                    // Hancurkan Isi Memorinya (Garbage Collection Manual)
                    if (ram->kavling[i].tipe == TIPE_TEKS && ram->kavling[i].nilai_teks) {
                        free(ram->kavling[i].nilai_teks);
                    } else if (ram->kavling[i].tipe == TIPE_OBJEK && ram->kavling[i].anak_anak) {
                        bebaskan_ram(ram->kavling[i].anak_anak);
                        free(ram->kavling[i].anak_anak);
                        ram->kavling[i].anak_anak = NULL;
                    }
                    
                    // Kembalikan wujudnya ke ketiadaan
                    ram->kavling[i].tipe = TIPE_TEKS;
                    ram->kavling[i].nilai_teks = strdup("KOSONG");
                    break;
                }
            }
        }
    }

    // --- 9. SIHIR BALIKAN (MESIN WAKTU) ---
    else if (node->jenis == AST_PERINTAH_BALIKAN) {
        KavlingMemori* target = temukan_atau_ciptakan_kavling(node->kiri, ram);
        KavlingMemori* induk = cari_induk_utama(node->kiri, ram); // 🔥 SUNTIKAN WAJIB
        
        // Kita mundur dari sejarah INDUK, bukan sejarah target!
        if (induk && target && induk->jumlah_riwayat > 0) {
            // 1. Mundur 1 langkah ke masa lalu
            induk->jumlah_riwayat--;
            JejakMasaLalu* masa_lalu = &induk->riwayat[induk->jumlah_riwayat];
            
            // 2. Hancurkan wujud masa kini DARI INDUK (Ganti Semesta)
            if (induk->tipe == TIPE_TEKS && induk->nilai_teks) free(induk->nilai_teks);
            else if (induk->tipe == TIPE_OBJEK && induk->anak_anak) {
                bebaskan_ram(induk->anak_anak); free(induk->anak_anak);
            }
            
            // 3. Bangkitkan wujud masa lalu KE INDUK
            induk->tipe = masa_lalu->tipe;
            induk->nilai_teks = masa_lalu->nilai_teks ? strdup(masa_lalu->nilai_teks) : NULL;
            induk->anak_anak = masa_lalu->anak_anak ? salin_ram_rekursif(masa_lalu->anak_anak) : NULL;
            
            // 4. Hapus salinan jejak
            if (masa_lalu->nilai_teks) free(masa_lalu->nilai_teks);
            if (masa_lalu->anak_anak) {
                bebaskan_ram(masa_lalu->anak_anak); free(masa_lalu->anak_anak);
            }
        }
    }

    // 7. HUKUM TABU (Try-Catch / setjmp)
    else if (node->jenis == AST_COBA_TABU) {
        int status_coba_lama = ram->dalam_mode_coba;
        
        // --- SUNTIKAN ANTI-TIMPA: Backup titik_kembali yang lama ---
        jmp_buf titik_kembali_lama;
        memcpy(titik_kembali_lama, ram->titik_kembali, sizeof(jmp_buf));
        // -------------------------------------------------------------

        ram->dalam_mode_coba = 1;
        
        // Tancapkan titik kordinat mesin waktu (Checkpoint)!
        int lontaran = setjmp(ram->titik_kembali);
        
        if (lontaran == 0) {
            // [STATUS NORMAL]
            if (node->kiri) eksekusi_program(node->kiri, ram); 
        } else {
            // [STATUS TERLONTAR (KERNEL PANIC)]
            
            // (Opsional) Masukkan pesan error ke dalam wadah variabel UNUL
            if (node->nilai_teks != NULL) {
                simpan_ke_ram(ram, node->nilai_teks, ram->pesan_error_tabu);
            }

            if (node->kanan) eksekusi_program(node->kanan, ram);
        }

        // BLOK TEBUS (FINALLY)
        // Ini akan selalu tereksekusi, baik sistem dalam status normal maupun terlontar
        if (node->blok_tebus) {
            eksekusi_program(node->blok_tebus, ram);
        }
        
        // --- RESTORE PERISAI & TITIK KEMBALI LAMA ---
        ram->dalam_mode_coba = status_coba_lama;
        memcpy(ram->titik_kembali, titik_kembali_lama, sizeof(jmp_buf));
        // --------------------------------------------
        
    }

    // --- PENANGKAP PENCIPTAAN FUNGSI ---
    if (node->jenis == AST_DEKLARASI_FUNGSI) {
        simpan_ke_ram(ram, node->nilai_teks, "<fungsi_kustom>");
        // Cari indeksnya dan tanamkan cetak birunya
        for (int i = 0; i < ram->jumlah; i++) {
            if (strcmp(ram->kavling[i].nama, node->nilai_teks) == 0) {
                ram->kavling[i].simpul_fungsi = node;
                break;
            }
        }
        return;
    }

    // --- PENANGKAP PERINTAH PULANG (RETURN) ---
    else if (node->jenis == AST_PULANG) {
        
        // Hancurkan memori nilai kembalian bekas sebelumnya (jika ada)
        if (ram->nilai_kembalian) {
            free(ram->nilai_kembalian);
            ram->nilai_kembalian = NULL;
        }

        if (node->kiri) {
            char* hasil = evaluasi_ekspresi(node->kiri, ram);
            if (hasil) {
                ram->nilai_kembalian = strdup(hasil); // Duplikasi tanpa batas
                free(hasil);
            }
        } else {
            // Jika pulang tanpa membawa apa-apa
            ram->nilai_kembalian = strdup("KOSONG");
        }
        
        ram->status_pulang = 1; 
        return;
    }

    // EKSEKUTOR MUTASI BEBAS
    // Memaksa mesin mengeksekusi x = y atau pemanggilan fungsi yang berdiri sendiri
    else if (node->jenis == AST_OPERASI_MATEMATIKA || node->jenis == AST_PANGGILAN_FUNGSI) {
        char* hasil_mutasi = evaluasi_ekspresi(node, ram);
        if (hasil_mutasi) free(hasil_mutasi); // Eksekusi lalu buang sisa teksnya (karena ini operasi diam)
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