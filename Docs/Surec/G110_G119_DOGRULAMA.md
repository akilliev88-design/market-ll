# G-110 / G-119 doğrulama — 06.10.2026

İş emri: `promptlar/codex_g110_g119_20261006.md`. Dal: `akis-cc2`.

G-110: `7254ec7`, cloud/akis-cc2 dalına gönderildi. Claude kaynaklarında derleme düzeltmesi gerekmedi; DERLE başarılı, TEST 184/184, Smoke başarılı. Kayıt sürümü 23.

G-119: `5e7a403`. Son kaynakla DERLE başarılı, TEST 184/184 (başarısız 0), Smoke ve StoreEntryReview başarılı.

G-119: mağazaya il kartındaki tür/sayı satırından girilir; seçim, sıralama ve başlık `MarketStoreVisit` kurallarından gelir. Alt çubuktaki doğrudan ilk mağazaya giriş kaldırıldı. Mağazalar listesindeki gerçek Gez düğmesi korunuyor; il kartındaki eski, tıklanamayan “Gez yakında” yazısı kaldırıldı. Şube sahnesi mevcut gezi sistemini kullanır: kendi görünümü, adı ve raf stok doluluğu. Zaman gezi sırasında durur; şubede elle işletme G-105 kapsamında kalır.

Tab aynı il ve türdeki sonraki mağazayı açar; tek mağazada yerinde kalır. Esc haritaya döner. Tek açık mağaza varsa yükleme sonrası içeride, birden fazla veya hiç mağaza yoksa haritada başlanır. İlk mağaza kapalıyken ilk sahneye giriş ve O engellenir; kapatma menüden haritaya götürür, raf görüntüsü yenilenir. Haritada M/Esc ve gün raporunun dönüş düğmesi ilk mağazayı gizlice açmaz.

## Tekrarlanabilir kontrol

`powershell -NoProfile -ExecutionPolicy Bypass -File StoreEntryTest.ps1`

Kontrol gerçek Unreal Slate düğmelerini ve oyuncu denetleyicisindeki Tab/Esc tuş bağlarını kullanır. Ülke ve il mevcut başlangıç verisinden alınır. Yalnız `MarketSim_StoreEntryReviewOnly` adlı geçici kayıt yazılır; kontrol sonunda silinir, normal yuvalar kullanılmaz.

Kontrol kapsamı:

- Finans: açık günde kapatma reddedilir; kapalı günde boş tut ile kapatma; bütün stok satırları boş, kasiyer görevden ayrılmış, 3B reyon görevlileri kaldırılmış; kapalı mağazaya O ve sahne girişi reddedilir; yeniden açma çalışır.
- İl kartındaki tür düğmesine tıklama şubeyi açar; başlık kuralla eşleşir. Gezi beklerken kampanya baytları değişmez.
- Gerçek Tab girişi diğer şubeyi açar; Esc haritaya döner; doğrudan menü kapatma haritayı terk etmez.
- İlk mağaza kapalı ve iki şubeli gerçek kayıt yüklenince harita açılır. Tek kalan şubede içeride, sıfır mağazada haritada açılış denetlenir.

Görüntüler: `Docs/Images/G110/` (Finans açık/kapalı), `Docs/Images/G119/` (il kartı, birinci/ikinci şube, kapalı kayıt haritası). Şube görüntülerinde 6/24 ve 18/24 stoklu iki mağazanın rafları farklı görünür.

## Kontrolün sınırı

Elle pencere kontrolü bu oturumda yapılmadı; yerel uygulama kontrolü kullanılamıyor. Yukarıdaki sonuçlar gerçek Unreal arayüz otomasyonudur. Mustafa'nın kısa elle kontrolü açık kalır. Finans'taki dört seçenek birlikte görünmez: açık mağazada üç kapatma düğmesi, kapalı ve satılmamış mağazada yeniden aç düğmesi görünür.

İlk G-119 kontrol derlemesinde yeni kontrol kodunun Slate çocuk const türü ve input başlığı düzeltildi. İlk arayüz koşusu, onay penceresini aynı karede aradığı için durdu; onay karesi beklenerek düzeltildi. G-110 kural kodu ve test beklentileri değiştirilmedi.
