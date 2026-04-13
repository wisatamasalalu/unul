#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "enki_scheduler.h"
#include "../core/enki_memory.h" // 🟢 MEMANGGIL DEWA MEMORI

// Helper Waktu
long hitung_jeda_ke_jadwal(const char* waktu_target) {
    time_t sekarang = time(NULL);
    struct tm* tm_lokal = localtime(&sekarang);
    
    int jam = 0, menit = 0, detik = 0;
    sscanf(waktu_target, "%d:%d:%d", &jam, &menit, &detik);
    
    struct tm tm_target = *tm_lokal;
    tm_target.tm_hour = jam;
    tm_target.tm_min = menit;
    tm_target.tm_sec = detik;
    
    time_t waktu_tujuan = mktime(&tm_target);
    if (waktu_tujuan <= sekarang) waktu_tujuan += 24 * 3600; // Jadwalkan besok jika sudah lewat
    
    return (long)difftime(waktu_tujuan, sekarang) * 1000;
}

long hitung_ms_interval(const char* teks) {
    int angka = atoi(teks);
    if (strstr(teks, "ms")) return angka;
    if (strstr(teks, "s")) return angka * 1000;
    if (strstr(teks, "m")) return angka * 60000;
    if (strstr(teks, "h")) return angka * 3600000;
    return angka; 
}

// Pelari Utas (Thread Runners)
void* pelari_jadwal(void* arg) {
    KapsulJadwal* kapsul = (KapsulJadwal*)arg;
    EnkiObject* target_obj = evaluasi_ekspresi(kapsul->simpul_waktu, kapsul->ram_paralel);
    char target_str[256] = ""; // Wadah statis yang aman
    
    if (target_obj && target_obj->tipe == ENKI_TEKS) {
        strcpy(target_str, target_obj->nilai.teks);
    }
    // Menggunakan variabel status_array_dinamis dari ram untuk menghancurkan (Hukum A)
    if (target_obj) hancurkan_objek(target_obj, kapsul->ram_paralel->status_array_dinamis);
    
    long jeda_ms = hitung_jeda_ke_jadwal(target_str);
    
    usleep(jeda_ms * 1000); // Hibernasi hingga waktu tiba
    eksekusi_program(kapsul->blok_eksekusi, kapsul->ram_paralel);
    
    bebaskan_ram(kapsul->ram_paralel);
    enki_bebas(kapsul->ram_paralel, 1); // 🟢 MENGGUNAKAN ENKI_BEBAS
    enki_bebas(kapsul, 1);              // 🟢 MENGGUNAKAN ENKI_BEBAS
    return NULL;
}

void* pelari_effort(void* arg) {
    KapsulJadwal* kapsul = (KapsulJadwal*)arg;
    EnkiObject* target_obj = evaluasi_ekspresi(kapsul->simpul_waktu, kapsul->ram_paralel);
    char interval_str[256] = ""; 
    
    if (target_obj && target_obj->tipe == ENKI_TEKS) {
        strcpy(interval_str, target_obj->nilai.teks);
    } else if (target_obj && target_obj->tipe == ENKI_ANGKA) {
        snprintf(interval_str, sizeof(interval_str), "%d", (int)target_obj->nilai.angka);
    }
    // Menggunakan variabel status_array_dinamis dari ram untuk menghancurkan (Hukum A)
    if (target_obj) hancurkan_objek(target_obj, kapsul->ram_paralel->status_array_dinamis);
    
    long jeda_ms = hitung_ms_interval(interval_str);
    
    while (1) {
        usleep(jeda_ms * 1000); // Hibernasi antar siklus
        eksekusi_program(kapsul->blok_eksekusi, kapsul->ram_paralel);
    }
    return NULL; // Secara logis ini berjalan abadi sampai OS mati
}

// Pintu Eksekusi dari Interpreter Utama
void eksekusi_jadwal_gaib(ASTNode* node, EnkiRAM* ram_utama) {
    pthread_t id_utas;
    KapsulJadwal* kapsul = (KapsulJadwal*)enki_alokasi(sizeof(KapsulJadwal), 1); // 🟢 MENGGUNAKAN ENKI_ALOKASI
    kapsul->simpul_waktu = node->kiri;
    kapsul->blok_eksekusi = node->blok_maka;
    kapsul->ram_paralel = salin_ram_untuk_utas(ram_utama);
    
    pthread_create(&id_utas, NULL, pelari_jadwal, kapsul);
    pthread_detach(id_utas);
}

void eksekusi_effort_gaib(ASTNode* node, EnkiRAM* ram_utama) {
    pthread_t id_utas;
    KapsulJadwal* kapsul = (KapsulJadwal*)enki_alokasi(sizeof(KapsulJadwal), 1); // 🟢 MENGGUNAKAN ENKI_ALOKASI
    kapsul->simpul_waktu = node->batas_loop;
    kapsul->blok_eksekusi = node->blok_siklus;
    kapsul->ram_paralel = salin_ram_untuk_utas(ram_utama);
    
    pthread_create(&id_utas, NULL, pelari_effort, kapsul);
    pthread_detach(id_utas);
}