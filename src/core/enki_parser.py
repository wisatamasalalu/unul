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
            
            if token[0] == 'TAKDIR':
                self.ast.append(self.parse_takdir())
            elif token[0] == 'FUNGSI' and token[1] == 'ketik':
                self.ast.append(self.parse_ketik())
            elif token[0] == 'FUNGSI' and token[1] in ['tunggu', 'jeda']:
                self.ast.append(self.parse_waktu())
            elif token[0] == 'KONTROL':
                self.ast.append(self.parse_kontrol())
            elif token[0] == 'KARMA' and token[1] == 'jika':
                self.ast.append(self.parse_karma())
            elif token[0] == 'SIKLUS' and token[1] == 'effort':
                self.ast.append(self.parse_siklus())
            elif token[0] == 'PENCIPTAAN' and token[1] == 'ciptakan':
                self.ast.append(self.parse_ciptaan())
            elif token[0] == 'IDENTITAS':
                self.ast.append(self.parse_panggilan_fungsi())
            elif token[0] == 'SOWAN':
                self.ast.append(self.parse_sowan())
            else:
                self.pos += 1 
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

    # --- PINTU MASUK EKSPRESI (KASTA TERENDAH: PENJUMLAHAN) ---
    def parse_ekspresi(self):
        return self.parse_penjumlahan()

    def parse_penjumlahan(self):
        kiri = self.parse_perkalian()
        while self.pos < len(self.tokens):
            token = self.panggil_token()
            if token and token[0] == 'OPERATOR' and token[1] in ['+', '-']:
                op = self.makan_token('OPERATOR')[1]
                kanan = self.parse_perkalian() # Minta kasta perkalian untuk sisi kanan
                kiri = {'tipe': 'OPERASI_MATEMATIKA', 'kiri': kiri, 'operator': op, 'kanan': kanan}
            else: break
        return kiri

    def parse_perkalian(self):
        kiri = self.parse_pangkat()
        while self.pos < len(self.tokens):
            token = self.panggil_token()
            if token and token[0] == 'OPERATOR' and token[1] in ['*', '/', '%']:
                op = self.makan_token('OPERATOR')[1]
                kanan = self.parse_pangkat() # Minta kasta pangkat untuk sisi kanan
                kiri = {'tipe': 'OPERASI_MATEMATIKA', 'kiri': kiri, 'operator': op, 'kanan': kanan}
            else: break
        return kiri

    def parse_pangkat(self):
        kiri = self.parse_faktor()
        while self.pos < len(self.tokens):
            token = self.panggil_token()
            if token and token[0] == 'OPERATOR' and token[1] == '^':
                op = self.makan_token('OPERATOR')[1]
                kanan = self.parse_faktor() # Pangkat adalah kasta tertinggi sebelum nilai tunggal
                kiri = {'tipe': 'OPERASI_MATEMATIKA', 'kiri': kiri, 'operator': op, 'kanan': kanan}
            else: break
        return kiri

    # --- PONDASI: MEMBACA NILAI TUNGGAL (FAKTOR) ---
    def parse_faktor(self):
        token_kiri = self.panggil_token()
        
        # 1. Cek Objek / Wujud ({})
        if token_kiri and token_kiri[0] == 'KURUNG_K_B':
            self.makan_token('KURUNG_K_B')
            elemen_objek = {}
            if self.panggil_token() and self.panggil_token()[0] != 'KURUNG_K_T':
                while True:
                    token_key = self.panggil_token()
                    kunci = self.makan_token(token_key[0])[1].strip('"\'')
                    self.makan_token('TITIK_DUA')
                    elemen_objek[kunci] = self.parse_ekspresi()
                    if self.panggil_token() and self.panggil_token()[0] == 'KOMA': self.makan_token('KOMA')
                    else: break
            self.makan_token('KURUNG_K_T')
            return {'tipe': 'STRUKTUR_OBJEK', 'isi': elemen_objek}

        # 2. Cek Array ([])
        elif token_kiri and token_kiri[0] == 'KURUNG_S_B':
            self.makan_token('KURUNG_S_B')
            elemen_array = []
            if self.panggil_token() and self.panggil_token()[0] != 'KURUNG_S_T':
                while True:
                    elemen_array.append(self.parse_ekspresi())
                    if self.panggil_token() and self.panggil_token()[0] == 'KOMA': self.makan_token('KOMA')
                    else: break
            self.makan_token('KURUNG_S_T')
            return {'tipe': 'STRUKTUR_ARRAY', 'isi': elemen_array}

        # 3. Cek Fungsi Dengar
        elif token_kiri and token_kiri[0] == 'FUNGSI' and token_kiri[1] == 'dengar':
            self.makan_token('FUNGSI'); self.makan_token('KURUNG_B'); self.makan_token('KURUNG_T')
            return {'tipe': 'FUNGSI_DENGAR'}

        # 4. Teks atau Angka literal
        elif token_kiri and token_kiri[0] in ['TEKS', 'ANGKA']:
            return self.makan_token(token_kiri[0])[1]

        # --- KEAJAIBAN BARU: Ekspresi Matematika Dalam Kurung ---
        elif token_kiri and token_kiri[0] == 'KURUNG_B':
            self.makan_token('KURUNG_B')                 # Makan kurung buka '('
            nilai_dalam_kurung = self.parse_ekspresi()   # Hitung semua yang ada di dalamnya
            self.makan_token('KURUNG_T')                 # Makan kurung tutup ')'
            return nilai_dalam_kurung                    # Kembalikan hasilnya!

        # 5. Identitas (Variabel, Fungsi, Index Array)
        elif token_kiri and token_kiri[0] == 'IDENTITAS':
            nama_id = self.makan_token('IDENTITAS')[1]
            token_cek = self.panggil_token()
            
            if token_cek and token_cek[0] == 'KURUNG_B': # Panggilan Fungsi
                self.makan_token('KURUNG_B')
                argumen = []
                if self.panggil_token() and self.panggil_token()[0] != 'KURUNG_T':
                    while True:
                        argumen.append(self.parse_ekspresi())
                        if self.panggil_token() and self.panggil_token()[0] == 'KOMA': self.makan_token('KOMA')
                        else: break
                self.makan_token('KURUNG_T')
                return {'tipe': 'PANGGILAN_FUNGSI', 'nama': nama_id, 'argumen': argumen}
            elif token_cek and token_cek[0] == 'KURUNG_S_B': # Baca Index Array
                self.makan_token('KURUNG_S_B')
                index_ekspresi = self.parse_ekspresi()
                self.makan_token('KURUNG_S_T')
                return {'tipe': 'BACA_ARRAY', 'nama': nama_id, 'index': index_ekspresi}
            else:
                return nama_id

        raise SyntaxError(f"Hukum Enlil Dilanggar! Nilai tidak valid: {token_kiri}")  
        
    def parse_ketik(self):
        self.makan_token('FUNGSI')
        self.makan_token('KURUNG_B')
        
        # --- PERBAIKAN: Gunakan parse_ekspresi agar ketik() bisa membaca Array / Fungsi ---
        target_ekspresi = self.parse_ekspresi()
        
        self.makan_token('KURUNG_T')
        return {
            'tipe': 'PERINTAH_KETIK',
            'target': target_ekspresi
        }
    
    def parse_karma(self):
        self.makan_token('KARMA') # makan 'jika'
        
        # --- Syarat Pertama ---
        kiri = self.makan_token('IDENTITAS')[1]
        pembanding = self.makan_token('PEMBANDING')[1]
        token_kanan = self.panggil_token()
        kanan = self.makan_token(token_kanan[0])[1]
        
        # --- KEAJAIBAN BARU: Cek Gerbang Logika (Syarat Kedua) ---
        logika = None
        kiri2 = None
        pembanding2 = None
        kanan2 = None
        
        token_cek = self.panggil_token()
        if token_cek and token_cek[0] == 'LOGIKA':
            logika = self.makan_token('LOGIKA')[1] # Makan 'dan' / 'atau'
            kiri2 = self.makan_token('IDENTITAS')[1]
            pembanding2 = self.makan_token('PEMBANDING')[1]
            t_kanan2 = self.panggil_token()
            kanan2 = self.makan_token(t_kanan2[0])[1]
        # ---------------------------------------------------------

        self.makan_token('KARMA') # makan 'maka'
        
        aksi = []
        aksi_lain = [] # Siapkan wadah untuk takdir alternatif
        
        # 1. Baca blok utama (maka)
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            # Berhenti jika ketemu 'putus' ATAU 'lain'
            if token_cek and token_cek[0] == 'KARMA' and token_cek[1] in ['putus', 'lain']:
                break 

            if token_cek[0] == 'TAKDIR': aksi.append(self.parse_takdir())
            elif token_cek[0] == 'FUNGSI' and token_cek[1] == 'ketik': aksi.append(self.parse_ketik())
            elif token_cek[0] == 'FUNGSI' and token_cek[1] in ['tunggu', 'jeda']: aksi.append(self.parse_waktu())
            elif token_cek[0] == 'KONTROL': aksi.append(self.parse_kontrol())
            elif token_cek[0] == 'KARMA' and token_cek[1] == 'jika': aksi.append(self.parse_karma())
            elif token_cek[0] == 'SIKLUS' and token_cek[1] == 'effort': aksi.append(self.parse_siklus())
            elif token_cek[0] == 'IDENTITAS': aksi.append(self.parse_panggilan_fungsi())
            else: self.pos += 1
                
        token_penutup = self.makan_token('KARMA') # Makan 'putus' atau 'lain'
        
        # 2. Jika penutupnya adalah 'lain', baca blok alternatifnya
        if token_penutup[1] == 'lain':
            while self.pos < len(self.tokens):
                token_cek = self.panggil_token()
                if token_cek and token_cek[0] == 'KARMA' and token_cek[1] == 'putus':
                    break 
                
                if token_cek[0] == 'TAKDIR': aksi_lain.append(self.parse_takdir())
                elif token_cek[0] == 'FUNGSI' and token_cek[1] == 'ketik': aksi_lain.append(self.parse_ketik())
                elif token_cek[0] == 'FUNGSI' and token_cek[1] in ['tunggu', 'jeda']: aksi_lain.append(self.parse_waktu())
                elif token_cek[0] == 'KONTROL': aksi_lain.append(self.parse_kontrol())
                elif token_cek[0] == 'KARMA' and token_cek[1] == 'jika': aksi_lain.append(self.parse_karma())
                elif token_cek[0] == 'SIKLUS' and token_cek[1] == 'effort': aksi_lain.append(self.parse_siklus())
                elif token_cek[0] == 'IDENTITAS': aksi_lain.append(self.parse_panggilan_fungsi())
                else: self.pos += 1
            
            self.makan_token('KARMA') # Makan kata 'putus' yang sesungguhnya

        return {
            'tipe': 'HUKUM_KARMA', 
            'kiri': kiri, 'pembanding': pembanding, 'kanan': kanan,
            'logika': logika, 'kiri2': kiri2, 'pembanding2': pembanding2, 'kanan2': kanan2,
            'aksi': aksi,
            'aksi_lain': aksi_lain # Simpan blok alternatif ke AST
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
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] == 'KARMA' and token_cek[1] == 'putus':
                break 

            # HAKIM ENLIL SEKARANG BISA MEMBACA SEMUA PERINTAH DI DALAM BLOK EFFORT
            if token_cek[0] == 'TAKDIR': aksi.append(self.parse_takdir())
            elif token_cek[0] == 'FUNGSI' and token_cek[1] == 'ketik': aksi.append(self.parse_ketik())
            elif token_cek[0] == 'FUNGSI' and token_cek[1] in ['tunggu', 'jeda']: aksi.append(self.parse_waktu())
            elif token_cek[0] == 'KONTROL': aksi.append(self.parse_kontrol())
            elif token_cek[0] == 'KARMA' and token_cek[1] == 'jika': aksi.append(self.parse_karma())
            elif token_cek[0] == 'SIKLUS' and token_cek[1] == 'effort': aksi.append(self.parse_siklus())
            elif token_cek[0] == 'IDENTITAS': aksi.append(self.parse_panggilan_fungsi())
            else: self.pos += 1
                
        self.makan_token('KARMA') # Makan kata 'putus'
        
        return {
            'tipe': 'HUKUM_SIKLUS', 'jumlah': jumlah, 'aksi': aksi
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
                elif token_cek[0] == 'FUNGSI' and token_cek[1] in ['tunggu', 'jeda']:
                    aksi.append(self.parse_waktu())
                elif token_cek[0] == 'KONTROL':
                    aksi.append(self.parse_kontrol())
                else:
                    break
        self.makan_token('KURUNG_T')
        self.makan_token('KARMA') # makan 'maka'
        
        aksi = []
        while self.pos < len(self.tokens):
            token_cek = self.panggil_token()
            if token_cek and token_cek[0] == 'KARMA' and token_cek[1] == 'putus':
                break
                
            # HAKIM ENLIL SEKARANG BISA MEMBACA SEMUA PERINTAH DI DALAM FUNGSI KUSTOM
            if token_cek[0] == 'TAKDIR': aksi.append(self.parse_takdir())
            elif token_cek[0] == 'FUNGSI' and token_cek[1] == 'ketik': aksi.append(self.parse_ketik())
            elif token_cek[0] == 'FUNGSI' and token_cek[1] in ['tunggu', 'jeda']: aksi.append(self.parse_waktu())
            elif token_cek[0] == 'KONTROL': aksi.append(self.parse_kontrol())
            elif token_cek[0] == 'KARMA' and token_cek[1] == 'jika': aksi.append(self.parse_karma())
            elif token_cek[0] == 'SIKLUS' and token_cek[1] == 'effort': aksi.append(self.parse_siklus())
            elif token_cek[0] == 'IDENTITAS': aksi.append(self.parse_panggilan_fungsi())
            else: self.pos += 1 
                
        self.makan_token('KARMA') # makan 'putus'
        
        return {
            'tipe': 'DEKLARASI_FUNGSI', 'nama': nama_fungsi, 'parameter': parameter, 'aksi': aksi
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

    def parse_sowan(self):
        self.makan_token('SOWAN') # Makan kata 'sowan'
        
        # Target file biasanya berupa TEKS (contoh: "modul.unul")
        target_file = self.makan_token('TEKS')[1]
        
        return {
            'tipe': 'PERINTAH_SOWAN',
            'target': target_file
        }

    def parse_waktu(self):
        self.makan_token('FUNGSI') # makan tunggu/jeda
        self.makan_token('KURUNG_B')
        target_token = self.panggil_token()
        jumlah = self.makan_token(target_token[0])[1]
        self.makan_token('KURUNG_T')
        return {
            'tipe': 'PERINTAH_WAKTU',
            'jumlah': jumlah,
            'is_variable': target_token[0] == 'IDENTITAS'
        }

    def parse_kontrol(self):
        jenis = self.makan_token('KONTROL')[1]
        
        if jenis == 'henti': 
            return {'tipe': 'PERINTAH_HENTI'}
        elif jenis == 'pergi': 
            return {'tipe': 'PERINTAH_PERGI'}
        elif jenis == 'pulang':
            nilai = self.parse_ekspresi() # Baca nilai/ekspresi yang mau dipulangkan
            return {'tipe': 'PERINTAH_PULANG', 'nilai': nilai}
        elif jenis == 'balikan':
            target = self.makan_token('IDENTITAS')[1] # Baca variabel yang mau di-undo
            return {'tipe': 'PERINTAH_BALIKAN', 'target': target}

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