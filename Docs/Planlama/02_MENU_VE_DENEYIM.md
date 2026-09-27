# Menü ve kullanıcı deneyimi şartnamesi

Durum: tasarım; ekran veya oyun kodu üretilmedi. Tesla referansı, temiz durum/komut sunumuna yöneliktir. Aşağıdaki renkler, ölçüler ve akışlar Miras Market için öneridir; Tesla'nın resmî tasarım değerleri değildir.

## 1. Görsel yaklaşım

Tesla uygulamasının resmî tanıtımında ürünün durumu ile temel kontroller aynı deneyimde sunuluyor. Oyunda bundan alınacak ilke, seçili işletmenin durumunu öne koymak ve önemli eylemleri kolay erişilir tutmaktır. [Tesla uygulaması](https://www.tesla.com/support/tesla-app), [resmî kontrol tanıtımı](https://www.tesla.com/support/videos/watch/vehicle-controls-tesla-app).

Önerilen görünüm: koyu grafit veya kırık beyaz yüzeyler, geniş boşluk, sade tipografi, az sayıda vurgu rengi, ince çizgili ikonlar, büyük şirket/mağaza görseli ve açık durum satırları. Şirketin kimlik rengi vurgu olarak uygulanır. Tesla logosu veya otomobil ekranı düzeni kullanılmaz; telefon uygulaması masaüstüne boyutlandırılıp bırakılmaz.

Başlangıç tasarım değerleri:

| Öğe | Öneri |
|---|---|
| Koyu zemin | #111315 |
| Panel | #1B1E22 |
| Ana yazı / yardımcı yazı | #F4F5F6 / #AAB1BA |
| Açık tema | #F4F5F7 zemin, #FFFFFF panel, #171A1E yazı |
| İlk marka vurgusu | #71C6AC; şirket kimliğine göre değişebilir |
| Durumlar | Başarılı, uyarı, hata için ikon + metin + renk |
| Aralık sistemi | 4/8/12/16/24/32 temel birim |
| Gövde yazısı | 1080p'de başlangıç adayı 16–18 px; ölçeklenebilir |
| Eylem alanı | Başlangıç adayı en az 44 px yükseklik; kullanıcı testiyle ayarla |
| Geçiş | 120–200 ms hafif hareket; azaltılmış hareket seçeneği |

Tüm renk kombinasyonlarının kontrastı ayrıca ölçülür. Seçili renk paleti tek başına erişilebilirlik garantisi değildir. Türkçe karakterler ve farklı para birimi sembolleri font test setine girer.

## 2. Ana menü

Arka planda kampanyanın mevcut aşamasını temsil eden sakin market görünümü. Gürültülü video zorunlu değildir.

- Devam et: son kayıt adı, tarih, şirket, mağaza sayısı ve kayıt sürümü.
- Kampanya yükle: otomatik/manuel kayıtlar; küçük önizleme ve hata durumu.
- Yeni kampanya: karakter, miras senaryosu, isim, zorluk ve zaman tercihleri.
- Serbest oyun: başlangıç sermayesi, açılmış sistemler ve olay yoğunluğu.
- Ayarlar: görüntü, ses, kontrol, arayüz, erişilebilirlik ve dil.
- İçerik durumu: eksik ürün paketlerini açıklayan kısa ekran; teknik ayrıntı isteğe bağlı.

Ürün Stüdyosu başlangıçta geliştirme/üretim aracı olarak Unreal Editor içinde çalışacak. Paketlenmiş oyunun ana menüsünde varmış gibi sunulmaz. İleride oyuncu mod araçları ayrı ürün kararıdır.

## 3. Oyun içi bilgi mimarisi

Sol ana gezinme: **Özet, Mağazalar, Ürünler, Siparişler, Fiyatlar, Personel, Finans, Lojistik, Pazar, Şirket.** Başlangıçta kullanılmayan bölümler sadeleştirilir; mağaza büyüdükçe açıklamalı biçimde açılır.

Üst bağlam satırı: şirket → ülke/bölge → mağaza seçici, tarih/saat, zaman durumu, arama, bildirim. Her ekran kapsamını belirtir: "Lüleburgaz Merkez" ile "Tüm Türkiye" karıştırılmamalı.

Özet ekranı: seçili marketin 3B/2B temsili, nakit, bugünkü faaliyet durumu, stok bulunurluğu, personel durumu ve en önemli üç istisna. Ciro ve kâr aynı başlık altında birbirinin yerine kullanılmaz.

Detay ekranında büyük ürün görseli ve kısa özet korunur. Çok ürünlü sipariş ve finans işlemleri için satır tablosu kullanılır. Bin ürünü dev kartlar içinde kaydırmak zorunlu tutulmaz.

Önerilen masaüstü yerleşimi:

```text
Şirket / Ülke / Seçili mağaza        Tarih  DURAKLATILDI    Ara  Bildirim
┌─────────────┬──────────────────────────────────┬──────────────────┐
│ Ana gezinme │ Başlık, filtre ve bağlam          │ Seçili öğe       │
│             │ Özet kartları / veri tablosu     │ Önizleme         │
│             │                                  │ Etki özeti       │
│             │ Liste veya mağaza planı          │ Birincil eylem   │
└─────────────┴──────────────────────────────────┴──────────────────┘
```

Sağ panel gerektiğinde kapanır. 16:9, 16:10 ve geniş ekranlarda içerik okunabilir kalır; küçük çözünürlükte panel alt çekmeceye dönüşebilir. Modal üstüne modal zinciri yerine tek ayrıntı paneli kullanılır.

## 4. Sipariş akışı

**Siparişler → mağaza/depo seç → ürün seç → tedarikçi karşılaştır → koli adedi → teslimat → özet → sipariş oluştur.**

Ürün tablosu: ad ve görsel, eldeki satışa uygun miktar, sepete/üretime ayrılan miktar, yoldaki miktar, günlük tahmin aralığı, tahmini tükenme günü, raf kapasitesi ve teklif birimi. Kullanıcı 12'li koliyi 12 koli sanmamalı; "3 koli × 12 adet = 36 adet" açıkça yazılır.

Teklif karşılaştırması: ürün maliyeti, nakliye, vergi gösterim tercihi, vade, minimum sipariş, koli katı, tahmini teslimat ve geçmiş güvenilirlik. Tedarikçinin yüzde indirimi toplam teslim maliyetiyle karıştırılmaz.

Sepet özeti: şimdi ödenecek, sonra ödenecek, teslimat sonrası depo doluluğu, tahmini stok süresi, nakit rezervi ve henüz kesin olmayan teslimat öğeleri. Önerilen sipariş miktarı gerekçesiyle gösterilir ve değiştirilebilir.

Durum zinciri: Taslak → Onaylandı → Tedarikçi kabul etti → Hazırlanıyor → Yolda → Kısmen/Tam teslim → Mutabakat tamamlandı. İptal/iade ayrı durumlar; aynı sipariş yeniden tıklamayla iki kez oluşmaz.

İptal koşulları siparişten önce görünür. Taslaktan çıkmak stok ve para yaratmaz. Fiyat/kapasite değiştiyse onay anında yeniden kontrol edilir ve fark kullanıcıya gösterilir. Nakliye ücreti son ekranda sürpriz olarak eklenmez.

Otomatik sipariş: hedef stok günü, minimum nakit, ürün/tedarikçi sınırı, azami sipariş bütçesi ve durma koşulları. Müdürün oluşturduğu taslaklar bir süre onaya düşebilir; güven/izin düzeyine göre daha sonra otomatikleşir.

## 5. Diğer temel ekranlar

| Ekran | İçerik | Ana eylem | Başarısızlık/uyarı örneği |
|---|---|---|---|
| Ürün | Görsel, satış birimi, dönem, kategori, fiyat geçmişi, parti | Ürün karmasına ekle | Ülke/yıl veya ekipman uyumsuz |
| Fiyat | Birim fiyat, maliyet, brüt marj, benzer sepet, tahmin | Yeni fiyat uygula/planla | Maliyet altı satış açıklaması |
| Kampanya | Hedef ürün, tarih, bütçe, stok güvencesi | Planla | Reklam talebini karşılamayan stok |
| Mal kabul | Sipariş karşılaştırması, miktar, hasar/uyuşmazlık | Kabul et/ayır | Eksik koli, uygunsuz parti |
| Personel | Kadro, vardiya, görev, ücret, eğitim | İşe al/görev ver | Vardiyada kritik rol boş |
| Müdür | Yetki sınırları, hedefler, istisnalar | Yetki düzenle | Bütçe dışı talep |
| Finans | Kâr-zarar, nakit akışı, bilanço özeti, borç takvimi | Ödeme/finansman planla | Yaklaşan nakit açığı |
| Mağaza planı | Ekipman, yollar, altyapı, trafik ve maliyet | Tasarım önizle/uygula | Çıkışa ulaşılamıyor |
| Şube açılışı | Konum, proje, bütçe, tarih, iş listesi | Yatırım başlat | Finansman veya operasyon eksiği |
| Pazar | Rakipler, sepet kıyası, pay, güven aralığı | Araştır/kampanya | Bilgi eski veya tahminî |
| Satın alma | Hedef, değerleme, finansman, inceleme, koşullar | Bağlayıcı olmayan teklif | Satıcı ilgisiz, borç belirsiz |
| Denetim | Sapma, kanıt, alternatif açıklamalar, erişim | İnceleme başlat | Kanıt yetersiz |
| Ülke | Talep, lojistik, politika, kâr/kur ayrımı | Pazara giriş planla | Risk senaryosunda sermaye açığı |

## 6. Fiziksel eylem ve menü ayrımı

Etkileşim tuşu rafı doldurma, nesne inceleme veya hizmet başlatma içindir. Satın alma, sipariş, işe alma, finansman ve şube açma menüde görünür bir seçimle yapılır. H/B/G gibi görünmez kısayollar yönetim işleminin tek yolu olmaz; istenirse ekran açma kısayolu olabilir.

Menü üzerinden seçilen iş bir emir üretir. "Rafa yerleştir" emri fiziksel/soyut görev oluşturur; uzak mağazadaki kutular anında ışınlanmaz. Yönetici ve çalışan kapasitesi işin süresini belirler.

## 7. Hata, bildirim ve karar güvenliği

Her ekran için yükleniyor, boş, sonuç bulunamadı, kısmi veri, yetki yetersiz, geçersiz seçim, başarı ve işlem sürüyor durumları tanımlanır. "Bir hata oluştu" yerine kararın engelini söyle: "Soğuk depo kapasitesi 24 koli yetersiz."

Bildirimler üç düzey: bilgi özeti, karar gerektiren istisna, ilerletmeyi durduran kritik durum. Tekrarlayan olaylar gruplanır. Her satış veya küçük fire ayrı bir modal açmaz.

Geri alınabilir düzenlemeler için geri al/yeniden yap. Büyük finansman ve hisse satışı için maliyet, sahiplik ve geri dönüş koşullarını gösteren açık son özet. Taslak ve bağlayıcı işlem görsel olarak farklı olmalı.

## 8. Öğretici ve erişilebilirlik

İlk gün tek seferde tüm ekranları öğretmez. Bir sipariş, bir mal kabul, bir yerleşim ve bir fiyat kararı üzerinden ilerler. Kavram açıklamaları örnek tutarla gösterilir: "100 TL'lik stok satın almak bugün nakdi azaltır; satılmayan kısmı stokta kalır."

Tuş atama, arayüz ölçeği, yüksek kontrast, renk dışı simgeler, hareket azaltma, altyazı ve tut/bas seçenekleri. Klavye odağı kaybolmaz; gamepad düşünülüyorsa liste ve modal odağı da tasarlanır. Kullanılabilirlik küçük font ve gri yazı uğruna feda edilmez.

Grafiklerde nominal/reel veya yerel/konsolide değer etiketi bulunur. Tahmin kesin sonuçtan ayrılır. Birim, para birimi, tarih ve kapsam görünür tutulur. Finans grafiğinde sıfır ekseni veya kırpılmış eksen açıkça belirtilir.

## 9. Kabul örnekleri

- Yeni oyuncu bir ürün için 3 koli/36 adet siparişi miktar ve ödeme zamanını doğru anlayarak verebilmeli.
- Şube değiştirince önceki şubenin sepeti yanlışlıkla yeni şubeye uygulanmamalı.
- Tekrarlı tıklama çift ödeme veya çift sipariş oluşturmamalı.
- Delege edilen siparişin gerekçesi, yapan kişi ve bütçe sınırı görünmeli.
- 1.000 ürünlük deneme listesinde arama, filtre ve toplu seçim akıcı olmalı; hedef donanımda ölçülmeli.
- 1080p ile 1440p'de, %100 ve büyük yazı ölçeğinde ana eylemler ekran dışında kalmamalı.
- Menü açıkken zaman tercihi tutarlı uygulanmalı; duraklatma finans/lojistik saatine de yansımalı.

Bir sonraki tasarım çıktısı, bu şartnameyi kullanan 8–12 ekranlık tıklanabilir UX maketi olabilir. Bu turda üretilmedi.
