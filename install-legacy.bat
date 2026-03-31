@echo off
echo === MENGINSTAL UNUL CLI (WINDOWS) ===
echo Membangkitkan ekosistem untuk Windows...

REM Menambahkan folder bin ke dalam PATH Windows (Hanya untuk User saat ini)
setx PATH "%PATH%;%cd%\bin"

echo.
echo [SUKSES] Instalasi Selesai!
echo Silakan RESTART terminal/CMD Anda agar efeknya terasa.
echo Setelah itu coba ketik: unul ayo tests\sabu_v0-0-1.unul
pause