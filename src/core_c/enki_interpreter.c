#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "enki_interpreter.h"

// =================================================================
// DAPUR MESIN INTERPRETER (EKSEKUTOR C)
// Jantung LinuxDNC yang memompa Pohon Logika menjadi kenyataan!
// =================================================================

// --- 1. MANAJEMEN RAM UTAMA (ENKI RAM) ---
EnkiRAM inisialisasi_ram() {
    EnkiRAM ram;
    ram.kapasitas = 64; // Ruang awal 64 variabel
    ram.jumlah = 0;
    ram.butuh_anu_aktif = 0; // <--- Mulai dengan kondisi tenang (off)
    ram.kavling = (KavlingMemori*)malloc(ram.kapasitas * sizeof(KavlingMemori));
    return ram;
}

void bebaskan_ram(EnkiRAM* ram) {
    for (int i = 0; i < ram->jumlah; i++) {
        if (ram->kavling[i].nama) free(ram->kavling[i].nama);
        if (ram->kavling[i].nilai_teks) free(ram->kavling[i].nilai_teks);
    }
    free(ram->kavling);
    ram->kavling = NULL;
    ram->jumlah = 0;
    ram->kapasitas = 0;
}

// Fungsi Internal: Menyimpan atau menimpa nilai di RAM
void simpan_ke_ram(EnkiRAM* ram, const char* nama, const char* nilai) {
    // 1. Cari apakah sudah ada di RAM (Timpa nilai lama)
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            free(ram->kavling[i].nilai_teks); // Buang ingatan lama
            ram->kavling[i].nilai_teks = strdup(nilai); // Masukkan ingatan baru
            return;
        }
    }
    
    // 2. Jika belum ada, buat kavling baru
    if (ram->jumlah >= ram->kapasitas) {
        ram->kapasitas *= 2;
        ram->kavling = (KavlingMemori*)realloc(ram->kavling, ram->kapasitas * sizeof(KavlingMemori));
    }
    
    ram->kavling[ram->jumlah].nama = strdup(nama);
    ram->kavling[ram->jumlah].nilai_teks = strdup(nilai);
    ram->jumlah++;
}

// Fungsi Internal: Membaca nilai dari RAM
const char* baca_dari_ram(EnkiRAM* ram, const char* nama) {
    for (int i = 0; i < ram->jumlah; i++) {
        if (strcmp(ram->kavling[i].nama, nama) == 0) {
            return ram->kavling[i].nilai_teks;
        }
    }
    return NULL; 
}

// Fungsi Bantuan: Menghapus kutip ganda/tunggal di ujung teks (seperti .strip() di Python)
void bersihkan_kutip(char* teks) {
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
    
    // 3. Fungsi Dengar (Input)
    if (node->jenis == AST_FUNGSI_DENGAR) {
        char buffer[1024] = {0};
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = '\0';
            return strdup(buffer);
        }
        return strdup("");
    }

    // 4. Operasi Matematika
    if (node->jenis == AST_OPERASI_MATEMATIKA) {
        char* hasil_kiri = evaluasi_ekspresi(node->kiri, ram);
        char* hasil_kanan = evaluasi_ekspresi(node->kanan, ram);
        char* hasil_akhir = (char*)malloc(1024);
        memset(hasil_akhir, 0, 1024);

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
                    pemicu_kernel_panic(ram, "Kehancuran Dimensi: Pembagian dengan nol (0) dilarang oleh Hukum Enlil!");
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

    return strdup(""); // Default return yang HALAL (berada di dalam fungsi)
}

// Fungsi Internal: Mengevaluasi Syarat Hukum Karma
int evaluasi_kondisi(ASTNode* kondisi, EnkiRAM* ram) {
    if (!kondisi || !kondisi->pembanding) return 0;
    
    char* kiri = evaluasi_ekspresi(kondisi->kiri, ram);
    char* kanan = evaluasi_ekspresi(kondisi->kanan, ram);
    int hasil_sah = 0;

    double num_kiri = atof(kiri);
    double num_kanan = atof(kanan);

    // Evaluasi Angka vs Angka dan Teks vs Teks
    if (strcmp(kondisi->pembanding, "==") == 0) hasil_sah = (strcmp(kiri, kanan) == 0);
    else if (strcmp(kondisi->pembanding, "!=") == 0) hasil_sah = (strcmp(kiri, kanan) != 0);
    else if (strcmp(kondisi->pembanding, ">") == 0) hasil_sah = (num_kiri > num_kanan);
    else if (strcmp(kondisi->pembanding, "<") == 0) hasil_sah = (num_kiri < num_kanan);
    else if (strcmp(kondisi->pembanding, ">=") == 0) hasil_sah = (num_kiri >= num_kanan);
    else if (strcmp(kondisi->pembanding, "<=") == 0) hasil_sah = (num_kiri <= num_kanan);

    free(kiri); free(kanan);
    return hasil_sah; // 1 (True) atau 0 (False)
}

