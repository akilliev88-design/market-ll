> **ARŞİV (06.10.2026):** Bu belge M69 öncesi kurguyu ve eski planı anlatır (2011, Lüleburgaz, "Miras Market", babadan kalan dükkân, hikâye bölümleri). **Güncel değildir; ajanlar buna göre kod ya da metin yazmasın.** Güncel kurgu `Docs/Kurgu/00_KURGU_KITABI.md`, kararlar `Docs/Kurgu/01_KARARLAR.md`, durum `Docs/Surec/DURUM.md`.

# Miras Market — ana plan, sürüm 0.2

Tarih: 27 Eylül 2026. **Bu paket yalnızca tasarımdır. Buradaki editörler, menüler ve sistemler henüz geliştirilmedi.** Oyun koduna, prototip ayarlarına ve varlıklarına müdahale edilmez. Önceki v0.1 belgeleri prototipin tarihçesidir; hedef oyun için bu paket önceliklidir.

## Tasarımın merkezi

Oyuncu, 2010'lar Lüleburgaz'ında devraldığı aile marketini; bizzat çalışarak, insan yetiştirerek, markasını kurarak ve ticari kararlar vererek uluslararası bir perakende grubuna dönüştürür. Marketin içinde yaşananlar ile şirket raporları aynı ekonomik olayların iki görünümüdür.

Her ayrıntının bir işlevi olmalıdır: karar, maliyet, sonuç, geri bildirim veya atmosfer. Gerçek hayattaki her evrakı oyuncuya tekrar yazdırmak derinlik sayılmaz. Sık tekrarlanan işler öğretildikten sonra personele devredilebilir.

## Kullanıcı tarafından belirlenmiş gereksinimler

1. Karma oynanış: markette birinci şahıs işler ve şirket yönetimi.
2. Unreal Engine; bu aşamada yalnızca planlama.
3. Tesla uygulamasını çağrıştıran modern, sade menü dili.
4. Sipariş ve yönetim işlemlerinin menülerden yapılması.
5. Ürün kutularında gerçek marka etiketleri; model/etiket üretimi başka araçlarda yapılacak.
6. Bu dış üretimi yönetecek standartlar ve kopyalanabilir promptlar.
7. Küçük dükkândan hipermarkete kadar farklı ekipman ve hizmet bölümleri.
8. İnandırıcı müşteri hareketleri, otomatik mağaza yerleşimi ve kurumsal kimlik.
9. Şirket satın almaları, rakiplerin oyuncuyu satın alma girişimleri, uluslararası rekabet ve hükümetlerle ilişkiler.
10. Yöneticiler, departmanlar, yetki devri, iç suistimal ve denetim.

## Öneri olarak kabul edilen varsayımlar

Başlangıç 2011, Windows PC, tek oyunculu ve çevrimdışı çalışabilen ana kampanya. Modern menüler oyuncuya ait bir yönetim katmanıdır; 2011'deki karakterin elinde günümüz telefonu varmış gibi sunulmaz. İlk mağazanın mülkiyeti ailede; kalan yükümlülükler senaryo seçimine bağlıdır. Şehir coğrafyası esinlenilmiş, dükkânlar ve dramatik olay kişileri kurgudur.

Oyun içindeki kesin fiyatlar, süreler, metrekare eşikleri, üretim miktarları ve performans bütçeleri henüz onaylanmış denge değerleri değildir. Ayrıntı seviyesi, sanat tarzı, zaman sıkıştırması ve üretim bütçesi ileride pilotlarla doğrulanacak.

## Belgeler

