#ifndef ENKI_SCHEDULER_H
#define ENKI_SCHEDULER_H

#include <pthread.h>
#include "enki_interpreter.h"

// Struktur Kapsul untuk dilempar ke dimensi paralel waktu
typedef struct {
    ASTNode* simpul_waktu;
    ASTNode* blok_eksekusi;
    EnkiRAM* ram_paralel;
} KapsulJadwal;

// Fungsi Publik untuk dieksekusi oleh Interpreter
void eksekusi_jadwal_gaib(ASTNode* node, EnkiRAM* ram_utama);
void eksekusi_effort_gaib(ASTNode* node, EnkiRAM* ram_utama);

#endif // ENKI_SCHEDULER_H