#!/bin/bash

echo "=== MENGINSTAL UNUL CLI ==="
echo "Membangkitkan ekosistem untuk LinuxDNC..."

# 1. Berikan hak eksekusi penuh pada file CLI
chmod +x bin/unul-legacy

# 2. Buat direktori lokal bin jika belum ada
mkdir -p ~/.local/bin

# 3. Buat jembatan (symlink) dari repo ke dalam sistem
ln -sf "$(pwd)/bin/unul-legacy" ~/.local/bin/unul-legacy


# 4. SMART CHECK: Cek apakah PATH sudah terdaftar di .bashrc
if ! grep -q "$HOME/.local/bin" ~/.bashrc; then
    echo "🔧 Menambahkan ~/.local/bin ke dalam PATH ~/.bashrc..."
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
    echo "⚠️ PENTING: Jalankan perintah ini SATU KALI agar terminalmu langsung sadar:"
    echo "source ~/.bashrc"
    echo ""
else
    echo "✅ Jalur PATH sudah aman."
fi

echo "🎉 Instalasi Sukses!"
echo "Coba ketik perintah di bawah ini:"
echo "unul-legacy ayo tests/main.unul"