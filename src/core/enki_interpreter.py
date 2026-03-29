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
    import os
    file_path = os.path.join(os.path.dirname(__file__), '../../tests/aplikasi.unul')
    
    with open(file_path, "r") as f:
        kode_user = f.read()
        
    # Proses Trinitas Enki
    tokens = enki_lexer(kode_user)
    parser = EnkiParser(tokens)
    pohon_logika = parser.parse()
    
    print("=== EKSEKUSI PROGRAM UNUL (LIVE) ===")
    interpreter = EnkiInterpreter(pohon_logika)
    interpreter.jalankan()
    print("====================================")