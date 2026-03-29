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

            # 3. Logika Hukum Karma (If)
            elif token[0] == 'KARMA' and token[1] == 'jika':
                self.ast.append(self.parse_karma())
            
            # 4. Logika Hukum Siklus (Looping / Effort)
            elif token[0] == 'SIKLUS' and token[1] == 'effort':
                self.ast.append(self.parse_siklus())
            
            else:
                self.pos += 1 # Abaikan yang tidak dikenal untuk sementara
        return self.ast

    def parse_takdir(self):
        jenis_takdir = self.makan_token('TAKDIR')[1] 
        nama_var = self.makan_token('IDENTITAS')[1]
        self.makan_token('ASSIGN')
        
        # 1. Ambil nilai kiri (bisa angka, teks, atau variabel lain)
        token_kiri = self.panggil_token()
        if token_kiri and token_kiri[0] == 'FUNGSI' and token_kiri[1] == 'dengar':
            self.makan_token('FUNGSI')
            self.makan_token('KURUNG_B')
            self.makan_token('KURUNG_T')
            kiri = {'tipe': 'FUNGSI_DENGAR'}
            nilai = kiri # Dengar tidak butuh operator matematika
        # -------------------------
        elif token_kiri and token_kiri[0] in ['TEKS', 'ANGKA', 'IDENTITAS']:
            kiri = self.makan_token(token_kiri[0])[1]
            
            # (Pindahkan logika pengecekan operator matematika ke dalam blok elif ini)
            # 2. Cek apakah ada operator matematika setelahnya
            token_selanjutnya = self.panggil_token()
            if token_selanjutnya and token_selanjutnya[0] == 'OPERATOR':
                operator = self.makan_token('OPERATOR')[1]
                
                token_kanan = self.panggil_token()
                if token_kanan and token_kanan[0] in ['ANGKA', 'IDENTITAS']:
                    kanan = self.makan_token(token_kanan[0])[1]
                else:
                    raise SyntaxError("Hukum Enlil Dilanggar! Angka/Variabel kedua tidak ditemukan.")
                
                nilai = {
                    'tipe': 'OPERASI_MATEMATIKA',
                    'kiri': kiri,
                    'operator': operator,
                    'kanan': kanan
                }
            else:
                nilai = kiri # Jika tidak ada operator
        else:
            raise SyntaxError(f"Hukum Enlil Dilanggar! Nilai tidak valid: {token_kiri}")
            
        return {
            'tipe': 'DEKLARASI_TAKDIR',
            'sifat': 'TETAP' if 'hard' in jenis_takdir else 'FLEKSIBEL',
            'nama': nama_var,
            'isi': nilai
        }
        
        # 2. Cek apakah ada operator matematika setelahnya
        token_selanjutnya = self.panggil_token()
        if token_selanjutnya and token_selanjutnya[0] == 'OPERATOR':
            operator = self.makan_token('OPERATOR')[1]
            
            token_kanan = self.panggil_token()
            if token_kanan and token_kanan[0] in ['ANGKA', 'IDENTITAS']:
                kanan = self.makan_token(token_kanan[0])[1]
            else:
                raise SyntaxError("Hukum Enlil Dilanggar! Angka/Variabel kedua tidak ditemukan.")
            
            # Kemas menjadi struktur operasi matematika
            nilai = {
                'tipe': 'OPERASI_MATEMATIKA',
                'kiri': kiri,
                'operator': operator,
                'kanan': kanan
            }
        else:
            nilai = kiri # Jika tidak ada operator, berarti nilai tunggal biasa
        
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
    
    def parse_karma(self):
        self.makan_token('KARMA') # Makan kata 'jika'
        
        # Siapa yang ditimbang? (Kiri, Pembanding, Kanan)
        kiri = self.makan_token('IDENTITAS')[1]
        pembanding = self.makan_token('PEMBANDING')[1]
        
        token_kanan = self.panggil_token()
        kanan = self.makan_token(token_kanan[0])[1]
        
        self.makan_token('KARMA') # Makan kata 'maka'
        
        # Apa aksinya jika syarat terpenuhi? (Sementara kita dukung 1 fungsi ketik dulu)
        aksi = []
        token_aksi = self.panggil_token()
        if token_aksi and token_aksi[0] == 'FUNGSI' and token_aksi[1] == 'ketik':
            aksi.append(self.parse_ketik())
            
        self.makan_token('KARMA') # Makan kata 'selesai'
        
        return {
            'tipe': 'HUKUM_KARMA',
            'kiri': kiri,
            'pembanding': pembanding,
            'kanan': kanan,
            'aksi': aksi
        }

    def parse_siklus(self):
        self.makan_token('SIKLUS') # Makan kata 'effort'
        
        # Berapa effort yang harus dikeluarkan? (Bisa angka langsung atau dari takdir/variabel)
        token_jumlah = self.panggil_token()
        if token_jumlah and token_jumlah[0] in ['ANGKA', 'IDENTITAS']:
            jumlah = self.makan_token(token_jumlah[0])[1]
        else:
            raise SyntaxError("Hukum Enlil Dilanggar! Butuh angka atau nama takdir untuk jumlah effort.")
            
        self.makan_token('SIKLUS') # Makan kata 'kali'
        self.makan_token('KARMA')  # Makan kata 'maka'
        
        # Apa aksinya? (Sementara fungsi ketik)
        aksi = []
        token_aksi = self.panggil_token()
        if token_aksi and token_aksi[0] == 'FUNGSI' and token_aksi[1] == 'ketik':
            aksi.append(self.parse_ketik())
            
        self.makan_token('KARMA') # Makan kata 'selesai'
        
        return {
            'tipe': 'HUKUM_SIKLUS',
            'jumlah': jumlah,
            'aksi': aksi
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