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
#include "enki_interpreter.h"

// =================================================================
// DAPUR MESIN INTERPRETER (EKSEKUTOR C)
// Jantung LinuxDNC yang memompa Pohon Logika menjadi kenyataan!
// =================================================================

// --- 1. MANAJEMEN RAM UTAMA (ENKI RAM) ---
EnkiRAM inisialisasi_ram() {
    EnkiRAM ram;
    
    // KEAJAIBAN MUTLAK C: Sapu bersih semua hantu memori dengan angka 0!
    memset(&ram, 0, sizeof(EnkiRAM)); 
    
    ram.kapasitas = 64; 
    // ram.jumlah dan ram.butuh_anu_aktif kini otomatis = 0 berkat memset
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
    ram->kavling[ram->jumlah].simpul_fungsi = NULL;
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
        char* input = readline("> ");
        if (input) {
            if (*input) add_history(input); // Ingat ketikan ini agar panah atas berfungsi
            return input; // readline menggunakan malloc, jadi aman langsung direturn
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
        strncpy(ram->pesan_error_tabu, pesan, sizeof(ram->pesan_error_tabu) - 1);
        ram->pesan_error_tabu[sizeof(ram->pesan_error_tabu) - 1] = '\0';
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
        if (node->kiri) {
            char* hasil = evaluasi_ekspresi(node->kiri, ram);
            if (hasil) {
                strncpy(ram->nilai_kembalian, hasil, sizeof(ram->nilai_kembalian) - 1);
                ram->nilai_kembalian[sizeof(ram->nilai_kembalian) - 1] = '\0';
                free(hasil);
            }
        } else {
            strcpy(ram->nilai_kembalian, "KOSONG");
        }
        ram->status_pulang = 1; 
        return;
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