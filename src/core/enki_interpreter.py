from enki_lexer import enki_lexer
from enki_parser import EnkiParser

class EnkiInterpreter:
    def __init__(self, ast):
        self.ast = ast
        self.memory = {}
        self.functions = {}

    def evaluasi_nilai(self, nilai_mentah):
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'FUNGSI_DENGAR':
            # Gunakan fungsi input() bawaan Python untuk mendengarkan user
            jawaban = input("> ") 
            return jawaban
        # Jika ini adalah operasi matematika
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'OPERASI_MATEMATIKA':
            kiri = self.evaluasi_nilai(nilai_mentah['kiri'])
            kanan = self.evaluasi_nilai(nilai_mentah['kanan'])
            op = nilai_mentah['operator']
            
            # Ubah ke integer murni untuk dihitung komputer
            kiri_int = int(kiri)
            kanan_int = int(kanan)
            
            if op == '+': return str(kiri_int + kanan_int)
            if op == '-': return str(kiri_int - kanan_int)
            if op == '*': return str(kiri_int * kanan_int)
            if op == '/': return str(kiri_int // kanan_int) # Pembagian bulat

        # Jika ini adalah variabel (IDENTITAS), ambil isinya dari memori
        if isinstance(nilai_mentah, str) and nilai_mentah in self.memory:
            return self.memory[nilai_mentah]['isi']
        
        # Jika teks literal, bersihkan kutip
        if isinstance(nilai_mentah, str) and ((nilai_mentah.startswith('"') and nilai_mentah.endswith('"')) or (nilai_mentah.startswith("'") and nilai_mentah.endswith("'"))):
            return nilai_mentah[1:-1]

        # TAMBAHAN BARU: STRUKTUR ARRAY
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'STRUKTUR_ARRAY':
            hasil_array = []
            for elemen in nilai_mentah['isi']:
                hasil_array.append(self.evaluasi_nilai(elemen))
            return hasil_array
        
        # --- TAMBAHAN BARU: BACA INDEKS ARRAY SPESIFIK ---
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'BACA_ARRAY':
            nama_array = nilai_mentah['nama']
            indeks = int(self.evaluasi_nilai(nilai_mentah['index']))
            
            if nama_array in self.memory:
                array_asli = self.memory[nama_array]['isi']
                if isinstance(array_asli, list):
                    if 0 <= indeks < len(array_asli):
                        return str(array_asli[indeks])
                    else:
                        print(f"🚨 KERNEL PANIC! Indeks {indeks} melampaui batas kavling '{nama_array}'!")
                        import sys; sys.exit(1)
                else:
                    print(f"🚨 KERNEL PANIC! Takdir '{nama_array}' bukan sebuah array!")
                    import sys; sys.exit(1)
            else:
                print(f"🚨 KERNEL PANIC! Takdir '{nama_array}' tidak ditemukan!")
                import sys; sys.exit(1)
        # -------------------------------------------------
            
        return nilai_mentah

    def eksekusi_node(self, node):
        if node['tipe'] == 'DEKLARASI_TAKDIR':
            nama = node['nama']
            isi_final = self.evaluasi_nilai(node['isi'])
            ukuran_kavling = node.get('ukuran')

            # --- KEMBALIKAN FITUR: KERNEL PANIC ARRAY STATIS ---
            if ukuran_kavling is not None and isinstance(isi_final, list):
                if len(isi_final) > int(ukuran_kavling):
                    print(f"🚨 KERNEL PANIC! Array '{nama}' kelebihan muatan ! Dipesan {ukuran_kavling} kavling, tapi diisi {len(isi_final)} data!")
                    import sys; sys.exit(1)

            # --- MESIN WAKTU: SIMPAN RIWAYAT JIKA TAKDIR DIUBAH ---
            if nama in self.memory:
                if 'riwayat' not in self.memory[nama]: self.memory[nama]['riwayat'] = []
                self.memory[nama]['riwayat'].append(self.memory[nama]['isi'])
                self.memory[nama]['isi'] = isi_final
            else:
                self.memory[nama] = {'isi': isi_final, 'sifat': node['sifat'], 'ukuran': ukuran_kavling, 'riwayat': []}
            
        elif node['tipe'] == 'PERINTAH_KETIK':
            # --- PERBAIKAN: Langsung serahkan apapun isinya ke evaluasi_nilai ---
            hasil_ketik = self.evaluasi_nilai(node['target'])
            print(hasil_ketik)
                
        elif node['tipe'] == 'PERINTAH_SOWAN':
            target_file = node['target'].strip('"\'')
            try:
                with open(target_file, "r") as f: kode_sowan = f.read()
            except FileNotFoundError:
                print(f"🚨 Bencana Sowan! Kitab '{target_file}' tidak ditemukan."); import sys; sys.exit(1)
            tokens_sowan = enki_lexer(kode_sowan)
            parser_sowan = EnkiParser(tokens_sowan)
            ast_sowan = parser_sowan.parse()
            for node_sowan in ast_sowan: self.eksekusi_node(node_sowan)
                
        elif node['tipe'] == 'DEKLARASI_FUNGSI':
            self.functions[node['nama']] = node
            
        elif node['tipe'] == 'PANGGILAN_FUNGSI':
            nama_fungsi = node['nama']
            fungsi_node = self.functions[nama_fungsi]
            params = fungsi_node['parameter']
            args = node['argumen']
            args_evaluated = [self.evaluasi_nilai(arg) for arg in args]
            
            backup = {}
            for i, param in enumerate(params):
                if param in self.memory: backup[param] = self.memory[param]
                self.memory[param] = {'isi': args_evaluated[i], 'sifat': 'FLEKSIBEL'}
                
            for aksi_node in fungsi_node['aksi']:
                hasil = self.eksekusi_node(aksi_node)
                if hasil == "HENTI": break # Jika ada 'henti' di dalam fungsi
                
            for param in params:
                if param in backup: self.memory[param] = backup[param]
                else: del self.memory[param]

        # ==========================================
        # 3 FITUR BARU: JEDA, PERGI, HENTI
        # ==========================================
        elif node['tipe'] == 'PERINTAH_WAKTU':
            jumlah = self.memory[node['jumlah']]['isi'] if node['is_variable'] else self.evaluasi_nilai(node['jumlah'])
            import time
            time.sleep(int(jumlah))

        elif node['tipe'] == 'PERINTAH_PERGI':
            import sys
            sys.exit(0) # Program ditutup dengan damai

        elif node['tipe'] == 'PERINTAH_HENTI':
            return "HENTI" # Kirim sinyal ke atas untuk mendobrak siklus

        # ==========================================
        # RESTORE: HUKUM KARMA & SIKLUS YANG HILANG
        # ==========================================
        elif node['tipe'] == 'HUKUM_KARMA':
            kiri_val = self.evaluasi_nilai(node['kiri'])
            kanan_val = self.evaluasi_nilai(node['kanan'])
            pembanding = node['pembanding']
            
            # Konversi otomatis ke angka jika bentuknya angka, agar matematika akurat!
            try:
                kiri_val = int(kiri_val)
                kanan_val = int(kanan_val)
            except ValueError:
                pass # Biarkan berupa teks jika bukan angka (berguna untuk ngecek nama == "Nabhan")

            sah = False
            if pembanding == '>': sah = kiri_val > kanan_val
            elif pembanding == '<': sah = kiri_val < kanan_val
            elif pembanding == '==': sah = kiri_val == kanan_val
            elif pembanding == '!=': sah = kiri_val != kanan_val
            elif pembanding == '>=': sah = kiri_val >= kanan_val
            elif pembanding == '<=': sah = kiri_val <= kanan_val
            
            if sah:
                for aksi_node in node['aksi']:
                    hasil = self.eksekusi_node(aksi_node)
                    if hasil == "HENTI": return "HENTI" # Teruskan sinyal dobrak ke siklus

        elif node['tipe'] == 'HUKUM_SIKLUS':
            jumlah_int = int(self.evaluasi_nilai(node['jumlah']))
            for _ in range(jumlah_int):
                berhenti = False
                for aksi_node in node['aksi']:
                    hasil = self.eksekusi_node(aksi_node)
                    if hasil == "HENTI":
                        berhenti = True # Sinyal diterima, siklus didobrak!
                        break
                if berhenti: break

    # Fungsi utama yang dijalankan pertama kali
    def jalankan(self):
        for node in self.ast:
            self.eksekusi_node(node)

# --- BLOK EKSEKUSI UTAMA ---
if __name__ == "__main__":
    import sys
    import os
    
    # Cek apakah user memasukkan nama file di terminal
    if len(sys.argv) < 2:
        print("🚨 Peringatan: Kamu lupa memasukkan kitab takdir!")
        print("Cara pakai : python src/core/enki_interpreter.py <nama_file.unul>")
        sys.exit(1)
        
    file_path = sys.argv[1]
    
    try:
        with open(file_path, "r") as f:
            kode_user = f.read()
    except FileNotFoundError:
        print(f"🚨 Bencana! Kitab '{file_path}' tidak ditemukan di alam semesta.")
        sys.exit(1)
        
    # Proses Trinitas Enki
    tokens = enki_lexer(kode_user)
    parser = EnkiParser(tokens)
    
    try:
        pohon_logika = parser.parse()
        print(f"=== EKSEKUSI PROGRAM UNUL: {os.path.basename(file_path)} ===")
        interpreter = EnkiInterpreter(pohon_logika)
        interpreter.jalankan()
        print("====================================")
    except Exception as e:
        print(f"\n🔥 GAGAL EKSEKUSI: {e}")