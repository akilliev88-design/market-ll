> **ARŞİV (06.10.2026):** Bu belge M69 öncesi kurguyu ve eski planı anlatır (2011, Lüleburgaz, "Miras Market", babadan kalan dükkân, hikâye bölümleri). **Güncel değildir; ajanlar buna göre kod ya da metin yazmasın.** Güncel kurgu `Docs/Kurgu/00_KURGU_KITABI.md`, kararlar `Docs/Kurgu/01_KARARLAR.md`, durum `Docs/Surec/DURUM.md`.

# Miras Market: yeni yol haritası

Tarih: 30.09.2026 · Hazırlayan: Claude

> **30.09.2026 gece: sıra ve öncelik için `06_GIDIS_YOLU.md` geçerlidir** (önce oyunun aklı, sonra dükkân içi simülasyon; oyunu Codex otomatik oyuncuyla oynatır). Bu belge hata durumları (§2) ve eski kapsam notları için ayrıntı olarak kalır; §4 sırası geçersiz.
Dayanak: `02_DERIN_INCELEME.md` (51 hata, Aşama 0–5), `01_KARARLAR.md` (J, K, L, M), `03_MAGAZA_AGI.md`, `04_MAGAZA_KITI.md`, `Docs/Surec/` (GOREVLER, DURUM, GUNLUK), `AGENTS.md` §8.

Bu belge `02_DERIN_INCELEME.md` §4'teki eski yol haritasının yerini alır. Eski harita yazıldıktan sonra Mustafa iki büyük karar turu verdi (L: dünya ve ülke seçimi, M: il bazlı mağaza ağı). Bu turlar eski planın bir kısmını gereksiz kıldı, bir kısmını da değiştirdi.

İşaretler: **Kapandı** (hangi görevle) · **Kısmen** · **Açık** · **Geçersiz** (yeni kararla konusu kalktı) · **Kontrol edilmeli** (emin değilim, koda ya da oyuna bakılmalı).

---

## 0. Bir bakışta

- Aşama 0'ın (acil düzeltmeler) büyük kısmı kodda ve **derlendi**. Ancak 29.09'daki G-073'ten beri **smoke testi hiç çalışmadı** ve oyunda elle deneme de yapılmadı.
- 51 hatanın 20'si kapandı, 11'i kısmen kapandı. 6'sının konusu yeni kararlarla tamamen ya da kısmen kalktı. 9'u açık, 5'i kontrol bekliyor (ayrıntı §2).
- Eski Aşama 2–5 (G-079…G-082) "Lüleburgaz → Trakya → Türkiye → dünya" sırasına ve gerçek yıllara göre yazılmıştı. Artık oyun her ülkede ve herhangi bir ilde başlıyor, yıl belirsiz. Bu yüzden bu görevlerin kapsamı yeniden yazıldı (§3).
- Yeni iş bölümü: **Claude** oyunun aklını yazar. **Codex** mağaza kitini, 3B işleri ve derleme/testi yapar. **Mustafa** karar verir ve oyunu dener (§4).
- İlk iş: doğrulama borcunu kapatmak. Sonra oyun testinden (G-055) önce açık kalan birkaç denge hatası giderilmeli.

---

## 1. Kısa durum

### 1.1 Biten ve derlenen