// FUNGSI PENCATAT BUKU HARIAN (DIARY LOGGING)
void pemicu_kernel_panic(EnkiRAM* ram, const char* pesan) {
    // 1. Deteksi Mode (Default 0 jika pragma butuh .anu tidak ada)
    const char* mode_debug = "0";
    if (ram->butuh_anu_aktif == 1) {
        const char* val = baca_dari_ram(ram, "MODE_DEBUG");
        if (val) mode_debug = val;
    }

    printf("🚨 KERNEL PANIC! %s\n", pesan);

    // 2. Tulis ke Diary dengan Gaya Arsitek
    FILE *log = fopen("enki_sistem.diary", "a");
    if (log) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        
        fprintf(log, "=== [TABU DILANGGAR] ===\n");
        fprintf(log, "Waktu Kejadian : %04d-%02d-%02d %02d:%02d:%02d\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        fprintf(log, "Pesan Alam     : %s\n", pesan);
        
        // --- JEJAK MEMORI (RAM DUMP) ---
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

        // --- PENYELAMATAN HUKUM TABU ---
        // Jika kita meledak di dalam blok 'coba', JANGAN MATI!
        if (ram->dalam_mode_coba == 1) {
            // 1. Simpan pesan error ke kotak P3K
            strncpy(ram->pesan_error_tabu, pesan, sizeof(ram->pesan_error_tabu) - 1);
        
            // 2. Lemparkan kesadaran kembali ke titik setjmp (bawa kode 1)
            longjmp(ram->titik_kembali, 1); 
        }

    // Jika di luar blok 'coba', biarkan OS meledak seperti biasa
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
    
    // 2. Eksekusi DEKLARASI TAKDIR (Input ke RAM)
    else if (node->jenis == AST_DEKLARASI_TAKDIR) {
        char* nama_variabel = node->kiri->nilai_teks;
        char* hasil_kanan = evaluasi_ekspresi(node->kanan, ram);
        
        simpan_ke_ram(ram, nama_variabel, hasil_kanan);
        free(hasil_kanan);
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
        int batas = atoi(nilai_batas); // Transmutasi teks/memori ke angka murni (Integer)
        free(nilai_batas); // Buang sampah agar memori aman

        // Perulangan ala C (Kecepatan Cahaya)
        for (int i = 0; i < batas; i++) {
            eksekusi_program(node->blok_siklus, ram);
        }
    }

    // 6. Sowan - ATM DARI BLUEPRINT: elif node['tipe'] == 'PERINTAH_SOWAN'
    else if (node->jenis == AST_PERINTAH_SOWAN) {
        char* target_file = strdup(node->nilai_teks);
        bersihkan_kutip(target_file); // Membuang tanda kutip " "
        
        // 1. with open(target_file, "r")
        FILE *file = fopen(target_file, "r");
        if (!file) {
            printf("🚨 Bencana Sowan! Kitab '%s' tidak ditemukan.\n", target_file);
            exit(1);
        }
        
        // Baca seluruh isi file ke memori (kode_sowan = f.read())
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *kode_sowan = malloc(fsize + 1);
        fread(kode_sowan, 1, fsize, file);
        kode_sowan[fsize] = '\0';
        fclose(file);
        
        // 2. Lexer -> Parser -> Eksekusi (Recursive Injection)
        TokenArray token_list_sowan = enki_lexer(kode_sowan); 
        Parser parser_sowan = inisialisasi_parser(token_list_sowan); 
        ASTNode* ast_sowan = parse_program(&parser_sowan);
        
        // 3. Gabungkan dan Eksekusi dengan RAM yang sama!
        eksekusi_program(ast_sowan, ram); 
        
        // 4. Bersihkan jejak setelah ilmu dari pustaka terserap ke RAM utama
        bebaskan_ast(ast_sowan);
        bebaskan_token_array(&token_list_sowan);
        free(kode_sowan);
        free(target_file);
    }

    // 7. HUKUM TABU (Try-Catch / setjmp)
    else if (node->jenis == AST_COBA_TABU) {
        // 1. Simpan state perisai sebelumnya (mendukung coba di dalam coba)
        int status_coba_lama = ram->dalam_mode_coba;
        
        // 2. Aktifkan perisai Hukum Tabu
        ram->dalam_mode_coba = 1;
        
        // 3. Tancapkan titik kordinat mesin waktu (Checkpoint)!
        int lontaran = setjmp(ram->titik_kembali);
        
        if (lontaran == 0) {
            // [STATUS NORMAL]
            // Mesin baru saja lewat sini, eksekusi blok 'coba' (kiri)
            if (node->kiri) {
                eksekusi_program(node->kiri, ram); // eksekusi_program karena isinya kumpulan perintah
            }
        } else {
            // [STATUS TERLONTAR]
            // Terjadi Kernel Panic! Mesin dilempar kembali ke sini membawa kode 'lontaran = 1'
            
            // Bersihkan error string yang tersangkut (Opsional: agar UNUL bisa baca alasannya)
            // simpan_ke_ram(ram, "ALASAN_TABU", ram->pesan_error_tabu);
            
            // Eksekusi blok 'tabu' (kanan) sebagai penebusan dosa
            if (node->kanan) {
                eksekusi_program(node->kanan, ram);
            }
        }
        
        // 4. Matikan perisai, kembalikan ke state semula
        ram->dalam_mode_coba = status_coba_lama;
    }
}

// --- 4. EKSEKUSI PROGRAM UTAMA ---
void eksekusi_program(ASTNode* program, EnkiRAM* ram) {
    if (!program || program->jenis != AST_PROGRAM) return;
    
    for (int i = 0; i < program->jumlah_anak; i++) {
        eksekusi_node(program->anak_anak[i], ram);
    }
}