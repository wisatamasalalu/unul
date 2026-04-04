#!/bin/bash

# Konfigurasi Jalur Biner (Disesuaikan dengan letak Makefile Anda)
UNUL_BIN="../../bin/unul"

# Warna Terminal untuk Estetika LinuxDNC
HIJAU='\033[0;32m'
MERAH='\033[0;31m'
BIRU='\033[0;34m'
KUNING='\033[1;33m'
NETRAL='\033[0m'

echo -e "${BIRU}======================================================${NETRAL}"
echo -e "${BIRU}🦅 MEMULAI PROTOKOL UJI MASSAL LINUXDNC (C-PHASE) 🦅${NETRAL}"
echo -e "${BIRU}======================================================${NETRAL}"

# Mencari semua file berakhiran .unul di dalam direktori saat ini dan subdirektorinya
find . -type f -name "*.unul" | sort | while read -r file; do
    echo -e "\n${KUNING}[*] MENYELAMI DIMENSI: ${file}${NETRAL}"
    echo "------------------------------------------------------"
    
    # Menjalankan UNUL dengan mode 'ayo' (Executor)
    $UNUL_BIN ayo "$file"
    
    # Menangkap status exit (0 = Sukses, selain 0 = Kiamat/Error)
    STATUS_KELUAR=$?
    
    echo "------------------------------------------------------"
    if [ $STATUS_KELUAR -eq 0 ]; then
        echo -e "${HIJAU}[+] MISI SELESAI TANPA BENCANA: ${file}${NETRAL}"
    else
        echo -e "${MERAH}[!] KIAMAT TERJADI PADA: ${file}${NETRAL}"
    fi
done

echo -e "\n${BIRU}======================================================${NETRAL}"
echo -e "${BIRU}🦅 SELURUH DIMENSI TELAH DIUJI. OS TETAP HIDUP! 🦅${NETRAL}"
echo -e "${BIRU}======================================================${NETRAL}"