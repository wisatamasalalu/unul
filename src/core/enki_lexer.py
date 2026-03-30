import re
import os

def enki_lexer(source_code):
    # Definisi Hukum Takdir & Perintah (Token)
    # Definisi Hukum Takdir & Perintah (Token)
    token_specification = [
        ('KOMENTAR',  r'\^\^.*'),              # Simbol ^^ untuk komentar
        ('TAKDIR',    r'takdir\.(hard|soft)'), # Mengenali takdir.hard atau takdir.soft
        ('FUNGSI',    r'\b(ketik|dengar|tunggu|jeda)\b'),               
        ('TEKS',      r"('[^']*'|\"[^\"]*\")"), 
        ('KARMA',     r'\b(jika|maka|putus|lain)\b'), 
        ('SIKLUS',    r'\b(effort|kali)\b'), 
        ('PENCIPTAAN',r'\b(ciptakan|fungsi)\b'), 
        ('KONTROL',   r'\b(henti|pergi|pulang|balikan)\b'),
        ('SOWAN',     r'\bsowan\b'), 
        ('HEADER',    r'\bdatang\b'),
        ('PRAGMA',    r'\b(untuk\s+array\.(dinamis|statis)|butuh\s+\.anu)\b'),
        ('PEMBANDING',r'==|!=|>=|<=|>|<'),         
        
        # --- KEAJAIBAN BARU (OMNI-SYMBOLS) ---
        ('LOGIKA',    r'\b(dan|atau|bukan)\b|&&|\|\||!'), # Gerbang Logika (AND/OR/NOT)
        ('IDENTITAS', r'[a-zA-Z_][a-zA-Z0-9_]*'), 
        ('TITIK_DUA', r':'),                   # Simbol Titik Dua (Krusial untuk Objek/Dictionary JSON)
        ('TITIK',     r'\.'),                  # Simbol Titik (Krusial untuk akses properti, misal: bos.nama)
        ('OPERATOR',  r'[+\-*/%^]'),           # Ditambah Modulo (%) sisa bagi, dan Pangkat (^)
        # -------------------------------------

        ('ASSIGN',    r'='),                   
        ('KURUNG_B',  r'\('),
        ('KURUNG_T',  r'\)'),
        ('KURUNG_S_B',r'\['),                  
        ('KURUNG_S_T',r'\]'),
        
        # --- KEAJAIBAN BARU (OBJEK/DICTIONARY) ---
        ('KURUNG_K_B',r'\{'),                  # Kurung Kurawal Buka (Untuk Wujud/Objek)
        ('KURUNG_K_T',r'\}'),                  # Kurung Kurawal Tutup
        # -----------------------------------------

        ('KOMA',      r','),
        # Mendukung Hex (0x), Biner (0b), Oktal (0o), Desimal (3.14), dan Bulat (10)
        ('ANGKA',     r'0[xX][0-9a-fA-F]+|0[bB][01]+|0[oO][0-7]+|\d+(\.\d+)?'),
        ('SPASI',     r'[ \t\n]+'),            
        ('MISMATCH',  r'.'),                   
    ]
    
    tok_regex = '|'.join('(?P<%s>%s)' % pair for pair in token_specification)
    tokens = []
    
    for mo in re.finditer(tok_regex, source_code):
        kind = mo.lastgroup
        value = mo.group()
        if kind == 'SPASI' or kind == 'KOMENTAR':
            continue
        elif kind == 'MISMATCH':
            print(f"Hukum dilanggar! Karakter asing: {value}")
        else:
            tokens.append((kind, value))
    return tokens

# --- BLOK PENGUJIAN LEXER ---
if __name__ == "__main__":
    # Path relatif otomatis ke folder tests/
    file_path = os.path.join(os.path.dirname(__file__), '../../tests/aplikasi.unul')
    
    try:
        with open(file_path, "r") as f:
            kode = f.read()
            hasil = enki_lexer(kode)
            print("=== LOGIKA TAKDIR TERDETEKSI (LEXER) ===")
            for t in hasil:
                print(f"Bagian: {t[0]:<12} | Isi: {t[1]}")
    except FileNotFoundError:
        print(f"File {file_path} hilang dari peradaban!")