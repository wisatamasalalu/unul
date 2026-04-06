#ifndef OTIM_PARSER_H
#define OTIM_PARSER_H

#include "otim_lexer.h"
#include "../core/enki_object.h"

// Menyulap token OTIM menjadi Pohon Objek bersarang (UI Tree)
EnkiObject* parse_otim(OtimTokenArray tokens);

#endif