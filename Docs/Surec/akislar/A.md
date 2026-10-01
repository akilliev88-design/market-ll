# Akış A — Codex

**Son teslim — C3 doğrulaması (01.10.2026, main):** DERLE + TEST 126/126 + Smoke + BranchVisitReview geçti; 88/88 menü PNG'si gözle incelendi. 12 uzun kampanya / 65.751 gün / 0 stok-satış hatası / 0 defter farkı. Derleme/test düzeltmesi 0 (hiçbir dosyada gerekmedi). Dengeli 10/20/30: ulusal 32/32/33, dünya 20/20/20; denge hedefi karşılanmadı. Beş denge bulgusu C3_30_yil_rapor.md'de, üç ana menü isteği aşağıdaki C3 bölümünde. Bu işin açık doğrulama adımı kalmadı; ayar ve menü düzeltmeleri C'ye bırakıldı.

Önceki A6 teslimi (01.10.2026): A1/A3/A4/A5/A6 hazır, DERLE + TEST 105/105 + Smoke geçti. A5 gerçek ağla 68/68 (örnek ağ yok). C teslimi, M27 ve bot push edildi; iki son uzun koşu 12 kampanya/65.751 gün/0 denetim hatası. Denge hedefi karşılanmadı: son dengeli 10/20/30. yılda ulusal 74/63/60, dünya 20/20/20. Raporlar ve C ayar önerileri aşağıda; kod sabitleri değiştirilmedi.

## Yapılanlar

- A0 main: daefd97; DERLE, TEST 84/84 ve Smoke geçti; GitHub'a gönderildi. Kaynak düzeltmesi gerekmedi. Test alt sınırı ve uyarıyla başarılı sayımı düzeltildi.
- akis-a / market-ll-A oluşturuldu; git lfs pull ve ilk DERLE geçti (77,94 sn).
- A1 commit 2415c5d (push edildi): MarketAutoPlay.h/.cpp/Tests.cpp, MirasAutoPlayCommandlet.h/.cpp, AUTOPLAY.cmd. Gerçek aktif katalog + planogram kapasiteleriyle MarketStart kurulumu; üç görünür karar tarzı, yalnız normal komutlar ve ortak fiyat adımı. Kayıt dosyalarına dokunmaz.
- MarketSimulation FDay: satış/sipariş/ana kapanış nakit eşitliği ve mal kabul korunum kontrolleri. AdjustPrice normal oyuncu ve bot için ortak; MarketGame eski fiyat hesabı bu işleve taşındı, kural aynı.

## Doğrulama

30.09.2026: A1 DERLE geçti. İlk 120 gün × 3 tarz × 3 tohum: 9 koşu, denetim hatası 0; 11,93 sn. Tüm tarzlar borç/nakit sıkıntısına girdi; ekonomi kuralı değiştirilmedi. Tam TEST 86/86 geçti. Son sipariş/İK/rapor ekleri DERLE + AutoPlay 2/2 ile doğrulandı; kısa bot 3×120 gün 1,5 saniye (60 sn altı). İlk pasif sipariş davranışı düzeltildi: büyük öneri bütçeye göre normal koli siparişlerine bölünür. Son 120. gün kasaları 7.534,55 / 14.259,81 / 20.657,32 TL; denetim hatası 0. Smoke GEÇTİ: 1 satış, sipariş, mal kabul, işe alma, gün kapama ve disk kayıt/yükleme.

## A3 — Zaman ve tur (a7304ca, push edildi)

- MarketSimulation::AdvanceTurn(State, CatalogBase, Products, ETurn::Day/Week/Month): FTurn sonucu; Requested, Played, Stop, Message, Total ve Period. Hafta tam 7 gün, ay mevcut gün dahil takvim ayının sonuna kadar; aradaki hafta raporu ayı/haftayı kesmez.
- EStop: Decision, NegativeCash, WeekReport, MonthReport, Chapter, ImportantEvent, InvalidCatalog, CampaignOver. Yeni fiyat savaşı, şube müdürünün yakalanması veya depo müdürünün yakalanması önemli olay olarak durdurur. Bekleyen kararlar ve kasa eksisi gün başlamadan da denetlenir.
- Summary(State, FirstDay, LastDay), WeekSummary(State, ClosedDay=0), MonthSummary(State, ClosedDay=0): History üzerinden gelir, kâr, müşteri, son kasa; eski/eksik kayıtta MissingDays belirtilir, kayıt değişmez. WeekSummary takvim haftasıdır; tam 7 günlük tur raporu LastAdvance.Period içindedir.
- RoutineSkill/ForgetPermille/DelayPriceRise: müdürün FamilyRule becerisi; müdür yoksa ailenin 55 becerisi. Unutma clamp((85-beceri)*4,0,340)/1000; günlük zam gecikmesi clamp((80-beceri)*6,0,480)/1000, kampanya tohumu+günle belirlenimci. Oyuncu yürürken bu rutini çağırmaz. Cömert müdürün önerisi de oyuncunun 9 koli sınırına indirilir; 11 koli yüzünden bütün sipariş reddedilmez.
- AMarketGameMode::AdvanceTime(ETurn), LastAdvance: kapalı aile dükkânından ilerletir; çalışan/koli/fiyat görünümünü yeniler, kaydeder, rapor açar. Menü dosyası değiştirilmedi.
- Bot ay turlarıyla aynı durma mekanizmasını kullanır; nakit sıkıntısında raporun kalan yılları kaybolmasın diye normal PlayDay ile günlük gözleme devam eder. Bedava mal/para ve test modu yok.
- Yeni MirasMarket.Simulation.TurnLengthsAndStops, PeriodsAndOlderSaves, RoutineSkillAndImperfections; test alt sınırı 89. Son kaynak hâli DERLE geçti; TEST 89/89 (88 temiz + 1 motor ağ kontrolü uyarısıyla başarılı, başarısız/çalışmamış 0); Smoke geçti. Kısa bot 3×120 gün yaklaşık 2,1 sn, denetim hatası 0.

## Yeni açık işlevler

- MarketAutoPlay::Run(Options, Base, Capacities, RestoreSeed=0): dünyasız belirlenimci kampanyalar; rapor bellekte; önceki ülke ve verilen RestoreSeed geri kurulur. Menü çağrısı oyuncunun RivalSeed değerini vermeli.
- LoadInputs, WriteReport, Validate; commandlet -run=MirasAutoPlay -Years=10 -Seeds=3 -Country=tr -Province=kirklareli. Dosya tarihi yalnız çıktı adında.
- MarketSimulation::AdjustPrice(State, Products, Index, bUp): oyunun mevcut ± fiyat adımı ve sınırları.

## C'ye istekler

- Menü: mevcut Advance komutunu `Game.AdvanceTime(MarketSimulation::ETurn::Day/Week/Month)` ile bağla; 1 ay düğmesi ekle. Oyuncuya `Game.LastAdvance.Message`; dönem kartına `LastAdvance.Period` (ciro/kâr/son kasa/müşteri). Menünün mevcut haftalık erken durma varsayımını kaldır. Bu dalda menüye dokunulmadı.
- İl/bölge yöneticisinin kasadan çalması için mevcut durumda türü belli günlük olay bayrağı yok. C yönetim sistemine bu bayrağı eklediğinde Simulation::FSignals::Important içine bağla; haber metnini karşılaştırarak tespit yapılmadı.

- B defteri hazır olunca Simulation::Shopper SellBasket, PlayDay SubmitOrder ve ana CloseDay para hareketleri B'nin Post çağrılarıyla bağlanmalı. Ana kapanıştan sonraki Director farkı ölçülür; sistemlerin her birinin korunumu şu an bağımsız denetlenmiyor. Rapor bunu açık yazar.
- Bot komut tablosunu yeni markalar, tedarik, lig ve dönem olaylarıyla genişletin. Mevcut kararların başarısızlığını sessizce atlamaz: reddedilen komut sayısı raporda.
- Global ülke/ekonomi nedeniyle koşular paralel çalıştırılmamalı. Menüden Run çağrısında RestoreSeed=oyuncunun RivalSeed değeri verilmelidir.

## Kararlar ve varsayımlar

Tarzlar: temkinli tampon 30 günlük maaş+temel gider, açılış bedeli ×2,5, 30 günlük büyüme ritmi, fiyat ×1,05, kredi yok; dengeli 14 gün/×1,5/14 gün/×1,00, kredi yok; atak 7 gün/×1,1/7 gün/×0,95, kredi var. Depo değerlendirme eşiği 12/8/4 mağaza. Adaylarda gizli beceri/dürüstlük okunmaz: listedeki ilk adayı alır. Kurgu kimliği 0/1/2; satış teklifini tüm tarzlar reddeder.

## Bilinen sorunlar

Tam defter denetimi B/C entegrasyonuna bağlı. Gider sıralaması ölçülebilen kalemleri kapsar; stok yatırımı ve nakit olmayan fire etiketli. Lig henüz bulunmadığından ilk 3/ilk 10 ölçülemez. İlk pasif sipariş denemesi bu yönde yanıltıcı sonuç verdi; bütçeye bölünmüş normal siparişler sonrası tam koşuda hiç nakit sıkıntısı görülmedi. Uzun koşuda 9/9 kampanya tek mağazada kaldı; bu, bütün büyüme sistemlerinin çalıştığını kanıtlamaz. Açılış için borç, kârlı gün, yerel %35 pay, bölüm ve para şartları var; hangisinin fiilen engellediği bu sürümde ayrı sayaçta tutulmuyor, tek nedene bağlanmamalı.

## İlk tam bot koşusu — 01.10.2026

`AUTOPLAY.cmd`: 10 yıl × 3 tarz × 3 tohum (21/22/23), her biri 3653 gün = 32.877 gün; 212,1 saniye (15 dakika hedefinin altında). Commandlet başarılı, hata/uyarı 0. Kaynak sürümü a7304ca. B/C yeni işleri bu dala birleştirilmedi; bu rapor A0 ekonomisinin başlangıç ölçümüdür.

- Teslim kopyası: `Docs/Surec/akislar/A_ilk_rapor.md`, üretilen raporun aynısı.
- Ham dosyalar: `Saved/AutoPlay/20261001-000114/rapor.md`, `gunluk.csv`, `haftalik.csv` (Saved git tarafından dışlanır, bu bilgisayarda A klasöründedir).
- CSV doğrulaması: günlük 32.877 satır, haftalık 4.698 satır; dokuz koşunun her birinde günler 1..3653 sıralı ve tekrarsız.

Mustafa için en önemli 5 bulgu:

