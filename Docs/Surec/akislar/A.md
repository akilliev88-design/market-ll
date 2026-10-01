# Akış A — Codex

Son teslim (01.10.2026): A1/A3/A4/A5/A6 hazır, DERLE + TEST 105/105 + Smoke geçti. A5 gerçek ağla 68/68 (örnek ağ yok). C teslimi, M27 ve bot push edildi; iki son uzun koşu 12 kampanya/65.751 gün/0 denetim hatası. Denge hedefi karşılanmadı: son dengeli 10/20/30. yılda ulusal 74/63/60, dünya 20/20/20. Raporlar ve C ayar önerileri aşağıda; kod sabitleri değiştirilmedi.

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