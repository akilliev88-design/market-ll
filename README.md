# Miras Market

2011 Lüleburgaz'da aileden kalan marketi büyütme fikri için **Unreal Engine 5.8.3 ile ilk oynanabilir teknik prototip**. Birinci şahıs market işleri ile işletme yönetimini birleştirir. Tam oyun veya bitmiş görsel demo değildir.

## Oyna

En kısa yol: **`OYNA.cmd` dosyasına çift tıkla.** Bu makinedeki Unreal kurulumunu kullanır.

Bu bilgisayarda PowerShell üzerinden:

```powershell
cd C:\Users\mtass\Desktop\market-ll
.\Play.ps1
```

Alternatif: `MirasMarket.uproject` dosyasını Unreal Editor ile açıp **Play** tuşuna bas. Market geometrisi oyun başladığında üretilir. Proje henüz bağımsız bir Windows oyunu olarak paketlenmedi; Unreal kurulumu gerektirir.

İlk derleme gerekiyorsa `DERLE.cmd` (veya `Build.ps1`) çalıştır. Kaynak kod değiştikten sonra yeniden derle. Motor farklı klasördeyse betiklere `-EngineRoot 'D:\Epic\UE_5.8'` parametresi verilebilir. C++ derlemesi için Visual Studio C++ araçları ve Windows SDK gerekir; bu makinedeki araçlarla doğrulanır.

## Ürün Stüdyosu