1. **Büyüme durmuş:** 9/9 koşuda en yüksek mağaza sayısı 1; ilk şube, 5 mağaza, depo, il müdürü ve ikinci ülke eşiklerine ulaşılmadı. Önce ilk şubeye geçişteki koşulları inceleyin; botun büyüme politikasını da birlikte değerlendirin.
2. **Nakit sıkıntısı yok:** 9/9 koşuda eksi kasa/sıkıntı günü 0; görülen en düşük kasa 847,82 TL. Başlangıç geçim dengesi bu üç politika için ayakta kalıyor; bu sonuç tüm oyuncuların zorlanmayacağı anlamına gelmez.
3. **Atak daha kazançlı:** son kasa 598.958–623.692 TL; temkinli 341.009–366.799 TL; dengeli 346.829–371.460 TL. Tarz ortalamalarında atak yaklaşık 1,72 kat temkinli, 1,68 kat dengeli. Son borcu 19.653–21.272 TL; temkinli borçsuz, dengeli 12.199–13.602 TL. Son kasalarda 4 kat istismar eşiği aşılmadı; en uç oran 1,83.
4. **En büyük para trafiği stok:** 10 yılda mal alımı 2,25–3,61 milyon TL, aile ücretleri 151.734–168.520 TL, vergi 149.760–213.185 TL, ücret dışı işletme giderleri yaklaşık 135.500 TL. Stok alımı kâr hesabındaki giderle aynı değildir; tam defter bağlanınca gerçek zarar nedenleri yeniden sıralanmalı.
5. **Denetim temiz, kapsam sınırlı:** 32.877 günde satış/sipariş/ana kapanış/mal kabul ve durum sınırlarında 0 hata. Şube, depo ve online gelirleri 0 olduğu için bu yollar uzun koşuda zorlanmadı. Ulusal pay bütün koşularda %0,0455; eski mağaza sayısı ölçümü olduğu için B'nin ciro tabanlı payıyla yeniden koşulmalı. Tam arka plan para korunumu ve lig hedefleri C entegrasyonundan sonra doğrulanmalı.

Son doğrulama: DERLE başarılı; TEST 89/89 (88 temiz + 1 motor HTTP bağlantısı uyarısıyla başarılı), başarısız/çalışmamış 0. Smoke başarılı: oyuncu, raf doldurma, çok ürünlü sipariş, arka kapı mal kabul, işe alma, 1 satış, gün kapanışı, disk kayıt/yükleme. Kısa otomatik oyuncu yaklaşık 2,1 saniye. A1/A3 kaynakları commit ve push edildi; A0 sonrası main klasöründe değişiklik yapılmadı.
## İkinci tur — A4 (cf5ee8c, push edildi; 01.10.2026)

- Kullanıcının izniyle main'deki Source/Docs/Config değişiklikleri dosya değiştirmeden b0af1e5 (`C1: mağaza görünümü şube hesabına (Akış C, derlenmedi)`) olarak commit + push edildi; akis-a'ya merge edildi. Teslim C1 dışında C2/marka/tedarik dosyalarını da içeriyordu, hepsi korundu.
- Birleşik sürüm DERLE geçti; ilk TEST 95/96: eski şube açılış maliyeti testi sabit ölçü bekliyordu. **En küçük düzeltme:** MarketBranchesTests.cpp:120,122–125: sabit `2*60000+400000` beklentisi yerine gerçek görünümün depozito+tadilat+stok toplamı karşılaştırılır. Ekonomi mantığı değişmedi. Main 86a0708, akis-a 66374c1, main'e de push edildi.
- A4: MarketBranchVisit.h/.cpp/Tests.cpp, MarketReviewAutomation.cpp; MarketGame girişleri ve kayıt/komut/ekonomi tick koruması; MarketStoreBuild ziyaret doluluğu. Menü dosyalarına dokunulmadı.
- `StartBranchVisit(int32 BranchIndex)` yalnız açık şubeyi açar; StoreView -> StoreViews::Find -> StoreKit::Find, görünüm yoksa türün ilk hazır şablonu. Planogram kategori sözleşmesiyle doldurulur; ürüne ait görsel adet toplamı × clamp(Units/Capacity,0,1). Boş raf boş kalır; aile stoğundan mal alınmaz.
- Aile aktörleri yok edilmez: görünürlük/çarpışma/tick ve StoreKit etiketi saklanır, geçici sahne kurulurken gizlenir; çıkışta birebir geri gelir. Planogram, tabela listeleri, kategori tercihleri, oyuncu dönüşümü/bakışı/hızı, kamera, menü ve hız durumu korunur. Ekonomi Tick'i durur; hareket için ziyaret zamanı akar. Test modu açılmaz, hiçbir kampanya kaydı yazılmaz. Esc `EndBranchVisit()` çağırır. Ziyaret komutları engellenir.
- Başlangıçta normal `MarketDirector::Command(State, Products, "VisitBranch", BranchIndex, Message)` çağrılır. Komut şu an yok; sessizce geçer. C komutu eklerse kendi ziyaret kazanımlarını uygulayabilir; ziyaret görseli State kopyasını geri yazarak bunları silmez.
- Görsel yoğunluk: aynı anda müşteri = ceil(LastShoppers/20), en çok 24; kuyruk = ceil(LastQueueLost/5), en çok 8; çalışan = Workers, en çok 24. Bu, son günlük sayının temsilidir; ziyaret yeni müşteri üretmez/satış yapmaz. Karne D/F ise mal kabulde 3 kutu yalnız görsel dağınıklıktır. İnsanlar mevcut kutu prototipidir; yürüyüş/MetaHuman ikinci aşamanın devamında.
- Doğrulama: DERLE başarılı; TEST 97/97 (96 temiz + 1 motor HTTP uyarısıyla başarılı); Smoke başarılı. `BranchVisitReview` gerçek render, 3×1280×720 PNG; çıkışta tüm State baytları, planogram, gün saati, oyuncu konumu/bakışı, test modu ve tüm mevcut .sav dosyaları aynı. Test kurulumu yalnız bellekte inceleme şubesi ekler; başlangıç/çıkış karşılaştırması bundan sonra yapılır.
- PNG'ler `Saved/Screenshots/BranchVisit/20261001-002815/01.png`, `02.png`, `03.png`; üçü gözle incelendi. Soluk bilgi şeridi ve kolona bakan kasa açısı düzeltildi; son görüntüde şerit okunuyor ve kasa kuyruğu görünüyor. İlk yöntemde HISM instance silmek engine NaN uyarıları üretti; raflar artık ilk kurulumda doğru adette üretiliyor, son koşuda bu uyarı yok.

### C'ye istek — ziyaret

Mağazalar/harita "Gez" düğmesi `Game.StartBranchVisit(BranchIndex)` çağırmalı. Aile dükkânının günü açık olsa da ziyaret çalışır; ekonomi donar ve çıkışta kaldığı saniyeden devam eder. Tadilat/kapalı şubede false döner. `VisitBranch` komutunda moral/ipuçları C'nin; para ve aile dükkânı stoğu değişmemeli. Geri dönüş `Game.EndBranchVisit()`; Esc hazır. İnsanlar ve bazı mağaza tabelalarının yazısı henüz prototip görünümdedir.
## İkinci tur — A5 (01.10.2026)

- `MarketMenuCapture.cpp`: `-MirasMenuCapture` gerçek katalogla 1096 gün × 3 tarz × 1 tohum oynar; atak kampanyasının son State'ini bellekte açar. `MarketAutoPlay::FOptions::bKeepFinalStates` isteğe bağlıdır (normal koşuda false); `FReport::FinalStates` yalnız istenince döner. Snapshot günü ve sayısı mevcut belirlenimcilik testinde denetlendi.
- 10 sayfa / 17 ana sayfa-sekme görünümü: haritanın mağaza/rakip/fırsat katmanları, Sipariş, Fiyat, Kampanya, yerel/ulusal/dünya rakipleri, Personel, Finans, Satış kanalları, Mağazalar/Yönetim/Şirket, gün/hafta raporu. Açık+koyu × 1920×1080+1280×720 = **68 PNG**. Sekmeler normal Slate düğmelerinin tıklama işleviyle açılır. Ürün/kategori filtrelerinin her değeri, açılır işe alma/yatırım katmanları ve aşağı kaydırılan her tablo satırı bu ilk katalogda ayrıca çoğaltılmaz.
- Çıktı: `C:\Users\mtass\Desktop\market-ll-A\Saved\Screenshots\Menu\20261001-004430\index.html`; her görünümün dört sürümü yan yana. `manifest.csv` 68 satır; `bot/rapor.md`, günlük/haftalık CSV gerçek koşunun ham verisidir. Saved Git'e dahil değildir; aynı bilgisayardaki A çalışma ağacında durur.
- Yakalama sırasında ekonomi ilerlemez ve kampanya kaydı yazılmaz. Başlangıç mesajı temizlenir, aile görevli listesi kampanyayla eşitlenir; HUD menü altında saklanır. Bütün sayfalardan sonra State'in serileştirilmiş baytları başlangıçla karşılaştırılır. Yakalama widget'ları motor kapanmadan bırakılır (ilk denemede kapanışta hata vardı; son koşu çıkış kodu 0).
- **A5'in dolu kampanya şartının sınırı:** gerçek bot üç yılda üç tarzda da tek mağazada kaldı. Bu yüzden şube/müdür/depo ekranları boş kalmasın diye yalnız inceleme kopyasına açıkça etiketlenmiş **4 örnek şube, 2 yönetici, 1 depo, 1 kamyon** eklenir. Bunlar botun açtığı/parasını ödediği büyüme değildir; gerçek üç yıllık şube+müdür+depo başarısı henüz sağlanmadı. Her PNG'nin üst açıklaması ve HTML bunu söyler. Bot politikasını veya ekonomi kurallarını bu görsel iş için değiştirmedim. Büyüme düzeldikten sonra aynı otomasyon gerçek ağı kullanır; örnek ekleme yalnız hiç şube yoksa yapılır.
- Gerçek koşu: 3288 gün, 24,6 saniye; denetim hatası 0, kasa eksi günü 0. Son kasalar temkinli 139.817,66 / dengeli 160.498,75 / atak 238.603,03 TL. Görüntüdeki ağın kartları bu tutarlarla karşılaştırılarak kârlılık sonucu çıkarılmamalı.
- Görsel denetim: 68/68 PNG açıldı; doğru iki çözünürlük, 68 farklı dosya özeti, 68 HTML görseli ve 68 manifest satırı. 34 ikili karşılaştırma görüntüsüyle bütün sayfa/tema/boyutlar gözle incelendi; sorunlu yerlerde asıl 1280×720 PNG de okundu. Boş/bozuk sayfa yok; aşağıdaki kesilme ve tutarsızlıklar var.