| Belge | Yanıtladığı soru |
|---|---|
| [01 — Dünya ve oynanış](01_DUNYA_VE_OYNANIS.md) | Hikâye, günlük hayat, büyüme, lojistik, teknoloji, unutulabilecek sistemler |
| [02 — Menü ve kullanıcı deneyimi](02_MENU_VE_DENEYIM.md) | Tesla esintisi nasıl bir oyun arayüzüne dönüşecek? |
| [03 — Ürün Stüdyosu ve dosya standardı](03_URUN_STUDYOSU_VE_VARLIK_STANDARDI.md) | Dışarıda üretilen kutu ve etiket oyuna nasıl güvenilir biçimde girecek? |
| [04 — Mağazalar, raflar ve bölümler](04_MAGAZA_RAFLAR_VE_BOLUMLER.md) | Hangi formatlar, teşhir biçimleri ve servis operasyonları olacak? |
| [05 — Davranış, otomatik yerleşim ve öğrenme](05_YAPAY_ZEKA_VE_OTOMATIK_YERLESIM.md) | Neyi kodlayacağız, nerede optimizasyon veya eğitim kullanacağız? |
| [06 — Şirket, rekabet ve finans](06_SIRKET_REKABET_VE_FINANS.md) | İnsanlar, para, suistimal, satın alma ve ülke riskleri nasıl işleyecek? |
| [07 — Dış araçlara verilecek promptlar](07_PROMPT_KUTUPHANESI.md) | Hangi sırayla, hangi girdilerle, ne üretilecek? |
| [08 — Üretim ve doğrulama planı](08_URETIM_VE_DOGRULAMA.md) | Araçlar, aşamalar, kabul kapıları, kaynak planı ve riskler |
| [09 — Kaynaklar ve karar kaydı](09_KAYNAKLAR_VE_KARARLAR.md) | Hangi bilgi doğrulandı, hangi tercih tasarım önerisi, ne açık? |

## Temel kararlar

| Alan | Önerilen yaklaşım | Gerekçe |
|---|---|---|
| Menüler | Sade genel görünüm + gerektiğinde yoğun tablo + ayrıntı paneli | Binlerce üründe hem estetik hem verimli kullanım |
| Ürün hazırlama | Şablon kutular + dış modeller + doğrulamalı Ürün Stüdyosu | Model ve marka üretimini tekrar kullanılabilir hale getirir |
| Raf | Ekipman yetenekleri + ölçülü yerleşim bölgeleri + planogram | Her ürün için ayrı raf kodu yazmayı önler |
| Müşteri | Karar puanlama + durum makinesi + yol bulma | Kontrol edilebilir, açıklanabilir ve test edilebilir davranış |
| Otomatik mağaza | Kısıtlı yerleşim araması + trafik değerlendirmesi | Önce kullanılabilirlik, sonra kâr/estetik hedefleri |
| Öğrenen modeller | İlk sürüm için zorunlu değil; ölçülmüş ihtiyaç sonrası deney | Veri olmadan eğitim vaadi verilmez |
| Ekonomi | Tek işlem kaynağı; aktif ve uzak şubeler ortak kurallara bağlı | Para/stok çoğalmasını ve tutarsız sonuçları engeller |
| Küresel oyun | Bölgesel özet simülasyon; ziyaret edilen mağazada ayrıntı | Bütün dünyayı sürekli fiziksel NPC'lerle çalıştırmaz |
| Yöneticiler | Yetki, hedef, raporlama ve denetim | Büyüme oyuncuyu mikro işlere gömmez |

## Kullanıcının dış üretime başlama sırası

Önce 03 ve 07 okunur. İlk üretim paketi yalnızca bir dikdörtgen kutu, bir şişe ve bir raf modülüdür. Ölçü/UV/import uyumu doğrulanmadan yüzlerce ürün üretilmez. Bu turda model, etiket, eğitim verisi veya çalışır editör üretimi yapılmadı; aşağıdaki klasörler gelecekte kullanılacak teslim standardıdır.

## Başarının üç ayrı ölçüsü

- **Oyuncu deneyimi:** kararların neden-sonuçları anlaşılır; büyüme hissedilir; elle çalışma gönüllü kalabilir.
- **Simülasyon doğruluğu:** para ve mal hareketleri izlenebilir; kurallar tüm firmalara tutarlı uygulanır.
- **Üretim güvenilirliği:** dış varlık kabul kontrolünden geçer; güncelleme kayıtları bozmaz; işlem geri alınabilir.

Bu ayrım, güzel görünen bir ekranı çalışan ekonomiyle veya başarılı bir model çıktısını sorunsuz oyun varlığıyla karıştırmayı önler.