| Görev | Ne | Doğrulama |
|---|---|---|
| G-076 | Aşama 0a: test modu kapalı, 3 kayıt yuvası, oyun sonu (J02), Rüyaymış (J03), menüde zaman ayarı | Derleme + 46/46 test |
| G-077 | Aşama 0b: rekabet payı, vadeli alım, KDV, müşavir, enflasyona bağlı tutarlar, depozito | Derleme + 46/46 test |
| G-078 (1. parça + #5) | Ortalama maliyet, katalog alanları, kapsamlı kampanya, raf düzeni kayda | Derleme + 48/48 test |
| G-079 (1. parça) | Bakkal, pazar, Bereket'in satılması, zincir tepkisi, ayartma | Derleme + 48/48 test |
| G-084 (1. ve 2. parça) | Ülke paketi, para birimi, kendi ekonomisi, aileden kalan market, yeni oyun | Derleme + 49/49, 50/50 test |
| G-086a, c, d, e | İl bazlı mağaza ağı, harita ana ekranı, tuval görsel dili, zıplamayan ekran | Derleme + 51/51 test; Mustafa oyunda açtı |

### 1.2 Derlenmeyen ya da durumu belirsiz

| İş | Durum |
|---|---|
| G-086f (il adı yerleşimi, sipariş penceresi, sekme geçişi) | Kod yazıldı, **derlenmedi** |
| G-084 3. parça (ülke takvimi, pazar yasası, kart payı, il alım gücü) | GOREVLER "derlenmedi" diyor. Ama G-086a'nın 51/51 test sonucunda bu parçanın testi (`Country.HolidaysAndLaw`) de bekleniyordu. Büyük ihtimalle derlendi. **Kontrol edilmeli**, GOREVLER düzeltilmeli |
| `MarketStoreAssign.h/.cpp` + testleri (G-088 Aşama C'nin mantık kısmı) | Dosyalar kaynakta var (30.09 sabahı), ama GUNLUK, GOREVLER ve AGENTS haritasında yazmıyor. **Derlenmedi**. Belgeye işlenmeli |
| G-088 Aşama A (Codex) | Başlamadı: `Config/magazalar.json` ve `MarketStoreKit` henüz yok |

### 1.3 Doğrulama borcu

Son başarılı smoke testi 29.09.2026'da Codex'in G-060…G-073 kontrolüydü. O günden beri oyun akışını değiştiren her iş smoke ve elle deneme bekliyor.

| Görev | Bekleyen kontrol | Kim |
|---|---|---|
| G-086f | Derleme + test + smoke + pencereyi oyunda deneme | Codex, sonra Mustafa |
| `MarketStoreAssign` | Derleme + `StoreAssign.*` testleri | Codex |
| G-074 | Smoke + menüyü elle deneme | Codex, Mustafa |
| G-075 | Tam ekran menü, hız düğmeleri: elle deneme | Mustafa |
| G-076 | Smoke + kayıt yuvaları, oyun sonu kartı, Rüyaymış: elle deneme | Codex, Mustafa |
| G-077 | Smoke (vadeli sipariş, pay toplamı) | Codex |
| G-078 | Smoke (raf düzeni kayda yazılıyor, oyun akışına dokunuyor) + kampanya sayfası elle | Codex, Mustafa |
| G-079 1. parça | Smoke + Bereket satın alma kararı elle | Codex, Mustafa |
| G-084 1–3 | Smoke + yeni oyun (ülke/il seçimi) elle; Almanya'da pazar günü kapalı | Codex, Mustafa |
| G-086a/c/d/e | Smoke. **Dikkat:** smoke eski boş dükkânla başlıyor; zorunlu il seçimi (`bNeedStart`) smoke'u durdurmuyor mu, bakılmalı | Codex |
| G-003, G-013, G-015, G-017 | Stüdyoda elle deneme (eski, acil değil) | Mustafa |

---

## 2. 51 hatanın durumu

"Kapandı" = GUNLUK'te o görevle kapandığı yazıyor ve kod derlendi (smoke hâlâ yok). Kodda ayrıca baktığım yerler "(koda bakıldı)" diye işaretli.

### 2.1 Kayıt, test modu, hikâye akışı

| # | Kısaca | Durum | Not |
|---|---|---|---|
| 1 | Test modu varsayılan açık | **Kapandı** (G-076) | |
| 2 | Kayıt üzerine yazılıyor | **Kapandı** (G-076) | 3 yuva, açılışta son yuva |
| 3 | Bölüm 99 oyunu kırıyor | **Kapandı** (G-076) | `bEnded`, J02 |
| 4 | Rüyaymış para açığı | **Kapandı** (G-076) | Satış parası kasaya girmiyor |
| 5 | Raf düzeni kayda değil dosyaya | **Kapandı** (G-078 #5) | |
| 6 | Kayıt sürümü sabit | **Kısmen** (G-076) | Sürüm 2 oldu; yüklenen verinin doğrulanması **kontrol edilmeli** |
| 7 | Kaydet-yükle ile geleceği bilmek | **Açık** | Hiçbir görev ele almadı; G-080'e |

### 2.2 Zaman ve oyun döngüsü

| # | Kısaca | Durum | Not |
|---|---|---|---|
| 8 | İlerletme kusursuz, hep açık | **Açık** | G-080 (ilerletme işi yapan kişinin becerisine bağlanacak). L07 birinci şahsı koruduğu için hâlâ önemli |
| 9 | Kapalı dükkânda zaman donuyor | **Kısmen** (G-079 1. parça) | Gün kayması düzeldi. "Sabah hazırlığı saati işlesin" G-080'de |
| 10 | Menü açıkken zaman | **Kapandı** (A02 kararı + G-076 ayarı) | |
| 11 | Zorluk oyun içinde değişiyor | **Kapandı** (G-076) | |
| 12 | "1 hafta" 7 gün ilerletmiyor | **Açık** | G-080'deki haftalık/aylık tur ile |
| 13 | 2011→2030 çok uzun | **Kısmen geçersiz** (L06, L09) | Gerçek yıllar kalktı. Ama "uzun oyun, aylık/çeyreklik tur yok" sorunu duruyor: G-080 |

### 2.3 Rekabet ve pazar payı

| # | Kısaca | Durum | Not |
|---|---|---|---|
| 14 | Bereket'in savaşı paya işlemiyor | **Kapandı** (G-077) | |
| 15 | Paylar %100 etmiyor | **Kapandı** (G-077) | |
| 16 | Pay arttıkça ilçe küçülüyor | **Kapandı** (G-077) | |
| 17 | A101 çift sayılıyor | **Kapandı** (G-077) | |
| 18 | Zincirlerin ekonomisi yok | **Kısmen** (G-079 1. parça) | Zincirler tepki veriyor. Bilanço, nakit kullanımı ve zorluk etkisi G-079'un kalanında |
| 19 | Kampanya adedi istismarı | **Açık** (koda bakıldı) | `MarketCompetitors.cpp:244`: hâlâ kampanya başına +%8 (tavan ×1,25) |
| 20 | Bereket öngörülebilir, satılmıyor | **Kapandı** (G-079 1. parça) | Bereket satılığa çıkıyor ve satın alınabiliyor. "Hedef reyon seçimi hâlâ öngörülebilir mi" **kontrol edilmeli** |
| 21 | Menü ile simülasyon farklı rakip fiyatı | **Kontrol edilmeli** | Menü G-086'da baştan yazıldı; eski hata taşındı mı bilinmiyor |
| 22 | Personel ayartma hatalı | **Kapandı** (G-079 1. parça) | |
| 23 | Şok tarih hatası | **Geçersiz** (L06 belirsiz yıl, L12 kurgu zincir adları, M01 Kırklareli varsayılanı yok) | Kodda "Şok 15 Temmuz 2011'de gelir" kuralı kaldıysa temizlenmeli: **kontrol edilmeli** |

### 2.4 Fiyat, kampanya, katalog

| # | Kısaca | Durum | Not |
|---|---|---|---|
| 24 | Fiyat esnekliği yok gibi | **Kısmen** (koda bakıldı) | G-078 kampanyalara ürün esnekliği ekledi. Ama rafta "alır mı" kararı aynı eğriyle duruyor (`MarketDemand.cpp` `BuyChance`) |
| 25 | Kampanya başka reyondan çalıyor | **Kontrol edilmeli** | G-078'de "tüm mağaza indirimi müşteri getirir" eklendi; müşteri listesinin uzunluğu hâlâ sabit mi, bakılmalı |
| 26 | Satılan malın maliyeti bugünkü fiyattan | **Kapandı** (G-078) | Ağırlıklı ortalama maliyet |
| 27 | Maliyet altı satış serbest, destekli kampanya raporu yanlış | **Açık** | |
| 28 | Gondol başı bedava ve süresiz | **Kısmen** (G-078) | Etki kapsama bağlandı. Kira ve süre yok |
| 29 | Referans fiyat hafızası yok | **Kısmen** (G-078) | Basit kampanya yorgunluğu ve evde stok var. Tam referans fiyat modeli yok |
| 30 | Online satış kampanyaları yok sayıyor | **Açık** | |
| 31 | "3 al 2 öde" müşteriyi 3'e zorluyor | **Kontrol edilmeli** | G-078'de mekanikler yenilendi |
| 32 | `products.json` veri sorunları | **Kısmen** (G-078) | Raf ömrü, KDV, alt kategori eklendi. Kurgu ad artık gerçek addan farklı (L12). Tek tip marj bilinçli bir karar (G10). "2011 fiyatları" L06 ile önemini yitirdi. Gramaj/birim fiyat yok |

### 2.5 Ekonomi, finans, personel

| # | Kısaca | Durum | Not |
|---|---|---|---|
| 33 | Vade sermaye sağlamıyor | **Kapandı** (G-077) | |
| 34 | KDV kâr sayılıyor | **Kapandı** (G-077) | |
| 35 | Müşavir istismarı | **Kapandı** (G-077) | |
| 36 | Sabitler enflasyona bağlı değil | **Kapandı** (G-077) | |
| 37 | Görünmeyen kayıplar | **Kısmen** (G-078) | Eksik/kırık mal rapora giriyor. Çift taraflı muhasebe defteri G-080'e kaldı |
| 38 | Depozito enflasyon istismarı | **Kapandı** (G-077) | Şehir mağazaları da G-086a ile kalktı |
| 39 | Ücret asgari altında, SGK/kıdem yok | **Kontrol edilmeli** | Ücret asgari ücret endeksini izliyor; `MarketStaff.cpp`'de bir kıdem hesabı görünüyor. SGK ve alt sınır G-080'de |
| 40 | Toptancı güveni şişiriliyor | **Kapandı** (G-077) | |
| 41 | Gecikme faizi abartılı | **Açık** | Hiçbir görevde kapandığı yazmıyor |
| 42 | Muhasebe modülden modüle farklı | **Kısmen geçersiz** (M01, G-086a) | Şehir mağazası kalktı. Yurt dışı kur yok (L05, L08 bekliyor). Şube cirosu ve KDV tutarlılığı **kontrol edilmeli** |
| 43 | Zarar devri yok, son gün indirimi, ipotek ödül gibi | **Açık** | |

### 2.6 Dünya, şube, şirket

| # | Kısaca | Durum | Not |
|---|---|---|---|
| 44 | Dünya liderliği aslında mahalle liderliği | **Açık** (koda bakıldı) | `MarketCompany.cpp:198`: hâlâ yerel pay ≥%40 + 60 mağaza. "İstasyon semti" adı M01 ile kalktı ama ölçüt aynı. G-082'de lig |
| 45 | Ulusal pay formülü şişik | **Kısmen** (koda bakıldı) | Artık nüfusa bölünüyor, ama hâlâ mağaza sayısı × 0,04 (`MarketCompany.cpp:66`). Ciroya bağlanmalı |
| 46 | 15 şehir ekonomik olarak aynı | **Geçersiz** (M01, G-086a) | 15 şehir kalktı; iller nüfustan alım gücü, kira ve rekabet alıyor. Yeni il ekonomisinin dengesi **kontrol edilmeli** |
| 47 | Yatırım 60–110 günde dönüyor | **Kontrol edilmeli** | Şube modeli G-086a'da yeniden yazıldı; geri dönüş süresi ölçülmedi |
| 48 | Hava ve takvim tek | **Kısmen** | Ülke takvimi ülkeye göre (G-084 3. parça). Ama iller arasında hava hâlâ ortak tohumdan geliyor (`MarketBranches.cpp:571`) |
| 49 | Lüleburgaz semt ölçeği 10 kat düşük | **Geçersiz** (M01) | Semtler kalktı. Yeni ölçek (40 bin kişiye 1 yer) **kontrol edilmeli** |
| 50 | Harita verisi sentetik ve 2011'de donmuş | **Kısmen** | İl değerleri nüfustan türetiliyor (G-086a). "2011'de donmuş" L06 ile önemini yitirdi. Nüfus hâlâ büyümüyor |
| 51 | "İkinci şube" hedefi erken tamamlanıyor, README "950 TL" | **Kısmen geçersiz** (M01, G-086a) | Şube açma yeniden yazıldı; hedefin ne zaman tamamlandığı **kontrol edilmeli**. `README.md:38` hâlâ "950 TL" diyor: **açık** |

### 2.7 Toplam

Her hata tek bir ana durumda sayıldı.

| Durum | Sayı | Numaralar |
|---|---|---|
| Kapandı | 20 | 1, 2, 3, 4, 5, 10, 11, 14, 15, 16, 17, 20, 22, 26, 33, 34, 35, 36, 38, 40 |
| Kısmen | 11 | 6, 9, 18, 24, 28, 29, 32, 37, 45, 48, 50 |
| Geçersiz ya da kısmen geçersiz | 6 | 13, 23, 42, 46, 49, 51 |
| Açık | 9 | 7, 8, 12, 19, 27, 30, 41, 43, 44 |
| Kontrol edilmeli | 5 | 21, 25, 31, 39, 47 |

20, 23, 46 ve 49'da da ayrıca kontrol edilmesi gereken birer ayrıntı var (tablolarda yazıyor).

---

## 3. Eski yol haritasının yeni kararlarla çelişen kısımları

### 3.1 Görev görev

| Görev | Eski kapsam | Hangi kararla çelişiyor | Yeni kapsam |
|---|---|---|---|
| **G-079** Aşama 2 "Yaşayan mahalle" | Semt verisi (hane, segment, satış noktaları), Lüleburgaz'da bakkal/pazar/fırın, Bereket | M01 (semt yok, yalnız il), L02 (her ülkede başlangıç), L10 (ülkeye göre geleneksel rakipler) | **"Yaşayan il pazarı":** her ilde rakip havuzu ilin rekabet değerinden kurulur. Bakkal, pazar, fırın ve aile marketinin adı ile türü ülke paketinden gelir (kiosk, Wochenmarkt…). Müşteri gruplarına göre mağaza seçimi (logit) il düzeyinde çalışır. "Bereket" Türkiye'deki yerel aile marketinin adı olarak kalır; öteki ülkelerde aynı rol paketteki adla gelir. Zincirlerin bilançosu ve aylık kararı aynen kalır. Yerel uzman toptancılar (G-083'ün ilk kısmı) birlikte gelir |
| **G-080** Aşama 3 "Rol katmanları ve zaman" | Tezgâh → mağaza müdürü → bölge → genel müdür → holding; haftalık/aylık/çeyrek tur; ilişki kaydı; ücret/SGK | M05–M06 (yönetim kademeleri ve 5 kişi sınırı; G-086b bunu yapıyor), L07 (birinci şahıs hiç kapanmaz), A02 (menü zamanı çözüldü), L09 (göreli takvim) | Rol kademeleri G-086b'ye taşındı. G-080'de kalan: **ilerletme işi yapan kişinin becerisine bağlı** (#8), kapalı dükkânda saat, **haftalık/aylık/çeyrek tur** (#12, #13), göstergeler paneli, **çift taraflı muhasebe defteri** (#37'nin kalanı, #42, #43), ücret alt sınırı ve SGK (#39), karakter ilişki kaydı. Karakter adları ülke paketinin isim havuzundan gelir |
| **G-081** Aşama 4 "Türkiye" | 81 il hücresi, zaman serili rakip mağaza sayıları, A/B/C yer, dağıtım merkezi, toptancı sözleşmesi, özel marka, 2018/2020/2021–23 makro olayları | M01, M07, M11 (81 il, bölgeler ve alt bölge depoları **G-086a'da yapıldı**), L02 (Türkiye tek başlangıç ülkesi değil), L06/L09 (gerçek yıl yok, olaylar adsız) | **"Ülke içi büyüme" (her ülke için):** il içinde A/B/C yer kalitesi ve rakiple yer yarışı; rakip mağaza sayısı gerçek yıla göre değil oyun ilerlemesine göre artar; toptancı sözleşmeleri ve üst katman toptancılar (G-083); özel marka üreticisi; **adsız dönem olayları** ("kur şoku", "salgın", "yüksek enflasyon"): sırası belli, zamanı tohumla değişir, ülkenin ekonomi karakterine göre sıklığı farklıdır |
| **G-082** Aşama 5 "Dünya" | Yeni `world.json` (25–35 ülke, 2011–2024 gerçek seriler), gerçek dev adları, J02 son tarihi 31.12.2040 | L05/L08 (kurgu kurlar), L06 (gerçek yıl yok), L10 (`ulkeler.json` zaten ülke paketi), L12 (kurgu adlar), M10 (Bulgaristan/Romanya dünya aşamasında) | Ayrı `world.json` yazılmaz; **`ulkeler.json` genişler** (yeni ülkeler + büyüme eğrisi + giriş yolları). Kurlar kurgu, ekonomi karakterine göre. Küresel 50 ligi **kurgu devlerle** (Stateline gibi). Son tarih gerçek tarih değil, **göreli** ("30. yılın sonu" gibi; karar gerekli, §5). Ülke ücret/kira çarpanları müşteri harcamasıyla birlikte burada açılır (G-084'te bilerek kapalı bırakıldı). #44 ve #45 burada kapanır |
| **G-083** Tedarik ağı | Yerel toptancılar G-079 ile, üst katmanlar G-081/G-082 ile; "Trakya distribütörü" | M07 (alt bölgeler her ülkede), L10 | Katmanlar aynı kalır. "Bölgesel distribütör" alt bölge başına kurulur. Toptancı adları ve kişilikleri (Selim, Trakya Gıda) Türkiye paketinde kalır, öteki ülkelerde paketten gelir |
| **G-087** Gezilebilir mağazalar | İl × tür başına gezilebilir mağaza, aile dükkânına müdür, raf şablonu paylaşımı | M17 (hazır mağaza kiti, G-088) | Gezilebilir mağaza kısmı **G-088 Aşama C'ye** geçti. G-087'de kalan: aile dükkânına müdür atamak (oyun haritadan açılır) ve raf düzenini aynı türdeki mağazalara uygulamak |

### 3.2 `02_DERIN_INCELEME.md` §3'te artık geçerli olmayan varsayımlar

| Yer | Eski varsayım | Yeni karar |
|---|---|---|
| §3.1 "İlerleme" 1–3 | Lüleburgaz (2011) → Trakya → Türkiye | M01–M03: herhangi bir ülke ve il; bütün ülke baştan açık |
| §3.1 ülke profili, referans tablosu | Gerçek yıllar, gerçek şirket adları (Walmart…) | L06, L08, L12: göreli zaman, kurgu kur, kurgu adlar |
| §3.2 aktörler | Bereket, "Salı Pazarı", "Halk Ekmek" Türkiye'ye özel | L10: aynı roller ülke paketinden adla |
| §3.4 makro dönemler, §3.6 hikâye yılları, §3.7 kilometre taşları | 2018, 2020, 2021–23; "Mart 2011" | L09: adsız olaylar, "1. yıl · İlkbahar" gibi göreli takvim |
| §3.5 katmanlı rol | Tezgâh → holding CEO | M05: mağaza müdürü → il müdürü → bölge müdürü → bölge direktörü → ülke müdürü; oyuncu genel müdür, 5 kişi sınırı |
| §3.5 tekrar oynanabilirlik | Farklı başlangıç şehri, mirasçı profili | L02 ve L03/L15 ile kısmen yapıldı (il seçimi, akraba tohumla) |
| §4 açık sorular | Menüde zaman, gerçek zincir adları | A02 ve L12 ile kapandı |

### 3.3 Belgelerdeki tutarsızlıklar (düzeltilmeli)

- **A12 ve L12 çelişiyor.** A12 "gerçek zincir adları kalır" diyor. L12 (daha sonra verildi) ise varsayılanın kurgu ad olduğunu söylüyor ve kodda `useFictional: true` var. A12 satırı L12'ye göre güncellenmeli.
- **J02'nin son tarihi 31.12.2040** gerçek bir tarih; L06 ile çelişiyor.
- **AGENTS.md haritası eski:** `MarketBranches` için "7 kurgu semt", `MarketCompany` için "Lüleburgaz dışı şehir mağazaları" yazıyor. `MarketCountry`, `MarketStart`, `MarketTheme` ve `MarketStoreAssign` haritada yok.
- `README.md` "950 TL ile ikinci şube" diyor (#51).
- GOREVLER'de G-084 3. parçanın durumu (§1.2).

---

## 4. Yeni sıra

İlk 3–4 adım net, sonrası kaba. Aynı dosyaya iki ajan aynı anda dokunmaz (AGENTS §8).

### 4.0 Adım 0: doğrulama borcunu kapat (herkes, önce bu)

| Kim | İş | Bitti sayılma şartı |
|---|---|---|
| Codex | `DERLE.cmd` + `TEST.cmd` (G-086f + `MarketStoreAssign`) + `SmokeTest.ps1` | Derleme geçti; en az 51 test + yeni `StoreAssign` testleri geçti; smoke geçti. Sonuç GUNLUK'te. Smoke il seçim ekranında takılıyorsa düzeltme notu Claude'a |
| Mustafa | Yeni oyun: ülke ve il seç, birinci şahısla ilk günü oyna, M ile ana ekran, sipariş penceresi, kayıt yuvası | Gördüğü sorunlar kısa bir listede |

### 4.1 Claude şeridi (oyunun aklı)

| Sıra | İş | Bitti sayılma şartı | Bağımlılık |
|---|---|---|---|
| **C1** | **Belge temizliği:** §3.3'teki tutarsızlıklar; `MarketStoreAssign` GUNLUK/GOREVLER/AGENTS'a; G-079…G-083 satırları §3.1'deki yeni kapsamla; 02'nin başına "yol haritası 05'te" notu | Belgeler kendi içinde çelişmiyor; GOREVLER'deki her satırın durumu gerçek | Yok. Hemen yapılabilir |
| **C2** | **Aşama 0c: oyun testi öncesi açık hatalar.** #19 (kampanya adedi yerine indirim derinliği), #24 (raftaki fiyat kararı esnek olsun), #27 (maliyet altı uyarısı, destekli kampanya raporu), #41 (gecikme faizine tavan), #30 (online kampanya), #21/#25/#31'in kontrolü, README | Her hata için yeni ya da güncel test; Codex derleme + test + smoke geçti; GUNLUK'te kapanan numaralar | Adım 0 (temiz bir başlangıç) |
| **C3** | **G-086b: yönetim kademeleri.** Müdür tarzı ve karne kararları (prim, uyar, değiştir, terfi); il müdürü, bölge müdürü, bölge direktörü, ülke müdürü; 5 kişi sınırı cezası; Yönetim sekmesi | Yeni testler (karne, sınır aşımı cezası, kademeye bağlanma); derleme + test + smoke; Mustafa menüde bir il müdürü atayabiliyor | G-086f derlenmiş olmalı (aynı menü dosyaları) |
| **C4** | **G-088 Aşama C: mağaza kitinin oyuna bağlanması.** Atamanın kayda yazılması (`FMarketState`), menüde "Mağazayı gez", `stats` → çeşit, kuyruk, tazelik, depo, açılış bedeli ve kira; raf kategorisi seçiminin şube mağazasında kaydı (M18) | `StoreAssign` testleri geçiyor; Codex'in 4 mağazasından biri menüden açılıp geziliyor; kayıt-yükleme sonrası aynı görünüm geliyor | Codex X2 (`magazalar.json` + `MarketStoreKit`). Mantık kısmı önceden yazılabilir (başladı) |
| C5 | G-079 kalanı ("yaşayan il pazarı", §3.1) + G-083'ün yerel uzman toptancıları | Kaba | C2 |
| C6 | G-080 kalanı (tur sistemi, ilerletme beceriye bağlı, muhasebe defteri, ücret/SGK, ilişki kaydı) | Kaba | C3 |
| C7 | G-084 kalanı: dünya haritalı başlangıç, metin tablosu (L14 onayıyla), rakiplerin pazar yasası | Kaba | L14 kararı |
| C8 | G-081 "ülke içi büyüme" (§3.1) | Kaba | C5, C6 |
| C9 | G-082 dünya ve lig; J02 yeni son koşulu | Kaba | C8, L08 ve J02 tarih kararı |
| C10 | G-087 kalanı (aile dükkânına müdür, raf şablonu paylaşımı) | Kaba | C3, C4 |

### 4.2 Codex şeridi (mağaza kiti, 3B, derleme/test)

| Sıra | İş | Bitti sayılma şartı | Bağımlılık |
|---|---|---|---|
| **X0** | Adım 0: derleme + test + smoke (§4.0) | GUNLUK'te sonuç | Yok |
| **X1** | **M18 aile dükkânında:** Türkçe harfli 3B yazı tipi, `UpperTurkish` (+ test), T ile raf kategorisi seçimi, "bu reyona ait değil" uyarısı | Tabelada İÇECEK, KURU GIDA doğru yazıyor; kategori değişikliği kayıt-yükleme sonrası kalıyor; test + smoke geçti; Mustafa gözle onayladı | X0 |
| **X2** | **G-088 Aşama A:** `Config/magazalar.json`, `MarketStoreKit` (+ testler), `validate_stores.py`, her türden 1 mağaza, `-MirasStorePreview` | 4 mağaza doğrulamadan hatasız geçiyor; her biri için 5 açıdan görüntü; hipermarkette ≥45 fps; Mustafa onayı | X1 (aynı yazı tipi ve tabela kodu) |
| **X3** | **G-088 Aşama B:** kalan 16 mağaza | 20 mağaza doğrulamadan geçiyor; aynı türün mağazaları gözle belirgin farklı | X2 onayı |
| X4 | Claude'un her derlenmemiş işini derleyip test etmek (sürekli) | Her Claude işi için GUNLUK'te derleme/test/smoke satırı | Claude'un teslimi |
| X5 | Sonra (kaba): L13 onaylanırsa maket (dollhouse) görünümünün sahnesi; dondurulanlar (G-021 servis reyonları, G-042 MetaHuman çeşitliliği); G-011 dönem etiketleri (dönemler L09'a göre oyun ilerlemesine bağlanır) | Kaba | L13 kararı, G-055 |

### 4.3 Mustafa'nın yapacakları

| Sıra | İş | Ne zaman |
|---|---|---|
| M1 | Adım 0'daki elle deneme | Hemen |
| M2 | §5'teki kararlar (özellikle L08, L09, L14, J02 tarihi) | Hemen; C7 ve C9'dan önce şart |
| M3 | X1 ve X2 önizlemelerini onaylamak | Codex teslim edince |
| M4 | **G-055 oyun testi:** yeni biri ilk 30 dakikada sipariş → satış → rapor döngüsünü anlıyor mu; fiyat, talep, maaş, borç sayıları | C2 bitince (denge hataları giderilmiş olsun) |
| M5 | İlk 20–40 ürünün gerçek ambalajı (paralel) | İstediği zaman |

### 4.4 Sıra özeti

```
Adım 0 (Codex derler/test eder, Mustafa dener)
   ├─ Claude: C1 belge → C2 Aşama 0c → C3 G-086b → C4 G-088 C → C5… 
   ├─ Codex:  X1 Türkçe tabela → X2 4 mağaza → X3 16 mağaza  (+ her Claude işini derler)
   └─ Mustafa: kararlar → C2'den sonra G-055 oyun testi → X2 onayı
```

---

## 5. Mustafa'nın vermesi gereken kararlar

| # | Karar | Claude önerisi |
|---|---|---|
| 1 | **L08 Kurlar:** Para birimi adları gerçek, kurlar kurgu mu olsun? | Evet. Her ülkenin ekonomi karakteri (istikrarlı / oynak / yüksek enflasyonlu) olsun; lig tek ortak birimde sıralansın |
| 2 | **L09 Zaman:** Takvim "1. yıl · İlkbahar" gibi göreli mi olsun, olaylar adsız mı gelsin? | Evet. Bugün kodda gün ve ay var, yıl göreli; bu hâli koruyalım |
| 3 | **L10 Ülke profilleri:** Her ülke veriyle tanımlı bir paket mi olsun? | Evet (kısmen kodda). Mod desteği yayından sonra |
| 4 | **L11 Başlangıç zorluğu:** Haritada her ilin rekabet, alım gücü, kira ve büyüme yıldızları görünsün mü? | Evet. İl panelindeki dört kutu yeni oyun ekranında yıldızla gösterilsin; küçük il kolay, büyükşehir zor |
| 5 | **L13 Maket görünümü:** Dükkânlar izometrik maket olarak da gösterilsin mi? | Şimdilik ertelensin. Önce birinci şahıs, harita ve mağaza kiti otursun; G-055 testinden sonra yeniden bakalım |
| 6 | **L14 Dil:** Türkçe ve İngilizce baştan mı olsun? | Metin tablosu altyapısı şimdi kurulsun, İngilizce çeviri yayından önce yapılsın |
| 7 | **L15 Başlangıç kasası:** Kasada 3 çalışanın 1 haftalık maaşı olsun mu? (Kodda var) | Evet, onaylansın. G-055 testinde bakılır |
| 8 | **Oyun sonu tarihi (J02):** 31.12.2040 yerine göreli bir son mu olsun? | "30. yılın sonu" olsun. Ligde birincilik ondan önce gelirse son erken gelir |
| 9 | **A12 ile L12 çelişkisi:** Zincirler varsayılan olarak kurgu adla mı görünsün? | Evet, L12 geçerli. Gerçek adlar yalnız geliştirme anahtarıyla (F8) görünsün |
| 10 | **Türkiye'ye özel karakterler:** Nermin teyze, Cem, Selim, Kadir Bereketoğlu öteki ülkelerde ne olsun? | Türkiye'de aynen kalsınlar. Öteki ülkelerde aynı roller ülke paketinin isim havuzundan gelsin |
| 11 | **Dönem olayları** (kur şoku, salgın, yüksek enflasyon): Sabit sırada mı, rastgele mi gelsin? | Sıra sabit kalsın, zamanı her oyunda ±2 yıl kaysın. Sıklığı ülkenin ekonomi karakterine bağlı olsun |
| 12 | **Zirveye süre:** Dünya birinciliğine kaç saatte ulaşılabilsin? | 40–60 saat; "dünya ölçeği" ayarıyla |
| 13 | **G-087 ile G-088'in birleşmesi:** Gezilebilir mağaza işi tek görev mi olsun? | Evet. Gezme G-088 C'de; G-087 yalnız aile dükkânına müdür ve raf şablonu paylaşımı |
| 14 | **Oyun testinin (G-055) zamanı:** Aşama 0c'den sonra mı yapılsın? | Evet. Kampanya adedi istismarı (#19) ve katı fiyat kararı (#24) düzelmeden yapılan test yanıltır |
