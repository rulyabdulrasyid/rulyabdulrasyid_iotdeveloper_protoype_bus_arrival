Prototype Bus Arrival Display
Ditulis Oleh: Ruly Abdul Rasyid
1.	Hardware yang Digunakan
    a.	ESP32
    Mikrokontroller utama dengan fasilitas WiFi bawaan untuk koneksi internet dan komunikasi dengan API LTA DataMall.
    b.	OLED Display (SSD1306, 128x64 pixel, I2C)
    Digunakan untuk menampilkan informasi jadwal kedatangan bus secara realtime. Oled merupakan alternatif yang saya gunakan untuk menggantikan e-ink display karena belum memiliki perangkat e-ink display. Ukuran yang kecil hanya menampilkan 9 baris text.

2.	Library 
    a.	WiFi.h (Menghubungkan ESP32 ke jaringan WiFi).
    b.	HTTPClient.h (Untuk melakukan request HTTP ke API).
    c.	ArduinoJson.h (Parsing (membaca dan memproses) data JSON dari API).
    d.	time.h (Singkroonisasi waktu dengan NTP untuk menghitung estimasi kedatangan bus)
    e.	AdaAdafruit_GFX.h & Adafruit_SSD1306.h (Library OLED Display untuk menampilkan text, mengatur posisi text).

3.	Langkah Kerja
    a.	Membaca dokumentasi LTA Datamall API endpoint dari website LTA Datamall API dan melakukan registrasi untuk mendapatkan API_key.
    b.	Melakukan uji coba get data dari API yang diberikan pada soal https://datamall2.mytransport.sg/ltaodataservicec/BusArrrivalv2 menggunakan ExpressJs. Namun setelah mencoba dan tidak dapat diakses dan melihat dokumentasi yang disediakan penyedia endpoint yang bisa diakses untuk Bus Arrival adalah https://datamall2.mytransport.sg/ltaodataservice/v3/BusArrival . 
    c.	Membuat rangkaian ESP32 dengan OLED.
    d.	Membuat program pada Arduino IDE dengan tahapan code berikut
        1)	Inisialisasi OLED dan Serial Monitor (Serial Monitor sebagai debugging)
        2)	Koneksi Ke WiFi jika berhasil OLED akan menampilkan “WiFi Connected”
        3)	Sinkronisasi waktu (NTP) server agar perhitungan menit kedatangan bus akurat
        4)	Melakukan akses API LTA Datamall mengirimkan request ke endpoint Bus Arrival dengan kode Bus Stop tertentu (52009 - Toa Payoh Int) dengan membaca response dalam format JSON. Dan mengambil data Nomor Bus (ServiceNo) dan waktu kedatangan (EstimatedArrival)
        5)	Mengurutkan Data dari ServiceNo yang terkecil ke besar.
        6)	Menampilkan ke OLED. 
        7)	Jika bus tiba maka keteranganya Adalah “Arr” atau artinya Arrival
        8)	Error Handling. Jika WiFi terputus maka akan tampil “WiFi Disconected” pada OLED
        9)	Error Handling. Jika parsing gagal atau API Error makan akan tampil “Parse Error” atau kode Error.
    e.	Sistem berhasil menampilkan jadwal kedatangan bus secara realtime pada layer OLED. Informasi diperbaharui dalam waktu 1 menit, dan ditampilkan secara scrolling agar seluruh data bus bisa terbaca meskipun layer terbatas.
