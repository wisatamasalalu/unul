# 📜 Kitab Dokumentasi UNUL (Fase 2)

Kitab ini adalah panduan lengkap untuk memahami struktur, kosakata, dan sihir yang ada di dalam ekosistem UNUL. 

---

## 📖 Glosarium: UNUL vs Bahasa Populer
Agar memudahkan transisi *developer*, berikut adalah tabel persilangan kosakata (sintaks) UNUL dengan bahasa modern seperti JavaScript, Python, atau Go.

| Konsep | Sintaks UNUL | JavaScript / JS | Python | Keterangan |
|---|---|---|---|---|
| **Cetak Teks** | `ketik("Halo")` | `console.log()` | `print()` | Mencetak teks/variabel ke terminal. |
| **Minta Input** | `dengar()` | `prompt()` | `input()` | Menunggu ketikan dari *user*. |
| **Variabel Fleksibel** | `takdir.soft x = 1` | `let x = 1` | `x = 1` | Nilainya bisa diubah kapan saja. |
| **Variabel Tetap** | `takdir.hard x = 1` | `const x = 1` | `X = 1` | Konstanta (disiapkan untuk mode Kernel/C). |
| **Logika / Percabangan**| `jika x > 1 maka ... putus` | `if (x > 1) { ... }` | `if x > 1:` | Hakim Karma penentu alur aplikasi. |
| **Perulangan / Loop** | `effort 5 kali maka ... putus`| `for(let i=0;i<5;i++)` | `for i in range(5):`| Melakukan tindakan berulang kali. |
| **Pembuatan Fungsi** | `ciptakan fungsi x() maka ... putus` | `function x() { ... }` | `def x():` | Membungkus blok logika agar bisa dipanggil ulang. |
| **Nilai Kembalian** | `pulang x` | `return x` | `return x` | Melempar nilai dari dalam fungsi ke variabel. |
| **Mesin Waktu** | `balikan x` | *(Tidak ada)* | *(Tidak ada)* | **Fitur Unik UNUL!** Mengembalikan variabel ke nilai sebelumnya. |
| **Import File** | `sowan "modul.unll"` | `import / require` | `import` | Menggabungkan file terpisah menjadi satu kesatuan. |

---

## 🧬 Tipe Data & Struktur Wujud

Mesin UNUL mendukung tipe data dinamis tingkat tinggi tanpa perlu deklarasi tipe yang rumit.

### 1. Teks & Angka (Dasar)
Mendukung matematika berderet, penggabungan teks, sisa bagi (`%`), dan pangkat (`^`).
```unul
takdir.soft namaku = "Unul"
takdir.hard umur = 2 ^ 3
ketik("Halo " + namaku)
2. Array (Kavling Data)
Bisa berupa Array dinamis layaknya web, atau Array statis ketat layaknya Kernel.

Cuplikan kode
^^ Array Dinamis (Web Mode)
takdir.soft pengguna = ["Nabhan", "Enki"]
ketik(pengguna[0]) ^^ Output: Nabhan

^^ Array Statis (Kernel Mode)
takdir.hard pin_hardware[2] = [12, 14]
^^ Jika diisi 3 data, mesin akan menembakkan KERNEL PANIC!
3. Objek / Wujud (Dictionary/JSON)
Format tingkat tinggi untuk menampung data kompleks atau merespons balasan dari cloud API.

Cuplikan kode
takdir.soft profil = {
    "nama": "Developer Kece",
    "sistem": "LinuxDNC"
}
ketik(profil["sistem"]) ^^ Output: LinuxDNC
⚖️ Gerbang Logika (Hukum Karma)
Hakim Karma (jika) di UNUL sangat cerdas karena dilengkapi dengan gerbang logika dan serta atau.

Cuplikan kode
takdir.soft umur = 18
takdir.soft tiket = "ada"

^^ Mengecek 2 syarat sekaligus!
jika umur >= 17 dan tiket == "ada" maka
    ketik("Akses stasiun LinuxDNC diizinkan!")
putus
🌌 The Tablet of Destinies (Pustaka Standar Bawaan)
Kamu tidak perlu menginstal npm atau pip eksternal. UNUL sudah membawa fungsi-fungsi sakti ini di dalam jantungnya (Interpreter).

waktu_sekarang(): Mengembalikan angka UNIX Timestamp detik ini.

acak(min, max): Menghasilkan angka acak antara min hingga max.

panjang_teks(teks): Menghitung jumlah huruf dalam sebuah teks/variabel.

huruf_besar(teks) / huruf_kecil(teks): Mengubah ukuran karakter teks.

ambil("url"): Mengambil teks/JSON dari internet (Metode GET).

setor("url", data): Mengirim teks/Objek UNUL ke server internet (Metode POST). Mesin menyamar sebagai browser Chrome secara otomatis!

📁 Ekosistem Ekstensi: .unul vs .unll
Di dalam arsitektur UNUL, terdapat pemisahan elegan untuk menjaga kerapian kode skala besar. Silakan pelajari folder example/ di repositori ini untuk melihat penggunaannya di dunia nyata!

.unul (UNUL Main Script):
Adalah ekstensi utama yang dieksekusi oleh pengguna. Biasanya berisi antarmuka, input user, dan alur utama aplikasi (Contoh: utama.unul).

.unll (Unified Native Language Library):
Adalah modul atau pustaka yang hanya dipanggil (di-import) oleh script .unul. Ekstensi ini menandakan bahwa file tersebut berisi fungsi-fungsi, rumus, atau struktur data backend (baik bawaan kreator maupun buatan komunitas).

Contoh Penggunaan:

Cuplikan kode
^^ File: utama.unul
ketik("Sistem Menyalakan Mesin...")

^^ Memanggil pustaka dari file terpisah
sowan "pustaka/kriptografi.unll"
sowan "pustaka/koneksi_database.unll"

buka_koneksi()
