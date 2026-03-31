import os
            path_absolut = os.path.abspath(file_path)
            interpreter.muat_anu(path_absolut)
@echo off
REM Eksekutor UNUL CLI untuk lingkungan Windows
python "%~dp0unul-legacy" %*