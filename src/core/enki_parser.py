import json
import os
from enki_lexer import enki_lexer

class EnkiParser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.ast = []

    def panggil_token(self):
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def makan_token(self, expected_kind):
        token = self.panggil_token()
        if token and token[0] == expected_kind:
            self.pos += 1
            return token
        else:
            raise SyntaxError(f"Hukum Enlil Dilanggar! Harusnya {expected_kind}, tapi ketemu {token}")

    def parse(self):
        while self.pos < len(self.tokens):
            token = self.panggil_token()
            
            # 1. Logika Deklarasi Takdir
            if token[0] == 'TAKDIR':
                self.ast.append(self.parse_takdir())
            
            # 2. Logika Perintah Ketik
            elif token[0] == 'FUNGSI' and token[1] == 'ketik':
                self.ast.append(self.parse_ketik())
            
            else:
                self.pos += 1 # Abaikan yang tidak dikenal untuk sementara
        return self.ast

    def parse_takdir(self):
        jenis_takdir = self.makan_token('TAKDIR')[1] # takdir.hard atau takdir.soft
        nama_var = self.makan_token('IDENTITAS')[1]
        self.makan_token('ASSIGN')
        
        # Ambil nilainya (bisa angka atau teks)
        token_nilai = self.panggil_token()
        if token_nilai[0] in ['TEKS', 'ANGKA']:
            nilai = self.makan_token(token_nilai[0])[1]
        
        # Simpan ke Pohon AST dengan metadata 'mutabilitas'
        return {
            'tipe': 'DEKLARASI_TAKDIR',
            'sifat': 'TETAP' if 'hard' in jenis_takdir else 'FLEKSIBEL',
            'nama': nama_var,
            'isi': nilai
        }

    def parse_ketik(self):
        self.makan_token('FUNGSI')
        self.makan_token('KURUNG_B')
        
        # Apa yang mau diketik? (Variabel atau Teks langsung)
        target_token = self.panggil_token()
        target = self.makan_token(target_token[0])[1]
        
        self.makan_token('KURUNG_T')
        return {
            'tipe': 'PERINTAH_KETIK',
            'target': target,
            'is_variable': target_token[0] == 'IDENTITAS'
        }

# --- BLOK PENGUJIAN PARSER ---
if __name__ == "__main__":
    file_path = os.path.join(os.path.dirname(__file__), '../../tests/aplikasi.unul')
    
    with open(file_path, "r") as f:
        kode_user = f.read()
        
    # 1. Masukkan ke Lexer
    tokens = enki_lexer(kode_user)
    
    # 2. Masukkan ke Parser
    parser = EnkiParser(tokens)
    pohon_logika = parser.parse()
    
    print("=== POHON LOGIKA AST (HUKUM ENLIL) ===")
    print(json.dumps(pohon_logika, indent=4))