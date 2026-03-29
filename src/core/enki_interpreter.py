from enki_lexer import enki_lexer
from enki_parser import EnkiParser

class EnkiInterpreter:
    def __init__(self, ast):
        self.ast = ast
        self.memory = {}

    def evaluasi_nilai(self, nilai_mentah):
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
            
        return nilai_mentah

    def jalankan(self):
        for node in self.ast:
            if node['tipe'] == 'DEKLARASI_TAKDIR':
                nama = node['nama']
                sifat = node['sifat']
                
                # Hitung dulu hasil akhirnya sebelum disimpan
                isi_final = self.evaluasi_nilai(node['isi'])
                
                self.memory[nama] = {
                    'isi': isi_final,
                    'sifat': sifat
                }
                
            elif node['tipe'] == 'PERINTAH_KETIK':
                target = node['target']
                if node['is_variable']:
                    if target in self.memory:
                        print(self.memory[target]['isi'])
                    else:
                        print(f"🚨 Bencana! Takdir '{target}' belum pernah diciptakan!")
                else:
                    target_bersih = self.evaluasi_nilai(target)
                    print(target_bersih)
                    
            elif node['tipe'] == 'HUKUM_KARMA':
                kiri = self.evaluasi_nilai(node['kiri'])
                kanan = self.evaluasi_nilai(node['kanan'])
                pembanding = node['pembanding']
                
                # Ubah ke angka untuk ditimbang
                kiri_int = int(kiri)
                kanan_int = int(kanan)
                
                # Keputusan Hakim
                sah = False
                if pembanding == '>': sah = kiri_int > kanan_int
                elif pembanding == '<': sah = kiri_int < kanan_int
                elif pembanding == '==': sah = kiri_int == kanan_int
                
                if sah:
                    # Eksekusi aksinya
                    for aksi_node in node['aksi']:
                        if aksi_node['tipe'] == 'PERINTAH_KETIK':
                            target = aksi_node['target']
                            if aksi_node['is_variable']:
                                print(self.memory[target]['isi'])
                            else:
                                print(self.evaluasi_nilai(target))

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