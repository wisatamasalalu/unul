#ifndef SNUL_PARSER_H
#define SNUL_PARSER_H

#include "snul_lexer.h"
#include "../core/enki_object.h"

// Menyulap token SNUL menjadi Peta Memori (Kamus Gaya UI)
EnkiObject* parse_snul(SnulTokenArray tokens);

#endif