#!/bin/bash

echo "=== MENGINSTAL UNUL CLI ==="
echo "Membangkitkan ekosistem untuk LinuxDNC..."

# 1. Berikan hak eksekusi penuh pada file CLI
chmod +x bin/unul

# 2. Buat direktori lokal bin jika belum ada (Aman tanpa sudo)
mkdir -p ~/.local/bin

# 3. Buat jembatan (symlink) dari repo ke dalam sistem
ln -sf "$(pwd)/bin/unul" ~/.local/bin/unul

echo "✅ Instalasi Sukses!"
echo "Sistem telah terhubung. Coba ketik perintah di bawah ini dari mana saja:"
echo "unul ayo tests/kalkulator.unul"