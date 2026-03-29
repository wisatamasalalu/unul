import re
import os

def enki_lexer(source_code):
    # Definisi Hukum Takdir & Perintah (Token)
    token_specification = [
        ('KOMENTAR',  r'\^\^.*'),              # Simbol ^^ untuk komentar
        ('TAKDIR',    r'takdir\.(hard|soft)'), # Mengenali takdir.hard atau takdir.soft
        ('FUNGSI',    r'\b(ketik|dengar)\b'),               # Perintah ketik (lowercase)
        ('TEKS',      r"('[^']*'|\"[^\"]*\")"), # Mengenali kutip ' atau "
        ('KARMA',     r'\b(jika|maka|putus)\b'), # Kata kunci takdir
        ('SIKLUS',    r'\b(effort|kali)\b'),
        ('PEMBANDING',r'==|!=|>=|<=|>|<'),         # Timbangan keadilan
        ('IDENTITAS', r'[a-zA-Z_][a-zA-Z0-9_]*'), # Nama variabel (namaku, umur)
        ('OPERATOR',  r'[+\-*/]'),             # Operator aritmatika
        ('ASSIGN',    r'='),                   # Sama dengan
        ('KURUNG_B',  r'\('),
        ('KURUNG_T',  r'\)'),
        ('KURUNG_S_B',r'\['),                  # Kurung Siku Buka
        ('KURUNG_S_T',r'\]'),                  # Kurung Siku Tutup
        ('KOMA',      r','),
        ('ANGKA',     r'\d+'),                 # Angka
        ('SPASI',     r'[ \t\n]+'),            # Abaikan spasi
        ('MISMATCH',  r'.'),                   # Ilegal
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