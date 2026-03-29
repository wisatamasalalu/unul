from enki_lexer import enki_lexer
from enki_parser import EnkiParser

class EnkiInterpreter:
    def __init__(self, ast):
        self.ast = ast
        # Ini adalah "Akhasic Record" - Tempat menyimpan takdir/variabel di memori
        self.memory = {}

    def jalankan(self):
        for node in self.ast:
            # 1. Jika menemukan hukum penciptaan takdir
            if node['tipe'] == 'DEKLARASI_TAKDIR':
                nama = node['nama']
                isi = node['isi']
                sifat = node['sifat']
                
                # Bersihkan tanda kutip jika isinya adalah teks literal
                if (isi.startswith('"') and isi.endswith('"')) or (isi.startswith("'") and isi.endswith("'")):
                    isi = isi[1:-1]
                
                # Simpan ke alam memori
                self.memory[nama] = {
                    'isi': isi,
                    'sifat': sifat
                }
                
            # 2. Jika menemukan perintah eksekusi ketik
            elif node['tipe'] == 'PERINTAH_KETIK':
                target = node['target']
                is_var = node['is_variable']
                
                if is_var:
                    # Cek apakah takdirnya sudah diciptakan di memori
                    if target in self.memory:
                        print(self.memory[target]['isi'])
                    else:
                        print(f"🚨 Bencana! Takdir '{target}' belum pernah diciptakan oleh Enki!")
                else:
                    # Langsung cetak teks literal (bersihkan kutip)
                    if (target.startswith('"') and target.endswith('"')) or (target.startswith("'") and target.endswith("'")):
                        print(target[1:-1])
                    else:
                        print(target)

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