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
        # --- PERBAIKAN: OPERASI MATEMATIKA LANJUTAN & PENGGABUNGAN TEKS ---
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'OPERASI_MATEMATIKA':
            kiri = self.evaluasi_nilai(nilai_mentah['kiri'])
            kanan = self.evaluasi_nilai(nilai_mentah['kanan'])
            op = nilai_mentah['operator']
            
            try:
                # Cerdas mendeteksi angka bulat atau desimal
                def jadikan_angka(n):
                    if isinstance(n, str):
                        n = n.strip('"\'')
                        # Keajaiban Python: Deteksi otomatis awalan 0x, 0b, 0o
                        if n.startswith(('0x', '0X', '0b', '0B', '0o', '0O')):
                            return int(n, 0) 
                    num = float(n)
                    return int(num) if num.is_integer() else num
                
                kiri_num = jadikan_angka(kiri)
                kanan_num = jadikan_angka(kanan)
                
                if op == '+': hasil_math = kiri_num + kanan_num
                elif op == '-': hasil_math = kiri_num - kanan_num
                elif op == '*': hasil_math = kiri_num * kanan_num
                elif op == '/': hasil_math = kiri_num / kanan_num # Pembagian kini bisa menghasilkan desimal!
                elif op == '%': hasil_math = kiri_num % kanan_num  
                elif op == '^': hasil_math = kiri_num ** kanan_num 
                
                # Kembalikan sebagai teks, buang .0 jika ternyata hasilnya bulat
                return str(int(hasil_math)) if isinstance(hasil_math, float) and hasil_math.is_integer() else str(hasil_math)
                
            except ValueError:
                # Jika gagal diubah jadi angka, berarti salah satunya adalah TEKS!
                if op == '+': 
                    return str(kiri) + str(kanan) # Gabungkan teksnya!
                else:
                    print(f"🚨 KERNEL PANIC! Tidak bisa menggunakan operator '{op}' pada teks/huruf!")
                    import sys; sys.exit(1)
        # ------------------------------------------------------------------

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
        
        # TAMBAHAN BARU: STRUKTUR OBJEK
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'STRUKTUR_OBJEK':
            hasil_objek = {}
            for kunci, nilai in nilai_mentah['isi'].items():
                hasil_objek[kunci] = self.evaluasi_nilai(nilai)
            return hasil_objek

        # --- PERBAIKAN: BACA INDEKS ARRAY & OBJEK ---
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'BACA_ARRAY':
            nama_var = nilai_mentah['nama']
            indeks_mentah = self.evaluasi_nilai(nilai_mentah['index'])
            
            if nama_var in self.memory:
                data_asli = self.memory[nama_var]['isi']
                
                # Jika dia Array [1, 2, 3]
                if isinstance(data_asli, list):
                    indeks = int(indeks_mentah)
                    if 0 <= indeks < len(data_asli): return str(data_asli[indeks])
                    else:
                        print(f"🚨 KERNEL PANIC! Indeks {indeks} melampaui batas kavling '{nama_var}'!"); import sys; sys.exit(1)
                
                # Jika dia Objek {"nama": "Unul"}
                elif isinstance(data_asli, dict):
                    kunci = str(indeks_mentah).strip('"\'')
                    if kunci in data_asli: return str(data_asli[kunci])
                    else:
                        print(f"🚨 KERNEL PANIC! Kunci '{kunci}' tidak ditemukan di dalam objek '{nama_var}'!"); import sys; sys.exit(1)
                
                else:
                    print(f"🚨 KERNEL PANIC! Takdir '{nama_var}' bukan array atau objek!"); import sys; sys.exit(1)
            else:
                print(f"🚨 KERNEL PANIC! Takdir '{nama_var}' tidak ditemukan!"); import sys; sys.exit(1)
        # --------------------------------------------

        # --- TAMBAHAN BARU: EKSEKUSI FUNGSI KUSTOM & BAWAAN (TABLET OF DESTINIES) ---
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'PANGGILAN_FUNGSI':
            nama = nilai_mentah['nama']
            args_evaluated = [self.evaluasi_nilai(a) for a in nilai_mentah['argumen']]

            # 1. Cek Fungsi Kustom (Ciptaan User)
            if hasattr(self, 'functions') and nama in self.functions:
                return self.eksekusi_fungsi_kustom(nama, args_evaluated)

            # 2. Pustaka Waktu
            elif nama == 'waktu_sekarang':
                import time
                return str(int(time.time()))

            # 3. Pustaka Matematika
            elif nama == 'acak':
                import random
                return str(random.randint(int(args_evaluated[0]), int(args_evaluated[1])))

            # 4. Pustaka Teks
            elif nama == 'panjang_teks':
                return str(len(str(args_evaluated[0])))
            elif nama == 'huruf_besar':
                return str(args_evaluated[0]).upper()
            elif nama == 'huruf_kecil':
                return str(args_evaluated[0]).lower()

            # 5. Protokol Utusan (Internet via GET)
            elif nama == 'ambil':
                import urllib.request
                url = str(args_evaluated[0]).strip('"\'')
                try:
                    # Menggunakan urllib bawaan agar tidak perlu install library eksternal
                    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'})
                    with urllib.request.urlopen(req) as respon:
                        teks_balasan = respon.read().decode('utf-8')
                        try:
                            import json
                            return json.loads(teks_balasan) # Otomatis disulap jadi Objek UNUL!
                        except json.JSONDecodeError:
                            return teks_balasan # Biarkan teks jika bukan JSON
                except Exception as e:
                    return f"🚨 Gagal Ambil Data dari Awan: {e}"

            elif nama == 'setor':
                import urllib.request
                import json
                
                url = str(args_evaluated[0]).strip('"\'')
                data_mentah = args_evaluated[1]
                
                try:
                    # Otomatis ubah Array UNUL jadi JSON
                    if isinstance(data_mentah, list) or isinstance(data_mentah, dict):
                        data_string = json.dumps(data_mentah)
                        headers = {'Content-Type': 'application/json', 'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/120.0.0.0'}
                    else:
                        data_string = str(data_mentah)
                        headers = {'Content-Type': 'text/plain', 'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/120.0.0.0'}
                        
                    data_bytes = data_string.encode('utf-8')
                    req = urllib.request.Request(url, data=data_bytes, headers=headers, method='POST')
                    
                    with urllib.request.urlopen(req) as respon:
                        teks_balasan = respon.read().decode('utf-8')
                        try:
                            import json
                            return json.loads(teks_balasan) # Otomatis disulap jadi Objek UNUL!
                        except json.JSONDecodeError:
                            return teks_balasan # Biarkan teks jika bukan JSON
                except Exception as e:
                    return f"🚨 Gagal Setor Data ke Awan: {e}"

            # --- PUSTAKA ANGKA & BASIS MESIN ---
            elif nama == 'bulatkan':
                angka = float(args_evaluated[0])
                # Jika user tidak menyebutkan jumlah digit, default ke 0
                digit = int(args_evaluated[1]) if len(args_evaluated) > 1 else 0
                hasil = round(angka, digit)
                return str(int(hasil)) if hasil.is_integer() else str(hasil)
            
            elif nama == 'ke_hex': return hex(int(args_evaluated[0]))
            elif nama == 'ke_biner': return bin(int(args_evaluated[0]))
            elif nama == 'ke_oktal': return oct(int(args_evaluated[0]))
            
            elif nama == 'ke_ascii': 
                teks = str(args_evaluated[0]).strip('"\'')
                return str(ord(teks[0])) if teks else "0"
            elif nama == 'dari_ascii': 
                return chr(int(args_evaluated[0]))
            # -----------------------------------

            # --- KEAJAIBAN BARU: FUNGSI EVALUASI (DYNAMIC EVAL) ---
            elif nama == 'evaluasi':
                teks_ekspresi = str(args_evaluated[0]).strip('"\'')
                
                # Kita panggil ulang Trinitas Enki khusus untuk teks ini
                from enki_lexer import enki_lexer
                from enki_parser import EnkiParser
                
                try:
                    # 1. Pecah teks menjadi token
                    sub_tokens = enki_lexer(teks_ekspresi)
                    # 2. Minta Parser membedah teks tersebut sebagai "Ekspresi"
                    sub_parser = EnkiParser(sub_tokens)
                    sub_ast_node = sub_parser.parse_ekspresi()
                    
                    # 3. Jalankan hasilnya menggunakan Interpreter yang sedang berjalan
                    return self.evaluasi_nilai(sub_ast_node)
                except Exception as e:
                    return f"🚨 Gagal Evaluasi: {e}"
            # ------------------------------------------------------

            # Jika tidak ada di mana-mana
            else:
                print(f"🚨 KERNEL PANIC! Fungsi '{nama}' tidak dikenal oleh alam semesta!")
                import sys; sys.exit(1)
        # ----------------------------------------------------------------------------
        return nilai_mentah

    def eksekusi_fungsi_kustom(self, nama_fungsi, args_evaluated):
        fungsi_node = self.functions[nama_fungsi]
        params = fungsi_node['parameter']
        backup = {}
        
        for i, param in enumerate(params):
            if param in self.memory: backup[param] = self.memory[param]
            # Parameter diberi riwayat kosong agar siap dipakai
            self.memory[param] = {'isi': args_evaluated[i], 'sifat': 'FLEKSIBEL', 'riwayat': []}
            
        nilai_pulang = None
        for aksi_node in fungsi_node['aksi']:
            hasil = self.eksekusi_node(aksi_node)
            # Jika menangkap sinyal PULANG, ambil nilainya dan hentikan blok fungsi
            if isinstance(hasil, dict) and hasil.get('aksi') == 'PULANG':
                nilai_pulang = hasil['nilai']
                break
            if hasil == "HENTI": break
            
        for param in params:
            if param in backup: self.memory[param] = backup[param]
            else: del self.memory[param]
            
        return nilai_pulang

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
            args_evaluated = [self.evaluasi_nilai(arg) for arg in node['argumen']]
            if nama_fungsi in self.functions:
                self.eksekusi_fungsi_kustom(nama_fungsi, args_evaluated)

        # ==========================================
        # FITUR BARU: PULANG & BALIKAN (MESIN WAKTU)
        # ==========================================
        elif node['tipe'] == 'PERINTAH_PULANG':
            nilai_final = self.evaluasi_nilai(node['nilai'])
            # Kirim sinyal berwujud Dictionary ke pemanggilnya
            return {'aksi': 'PULANG', 'nilai': nilai_final}
            
        elif node['tipe'] == 'PERINTAH_BALIKAN':
            target = node['target']
            if target in self.memory and len(self.memory[target].get('riwayat', [])) > 0:
                # Ambil ingatan masa lalu (pop), dan jadikan masa kini
                nilai_masa_lalu = self.memory[target]['riwayat'].pop()
                self.memory[target]['isi'] = nilai_masa_lalu
            else:
                print(f"🚨 Peringatan: Takdir '{target}' tidak memiliki masa lalu untuk dibalikkan!")

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
            # --- FUNGSI BANTUAN UNTUK MENGEVALUASI 1 SYARAT ---
            def evaluasi_syarat(k_mentah, pemb, kan_mentah):
                k_val = self.evaluasi_nilai(k_mentah)
                kan_val = self.evaluasi_nilai(kan_mentah)
                try:
                    # Ubah ke float agar bisa membandingkan 3.14 > 3
                    k_val, kan_val = float(k_val), float(kan_val)
                except ValueError:
                    k_val, kan_val = str(k_val).strip('"\''), str(kan_val).strip('"\'')
                
                if pemb == '>': return k_val > kan_val
                elif pemb == '<': return k_val < kan_val
                elif pemb == '==': return k_val == kan_val
                elif pemb == '!=': return k_val != kan_val
                elif pemb == '>=': return k_val >= kan_val
                elif pemb == '<=': return k_val <= kan_val
                return False

            # 1. Timbang Syarat Pertama
            sah1 = evaluasi_syarat(node['kiri'], node['pembanding'], node['kanan'])
            if node.get('pembalik1'): sah1 = not sah1 # Sihir Pembalik Bekerja!
            
            # 2. Timbang Syarat Kedua (Jika Ada Gerbang Logika)
            sah_final = sah1
            if node.get('logika'):
                sah2 = evaluasi_syarat(node['kiri2'], node['pembanding2'], node['kanan2'])
                if node.get('pembalik2'): sah2 = not sah2 # Sihir Pembalik Bekerja!
                
                logika = node['logika']
                if logika in ['dan', '&&']: sah_final = sah1 and sah2
                elif logika in ['atau', '||']: sah_final = sah1 or sah2
                                
            # 3. Eksekusi sesuai hasil timbangan
            if sah_final:
                # Jika syarat terpenuhi, jalankan blok 'maka'
                for aksi_node in node['aksi']:
                    hasil = self.eksekusi_node(aksi_node)
                    if hasil == "HENTI": return "HENTI"
            elif len(node.get('aksi_lain', [])) > 0:
                # Jika syarat GAGAL dan ada blok 'lain', jalankan alternatifnya!
                for aksi_node in node['aksi_lain']:
                    hasil = self.eksekusi_node(aksi_node)
                    if hasil == "HENTI": return "HENTI"

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