Çalıştırma (Editor kapalı, A klasöründe):

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "$PWD\MirasMarket.uproject" -game -RenderOffscreen -unattended -nosound -nop4 -MirasMenuCapture -ResX=1920 -ResY=1080 -windowed
```

### C'ye istek — görüntülerdeki menü sorunları

Görüntülerin hepsi yukarıdaki `20261001-004430` klasöründe. Menüye dokunulmadı.

1. **Eski şehir ve rakip kimliği:** `04_products_*` başlığı hâlâ “RAKİP FİYATLARI · LÜLEBURGAZ”, üstte Kırklareli yazıyor. Fiyat kartları BİM/Migros/A101; `06_rivals_local_*` içinde yeni BİN/Migron/A110 listesiyle eski isimli haber kartları aynı ekranda. Şehir başlığını seçilen aile ilinden, kart kimliklerini aynı ülke/rakip kaynağından üretin. İki tema/iki boyutta görülüyor.
2. **Ulusal ligde tutar kesiliyor:** `07_rivals_national_*` ilk dört büyük cironun sonundaki TL görünmüyor; alttaki küçük tutarlarda TL görünüyor. İki boyutta da var. Sabit tutar sütununu büyütün veya milyon/milyar kısa gösterim + tam tutar ipucu kullanın.
3. **Borç ayrıntısı kesiliyor:** `10_finance_*` üst Borçlar kartında banka+toptancı+vergi satırı sağdan kesiliyor (1920'de de var). Kart ayrıntısında satır kaydırma veya kısa alt toplamlar kullanın. `12_shops_*` sağdaki “Karne nasıl verilir” açıklamasının sonu da özellikle 1280'de kesiliyor.
4. **Sipariş sütunları sıkışık:** `03_orders_*_1280x720.png` Yolda / Dün sat. / Boş raf başlıkları bitişik okunuyor. Başlıkların aralığını/satırını ayırın; uzun ürün sütunu kadar boş alan var. 1920'de daha az belirgin.
5. Haritanın rakip/fırsat renkleri için açıklama görünmüyor (`01_map_rivals_*`, `02_map_opportunities_*`); renk tek başına neyi gösterdiğini söylemiyor. Küçük bir renk anahtarı eklenebilir. Bu taşma değil, okunabilirlik isteği.

### Ziyaret ve menüden Mustafa için en önemli 5 bulgu

1. Fiyat/yerel rakip ekranlarında eski şehir-rakip adları yeni pazarla karışıyor.
2. Ulusal ligde büyük cironun para birimi kesiliyor.
3. Finans borç ve mağaza karne açıklamaları kesiliyor.
4. 720p sipariş tablosunun başlıkları sıkışıyor.
5. Ziyarette bazı reyon tabelaları uzaktan parçalı/üst üste görünüyor; müşteri ve çalışanlar kutu prototipi. Bu mağaza görünümü borcu A'nın ilerideki simülasyon/görsel turunda ele alınmalı; C'nin menü dosyasında çözülmez. Üst ziyaret şeridi okunuyor, boş/dolu raf ve kuyruk ayrımı görülebiliyor.

Bağlama: C “Gez” -> `StartBranchVisit(Index)`, ziyaret kazanımları -> Director `VisitBranch`; Esc çıkışı hazır. A3 gün/hafta/ay -> `AdvanceTime` istekleri hâlâ geçerli. Bu otomasyon C'nin sonraki menü değişikliklerinde yeniden çalıştırılabilir.

### Son doğrulama — A5

01.10.2026 00:54: DERLE başarılı. TEST **97/97** (96 temiz + 1 motorun google generate_204 HTTP zaman aşımı uyarısıyla başarılı); başarısız/çalışmamış/devam eden 0. MirasMenuCapture **68/68**, State aynı, motor çıkış kodu **0**; bütün PNG boyutları ve HTML/CSV eşleşmesi geçti. Smoke **başarılı**, çıkış kodu 0: oyuncu, raf doldurma, çoklu sipariş, arka kapı kabul, işe alma, 1 satış, gün kapama, disk kayıt/yükleme. A4 ve A5 ayrı commit; akis-a origin dalına gönderildi. Önceki A4 BranchVisitReview üç görüntü ve kayıt/plan/saat/oyuncu/disk korunumu kontrolünden geçti.

## A6 — C teslimi ve M27 kayıt kapısı (01.10.2026)

- Main'deki yalnız Source/Docs/Config değişiklikleri dosyalar değiştirilmeden 545f5dd (`Akış C: C2b, C2c, M25, M26 ve sonrası (derlenmedi)`) commit + push edildi; akis-a'ya birleşti. C1/C2b/C2c/M25 önceki teslimden de vardı; bu teslim M26 ve üçüncü tur sözleşmesini ekledi.
- Birleşik kaynak: DERLE başarılı, TEST 99/99. **C kaynaklarında derleme/test düzeltmesi: 0.** Main'e geri uygulanacak düzeltme çıkmadı.
- M27: `MarketEconomy.cpp` IsStructurallyValid yalnız `Version == FMarketState::CurrentVersion` kabul eder. Tek sürüm sabiti hâlâ MarketEconomy.h: `CurrentVersion = 2`; artırılmadı, C3'te bir kez artırılacak.
- `MarketGame::LoadCampaign` sürüm farklıysa kampanyayı değiştirmeden reddeder ve sessiz otomatik yüklemede de “Bu kayıt oyunun eski bir sürümünden; yeni oyun başlat.” der. Sürümü sessizce yenileme ve bu yükleme yolundaki Staff/Branches Migrate çağrıları kaldırıldı. C/B dosyalarındaki mevcut dönüşüm kodunu C3/B7 kaldıracak.
- Yeni `MirasMarket.Save.ExactVersion` gerçek SaveGame bellek yazma/yükleme turuyla eski/güncel/gelecek sürümleri denetler. DERLE başarılı, TEST **100/100**, Smoke başarılı (disk kayıt/yükleme dahil). Kayıt uyumu eklenmedi.
- C'ye istek: menü yuva kartında uyumsuz sürüm için aynı yeni oyun cümlesi; bütün kayıt okuyucuları sürümü tek `FMarketState::CurrentVersion` sabitinden karşılaştırmalı.
## A6 — Botun C kararları (01.10.2026)

C kararları ayrı, dünyasız `MarketAutoPlayC.*` içindedir; bütün değişiklikler `MarketDirector::Command` üzerinden geçer. Reyon açma öncesi şubelerin toplam tadilat/stok bedeli ve kasa yedeği denetlenir; olgun (30 gün) reyonun son 30 günlük toplam kârı negatifse aynı tür mağazalarda kapanır. Yeniden denemeden önce üç karar aralığı beklenir. Açılış sırası en son gözlenen 30 günlük kâra göre; daha önce denenmemiş reyon için halka açık Ratio × (Margin − Waste − Shrink) ilk deneme sırasıdır. Süpermarket %22, hiper %70 gerçek alan sınırına ek olarak tarzın aşağıdaki alan payı kullanılır.

| Tarz | İlk C kararı / aralık (gün) | Turda en çok yeni reyon | Fiyat duruşu | Alan sınırının kullanımı | Tedarik asgarisi yedeği | Marka hedefi yedeği | Zincir alımı / kasa yedeği |
|---|---:|---:|---|---:|---:|---:|---|
| Temkinli | 365 / 60 | 1 | pahalı | %80 | 1,40× | 1,15× | yok |
| Dengeli | 180 / 30 | 2 | normal | %95 | 1,20× | 1,05× | var / 1,50× bedel + işletme yedeği |
| Atak | 60 / 15 | 3 | ucuz | %100 | 1,00× | 1,00× | var / 1,10× bedel + işletme yedeği |

Tedarik tahmini mevcut ayın en az yedi günlük gerçekleşen alımını 30 güne uzatır; ay yeni başlamışsa son 30 gözlenen günlük alım kullanılır. C'nin ay kapanışında sıfırladığı son günlük tutar gözlemden düşebilir: tahmin bilinçli olarak aşağı yönlüdür, alım icat edilmez. `CanSet` uygunluk kapısı ve tahmin asgarisi birlikte geçilmeden yükselmez; gerçekleşen hacim mevcut asgarinin %80'inin altına düşerse geri iner. Marka: mevcut raf payı / gerçekleşen aylık satış hedefi yedeği karşılıyorsa kabul, listeme teklifinde ürünün raf kapasitesi varsa kabul; aksi durumda ret. Kapasiteyi veya geçmiş satışı teklif için değiştirmez. Zayıf ustalar yalnız oyuncunun görebildiği toplu `WeakMasters` sonucuyla, yedek kasa varsa değiştirilir. Temkinli hiç zincir almaz.

Büyüme de oyuncu yolunda iyileştirildi: 90. günden sonra ilk şubeyi hâlâ açamamış ve yerel payı %40'ın altında kalmış bot, haftalık normal fiyat adımlarıyla katalog fiyatının temkinli %95 / dengeli %88 / atak %82'sini dener; maliyetin %105'inin altına inmez. Süpermarket eşikleri 8 / 6 / 3 mağaza; hiper eşikleri 30 / 20 / 12. Bunlar bot stratejisidir; C'nin oyun sabitlerine dokunulmadı. Üç yıllık pilot (tohum 21): temkinli 1, dengeli 20, atak 12 mağaza; denetim hatası 0; atakta 1 kasa eksi günü. Dengeli ilk şube 107, ilk depo 449. gün.

Ölçümler: `rapor.md` içinde C bölümü; `lig.csv` her yıl, `reyonlar.csv` her 30 günde türe/mağaza türüne göre toplam son 30 günlük kâr/ciro, `tedarik.csv` bütün kademe değişimleri; günlük/haftalık CSV'de önbellek lig sıraları. Kâr aralıkları bütün günlerde gözlenir; CSV örnekleri 30 günlük aralıklarla alınır. Piyasadan çekilme, satılık zincir ve fiyat savaşı tekrarları kimlikle ayıklanır. **İflas için C'de kapanma nedeni bayrağı yok:** görünür iflas haberleri alt sınır; tüm kapanmalar iflas diye yazılmadı. Sıkıcı dönem = olay/karar/mağaza değişimi olmadan en az 31 gün; C teklif/sıra/kademe/savaş hareketleri eklenince aynı kampanyanın ikinci sayımı verilir. Felaket yığılması = son yedi günde dört olumsuz olay/savaş; bir kesintisiz kümeye bir sayım. B'nin henüz birleşmemiş dönem/akış sistemleri bu sayımda yoktur.

Yeni testler: `AutoPlay.SupplyAndBrands` (asgariyi tutturmadan çıkmama, hacimle çıkma/geri inme, para icat etmeme, karşılanabilir marka koşulu); `AutoPlay.DepartmentLossAndSpace` (gerçek komutlarla açılma, %22 sınırı, maliyet, zarar kapanışı ve ikinci kez stok iadesi olmaması); `AutoPlay.CObservation` (sıkıcı dönem/tedarik geçişi/felaketin tekrar sayılmaması). İlk DERLE ve TEST **103/103** geçti; son rapor eklemesi sonrası DERLE geçti, TEST + Smoke sürüyor. Tamamlandı denmedi; uzun koşu ve gerçek ağla A5 yenilemesi sırada.

Son bot doğrulaması: DERLE başarılı, TEST **103/103** (102 temiz + 1 motor HTTP zaman aşımı uyarısıyla başarılı); başarısız/çalışmamış/devam eden 0. AutoPlay.Short **0,693 saniye**. Smoke başarılı, işlem çıkış kodu 0. Uzun koşu raporları henüz alınmadı.

### A5 gerçek ağ şartı kapandı (01.10.2026 08:43)

`Saved/Screenshots/Menu/20261001-083810/index.html`: gerçek 3 yıllık atak bot (tohum 21), 1097. gün, **11 şube / 3 müdür / 1 depo**. Örnek şube/müdür/depo eklenmedi (`review-only network 0`). Menü görüntü otomasyonu **68/68** ve kampanya değişmedi kontrolü geçti, işlem çıkışı 0; tüm PNG boyutları doğru. 17 karşılaştırma sayfasındaki dört PNG'nin hepsi gözle incelendi (`Review/` yalnız inceleme kopyaları). Böylece önceki A5'te açık bırakılan “botla kazanılmış dolu kampanya” şartı karşılandı.

C'ye istek: önceki beş menü bulgusu gerçek kampanyada da var: Fiyat kartındaki Lüleburgaz + eski rakip adları; ulusal ligde büyük cironun para birimi kesilmesi; Finans/Borç ayrıntısı ve mağaza karne açıklaması kesilmesi; 720p sipariş başlıklarının bitişmesi; harita rakip/fırsat renk anahtarı eksikliği. Sayfaların hiçbiri bozuk/boş açılmadı. C'nin yeni reyon/tedarik/marka bölümleri uzun sayfalarda aşağı kaydırılır; bu çekim her sayfa/sekmenin ilk görünümüdür, tüm kaydırma konumlarının taraması değildir. Menü kaynakları değiştirilmedi.

### A6 ilk uzun koşudan bot düzeltmesi

İlk 30 yıl × 3 tarz × 1 tohum ve 10 yıl × 3 tarz × 3 tohum tamamlandı; ikisi de çıkış 0 ve denetim hatası 0. İlk 30 yıl raporu `Saved/AutoPlay/20261001-084043`: dengeli 10/20/30. yılda ulusal 18/18/18, dünya 20/20/20; 61 mağaza. **Bu son denge raporu değildir.** Bot hiper seçtiğinde, sıralamanın başındaki ucuz ama 500 bin altı nüfuslu ile tekrar bakıp takılıyordu (dengeli 703, atak 1517 ret). Bu C'nin hatası değil; oyun kapısını doğru uyguladı. Bot yer listesi artık halka açık nüfus, bölüm ve depo menzilini filtreler; açılış yine `CanOpen` ve normal `OpenBranch` komutuyla. Bölüm kilitliyse süpermarketle büyümeyi sürdürür. `AutoPlay.ExpansionSite` küçük il, büyük il/deposuz, büyük il/depolu, bölüm kilidi için testlidir.

İlk koşuda temkinli 30 yıl tek dükkânda kaldığından “az ve geç” tarzı gerçek büyümeyle sınanamadı. İlk şube fiyat denemesi temkinlide 365. günden sonra %88; dengelide 90. günden sonra %88, atakta 60. günden sonra %82 olarak güncellendi. Temkinlinin 30 günlük karar aralığı, 2,5× açılış yedeği ve borçsuz politikası korunur. `AutoPlay.LateCarefulGrowth` gerçek 600 günlük kampanyada temkinlinin ilk yıldan sonra ve dengeliden geç şube açmasını, para/stok denetimini kontrol eder. İlk tabloyun %95 temkinli fiyatı bu son kararla değiştirilmiştir. C sabitleri yine değişmedi. DERLE/TEST/Smoke ve uzun koşular yeniden çalıştırılıyor.

Bot yer/geç büyüme düzeltmesi doğrulandı: DERLE başarılı; TEST **105/105** (104 temiz + 1 motor HTTP zaman aşımı uyarısıyla başarılı), başarısız/çalışmamış/devam eden 0; Smoke başarılı (disk kayıt/yükleme dahil), işlem çıkışları 0. 600 günlük gerçek strateji testi 4,35 sn: temkinli 8, dengeli 12, atak 15 mağaza; hepsinde denetim hatası 0. Atak kasası -706.405,27 TL: yeni hiper kararının ciddi ekonomik riski uzun raporda incelenecek. Son uzun koşular A6_final_30Years.log / A6_final_10Years.log ile başlatıldı.

## A6 — Son doğrulama ve teslim (01.10.2026 09:07)

- C kaynaklarında derleme/test düzeltmesi **0**; main'e geri uygulanacak düzeltme yok. Main teslimi 545f5dd, akis-a birleşmesi 37d6669. M27 b84a5ea, C bot kararları 23aa03f, yer/geç büyüme d190bfe; hepsi push edildi. Sürüm 2, artmadı.
- Son kod DERLE başarılı; TEST **105/105** (104 temiz + 1 motor HTTP zaman aşımı uyarısıyla başarılı), başarısız/çalışmamış/devam eden 0; Smoke başarılı, işlem çıkışları 0. 600 günlük gerçek tarz testi 4,35 sn; kısa bot 60 sn sınırının çok altında. Son koddan sonra yalnız rapor/belge eklendi.
- **Son 30 yıl × 3 tarz × 1 tohum:** Saved/AutoPlay/20261001-090217, 523,7 sn. **Son 10 yıl × 3 tarz × 3 tohum:** Saved/AutoPlay/20261001-090707, 279,6 sn. İki commandlet çıkışı 0. **12 kampanya / 65.751 gün / 0 para-stok-sayı denetim hatası.** İlk, yer seçiminde takılan koşu son sonuçlara karıştırılmadı.
- Committe taşınan raporlar: [30 yıl ve değerlendirme](A6_30_yil_rapor.md), [10 yıl / üç tohum](A6_10_yil_rapor.md), yıllık sıralar `A6_lig_30_yil.csv` / `A6_lig_10_yil.csv`, şube başına reyon kontrolü `A6_reyon_ozet.csv`. Tam günlük/haftalık/reyon/tedarik CSV'leri Saved/AutoPlay'daki iki klasörde.
- Son dengeli, tohum 21: ulusal **74 / 63 / 60**, dünya **20 / 20 / 20** (10/20/30. yıl). İlk kasa açığı 2.640. gün; 30 yılda 151 mağaza ve negatif kasa. Üç tohumlu 10 yılda da dengeli/atak 6/6 nakit açığı; temkinli 3/3 artıda ve ulusal 8. sırada. Temkinli 30 yılda 198 mağazaya, 6.371. günde nakit açığına geldi. **Denge hedefi karşılanmadı.** B/B7/C3 henüz birleşmedi.
- 30 günlük toplam kâr uçları (aynı mağaza türündeki tüm şubeler, nominal TL, açılış dahil): en büyük zarar hiper **elektronik −1.092.495,88**, **evcil −934.376,36**, **manav −748.669,14**; en yüksek kâr hiper **kasap 5.899.083,07**, **manav 4.117.632,66**, **giyim 1.987.648,33**. Toplamlar şube sayısı/tarih/enflasyondan etkilenir; tek şube ya da kampanya toplamı değildir. C'nin Last30Profit'i 29/30 sönümlü yaklaşık toplamdır. Yapısal olarak balık süper 3/3 ve hiper 15/15, hiper evcil 23/23, hiper elektronik 9/9 aylık örnekte zarar; süper evcil 695 örnekte zarar yok, örnek kâr/ciro %33,35.
- Tedarik yükseliş/düşüş: temkinli **12/12**, dengeli **12/12**, atak **5/5**; tarih listeleri raporlarda. 30 yıl marka gelirleri **23.291.816,31 / 2.901.902,19 / 2.210.812,64 TL**; küsen marka **11 / 11 / 10**. Son koşuda zincir alımı 0 (satış geldiğinde kasa yetmedi); önceki hatalı yer seçimi pilotundaki 6'şar normal alım son koşu diye sunulmadı.
- Rakip zincir tepe sayısı **105 / 92 / 49**, ayrı satılık **35 / 29 / 5**, piyasadan çekilme **35 / 27 / 5**, savaş **457 / 468 / 48**; ezeli rakip temkinli/dengelide BİN/Haluk Sezer, atakta yok. **Tam iflas sayısı ölçülemiyor:** C kapanma nedenini saklamıyor ve görünür haber tavanı var; haberden 0 alt sınır, “tüm kapanmalar iflas” denmedi. C'ye istek: kalıcı kapanma nedeni/sayaç.
- Sıkıcı dönem proxy 0; bu “oyuncu sıkılmaz” kanıtı değil, olaylar 31 gün sessizlik bırakmıyor. 30 yıl felaket kümesi mahalle / C dahil **56/141**, **56/149**, **56/64**. Aynı kampanyanın iki gözlemi; farklı illerdeki eşzamanlı savaşlar şirket düzeyinde sayılır, B dönemleri henüz dahil değildir.
- Bot sınırları: zararlı reyonu kapatır ama zarar eden şubeyi otomatik tasfiye etmez; yedek kasa aile dükkânı giderlerinden, tüm ağ giderinden hesaplanmıyor; marka için raf karışımını değiştirmez. Nakitsiz şubelerin yıllarca gideri dolayısıyla sonuç iyi bir insan oyuncunun zorunlu kaderi değildir. C3'ten sonra ağ yedeği/şube tasfiyesi ve B7 etkileriyle yeni denge turu gerekir. A6 kodu/ölçüm işi bitti; oyun dengesi bitmiş diye gösterilmedi.

## C'ye ayar önerileri (A6, uygulanmadı)

Bunlar **kontrollü olarak denenecek ilk adaylar**, doğrulanmış denge ayarı değil. C dosyalarındaki hiçbir sabit A tarafından değiştirilmedi. Tam gerekçe ve ölçüler 30 yıllık raporun başında.

1. **Hiper sabit yük:** Workers **20 → 14**, Running **5 → 3**. Dengeli üç tohumda da ilk 10 yılda açık, atak tohum 21 ilk hiper sonrasında 435. günde açık. Önce şube gider/kâr dökümüyle dene; bölüm personeli ek ücretini ve 30 günlük alışmayı birlikte ele al.
2. **Sürekli zarar eden reyonlar:** balık Ratio **0,04 → 0,08**, Margin **0,28 → 0,34**; elektronik Ratio **0,14 → 0,24**, Margin **0,11 → 0,20**; hiper evcil `Staff` **1 → 0** (kat görevlisiyle paylaşım). İlgili bütün aylık örnekler zarar; süper evcil personel paylaşımıyla %33,35 kâr/ciro. Her reyon ayrı denensin. Manav/kasap marjı artırılmasın; normal örnekleri zaten olumlu.
3. **Rekabet/lig ölçeği birlikte:** nakit düzeldikten sonra `LeagueCompression` **0,04 → 0,004** ve ulusal roster başlangıç mağazaları **×0,1** (BİN 3.500 → 350, A110 1.900 → 190 vb.; Config ve tablo aynı kaynak). Compression sadece devleri küçültüyor, ulusal zincirlerin dünya satırını küçültmüyor. Dengeli son ortak ciro 0; bu koşudan kesin doğru katsayı çıkarılamaz. Temkinli 10. yıl 68,8 milyon / lider 3,36 milyar; hedef zamanlar için önerilen ilk deneme aralığı, kazanma garantisi değil.
4. **Savaş dinlenmesi:** WarDays **45 → 21**, WarPressure **1,30 → 1,10**, aynı ilde yeni savaş öncesi **60 gün** dinlenme. Dengelide 468 savaş ve kümeler 56 → 149. B akış bütçesine öncelik/il üzerinden bağla; bütün illeri ayrı acil seçim yapma.
5. **Marka teklif tavanı:** MaxOpenOffers **3 → 2**. Dengeli 256 kabul / 763 ret (yaklaşık %75 ret); bot raf değiştirmediği için yüksek ret tek başına teklif kuralının hatası değil, ama düşük değerli eşzamanlı kararlar azaltılabilir. Aylık ödeme takvimi sabit kalsın. Tedarik asgarileri şimdilik **15.000 / 60.000 / 200.000 başlangıç TL**: çıkışlar gerçekleşti, nakit çökünce alım 0'a indi; asgariyi indirmek bunu çözmez.

### C3 bağlantıları / açık sınırlar

- B7 defter/dönem etkilerini C3'te bağla; bu rapor B/B7 olmadan ölçüldü.
- Kalıcı iflas/kapanma nedeni ve olay sayaçları; şu an tam toplam verilemiyor.
- M27 yalnız tek sürüm artışı; slot kartında uyumsuz kayıt cümlesi. A yükleme yolu hazır, CurrentVersion = 2 kaldı.
- A4 “Gez” → StartBranchVisit / EndBranchVisit; A3 AdvanceTime gün/hafta/ay; önceki imzalar hazır.
- A5 gerçek üç yıllık ağ 68/68 tamamlandı; index ve beş menü isteği yukarıda. Son derlenmiş C menüsü gözle incelendi, menü kaynaklarına dokunulmadı.
## C3 doğrulaması — 01.10.2026

- main c26372b temiz alındı; git pull güncel. İlk DERLE geçti, TEST 125/125 (123 temiz, 2 uyarılı; başarısız/çalışmamış 0), Ledger.CashAudit fark 0. Smoke geçti. Derleme/test düzeltmesi: 0 dosya, 0 düzeltme.
- Sıradaki: bot raporuna C3 defter/dönem/hedef/ritim/kapanma ölçümleri; yeni menünün görüntüleri; uzun koşular. Bu ekler henüz doğrulanmadı.

- C3 ölçümleri: defterin fark günleri, imzalı ve mutlak toplamı; dönem planı ve dönem içindeki günlük defter kârı/kasa; hedef ve kutlamalar, gerçek ritim sayaçları; zincir kapanma nedenleri. donemler.csv/c3.csv eklendi. Aynı günü tekrar gözleme testi ve 600 günlük tarzlarda defter sıfır testi geçti. DERLE + TEST 126/126 (125 temiz + 1 motor ağ uyarısı) + Smoke geçti; Short 0,69 sn. Test.ps1 alt sınırı 126.

### C3 uzun bot — sonuç ve C'ye ayar önerileri

12 kampanya, 65.751 gün; satış/stok denetim hatası 0, defter fark günleri/toplam/mutlak toplam 0. 30 yıl 44,5 sn, 10 yıl × 9 43,9 sn; her iki commandlet çıkış 0. Dengeli ulusal 10/20/30: 32/32/33; dünya 20/20/20, üç mağaza, ilk kasa eksisi 141. gün. Büyüme hedefi karşılanmadı. C3_30_yil_rapor.md: beş sayılı bulgu/ayar adayı ve ölçüm sınırlamaları; C3_10_yil_rapor.md ve on CSV teslim edildi. Süper/hiper ve üst tedarik kademesi bu gerçek kampanyalarda açılmadı; reyon denge ayarları doğrulanmış sayılmaz. MarketBrands.cpp:67,75,334,365: boş raf kapasitesi de marka ödemesine yetiyor; mantık hatası/istismar adayı, değiştirilmedi. Aynı raporun 3. önerisi gerçek rafta bulunma koşulu. Bot aile rezerviyle büyür, zarar eden şubeyi kapatmaz; insanın toparlanma stratejilerini tüketmez.

### C3 menü ve şube ziyareti — C'ye istek

- MarketMenuCapture.cpp: Rekorlar ve Finans'ın Dün/Bu hafta/Bu ay/Bu yıl seçimleri eklendi; mali kartları gerçekten görmek için sayfa sonuna kaydırır. DERLE + TEST 126/126 + Smoke geçti. 88/88 PNG, 88 manifest satırı, boyut hatası 0, kampanya değişmedi, örnek ağ kullanılmadı (3 yıllık gerçek atak kampanyası: 3 şube, 1 müdür, 0 depo). Bütün 88 görüntü 22 karşılaştırma sayfasında gözle incelendi; üç sorun tam boyutta da doğrulandı.
- Galeri: Saved/Screenshots/Menu/20261001-101750/index.html. Yeni hedefler, zaman düğmeleri, Rekorlar, dört mali dönem ve Gez görünür. Çökme/boş sayfa yok; mali tablolardaki sıfır satış gerçektir, kırık veri değildir.
- **C'ye istek 1 — Finans tutarları kesiliyor.** 11–14_finance_* iki tema ve iki boyutta: Gider/Net ve Bilanço tutarlarının son haneleri/TL sığmıyor. Özellikle 14_finance_year_light_1280x720.png: net '-49.947,0' diye kesiliyor; özkaynak '-723.132,02 T…'. MarketMenuPages.cpp:1618 ve :1648 civarı, LedgerCards dört/üç eşit Stat sütunu. Öneri: kısa tutar + tam tutar ipucu, sığmazsa alt alta ya da yazıyı küçültmek. Menüye dokunmadım.
- **C'ye istek 2 — Üst zaman hapı harita sekmelerini kapatıyor.** 00–02_map_* bütün varyantlar; 00_map_shops_light_1280x720.png'de Mağazalar katmanı tümüyle, Rakipler'in başı kısmen örtülüyor. MarketMenuWidget.cpp:1195 zaman düğmeleri; MarketMenuPages.cpp:1845 katman tepsisi aynı üst hatta. Ayrı sıra/yatay yerleşim gerekir.
- **C'ye istek 3 — Şube müdürünün kötü karne açıklaması kesiliyor.** 16_shops_* özellikle 720p; '140 hafta kötü k…' düğmelerin altında kalıyor. MarketMenuPages.cpp:277 ManagerLine ve Mağazalar satırındaki düğmeler: açıklamayı ayrı satıra/sarmaya al. Gez düğmesi var; sağdaki karne açıklaması artık okunuyor.
- Ek: harita renk anahtarı hâlâ yok; GoalsCard kutlama yıldızı (MarketMenuWidget.cpp:1422, U+2605) fontta bulunmuyor ve kutu görünüyor (log doğruluyor). Kutlama metinleri 1. yıldan kalmışken ekranda 4. yıl gösteriliyor; ya tarih gösterin ya yakın günlerle sınırlayın. Yönetimdeki 'beceri 87/30', mevcut/gereken beceridir; tavan ihlali diye yorumlanmadı.
- BranchVisitReview çıkış 0: Saved/Screenshots/BranchVisit/20261001-102602, üç PNG gözle incelendi. C3 VisitBranch işaretinden **sonraki** kampanya, plan, saat, oyuncu konum/bakış ve disk kayıtları birebir korundu. Test modu açılmadı. İnsanlar/tabelalar önceki prototip düzeyinde; yeni ekonomi hatası görülmedi. PNG'ler 888×500; ziyaret kontrolünde çözünürlük şartı yok, menüde iki istenen çözünürlük doğrulandı.
- Oturum sırasında Docs/Surec/bekleyen/M28_M29_sirket_finansi.patch geldi (10:19). Başkasının bekleyen teslimi; okunmadı/uygulanmadı, bu doğrulama veya commitlere dahil edilmedi.
## C4 doğrulaması — 01.10.2026

- Claude teslimi: 9d22338, main'e gönderildi. Eski bekleyen yamalar uygulanmadı.
- İlk DERLE: başarılı (62,12 sn); TEST: 135/135; Smoke: PASSED, çıkış 0. Claude dosyalarında derleme/test düzeltmesi: 0.
- Bu adımlar tamamlandı; son ölçüm ve açık denge istekleri aşağıdaki C4 teslim bölümünde.


### C4 bot ve ölçüm altyapısı
- Yeni MarketAutoPlayFinance: normal Director komutlarından şirket kredisi, limit/otomatik kullanım/geri ödeme, yapılandırma, finansmanlı alım/teklif, bağlı şirket çevirme/satışı. Temkinli şirket borcu almaz; dengeli yılda, atak altı ayda teklif değerlendirir; aylık karar bir kez verilir. Gizli teklif kabul çekilişi okunmaz.
- Açılıştan sonra mevcut ağ + yeni şubenin bir aylık sabit gideri kalır. Yatırım kredisi bir sonraki açılıştan sonra üç aylık gider kalacak tutarla değerlendirilir. 90 günden eski, iki ardışık aylık incelemede zarar eden şube normal CloseBranch ile kapanır.
- sube180.csv gerçek defterden ciro/brüt/kira/ücret/SGK/işletme/lojistik/fire/net; banka.csv yıllık not, şirket borcu/FAVÖK, faiz/limit/bağlı şirket; c4.csv kurtarma, kapanma, teklif, finansman, dönüşüm ve çıkış sayaçları. Aynı gün iki kez sayılmaz; yatırım gideri faaliyet kârı değildir.
- Üç yeni test: NetworkReserve, TwoLosingMonths, First180Books. DERLE geçti, TEST 138/138 (137 temiz + 1 motor ağ uyarısı), Smoke PASSED. Test.ps1 alt sınırı 138. C kaynaklarında düzeltme 0. Uzun koşular ve PNG incelemesi sürüyor.


### C4 büyük ağ ölçümü
- 22. tohumun atak koşusu büyüdüğü için ilk şube ölçümü (her şubede 120 günlük defteri yeniden tarama) pahalıydı. Yalnız rapor hesabı tek defter geçişine indirildi; yıllık ilerleme satırı eklendi. C oyun mantığı değişmedi.
- Son DERLE + TEST 138/138 (tamamı temiz) + Smoke PASSED. 30 yıllık yeniden koşuda gunluk.csv, sube180.csv, banka.csv, c4.csv ve c3.csv önceki sürümle bayt bayt aynı. Süre 159,2 → 54,7 sn (ilk koşu diğer süreçlerle eşzamanlıydı; saf hız kıyası değildir).
- Genel rapor artık birleşmiş defter denetimini anlatır; son açık mağaza sayısını tüm koşunun büyümesi gibi sunmaz. Finansmanlı alımların C sayacı gerçek OurBuys üzerinden okunur.


### C4 yedek hesabının son düzeltmesi
- MarketAutoPlayFinance.cpp:17: aile personelinin SGK'sı, atanmış yöneticilerin ücreti/SGK'sı, depo kirası, kamyon ve karanlık mağaza sabit gideri de aylık yedeğe katıldı. Şube çalışan/müdür maliyeti MonthlyFixedCost içinde olduğundan ayrıca tekrar sayılmadı. C'nin oyun hesabı değişmedi.
- NetworkReserve testine aile SGK ve boşta kamyon gideri kontrolü eklendi. DERLE + TEST 138/138 + Smoke PASSED. Önceki 153450/153911 koşuları bu son yedek kuralını içermez; teslim raporları son reserve koşularından üretilecek.


### C4 menü görüntüsü doğrulaması
- Son yedek kuralıyla gerçek üç yıllık bot kampanyası: Saved/Screenshots/Menu/20261001-154910/index.html. 23 hedef × iki tema × iki çözünürlük = 92 PNG; 92/92 doğru boyut, boş/bozuk dosya yok, PASSED ve kampanya değişmedi. Dört varyantlı 23 temas sayfasının tamamı gözle incelendi. Banka kartına başlık üzerinden kaydıran ayrı finance_banking hedefi eklendi (yalnız MarketMenuCapture.cpp).
- Son DERLE + TEST 138/138 (137 temiz, bir motor HTTP bağlantı uyarısı; başarısız/çalışmamış 0) + Smoke PASSED. Claude derleme/test düzeltmesi yine 0. Menü kaynakları değiştirilmedi.
- **C'ye istek 1:** 00–02 harita katmanları alt menünün arkasında; MarketMenuPages.cpp:1979 sabit 82 px yerine gerçek alt menü üst sınırının üzerinde boşluk bırak. İki tema ve iki çözünürlükte de sürüyor; üst zaman/il hapı çakışması giderilmiş.
- **C'ye istek 2:** 12/13/15_finance_* gelir tablosunda gider/net TL eki ve yıllık ciro rakamları kesiliyor. MarketMenuPages.cpp:1755 dört sütun yarım sayfaya sığmıyor; küçük tutarların da sığması için iki sütun/satır ya da metin ölçüsüne göre boyutlandır. Büyük tutarın tam değer alt satırı yararlı; sorun yalnız milyon kısaltması değildir.
- **C'ye istek 3:** MarketMenuPages.cpp:1504 bütün eski aile kredilerini açıyor; şirket banka kartı ilk Finans görünümünde aşağıda kayboluyor. En yakın üç taksit + toplam/ayrıntı açılımı; şirket notu/limitini üstte göster. Otomasyon özel banka hedefiyle kartı doğruladı; oyuncunun normal girişi hâlâ uzun listeye takılıyor.
- Ek istek: 18_management_1280x720 aile müdürü yer açıklaması düğmeler altında kesiliyor; ayrı satır/sarma. Harita renk anahtarı eksik. Ana ekranda yakın tarihli kutlamalar ve kutu yerine nokta düzelmiş.
- Kapsam sınırı: bu gerçek kampanyada bütün uzak şubeler kapanmış ve bağlı şirket alınmamış. Şirket kartının boş durum açıklaması doğru; dolu bağlı şirket satırı ve açık şube müdürü satırının görsel taşması bu galeriyle doğrulanmadı. Para/mağaza ekleyerek sonucu değiştirmedim; C dolu durum incelemesini ayrıca yapmalı.

### C4 teslim — son koşular
- C4_finans_rapor.md sonuç ve beş gerekçeli öneri; C4_30_yil_rapor.md / C4_10_yil_rapor.md ve 16 ham CSV Docs/Surec/akislar altında. Son kaynak f1b9d75; menü 7f4dd3f. Son reserve koşuları: 155215 (30 yıl, 215,9 sn), 160337 (10 yıl, 906,9 sn). 12 kampanya / 65.751 gün; satış/stok denetimi ve defter farkı 0.
- Dengeli 21 ulusal 10/20/30: 35/33/31; dünya 25/32/32. Kurtarma 30 yıl temkinli/dengeli/atak 150/165/5 = 320; 10 yıl dokuz koşuda 230. Seriler ilk on yılı tekrar içerir. Dengeli hedefi karşılanmadı; atak 22/23 146/159 mağaza, diğer yedi 10 yıllık koşu bir mağaza.
- Tam 180 gün mahalle 22 örnek ortalama 7.718,32 TL net, zarar 0; eksik beş mahalle ayrı. Süper 18 örnek 56.119,51 TL, hiper 390 örnek 56.259,59 TL (58 zarar). Nominal/enflasyon ve hayatta kalan örnek seçimi sınırlamaları raporda; ilk 180 gün her şubenin bütün gider kalemleri CSV'de.
- Gerçek teklifler: atak 22 4/1/3, atak 23 6/2/4 (teklif/kabul/ret). Finansmanlı komut kullanıldı ancak ilave satın alma kredisi 0; dönüştürme/satış ve limit geri ödeme doğal koşulları oluşmadı. Banking/Chains testleri geçti; doğal oyunla dolu bağlı şirket yaşam döngüsü ayrıca sınanmalı.
- **C'ye ayar önerileri:** rapordaki beş başlık: kurtarma tekrar faizine tavan/eski borcu birleştirme, aile gider+stok bazlı nefes bütçesi/boş merkez yükünü azaltma, kredi notunda aile borcu kapsamı, mahalle manav/hiper bebek personel yükü, gerçek raf doluluğundan marka ödemesi. Hiçbiri kaynakta uygulanmadı.
- **C'ye mantık isteği:** MarketBrands.cpp:68 depodaki ürün ve bir adet dolu raf bütün kapasiteyi saymaya devam ediyor. EmptyShelfEarnsNothing yalnız raf ve depo ikisi sıfır durumunu koruyor. Depo=1/raf=0 ve kapasite=100/raf=1 ayrı testleri gerekir; C4 marka gelirleri bu sınırlamayla okunmalı.
- **C'ye kayıt isteği:** MarketEconomy.h:749 CurrentVersion hâlâ C3 sürümü 3; yeni Banking/Rescues alanları için M27 gereği sonraki birleşimde tek artış + C3 kaydını reddetme testi. Eski kayıt dalı yazılmadı; bu tur sabit/kural değiştirilmedi.
- Son doğrulama DERLE + TEST 138/138 (137 temiz + motor HTTP uyarısı, başarısız/çalışmamış 0) + Smoke PASSED. 92 menü PNG ve kampanya koruma PASSED. Claude kaynaklarında derleme/test düzeltmesi 0; dosya:satır listesi bu yüzden boş. Kendi bot rapor hesabı/aylık gider yedeği düzeltmeleri önceki bölümde.
## C5 — M32 ana klasör doğrulaması
- Claude teslimi 1bf14f5, değiştirilmeden main'e commit/push edildi; eski bekleyen yamalar uygulanmadı.
- Derleme düzeltmesi 1: MarketOnline.cpp:573, AtLevel çağrısına MarketOnlineLocal namespace'i eklendi; mantık değişmedi.
- DERLE geçti (13,12 sn), TEST 140/140 (139 temiz + bir motor HTTP uyarısı), Smoke PASSED (satış, gün kapanışı, sipariş/mal kabul/personel/disk kayıt). Ledger.CashAudit geçti.
- Bundan sonra akis-a worktree: internet komutları/kartlar/ölçümler, internetsiz karşılaştırma ve üç dönem menü incelemesi. Ana klasör Claude'a bırakılıyor; final notları yalnız bu A.md'de.

### C5 A botu ve ölçüm altyapısı
- MarketAutoPlayOnline.*: yeni OnlineOpen/Close, DarkStore, OnlineAds/Fee/Hire/AutoPolicy oyuncu komutları; uygulama tarz 0/1/2, karşılanamıyorsa 3; salgın temkinli bekler, diğerleri platforma geçer; platform marketinde devam; yeni komisyon %24'ü aşarsa çıkar. İl önerisi normal Decide kartı: kanal aç/kapat onay, depo için kasa en az 5× bedel; alan bayraklarına elle dokunulmaz.
- Aylık ağ yedeği M32 web, uygulama bakımı, reklam, e-ticaret müdürü/SGK ve il karanlık depo sabit giderlerini de içerir. C4 finans/ilk 180 gün/zararlı şube kapatma korunur. Eski online komut adları A botunda yok.
- internet.csv: yıllık kanal sipariş/ciro/katkı kârı, ortak gider/toplam net, ülke payı, fiziksel şirket cirosu, karanlık depo, salgın sipariş/müşteri ve trafik çarpanı. Ortak gider/net/şirket ciro sütunları dört kanal satırında tekrar eder; yıl başına yalnız bir kez okunmalı. Ortak masraf kanallara keyfî dağıtılmadı. internet_olaylar.csv: açılış/kapanış, görünür rakip haber günleri, kart önerileri/seçimleri, komutlar.
- Ayrı internetsiz karşılaştırma: aynı Run işlevinde FOptions.bOfflineCareful ya da commandlet -MirasNoInternet. Yalnız temkinli oynar, hiçbir online kanal/kartla açılmaz; açılırsa denetim hatasıdır. Asıl serinin temkinlisi internete katılabilir.
- Yeni OnlineChoices ve OnlineMonthAndYearBooks testleri: tarz/komisyon/depo eşikleri, internetsiz kampanya, ay devrindeki sipariş/para, ortak gider korunumu, yıl ayrımı ve aynı günü iki kez saymama. DERLE geçti; TEST 142/142 (141 temiz + motor HTTP uyarısı). Test.ps1 sahipliği C'de: alt sınır 140 → 142 yapılması C'ye istek.
- Menü otomasyonu -MirasMenuPhase=start/before/after, -MirasMenuSeed=22; başlangıç gerçek birinci gün, diğerleri salgına -365/+1095 gün. İnternetin il/politika/istatistik kartlarına ayrı hedefler; 26 hedef × 4 = 104 PNG/dönem. Gerçek kampanya kullanılır; örnek ağ ekleme çağrısı kaldırıldı. Kendi çekim kodunda fazladan parantez düzeltildi; Claude kaynak düzeltmesi toplam hâlâ 1.
- Son worktree Smoke PASSED, çıkış 0. Yeni bot/çekim kaynakları DERLE + TEST 142/142 + Smoke ile doğrulandı; uzun koşu ve PNG sonuçları henüz bu aşamanın tamamlandı iddiasına dahil değil.

### C5 — uzun koşu teslimi (01.10.2026)

- Asıl koşular tamamlandı: 30 yıl × 3 tarz × tohum 21 (`Saved/AutoPlay/20261001-200324`); 10 yıl × 3 tarz × tohum 21/22/23 (`20261001-200905`). İnternetsiz karşılık 30 yıl × temkinli 21 (`20261001-200328`) ve 10 yıl × temkinli 21/22/23 (`20261001-200527`). 16 kampanya, toplam 87.668 gün; ilk on yıllar bağımsız örnek değildir. Hepsinde kasa farkı (işaretli ve mutlak) 0, satış/stok denetimi 0 hata.
- Sonuç önce, ayrıntı/ham rapor bağlantıları: [C5 raporu](../../../Saved/AutoPlay/C5_20261001/rapor.md). Her asıl raporda C4 banka/kurtarma/ilk 180 gün başlıkları, yıllık internet bölümü ve CSV var. Kanal kârı katkıdır; ortak masraf ayrı, toplam net bunlardan sonra. Ortak CSV alanları dört kanal satırında tekrar eder, bir kez toplanır. Ülke payı yıl sonu, bizim payımız aile+açık fiziksel şubeler+internet cirosunda; bağlı şirket toplu cirosu hariç.
- Dengeli 21'in 10/20/30. yıl ulusal sırası **35/33/31**, dünya **25/32/32**; internet ciro payı 10/20. yılda **%0/%0**, ülke payı **%2,41/%10,25**. Hedef sıra yakalanmadı. 30. yıl borcu temkinli **461,66 milyon**, dengeli **529,33 milyon**, atak **490,02 milyon TL**; hepsi bir dükkâna döndü. Nominal tutarlar enflasyon içerir.
- Kurtarma: asıl 30 yıllık seri **321** (150/165/6), asıl dokuz 10 yıllık seri **235**, toplam **556**; internetsiz kontroller ayrıca 150+95, bütün seriler 801. Aynı kampanyanın ilk on yılı iki kez geçtiğinden bunu 801 bağımsız vaka gibi okumayın.
- 10. yılda atak 21/22/23 son mağaza **1/131/138**; kasa **-2.621,75 / 6.903.115,38 / 7.057.335,77 TL**. Atak 22/23 internet payı **%0,4618/%0,2806**, yıllık sipariş **24.669/20.499**, yıllık net **163.235,81/15.147,63 TL**, 15'er depo. Dengeli 22/23 pay **%7,5925/%8,7970**, fakat yalnız **62/74 sipariş**, net **-2.930,53/-3.253,23 TL**: yüksek pay küçük fiziksel cirodan geliyor.
- İnternetsiz fark (internetsiz eksi standart): 21'de 10/30 yıl aynı; standart da internete girmedi, teknoloji etkisi ölçülemedi. 22'de kasa **-20.342,55**, borç **-99.045,06**, kasa−borç **+78.702,51 TL**; 23'te **-3.574,92 / -107.669,42 / +104.094,50 TL**. 22/23 internetsiz birer kurtarma daha az; bütün temkinliler sonunda tek dükkân. İnterneti kaçırmanın ek tökezlemesi bu örneklerde görünmedi; iki taraftaki borç döngüsü sürdü. Kasa−borç toplam özkaynak değildir. A botunun aylık ağ yedeği giriş şartı da sonucu etkiliyor; tek başına M32 sabitlerine yüklenemez.
- Kart/il önerileri: atak 22 **114/114/0**, atak 23 **161/161/0** (öneri/onay/ret), diğerleri 0. Bir ilde tekrarlar: atak 23 Şanlıurfa 33, Diyarbakır 23, Samsun 21. Komisyon %24 üstü çıkış ve depo kasası yetersiz ret doğal koşuda oluşmadı; yeni iki testte sınandı. Rakip internet haberleri tohum 21/22/23 için 14/18/20; ilk görünür gün 1315/526/179. Bunlar gizli açılış değil haber gözlemi; tek tek zincir/kart/gün internet_olaylar.csv'de.
- Salgının tam yılı: atak 22 yıl 9, 366 gün, **35.471 sipariş**, **11.630.022 müşteri**, trafik ort. **0,7563**; atak 23 yıl 8, 365 gün, **28.579**, **10.984.076**, **0,7565**. M32 salgın aralığı ile Eras şok aralığı ayrı. Diğer yıllar CSV'de.
- C4 korunumu: tam 180 gün mahalle 27 örnek ort. **7.112,54 TL** net (1 zarar), büyük 17 **54.936,65** (0), hiper 380 **56.149,85** (49). Eksik 180 gün mahalle 3/büyük 1/hiper 119 ayrı. Gerçek zincir teklif/kabul/ret atak 22 **2/1/1**, 23 **4/2/2**; finansmanlı ek kredi/dönüştürme 0, doğal örnek yok. Sıkıcı dönem 0, en uzun sessizlik 19 gün; eski C4 ile bağımsız sürüm karşılaştırması yapılmadı.

**C'ye en önemli beş denge isteği (sabit değiştirilmedi):**

1. Kurtarma tekrar faizini/boş ağ giderini önce incele: tohum 21 üç tarzda 321 plan, 461–529 milyon TL borç, tek dükkân. Kurtarma borcu yeniden büyütmeyen bir senaryoyu test et.
2. Tohum etkisini ayır: atak on yılda 1/131/138 mağaza. Başlangıç, teknoloji takvimi, şube kârlılığı ve finansmanın ayrıştırılmış deneyi gerek; başarılı iki tohum genel denge kanıtı değil.
3. İnternet hacmi ile sabit gideri birlikte ölç: küçük ağ yılda 62–74 siparişle 2,9–3,3 bin TL zarar, büyük ağ 15 depoyla yalnız %0,28–0,46 ciro payı. Önce mağaza/il talep ve kapasitesi, sonra web/app/depo giderleri; ülke payını yükseltmek tek başına çözmez.
4. İl teklifleri ve platform katkısı: 275 onay/0 ret, bir ilde 33 tekrar; zarar kapatma→yeniden açma bekleme süresini izle. Atak 22 bütün on yılda platform katkısı sipariş başına **4,56 TL**, uygulama **33,86 TL**, ortak gider öncesi. Reklam/depo önerisi düşük katkıyı aşan masrafı da gerekçelendirsin.
5. Hiper reyon yükü: gözlenen 30 günlük toplam en kötü balık **-152.537,80 TL**, ev **-100.398,37**, kırtasiye **-52.830,75**. Hiper bebek **13/13** gözlem zararlı (**-18.153,92..-61,96**); kasap/manav/fırın yüksek kârda. Toplam şube sayısından etkilenir; Ratio/Margin/personel ayarını şube başına olgun örnekle belirle.

### Menü sadeleştirme listesi — C'ye istek (C5 / A8 hazırlığı)

Görüntü kökü `Saved/Screenshots/Menu/`: **S**=`20261001-200405_start` (gerçek 1. gün), **Ö**=`20261001-200649_before` (2522. gün, salgın 2887; -365), **A**=`20261001-201646_after` (3982. gün; +1095). Tohum 22, atak, gerçek oyuncu komutları; bedava para/ağ eklenmedi. Her dosyanın `_light_1920x1080`, `_dark_1920x1080`, `_light_1280x720`, `_dark_1280x720` sürümleri var. Aşağıdaki adlar ortak köktür; `Review/00..25.png` dördünü yan yana gösterir, dönemlerin index.html dosyaları tek PNG bağlantılarını içerir. Menüye/C kaynaklarına dokunulmadı.

(a) kesilme/taşma/boşluk; (b) anlamı belirsiz sayı/düğme; (c) yeni oyuncuya erken görünen; (d) tekrar. Kaydırılabilir listenin ekran dışında devam etmesi tek başına hata sayılmadı. Tablo gözle görülen kısımları kapsar; açılır açıklamalar ve bütün liste satırları ayrı etkileşim testi değildir.

| Sayfa / kaynak | Görüntü | (a) | (b) | (c) | (d) |
|---|---|---|---|---|---|
| Ana ekran; MarketMenuPages.cpp:2088,2105,2246 | S/Ö/A `00_map_shops`, `01_map_rivals`, `02_map_opportunities` | Harita katman seçimleri alt gezinti şeridinin arkasında, iki boy/temada. | Rakip/fırsat renklerinin görünür açıklaması yok. Hedefte “Bu hafta: Bu hafta” / “Bu yıl: Bu yıl” tekrarı. | İlk gün ülke/yurt dışı/bölge çipleri tüm haritayla birden sunuluyor; önce aile dükkânı/ilk iş görünmeli. | Mağaza adetleri Şirket; hedef/kutlama Rekorlar; kasa üst şerit+Finans. |
| Sipariş; MarketMenuWidget.cpp:1477 | S/Ö/A `03_orders` | Belirgin yatay kesilme yok; satırlar normal kaydırılıyor. | Raf/depo/kabul/yolda/dün satış/boş raf/öneri/liste sekiz sütun; satın alma kararını öne çıkar. | İlk gün faturalar/zammı yansıt/ucuz toptancı seçenekleri, yapılacak iş yokken de görünür. | Raf/depo/dün satış/öneri Fiyat ayrıntısında tekrar. |
| Ürünler ve fiyat; MarketMenuPages.cpp:572,762 | S/Ö/A `04_products` | Belirgin kesilme yok; 720'de kategori ikinci satıra geçiyor. | “Alan müşteri ~%88” tahminin hangi müşteriye/talebe ait olduğu açık değil; alış/liste/karşılaştırma/marj bir kartta yoğun. | İlk ürün fiyatını öğrenirken birden üç rakip ayrıntısı gereksiz yük. | Stok/öneri Sipariş'te, kampanya bağlantısı Kampanyalar'da; yalnız karar için gerekli özet kalsın. |
| Kampanyalar; MarketMenuPages.cpp:820 | S/Ö/A `05_promotions` | Açık alanda bozuk/boş kart yok; kaydırma var. | Ürün/marka/alt grup/reyon/tüm mağaza, dört kampanya türü, oran ve süre aynı anda; beklenen maliyet/kâr önce gelmeli. | İlk gün marka/tüm mağaza/gondol seçeneklerini basit ürün indiriminin arkasına al. | Seçili ürün Fiyat'ta, biten kampanya sonucu gün raporunda; bağlantı kullan. |
| Rakipler; MarketMenuPages.cpp:1056 | S/Ö/A `06_rivals_local`, `07_rivals_national`, `08_rivals_world` | İlk gün ulusal/dünya listesi tek satır ve büyük boşluk; açıklamalı kilit daha doğru. Sonraki listeler normal kaydırılıyor. | İlk gün 0 ciroyla “1. sıradasın” hem ülke hem dünya için yanıltıcı. “Senden çekildiği savaş:89” sonucunun etkisi belirsiz. | Dünya listesi/yurt dışı açıklaması ve milyarlarca liralık zincire Teklif ver çok erken. | Yerel fiyatlar Fiyat kartında, rakip haberleri Gün sonu; ulusal sıralama Şirket/Rekorlar ile ilişkilendirilmeli. |
| Personel; MarketMenuPages.cpp:1309 | S/Ö/A `09_staff` | İlk gün başvurular kartı boş ama üstte “Aday listesinde İK müdürü var” yazıyor; havuz/veri koşulunu incele. Geç dönemde liste kaydırması normal. | Moral/yorgunluk/beceri/hız tek satırda; “hız 57” yerine kasaya/rafa etkisi. | İK müdürü ve muhasebe kartı ilk gün ekibin görevlerinden önce dikkat çekiyor. | Vergi özeti Finans'ta, boşta kişilerin özeti çalışan listesinde; öneriye dönüştür. |
| Finans; MarketMenuPages.cpp:1452,1609,1710,1755; MarketMenuWidget.cpp:330 | S/Ö/A `10_finance`, `11_finance_banking`, `12_finance_day`..`15_finance_year` | Büyük tutarların TL/milyon sonu gelir tablosunun dört dar kartında kesiliyor; özellikle Ö/A 720 ve 1080. Küçük tam tutar kurtarıyor ama büyük sayı bozuk. | FAVÖK, oran, not, beş banka ve birçok vade tek blok; net aile mi şirket mi belli olsun. Banka borcu ana kart 652 bin, aile kartı 0; kapsamı etiketle. | İlk gün kilitli şirket kredisi/tahvil/limitin tamamı görünüyor, aynı kilit cümlesi beş kez. | Kasa üst şerit; vergi Personel; gelir dönemleri Raporlar; taze ürünün yeri satış/ürün politikasına bağlanabilir. |
| Satış kanalları; MarketMenuPages.cpp:1809,1895,1913,1978; MarketOnline.cpp:958,1312 | S/Ö/A `16_channels`, `17_channels_areas`, `18_channels_policy`, `19_channels_stats` | S yalnız ödeme: doğru. Ö/A iki sütunun uzunluk farkı ve alt kaydırma büyük boşluk bırakıyor; istatistik üst/alt görünürlük için `17` esas alınmalı. | Ö'de “30 gün ~0 sipariş, kâr630TL”: tamsayıyla azalan sipariş tahmini, kâr başka azalan sayaç; gerçek dönem toplamı sanılıyor. `~` eksi değildir. A'da 15 depo/145 mağaza ama dün49 sipariş; kapsam açıklansın. Katkı kârı ortak gider sonrası netten ayrı adlandırılsın. | M32 tarih kilidi başlangıçta iyi çalışıyor; sonradan yetkili müdür olmayan oyuncuya tüm il düğmelerini yığma. | İnternet özeti+dört kanal satırı+geçen ay metni; müdür becerisi Yönetim/Personel'e bağlantıyla; politika ayrıntısı ihtiyaç üzerine. |
| Mağazalar / Yönetim / Şirket; MarketMenuPages.cpp:2737,3253,3633,3888 | S/Ö/A `20_shops`, `21_management`, `22_company` | Boş ilk yönetim/şirket panelleri çok yer kaplıyor; sonradan listeler normal kaydırılıyor. | Şube açıldıktan sonra hâlâ “İlk şube için” kartı ve eski şartlar; beceri72/30, depo128/60 etkileri metinle açıklansın. | İlk gün kilitli aile müdürü, merkez alım, özel marka, kamyon/satın alma seçenekleri mevcut. | Bağlı5kişi özeti hem mağaza hem yönetimde; şirket adetleri harita ve üst şerit; müdür suçlaması ana bildirim+Yönetim. |
| Raporlar / gün / hafta / rekor; MarketMenuWidget.cpp:1616,1728,1443 | S/Ö/A `23_reports_day`, `24_reports_week`, `25_reports_records` | Gün sonu haber yığını kaydırmalı; önemli karar önce. İlk gün grafik/rekor sıfırları büyük boş alan, veri yok durumunu kısa tut. | İlk gün “Herkes aradığını buldu” henüz satış yokken yazıyor. Gün neti ağ dahil, ciro aile dükkânı: aynı kapsam sanılmasın. Grafikte eksi/arti sıfır çizgisi açık olsun. | Daha gün kapanmadan tüm net/ciro/gider/kayıp/rekor kartları; ilk gün kapanışı yönlendirmesi yeter. | Dün net üst şerit/Finans; kayıp müşteri Fiyat; rakipler Rakipler; hedef kutlaması ana ekran. |

**Çekim sınırı:** `19_channels_stats` başlığa kaydırmada kısa sağ sütunu yukarı taşıdığı için bazı istatistik satırları kesik; `17_channels_areas` ve `18_channels_policy` aynı istatistik kartını tam gösteriyor. Bu özel kaydırma A çekiminin sınırlamasıdır, C menüsünün bozuk veri ürettiği iddiası değil. Ö'de uygulama salgından 547 gün önce açılmıştır; “yalnız web/platform” gibi sahte durum zorlanmadı. A'da dört kanal, 15 depo, dolu il/politika/istatistik gerçek durumdan.

**C'ye bağlama:** akis-a'nın M32 bot/ölçüm ve üç dönem çekim commit'lerini birleştir; yukarıdaki menü listesini C/A8 uygulasın. Test.ps1 alt sınırını 142 yap. M27 için M32 kayıt alanları da dahil sonraki birleşimde CurrentVersion'ı tek kez artırıp önceki sürüm reddini sınayın (A bu tur kayıt sabiti sahibi değil). Menü veya denge sabiti değişmedi.

### C5 son doğrulama ve teslim
- Son kaynakta DERLE başarılı; TEST **142/142** (141 temiz, bir motor HTTP uyarısı; başarısız/çalışmamış 0), Smoke **PASSED**. Claude kaynağında derleme düzeltmesi **1**, test düzeltmesi **0**; oyun sabitleri değiştirilmedi.
- Üç çekim de **PASSED: 104 PNG, campaign unchanged**: toplam **312 PNG**, boyut hatası **0**. Dört varyantı birlikte gösteren **78 temas görüntüsünün tamamı gözle incelendi**. Başlangıç yalnız ödeme kartı; sonraki dönemler gerçek dolu kampanya. İstatistik kaydırma sınırı yukarıda açıklandı.
- [Üç dönem karşılaştırma galerisi](../../../Saved/Screenshots/Menu/C5_20261001/index.html), [denge raporu](../../../Saved/AutoPlay/C5_20261001/rapor.md) ve [internetsiz fark CSV](../../../Saved/AutoPlay/C5_20261001/internetsiz_fark.csv). Ham CSV/PNG dosyaları Saved altında yerel çıktıdır, git'e eklenmez; takip edilen teslim ve C'nin uygulayacağı liste bu A.md'dir.

## C6 — ilk birleşim ve doğrulama (01.10.2026)
- C5 caf32d2 main'e birleştirildi/push edildi. Claude M33–M35 teslimi 37eae48; kullanılmayan MarketRetail.h/.cpp kullanıcı talimatıyla silindi. Bekleyen eski finans yamaları alınmadı.
- Birleşim derleme düzeltmeleri (A botunun kaldırılan M32 reklam alanları): MarketAutoPlayFinance.cpp:26 eski AdsMonthly/Online.Ads yerine MarketAdvertising aylık kanal gideri ve müdür ücreti; MarketAutoPlayOnline.cpp:74 eski Online.Ads yerine Search LevelOf. C oyun mantığı/sabitleri değişmedi. Test.ps1:10 C5 iki testi dahil alt sınır 145.
- İlk DERLE bu iki eski alan nedeniyle başarısız; düzeltme sonrası DERLE/TEST/Smoke sürüyor. C6 bitmiş değildir.

- İlk birleşim son doğrulama: DERLE geçti (13,59 sn), TEST **145/145 temiz**, Smoke **PASSED**, Ledger.CashAudit geçti. C kaynak düzeltmesi 0, A'nın eski reklam bağlantısı 2 uyarlama. Bundan sonra akis-a worktree; C6 reklam/komuta botu ve koşular sürüyor.


### C6 A botu ve ölçümleri
- MarketAutoPlayCommand.*: yalnız Director komutlarıyla AdLevel/AdHire/AdAuto/AdBudget. Temkinli kapalı; dengeli 10 mağazada broşür+sosyal 1, internet varsa arama 1; atak 30 mağazada TV/radyo 1+sosyal 2. 20 mağazada yedek+aylık ücret karşılanabiliyorsa reklam müdürü; karışım onda, bütçe dengeli binde20/atak binde30. Müdür devralınca manuel tarz karışımı uygulanmaz. Online botunun eski arama kararı kaldırıldı; reklam karışımını ezmez.
- Komuta açma kartı CanOpen + C4 ağ yedeği/aylık yeni şube gideri; kapama prompttaki “3 aydan uzun” gereği LossMonths >3 ve halen zarar. Onay/ret normal Decide. C4 iki zarar ayı kapatma kuralı korundu; bu iki eşik farklıdır.
- reklam.csv takvim yılı/ülke/kanal harcama ve tahmini ek ciro. C'nin ciro tahmini (internet satışını da içerir) beş marka kanalının akılda kalan payına dağıtılır, yuvarlama toplamı korunur; gerçek kazanç/kâr deneyi değildir. Aramanın ayrı internet ek ciro tahmini C'de yok: 0/ölçülmedi, etkisiz demek değil. Müdür gideri ayrı. İlk/son takvim yılı kısmi; lig yılının sınırıyla karıştırılmamalı.
- komuta_olaylar.csv açma/kapama öneri ve seçimleri, gerçek yeni stok eritme ürünleri/haftalık metni, sokak adları/kapanışlar, ülke kişi/toptancı/banka/platform adları. İlk açık/kapalı durum ve semt pazarının kurulup toplanması kalıcı kapanış sayılmaz. ilk_yil_hava.csv ilk365 günün hava/sıcaklık/yasal kapanış/tatil kaydı.
- Yeni iki test: CommandAndAdvertisingChoices (tarz/eşik/müdür/bütçe/aynı gün/yedek), AdvertisingBooksAndClearance (ay devri/katkı dağıtımı/arama kapsamı/stok indirimi/aynı gün/ilk durum-kapanış ayrımı). Son DERLE başarılı, TEST **147/147 temiz**, Smoke **PASSED**. Kısa bot da tam test kümesinde geçti. Test.ps1 alt sınırı 145; 147 yapılması C'ye istek (worktree sahiplik kuralı).
- Kendi rapor hatası: ilk denemede pazarın haftalık kapanışı sokak kapanışı başlığındaydı; son kaynakta ayrı etiket ve ilk durum ayrımı, test eklendi. Oyun kararı değişmedi. İlk 205802/205853/210217 çıktıları son teslim değildir; son kaynakla koşular tekrar başladı.
- Uzun koşular ve C6 PNG gözle incelemesi sürüyor; bu alt adım bot/ölçüm altyapısını doğrular, C6 son teslimi değildir.
