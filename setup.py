from setuptools import setup, find_packages

setup(
    name='unul',
    version='0.1.0',
    description='Bahasa Pemrograman UNUL - The Omnidisciplinary Language',
    author='Wisata Masa Lalu',
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    scripts=['bin/unul-legacy'], # Ini yang akan otomatis jadi perintah global "unul"
)