Dışarıda hazırladığın kutu/etiket görsellerini ve modelleri oyuna eklemek için **`STUDYO.cmd`** (veya Unreal Editor'da **Tools > Ürün Stüdyosu**). Ölçü girip kutu şablonu oluşturursun ya da kendi modelini içe alırsın, etiket görsellerini yüklersin, ürün bilgisini yazarsın ve **Oyuna ekle** dersin; doku, materyal ve katalog dosyaları kendiliğinden hazırlanır. Ayrıntı: [Docs/URUN_STUDYOSU.md](Docs/URUN_STUDYOSU.md).

## Birlikte geliştirme

Claude ve Codex aynı klasörde sırayla çalışır. Kurallar [AGENTS.md](AGENTS.md); güncel durum [Docs/Surec/DURUM.md](Docs/Surec/DURUM.md), görevler [Docs/Surec/GOREVLER.md](Docs/Surec/GOREVLER.md), oturum geçmişi [Docs/Surec/GUNLUK.md](Docs/Surec/GUNLUK.md).

## İlk gün

1. WASD ve fareyle dolaş. Her rafa yaklaş ve **E** ile depodan doldur.
2. **O** ile marketi aç. Müşteriler ürün seçip kasaya gelir.
3. Sağdaki kasaya yaklaş, **E** ile bekleyen müşterinin ödemesini al.
4. Soldaki yönetim masasında **Tab** ile ürün seç; **B** ile koli sipariş et (varsayılan 12 adet, ürüne göre değişebilir). Ücret hemen düşer, mallar gün bitince depoya ulaşır.
5. Yönetim masasında **+ / −** ile seçili ürünün fiyatını değiştir. Rakibin kampanyasını ve kayıp müşteriyi izle.
6. Gün 4 dakika sürer. **O** ile erken kapatabilirsin; tamamlanmamış alışverişler kayıp sayılır ve o günün giderleri uygulanır.
7. Gün sonunda siparişleri rafa taşı. **O** ile yeni günü başlat.

İlk hedef: **950 TL, 3 kârlı gün ve %35 yerel müşteri payıyla ikinci şubeyi açmak.** Masada G'ye bas. Bu prototipte ikinci şubenin ayrı haritası yoktur; gün sonu net katkısı hesaplanır.

| Tuş | İşlev |
|---|---|
| WASD / fare | Hareket / bakış |
| E | Yakındaki rafı doldur veya kasada ödeme al |
| O | Günü aç / erken kapat |
| Tab | Masada ürün seç |
| B | Masada seçili üründen bir koli sipariş |
| + / − | Masada fiyatı 0,25 TL değiştir |
| H | Masada kasiyer al: 120 TL başlangıç + 20 TL/gün |
| J / K | Masada reyon görevlisi al (en fazla 3; 120 TL + 20 TL/gün) / çıkar |
| G | Masada ikinci şubeye yatırım |
| F8 | Gerçek marka adları / kurgu adlar |
| F5 / F9 | Kapalıyken kaydet / yükle |
| F6, ardından tekrar F6 | Kapalıyken yeni kampanya; 5 saniyelik onay aralığı |
| Escape | Oyundan çık; editör Play oturumunu da sonlandırabilir |

Editor Play sırasında fareyi serbest bırakmak için Unreal'ın Shift+F1 kısayolu kullanılabilir.

## Şu anda bulunanlar

Gezilebilir market ve depo alanı, katalogdaki ürün sayısına göre büyüyen raf düzeni (ürün sayısı sınırı yok), Ürün Stüdyosu'ndan gelen gerçek kutu ve etiketler, satışla azalan raf görüntüsü, metin olarak gerçek marka isimleri, fiyat değiştirme, stok/raf/depo ayrımı, ertesi gün teslimat, nakit kısıtı, müşteri sepeti ve kasa kuyruğu, sabır süresi, rakip indirim takvimi, basitleştirilmiş yerel müşteri payı, kasiyer otomasyonu, günlük maliyet/kâr raporu ve tek yuvalı kayıt.

Kayıt: `Saved/SaveGames/MirasMarket_Campaign_v1.sav`. Gün sonunda otomatik kayıt yapılır. Açılışta kayıt kendiliğinden yüklenmez; **F9** kullan. Gün ortası kayıt yoktur. Kaydedilmemiş hazırlık değişiklikleri çıkışta kaybolur. F6 ile sıfırlama mevcut dosyayı hemen silmez; sonraki manuel veya otomatik kayıt aynı yuvanın üzerine yazar.

Tüm tutarlar TL/kuruş cinsinden **kurgu denge değerleridir**; gerçek 2011 fiyatları değildir. Başlangıç nakdi 350 TL, her üründen 8 adet rafta ve 24 adet depoda miras stokudur. Günlük temel gider 22 TL'dir. Stok için ödenen para, satış gerçekleşmeden ikinci kez satılan mal maliyeti yazılmaz. Kasiyer başlangıç bedeli ve şube yatırımı nakitten düşer; prototipin günlük raporu yatırım harcamalarını içermeyen faaliyet sonucudur.

## Bilinen prototip sınırları

- Temel geometriler ve basit HUD vardır; gerçek ambalajlar, animasyon, ses, hikâye sahnesi ve ayrıntılı Lüleburgaz çevresi yoktur.
- Müşteriler geçici doğrusal rotalar kullanır, rafların içinden geçebilir. NavMesh ve gerçek sepet animasyonları sonraki aşamadadır.
- Raftaki kutu sayısı stoğu izler. Raf kapasitesi sabit değildir: raf planındaki önde adet × derinliktir ve oyun her seviyenin boş genişliğini ve raf derinliğini mevcut ürünlerle doldurur (`planograms.json` → `autoFill`).
- Mağaza açılışta tüm raf blokları dolu başlar. Duvar reyonları da raf planı ekipmanıdır; boş seviyeler aynı reyonun/kategorinin ürünleriyle dolar.
- Test modu (varsayılan açık, `DefaultGame.ini` → `bTestModeAtStart`): E rafı bedava doldurur, F3 tüm rafları doldurur, masada B bedava ve anında depoya getirir; F2 açar/kapatır. F1 paneller, F4 ışık havası (Sıcak / Aydınlık / Akşam). Motorun F1–F5/F9 hata ayıklama görünüm kısayolları kapatıldı.
- Kasiyer işlevsel otomasyondur; fiziksel çalışan modeli yoktur.
- Ürün koli teslimi depoya sayısal eklenir; elle koli taşıma henüz yoktur.
- Müşteri almak istediği miktarı rezerve eder; stok kasada düşer. Sepete girişteki fiyat korunur. Kuyruktan ayrılınca rezervasyon kalkar.
- Rakip kendi bütçesiyle karar veren tam yapay zekâ değildir; gün bazlı indirim uygular.
- Yerel müşteri payı bir oyun göstergesidir. Gelir ve stoktan bağımsız gerçek nüfus modeli yoktur.
- Kasa bakiyesi sabit giderlerle negatife düşebilir; ayrıntılı kredi, iflas ve toparlanma sistemleri henüz yoktur.
- İkinci şube günlük net katkı modelidir. İl/ulusal/uluslararası aşamalar yalnızca tasarım belgesindedir.
- HUD prototip metinleri ASCII Türkçedir. Tuş atama, ayarlar ve fareyle yönetim ekranı sonraki aşamadadır.
- Katalogdaki aktif ürün sayısının üst sınırı yoktur. Kayıt yüklenirken stok ürün kimliğiyle eşleştirilir: yeni ürün boş rafla gelir, kaldırılan ürünün stoğu düşer. Ürün kimliği değişirse o ürünün stoğu sıfırlanır.
- Yerel testler performans sertifikasyonu veya tüm oyunun manuel oynanış testi sayılmaz.

## Tasarım ve geliştirme

- [Tüm oyun tasarımı](Docs/OYUN_TASARIMI.md)
- [Aşamalar ve Unreal mimarisi](Docs/GELISTIRME_PLANI.md)
- [Ürün ve marka yapılandırması](Config/products.json)

Ekonomi ve kayıt testleri: önce derle, sonra `Test.ps1`. Test raporları `Saved/TestReports` altında oluşur. Materyal varlığı yeniden üretilecekse Unreal Editor'ın Python komutuyla `Tools/create_material.py` çalıştırılır; normal oynamak için gerekmez.

Gerçek oyun dünyasında otomatik etkileşim kontrolü: `SmokeTest.ps1`. Oyuncuyu raf ve masaya yerleştirir, etkileşim komutlarını çalıştırır, müşteri satışlarını bekler, günü kapatır ve kaydı yükler. Oyuncunun kampanyasını değiştirmeyen `MirasMarket_TestOnly` kayıt yuvasını kullanır. Fiziksel klavye/fareyle elle oynama testi değildir.
