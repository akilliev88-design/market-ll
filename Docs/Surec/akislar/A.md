# Akış A — Codex

Son teslim (01.10.2026): A1/A3 hazır ve push edilmiş; A4 cf5ee8c hazır. A5 görüntü otomasyonu doğrulandı; gerçek botla üç yılda dolu şube/müdür/depo şartı karşılanmadı, aşağıdaki açıkça etiketli inceleme örnekleri kullanıldı. Menüye dokunulmadı.

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
