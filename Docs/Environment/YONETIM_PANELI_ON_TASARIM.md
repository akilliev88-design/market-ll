# Yönetim paneli: ana ekran ve harita

05.10.2026 · Codex · G-106 · Önceki iki düzen reddedildi; dört yeni yön değerlendirmede.

## Güncel karşılaştırma

Mustafa Harita masası / Şirket masası tasarımlarını beğenmedi. Bu iki düzen uygulanmayacak; aşağıdaki eski açıklama yalnız süreç kaydıdır.

- **Operasyon terminali:** koyu yüzey, dar ikon menüsü, geniş harita ve dikey il incelemesi; sayılar araç göstergesi gibi üstte.
- **Yönetim gündemi:** serif başlıklar, üstte yatay menü; solda günlük kararlar, sağda coğrafya ve il özeti. Dokusuz güncel editoryal düzen.
- **Büyüme sahası:** harita ana sahne, seçili il altta geniş bölümde; yönetim menüsü en altta, yumuşak köşeler ve yeşil yüzeyler.
- **Şirket dosyası:** tam yazılı sol menü, büyük mağaza/il toplamı, harita yanında il sonuçları; altta seçili il ve bugünün işleri.

Aynı örnek şirket ve sayılarla karşılaştırılır. İl/katman seçimi, teklif ve örnek gün ilerletme yereldir. Renk, tipografi ve gezinme yerleşimi birbirinden farklıdır; kullanıcı değerlendirmesi olmadan hiçbir yön önerilmiş veya seçilmiş sayılmaz. Ana oyun koduna dokunulmadı.

Yeni kaynak: `C:/Users/mtass/.codex/visualizations/2026/10/04/01a108e2-405a-7a01-a7bd-83970e6ea9f8/yonetim-yonleri.html`.

**Yeni tur doğrulaması:** Edge 1024/736/360 × dört yön: il/katman, teklif, örnek gün, ayrıntı ve menü kontrolleri geçti. Her tasarımda 81 il, yatay taşma ve JavaScript hatası yok. Dört masaüstü ve dar ekran PNG'si incelendi. Son CSS kontrolünde aynı genişlikte dört sahne eşit yükseklikte (1200/1340/2110 px). Kaynak 207 KB; gerçek oyun kodu değişmedi, DERLE/TEST/Smoke çalıştırılmadı.

## Reddedilen önceki tur

Mustafa 3B denemeyi sonraya bıraktı; yönetim panelinde ilk odak olarak ana ekran ve haritayı seçti. Eski atlas, doku, nostalji ve minyatür önerileri kullanılmadı. Yeni çalışma iki etkileşimli yerleşim içerir:

- **Harita masası:** geniş il haritası, yanında seçili pazar, altta bugünün kararları ve şirket gündemi. Başlangıç önerisi.
- **Şirket masası:** bekleyen teklif, daha küçük harita ve il bazında mağaza/kâr/pay karşılaştırması.

Ortak düzen: üstte oyuncunun şirket adı, kampanya yılı/gün, zaman ve kasa; altta yönetim sayfaları ve Dükkâna git. Düz yüzeyler, ince ayırıcılar, IBM Plex Sans ve mavi seçim vurgusu. Harita il sınırları mevcut Config/iller.json geometrisinden. Bu örnekte Türkiye gösterilir; gerçek uygulama ülke paketinden beslenmelidir.

İl seçimi, mağaza/rekabet/kârlılık katmanları, batıya odaklanma, teklif inceleme/kabul/geç ve örnek gün ilerletme çalışır. Mağaza seçimi yalnız yerel önizlemedir. Diğer sayfaların ayrıntılı tasarımı bu tur kapsamına alınmadı.

Şirket ve bütün sayısal değerler örnektir; gerçek kayıt veya oyun simülasyonu bağlı değildir. Örnek Market oyuncu şirketinin yer tutucusudur. Kaynak:

`C:/Users/mtass/.codex/visualizations/2026/10/04/01a108e2-405a-7a01-a7bd-83970e6ea9f8/yonetim-harita.html`

**Doğrulama:** Edge ile 1024/736/360 pencere genişliklerinde iki yerleşim; il, katman, odak, teklif ve gün akışı geçti. Üçünde de 81 il çizildi; yatay taşma ve JavaScript hatası yok. Masaüstü ve dar ekran PNG'leri incelendi. Dar haritada yalnız seçili ilin adı gösterilir; diğer mağaza noktaları kalır. Aynı genişlikte iki tasarımın sahne yüksekliği eşittir.

Unreal menü koduna uygulanmadı. Yerleşim, font, renk ve il seçimi mevcut Slate sistemiyle yapılabilir; gerçek veriler bağlanmadan tamamlanmış sayılmaz. MarketMenu*, MarketTheme* ve ekonomi dosyaları Claude sahipliğinde kalır. Mustafa yerleşimi değerlendirdikten sonra uygulama işi dosya sahipliği gözetilerek ele alınır.

Bu tur yalnız önizleme ve tasarım belgesi değişti; DERLE/TEST/Smoke çalıştırılmadı. Önceki gerçek motor denemesi korunur.
