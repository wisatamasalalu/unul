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
            
            # 5. Fungsi untuk membuat fungsi sendiri
            elif token[0] == 'PENCIPTAAN' and token[1] == 'ciptakan':
                self.ast.append(self.parse_ciptaan())

            elif token[0] == 'IDENTITAS': # Jika ada kata bebas, asumsikan itu panggilan fungsi
                self.ast.append(self.parse_panggilan_fungsi())
            
            else:
                self.pos += 1 # Abaikan yang tidak dikenal untuk sementara
        return self.ast

    def parse_takdir(self):
        jenis_takdir = self.makan_token('TAKDIR')[1] 
        nama_var = self.makan_token('IDENTITAS')[1]
        
        # --- OMNIDISCIPLINARY: CEK ARRAY STATIS ---
        ukuran_array = None
        token_cek = self.panggil_token()
        if token_cek and token_cek[0] == 'KURUNG_S_B':
            self.makan_token('KURUNG_S_B')
            ukuran_array = self.makan_token('ANGKA')[1]
            self.makan_token('KURUNG_S_T')
        # ------------------------------------------
        
        self.makan_token('ASSIGN')
        
        # Delegasikan pembacaan nilai ke parse_ekspresi
        nilai = self.parse_ekspresi()
        
        return {
            'tipe': 'DEKLARASI_TAKDIR',
            'sifat': 'TETAP' if 'hard' in jenis_takdir else 'FLEKSIBEL',
            'nama': nama_var,
            'ukuran': ukuran_array, # Simpan ukuran kavling RAM (jika ada)
            'isi': nilai
        }

    def parse_ekspresi(self):
        token_kiri = self.panggil_token()
        
        # --- CEK APAKAH INI ARRAY (KOTAK DATA) ---
        if token_kiri and token_kiri[0] == 'KURUNG_S_B':
            self.makan_token('KURUNG_S_B')
            elemen_array = []
            
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] != 'KURUNG_S_T':
                while True:
                    elemen = self.parse_ekspresi() # Rekursif canggih
                    elemen_array.append(elemen)
                    
                    token_koma = self.panggil_token()
                    if token_koma and token_koma[0] == 'KOMA':
                        self.makan_token('KOMA')
                    else:
                        break # Berhenti jika tidak ada koma
            
            self.makan_token('KURUNG_S_T')
            kiri = {
                'tipe': 'STRUKTUR_ARRAY',
                'isi': elemen_array
            }
        # -----------------------------------------
        # Cek apakah ini Fungsi Dengar
        elif token_kiri and token_kiri[0] == 'FUNGSI' and token_kiri[1] == 'dengar':
            self.makan_token('FUNGSI')
            self.makan_token('KURUNG_B')
            self.makan_token('KURUNG_T')
            kiri = {'tipe': 'FUNGSI_DENGAR'}
        # Jika bukan, berarti Teks/Angka/Variabel
        elif token_kiri and token_kiri[0] in ['TEKS', 'ANGKA', 'IDENTITAS']:
            kiri = self.makan_token(token_kiri[0])[1]
        else:
            raise SyntaxError(f"Hukum Enlil Dilanggar! Nilai tidak valid: {token_kiri}")

        # Looping untuk matematika berderet
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] == 'OPERATOR':
                operator = self.makan_token('OPERATOR')[1]
                
                token_kanan = self.panggil_token()
                if token_kanan and token_kanan[0] in ['ANGKA', 'IDENTITAS']:
                    kanan = self.makan_token(token_kanan[0])[1]
                    
                    kiri = {
                        'tipe': 'OPERASI_MATEMATIKA',
                        'kiri': kiri,
                        'operator': operator,
                        'kanan': kanan
                    }
                else:
                    raise SyntaxError("Hukum Enlil Dilanggar! Angka/Variabel selanjutnya hilang.")
            else:
                break # Keluar dari loop jika bukan operator
                
        return kiri  
        
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
        self.makan_token('KARMA') # jika
        kiri = self.makan_token('IDENTITAS')[1]
        pembanding = self.makan_token('PEMBANDING')[1]
        token_kanan = self.panggil_token()
        kanan = self.makan_token(token_kanan[0])[1]
        self.makan_token('KARMA') # maka
        
        aksi = []
        # Baca semua perintah sampai ketemu 'putus'
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] == 'KARMA' and token_cek[1] == 'putus':
                break # Rantai karma diputus
                
            token_aksi = self.panggil_token()
            if token_aksi[0] == 'FUNGSI' and token_aksi[1] == 'ketik':
                aksi.append(self.parse_ketik())
            else:
                raise SyntaxError(f"Hakim Enlil bingung dengan aksi ini di dalam Karma: {token_aksi}")
                
        self.makan_token('KARMA') # Makan kata 'putus'
        
        return {
            'tipe': 'HUKUM_KARMA',
            'kiri': kiri,
            'pembanding': pembanding,
            'kanan': kanan,
            'aksi': aksi
        }

    def parse_siklus(self):
        self.makan_token('SIKLUS') # effort
        token_jumlah = self.panggil_token()
        if token_jumlah and token_jumlah[0] in ['ANGKA', 'IDENTITAS']:
            jumlah = self.makan_token(token_jumlah[0])[1]
        else:
            raise SyntaxError("Hukum Enlil Dilanggar! Butuh angka/takdir untuk effort.")
            
        self.makan_token('SIKLUS') # kali
        self.makan_token('KARMA')  # maka
        
        aksi = []
        # Baca semua perintah sampai ketemu 'putus'
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] == 'KARMA' and token_cek[1] == 'putus':
                break # Siklus diputus
                
            token_aksi = self.panggil_token()
            if token_aksi[0] == 'FUNGSI' and token_aksi[1] == 'ketik':
                aksi.append(self.parse_ketik())
            else:
                raise SyntaxError(f"Hakim Enlil bingung dengan aksi ini di dalam Siklus: {token_aksi}")
                
        self.makan_token('KARMA') # Makan kata 'putus'
        
        return {
            'tipe': 'HUKUM_SIKLUS',
            'jumlah': jumlah,
            'aksi': aksi
        }

    def parse_ciptaan(self):
        self.makan_token('PENCIPTAAN') # makan 'ciptakan'
        self.makan_token('PENCIPTAAN') # makan 'fungsi'
        nama_fungsi = self.makan_token('IDENTITAS')[1]
        
        self.makan_token('KURUNG_B')
        parameter = []
        token_cek = self.panggil_token()
        if token_cek and token_cek[0] == 'IDENTITAS':
            while True:
                param_nama = self.makan_token('IDENTITAS')[1]
                parameter.append(param_nama)
                if self.panggil_token() and self.panggil_token()[0] == 'KOMA':
                    self.makan_token('KOMA')
                else:
                    break
        self.makan_token('KURUNG_T')
        self.makan_token('KARMA') # makan 'maka'
        
        aksi = []
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] == 'KARMA' and token_cek[1] == 'putus':
                break
                
            if token_cek[0] == 'FUNGSI' and token_cek[1] == 'ketik':
                aksi.append(self.parse_ketik())
            elif token_cek[0] == 'IDENTITAS':
                aksi.append(self.parse_panggilan_fungsi())
            else:
                self.pos += 1 # Abaikan token asing/komentar
                
        self.makan_token('KARMA') # makan 'putus'
        
        return {
            'tipe': 'DEKLARASI_FUNGSI',
            'nama': nama_fungsi,
            'parameter': parameter,
            'aksi': aksi
        }

    def parse_panggilan_fungsi(self):
        nama_fungsi = self.makan_token('IDENTITAS')[1]
        self.makan_token('KURUNG_B')
        
        argumen = []
        token_cek = self.panggil_token()
        if token_cek and token_cek[0] != 'KURUNG_T':
            while True:
                argumen.append(self.parse_ekspresi())
                if self.panggil_token() and self.panggil_token()[0] == 'KOMA':
                    self.makan_token('KOMA')
                else:
                    break
        self.makan_token('KURUNG_T')
        
        return {
            'tipe': 'PANGGILAN_FUNGSI',
            'nama': nama_fungsi,
            'argumen': argumen
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