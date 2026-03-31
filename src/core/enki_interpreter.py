from enki_lexer import enki_lexer
from enki_parser import EnkiParser
import copy
import sys
import os
import re  # Hukum Pola Sakti (Regex)
import json
import time
import random
import urllib.request
import copy
try:
    import readline
except ImportError:
    pass

class EnkiInterpreter:
    def __init__(self, ast):
        self.ast = ast
        self.memory = {}
        self.functions = {}
        self.mode_kompilasi = 'dinamis' # Default (Garbage Collected / .ko)
    
    # --- PEMBACA BERKAS RAHASIA (.anu) SEPERTI .env ---
    # --- KEAJAIBAN BARU: PEMBACA BERKAS RAHASIA (.anu) ---
    def muat_anu(self, path_file_utama):
        import os
        direktori = os.path.dirname(os.path.abspath(path_file_utama))
        path_anu = os.path.join(direktori, '.anu')

        # Cek apakah kode mewajibkan .anu
        wajib_anu = any(node.get('tipe') == 'PRAGMA_BUTUH_ANU' for node in self.ast)

        if os.path.exists(path_anu):
            with open(path_anu, 'r') as f:
                for baris in f:
                    baris = baris.strip()
                    if not baris or baris.startswith('^^'): continue
                    
                    if '=' in baris:
                        kunci, nilai = baris.split('=', 1)
                        kunci = kunci.strip()
                        nilai = nilai.strip()
                        
                        if (nilai.startswith('"') and nilai.endswith('"')) or (nilai.startswith("'") and nilai.endswith("'")):
                            nilai = nilai[1:-1]
                        else:
                            try:
                                if '.' in nilai: nilai = float(nilai)
                                else: nilai = int(nilai)
                            except ValueError:
                                pass 
                                
                        self.memory[kunci] = {'isi': nilai, 'sifat': 'TETAP', 'riwayat': []}
        else:
            if wajib_anu:
                raise RuntimeError(f"🚨 KERNEL PANIC! Kitab ini mewajibkan file rahasia '.anu', tapi tidak ditemukan di folder '{direktori}'!")
    # -----------------------------------------------------

    def evaluasi_nilai(self, nilai_mentah):
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'FUNGSI_DENGAR':
            # --- KEAJAIBAN BARU: Dukungan Navigasi Panah (Arrow Keys) ---
            try:
                import readline # Mengaktifkan navigasi kursor & riwayat di Linux/Mac
            except ImportError:
                pass # Abaikan jika di Windows, karena Windows CMD/PowerShell sudah otomatis bisa
                
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
                elif op == '/': 
                    if kanan_num == 0:
                        raise RuntimeError("🚨 KERNEL PANIC! Kehancuran Dimensi: Pembagian dengan nol (0) dilarang oleh Hukum Enlil!")
                    hasil_math = kiri_num / kanan_num 
                elif op == '%': 
                    if kanan_num == 0:
                        raise RuntimeError("🚨 KERNEL PANIC! Kehancuran Dimensi: Pembagian dengan nol (0) dilarang oleh Hukum Enlil!")
                    hasil_math = kiri_num % kanan_num  
                elif op == '^': hasil_math = kiri_num ** kanan_num
                
                # Kembalikan sebagai teks, buang .0 jika ternyata hasilnya bulat
                return str(int(hasil_math)) if isinstance(hasil_math, float) and hasil_math.is_integer() else str(hasil_math)
                
            except ValueError:
                # Jika gagal diubah jadi angka, berarti salah satunya adalah TEKS!
                if op == '+': 
                    return str(kiri) + str(kanan) # Gabungkan teksnya!
                else:
                    raise TypeError(f"🚨 KERNEL PANIC! Tidak bisa menggunakan operator '{op}' pada teks/huruf!")
        # ------------------------------------------------------------------

        # --- PERBAIKAN: BACA VARIABEL ATAU TEKS BEBAS (LOOSE TEXT MODE) ---
        if isinstance(nilai_mentah, str):
            # 1. Jika teks literal ("..."), bersihkan kutip
            if (nilai_mentah.startswith('"') and nilai_mentah.endswith('"')) or (nilai_mentah.startswith("'") and nilai_mentah.endswith("'")):
                return nilai_mentah[1:-1]
            # 2. Jika ada di memori RAM (Variabel Sah)
            elif nilai_mentah in self.memory:
                return self.memory[nilai_mentah]['isi']
            # 3. Jika Angka Mesin/Literal, biarkan
            else:
                try:
                    float(nilai_mentah)
                    return nilai_mentah
                except ValueError:
                    if nilai_mentah.startswith(('0x', '0X', '0b', '0B', '0o', '0O')):
                        # 🔥 KEAJAIBAN BARU: Terjemahkan ke basis Desimal murni!
                        return str(int(nilai_mentah, 0))
                    
                    # 🔥 KESAKTIAN ARSITEK: JIKA TIDAK ADA DI RAM, JADIKAN TEKS BIASA! 🔥
                    return nilai_mentah
        # -------------------------------------------------------------
        
        # --- PERBAIKAN: BACA PROPERTI TITIK (BOS.KITA.NAMA) ---
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'BACA_PROPERTI':
            rantai = nilai_mentah['rantai']
            nama_root = rantai[0]
            
            # Jika domain tidak ada, kembalikan sebagai teks bersambung (contoh: "kucing.lucu")
            if nama_root not in self.memory:
                return ".".join(rantai)
                
            current = self.memory[nama_root]['isi']
            for prop in rantai[1:]:
                if isinstance(current, dict) and prop in current:
                    current = current[prop]
                else:
                    return ".".join(rantai) # Fallback ke teks literal jika sub-domain kosong
            return current
        # -----------------------------------------------------------

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
            rantai = nilai_mentah['nama'] # Sekarang ini berupa List (contoh: ['pengguna'])
            nama_root = rantai[0]
            indeks_mentah = self.evaluasi_nilai(nilai_mentah['index'])
            
            if nama_root in self.memory:
                data_asli = self.memory[nama_root]['isi']
                
                # Menelusuri domain bertingkat jika ada (contoh: bos.karyawan[0])
                for prop in rantai[1:]:
                    if isinstance(data_asli, dict) and prop in data_asli:
                        data_asli = data_asli[prop]
                    else:
                        raise KeyError(f"🚨 KERNEL PANIC! Properti '{prop}' tidak ditemukan pada wujud '{nama_root}'!")
                
                # Jika dia Array [1, 2, 3]
                if isinstance(data_asli, list):
                    indeks = int(indeks_mentah)
                    if 0 <= indeks < len(data_asli): return str(data_asli[indeks])
                    else:
                        raise IndexError(f"🚨 KERNEL PANIC! Indeks {indeks} melampaui batas kavling '{'.'.join(rantai)}'!")
                
                # Jika dia Objek {"nama": "Unul"}
                elif isinstance(data_asli, dict):
                    kunci = str(indeks_mentah).strip('"\'')
                    if kunci in data_asli: return str(data_asli[kunci])
                    else:
                        raise KeyError(f"🚨 KERNEL PANIC! Kunci '{kunci}' tidak ditemukan di dalam objek '{'.'.join(rantai)}'!")
                
                else:
                    raise TypeError(f"🚨 KERNEL PANIC! Takdir '{'.'.join(rantai)}' bukan array atau objek!")
            else:
                raise NameError(f"🚨 KERNEL PANIC! Takdir '{nama_root}' tidak ditemukan!")
        # --------------------------------------------

        # --- EKSEKUSI FUNGSI KUSTOM & BAWAAN (TABLET OF DESTINIES) ---
        if isinstance(nilai_mentah, dict) and nilai_mentah.get('tipe') == 'PANGGILAN_FUNGSI':
            nama = nilai_mentah['nama']
            args_evaluated = [self.evaluasi_nilai(a) for a in nilai_mentah['argumen']]

            # 1. Cek Fungsi Kustom (Ciptaan User)
            if hasattr(self, 'functions') and nama in self.functions:
                return self.eksekusi_fungsi_kustom(nama, args_evaluated)

            # 2. Pustaka Interaksi & Input (WAJIB ADA)
            elif nama == 'tanya':
                prompt = str(args_evaluated[0]) if args_evaluated else "> "
                return input(prompt)

            # 3. Pustaka Waktu & Angka Acak
            elif nama == 'waktu_sekarang':
                import time
                return str(int(time.time()))
            elif nama == 'acak':
                import random
                return str(random.randint(int(args_evaluated[0]), int(args_evaluated[1])))

            # 4. MATRIKS TRANSMUTASI (Dec, Hex, Bin, Oct, ASCII) - BARU!
            elif nama in ['ke_desimal', 'ke_angka']:
                try: return str(int(str(args_evaluated[0]), 0))
                except: return "0"
            elif nama == 'ke_hex':
                try: return hex(int(str(args_evaluated[0]), 0))
                except: return "0x0"
            elif nama == 'ke_biner':
                try: return bin(int(str(args_evaluated[0]), 0))
                except: return "0b0"
            elif nama == 'ke_oktal':
                try: return oct(int(str(args_evaluated[0]), 0))
                except: return "0o0"
            elif nama == 'ke_ascii':
                teks = str(args_evaluated[0]).strip('"\'')
                return str(ord(teks[0])) if teks else "0"
            elif nama in ['dari_ascii', 'ke_karakter']:
                try: return chr(int(str(args_evaluated[0]), 0))
                except: return ""

            # 5. Pustaka Teks & Pola Murni (True Regex)
            elif nama in ['panjang_teks', 'panjang']:
                return str(len(str(args_evaluated[0])))
            elif nama == 'huruf_besar':
                return str(args_evaluated[0]).upper()
            elif nama == 'huruf_kecil':
                return str(args_evaluated[0]).lower()
            elif nama == 'cocok_pola':
                import re
                teks, pola = str(args_evaluated[0]), str(args_evaluated[1])
                try:
                    return "1" if re.search(pola, teks) else "0"
                except: return "0"

            # 6. Protokol Utusan (Internet via GET/POST)
            elif nama == 'ambil':
                import urllib.request, json
                url = str(args_evaluated[0]).strip('"\'')
                try:
                    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
                    with urllib.request.urlopen(req) as r:
                        resp = r.read().decode('utf-8')
                        try: return json.loads(resp)
                        except: return resp
                except Exception as e:
                    raise ConnectionError(f"🚨 KERNEL PANIC! Gagal Ambil data dari Awan: {e}")

            elif nama == 'setor':
                import urllib.request, json
                url, data_mentah = str(args_evaluated[0]).strip('"\''), args_evaluated[1]
                try:
                    data_string = json.dumps(data_mentah) if isinstance(data_mentah, (list, dict)) else str(data_mentah)
                    req = urllib.request.Request(url, data=data_string.encode(), method='POST', headers={'Content-Type': 'application/json', 'User-Agent': 'Mozilla/5.0'})
                    with urllib.request.urlopen(req) as r:
                        resp = r.read().decode('utf-8')
                        try: return json.loads(resp)
                        except: return resp
                except Exception as e:
                    raise ConnectionError(f"🚨 KERNEL PANIC! Gagal Setor data ke Awan: {e}")

            # --- PUSTAKA ANGKA & BASIS MESIN (TRANSMUTASI MATRIX) ---
            elif nama == 'bulatkan':
                angka = float(args_evaluated[0])
                digit = int(args_evaluated[1]) if len(args_evaluated) > 1 else 0
                hasil = round(angka, digit)
                return str(int(hasil)) if hasil.is_integer() else str(hasil)
                
            elif nama in ['ke_desimal', 'ke_angka']:
                try: return str(int(str(args_evaluated[0]), 0))
                except Exception: return "0"
            
            elif nama == 'ke_hex': 
                try: return hex(int(str(args_evaluated[0]), 0))
                except Exception: return "0x0"
                
            elif nama == 'ke_biner': 
                try: return bin(int(str(args_evaluated[0]), 0))
                except Exception: return "0b0"
                
            elif nama == 'ke_oktal': 
                try: return oct(int(str(args_evaluated[0]), 0))
                except Exception: return "0o0"
            
            elif nama == 'ke_ascii': 
                try: 
                    teks = str(args_evaluated[0]).strip('"\'')
                    return str(ord(teks[0])) if teks else "0"
                except Exception: return "0"
                
            elif nama in ['dari_ascii', 'ke_karakter', 'ke_huruf']: 
                try: return chr(int(str(args_evaluated[0]), 0))
                except Exception: return ""
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
                    raise RuntimeError(f"🚨 KERNEL PANIC! Gagal Evaluasi sintaks dinamis: {e}")
            # ------------------------------------------------------

            # Jika tidak ada di mana-mana
            else:
                raise NameError(f"🚨 KERNEL PANIC! Fungsi '{nama}' tidak dikenal oleh alam semesta!")
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
        # --- KEAJAIBAN BARU: EKSEKUSI HEADER & PRAGMA ---
        if node['tipe'] == 'DEKLARASI_DATANG':
            # Ini adalah penanda pintu masuk. Kita bisa diam saja atau mencetak log tersembunyi.
            pass 
            
        elif node['tipe'] == 'PRAGMA_KOMPILASI':
            self.mode_kompilasi = node['mode']
            # Secara opsional, berikan info visual untuk debugging Arsitek
            # print(f"[SISTEM] Mode Alokasi Memori: {self.mode_kompilasi.upper()}")

        elif node['tipe'] == 'DEKLARASI_TAKDIR':
            rantai_nama = node['nama']
            isi_final = self.evaluasi_nilai(node['isi'])
            ukuran_kavling = node.get('ukuran')
            sifat = node['sifat']
            
            # Amankan dari bentrokan versi lama/baru (Pastikan selalu berbentuk List)
            if isinstance(rantai_nama, str): rantai_nama = [rantai_nama]
            
            # --- JIKA VARIABEL TUNGGAL (Misal: takdir.soft hasil = 10) ---
            if len(rantai_nama) == 1:
                nama = rantai_nama[0] # Ambil wujud teksnya (string)
                
                if ukuran_kavling is not None and isinstance(isi_final, list):
                    if len(isi_final) > int(ukuran_kavling):
                        raise MemoryError(f"🚨 KERNEL PANIC! Array '{nama}' kelebihan muatan ! Dipesan {ukuran_kavling} kavling, tapi diisi {len(isi_final)} data!")

                if nama in self.memory:
                    if self.memory[nama].get('sifat') == 'TETAP':
                        raise RuntimeError(f"🚨 KERNEL PANIC! Takdir '{nama}' bersifat TETAP (Hard) dan tidak bisa diubah!")
                        
                    if 'riwayat' not in self.memory[nama]: self.memory[nama]['riwayat'] = []
                    self.memory[nama]['riwayat'].append(self.memory[nama]['isi'])
                    self.memory[nama]['isi'] = isi_final
                else:
                    self.memory[nama] = {'isi': isi_final, 'sifat': sifat, 'ukuran': ukuran_kavling, 'riwayat': []}
            
            # --- JIKA NESTED OBJECT / DOMAIN (Misal: takdir.soft bos.kita.nama = "Unul") ---
            else:
                nama_root = rantai_nama[0]
                
                if nama_root not in self.memory:
                    self.memory[nama_root] = {'isi': {}, 'sifat': sifat, 'riwayat': []}
                    
                if self.memory[nama_root].get('sifat') == 'TETAP':
                    raise RuntimeError(f"🚨 KERNEL PANIC! Wujud '{nama_root}' bersifat TETAP, propertinya tidak bisa dimodifikasi!")
                    
                current = self.memory[nama_root]['isi']
                if not isinstance(current, dict):
                    raise TypeError(f"🚨 KERNEL PANIC! Takdir '{nama_root}' bukan sebuah wujud/objek yang bisa ditambahi titik!")
                    
                for prop in rantai_nama[1:-1]:
                    if prop not in current or not isinstance(current[prop], dict):
                        current[prop] = {} 
                    current = current[prop]
                    
                kunci_akhir = rantai_nama[-1]
                current[kunci_akhir] = isi_final
        
        # ==========================================
        # KEAJAIBAN BARU: MODIFIKASI DOMAIN BERTINGKAT
        # (kucing.lucu.imut = "Banget")
        # ==========================================
        elif node['tipe'] == 'MODIFIKASI_TAKDIR':
            rantai = node['rantai']
            isi_final = self.evaluasi_nilai(node['isi'])
            nama_root = rantai[0]
            
            # ATURAN ARSITEK: Leluhur harus di-define dulu!
            if nama_root not in self.memory:
                # 🛠️ UBAH JADI EXCEPTION: NameError
                raise NameError(f"🚨 KERNEL PANIC! Domain leluhur '{nama_root}' belum diciptakan! Gunakan 'takdir.soft {nama_root} = {{}}' terlebih dahulu.")
                
            if self.memory[nama_root].get('sifat') == 'TETAP':
                # 🛠️ UBAH JADI EXCEPTION: RuntimeError
                raise RuntimeError(f"🚨 KERNEL PANIC! Domain '{nama_root}' bersifat TETAP (Hard). Silsilahnya tidak bisa diubah!")
                
            # --- SUNTIKAN DEEPCOPY UNTUK MENYIMPAN RIWAYAT (OPSI A & B) ---
            if 'riwayat' not in self.memory[nama_root]:
                self.memory[nama_root]['riwayat'] = []
            self.memory[nama_root]['riwayat'].append(copy.deepcopy(self.memory[nama_root]['isi']))
            # -------------------------------------------------------------
                
            # Jika user memodifikasi variabel biasa (contoh: umur = 20)
            if len(rantai) == 1:
                self.memory[nama_root]['isi'] = isi_final
                
            # Jika user membangun Sub-Domain Bertingkat (kucing.lucu.imut)
            else:
                current = self.memory[nama_root]['isi']
                
                # Cek apakah root-nya benar-benar Wujud (Objek/Dictionary)
                if not isinstance(current, dict):
                    # 🛠️ UBAH JADI EXCEPTION: TypeError
                    raise TypeError(f"🚨 KERNEL PANIC! '{nama_root}' bukan sebuah wujud/objek, tidak bisa memiliki sub-domain!")
                    
                # Menelusuri rantai dan MENCIPTAKAN ANAK secara gaib jika belum ada
                for prop in rantai[1:-1]:
                    if prop not in current or not isinstance(current[prop], dict):
                        current[prop] = {} # Otomatis melahirkan child!
                    current = current[prop]
                    
                kunci_akhir = rantai[-1]
                current[kunci_akhir] = isi_final
        # ==========================================
        
        elif node['tipe'] == 'PERINTAH_KETIK':
            # --- PERBAIKAN: Langsung serahkan apapun isinya ke evaluasi_nilai ---
            hasil_ketik = self.evaluasi_nilai(node['target'])
            print(hasil_ketik)
                
        elif node['tipe'] == 'PERINTAH_SOWAN':
            target_file = node['target'].strip('"\'')
            try:
                with open(target_file, "r") as f: kode_sowan = f.read()
            except FileNotFoundError:
                raise FileNotFoundError(f"🚨 Bencana Sowan! Kitab '{target_file}' tidak ditemukan.")
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
            target_chain = node['target']
            
            if isinstance(target_chain, str):
                target_chain = [target_chain]

            root_var = target_chain[0]

            if root_var not in self.memory:
                raise NameError(f"🚨 KERNEL PANIC! Takdir '{root_var}' belum diciptakan!")

            # --- KALIBRASI: Menggunakan 'riwayat' dan 'isi' sesuai strukturmu ---
            riwayat = self.memory[root_var].get('riwayat', [])
            if len(riwayat) == 0:
                raise ValueError(f"🚨 KERNEL PANIC! Takdir '{'.'.join(target_chain)}' tidak memiliki masa lalu!")

            # OPSI A: DEEP UNDO (Balikan Total Induk)
            if len(target_chain) == 1:
                masa_lalu = riwayat.pop()
                self.memory[root_var]['isi'] = copy.deepcopy(masa_lalu)
            
            # OPSI B: TARGETED UNDO (Balikan Titik Spesifik)
            else:
                masa_lalu = riwayat.pop() 
                
                # 1. Ambil nilai spesifik dari MASA LALU
                nilai_lama = masa_lalu
                try:
                    for part in target_chain[1:]:
                        nilai_lama = nilai_lama[part]
                except (KeyError, TypeError):
                    raise ValueError(f"🚨 KERNEL PANIC! Sub-domain '{'.'.join(target_chain)}' belum ada di masa lalu!")
                
                # 2. Cari posisi di MASA KINI untuk ditimpa
                kini = self.memory[root_var]['isi']
                for part in target_chain[1:-1]:
                    kini = kini[part]
                
                # 3. Timpa nilai masa kini dengan nilai dari masa lalu
                kini[target_chain[-1]] = copy.deepcopy(nilai_lama)

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
                except (ValueError, TypeError):
                    # Jika gagal (karena itu Teks atau Objek), periksa operatornya!
                    if pemb in ['>', '<', '>=', '<=']:
                        raise TypeError(f"🚨 KERNEL PANIC! Hukum Alam menolak membandingkan ukuran antara '{k_val}' dan '{kan_val}'. Operator '{pemb}' hanya untuk angka yang sah!")
                
                if pemb == '>': return k_val > kan_val
                elif pemb == '<': return k_val < kan_val
                elif pemb == '==': return k_val == kan_val
                elif pemb == '!=': return k_val != kan_val
                elif pemb == '>=': return k_val >= kan_val
                elif pemb == '<=': return k_val <= kan_val
                elif pemb == 'berisi': return str(kan_val) in str(k_val)
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
                for aksi_node in node['aksi']:
                    hasil = self.eksekusi_node(aksi_node)
                    # 🔥 TERUSKAN SINYAL PULANG KE ATAS
                    if isinstance(hasil, dict) and hasil.get('aksi') == 'PULANG':
                        return hasil
                    if hasil == "HENTI": return "HENTI"
            elif len(node.get('aksi_lain', [])) > 0:
                for aksi_node in node['aksi_lain']:
                    hasil = self.eksekusi_node(aksi_node)
                    # 🔥 TERUSKAN SINYAL PULANG KE ATAS JUGA
                    if isinstance(hasil, dict) and hasil.get('aksi') == 'PULANG':
                        return hasil
                    if hasil == "HENTI": return "HENTI"

        elif node['tipe'] == 'HUKUM_SIKLUS':
            jumlah_int = int(self.evaluasi_nilai(node['jumlah']))
            for _ in range(jumlah_int):
                berhenti = False
                for aksi_node in node['aksi']:
                    hasil = self.eksekusi_node(aksi_node)
                    # 🔥 TERUSKAN SINYAL PULANG DARI DALAM LOOP
                    if isinstance(hasil, dict) and hasil.get('aksi') == 'PULANG':
                        return hasil
                    if hasil == "HENTI":
                        berhenti = True
                        break
                if berhenti: break

        elif node['tipe'] == 'HUKUM_TABU':
            error_terjadi = False
            pesan_error = ""
            stack_trace = ""
            
            # 1. Jalankan blok COBA
            try:
                for perintah in node.get('blok_coba', []):
                    hasil = self.eksekusi_node(perintah)
                    if isinstance(hasil, dict) and hasil.get('tipe') == 'PULANG':
                        return hasil
            except Exception as e:
                import traceback
                error_terjadi = True
                pesan_error = str(e)
                stack_trace = traceback.format_exc()
                
                # Sihir Pencatatan Jejak Memori (.diary)
                self.catat_diary(pesan_error, stack_trace)

            # 2. Jika Sistem Hancur, jalankan blok TEBUS/TABU
            if error_terjadi:
                if node.get('pesan_tabu'):
                    # Menyimpan pesan error secara sah ke dalam RAM UNUL
                    self.memory[node['pesan_tabu']] = {'sifat': 'SOFT', 'isi': pesan_error}
                
                for perintah in node.get('blok_tebus', []):
                    hasil = self.eksekusi_node(perintah)
                    if isinstance(hasil, dict) and hasil.get('tipe') == 'PULANG':
                        return hasil
            return None        

    # Fungsi utama yang dijalankan pertama kali
    def jalankan(self):
        for node in self.ast:
            self.eksekusi_node(node)

    def catat_diary(self, pesan, stack_trace):
        """Menulis Kernel Panic & Stack Trace ke file .diary"""
        import datetime
        waktu = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Integrasi: Cek mode di .anu (1 untuk Debug / Full Stack Trace, 0 untuk Minimal)
        node_debug = self.memory.get('MODE_DEBUG')
        mode_debug = str(node_debug['isi']) if node_debug else "0"
        
        with open("enki_sistem.diary", "a") as f:
            f.write(f"=== [TABU DILANGGAR] ===\n")
            f.write(f"Waktu Kejadian : {waktu}\n")
            f.write(f"Pesan Alam     : {pesan}\n")
            
            # Tulis full stack trace hanya jika sedang mode DEBUG
            if mode_debug == "1":
                f.write(f"Jejak Memori (Stack Trace):\n{stack_trace}\n")
            f.write("========================\n\n")        

# --- BLOK EKSEKUSI UTAMA ---
if __name__ == "__main__":
    import sys
    import os
    
    # Cek apakah user memasukkan nama file di terminal
    if len(sys.argv) < 2:
        print("🚨 Peringatan: Kamu lupa memasukkan kitab takdir!")
        print("Cara pakai : unul ayo <nama_file.unul>")
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
        interpreter.muat_anu(file_path)
        interpreter.jalankan()
        print("====================================")
    except Exception as e:
        print("\n=================================================")
        print(f"🔥 KIAMAT SISTEM (FATAL UNCAUGHT ERROR) 🔥")
        print(f"Pesan Alam Semesta: {e}")
        print("=================================================")
        sys.exit(1) # Ini adalah satu-satunya Hard Exit yang halal!