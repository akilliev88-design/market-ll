# Akış A — Codex

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
## İkinci tur — A4 (01.10.2026)

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