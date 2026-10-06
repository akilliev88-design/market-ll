# Kurgu kitabı

Sahibi: Claude (Mustafa'nın 28.09.2026 kararı: 3B model üretimi ve arayüz tasarımı dışında oyunun bütün kurgusu ve arka plan kodu Claude'da).
Bu belge oyunun **bugünkü** kurgusudur. Geçmiş kararların gerekçesi `01_KARARLAR.md`'de durur; geçersiz kalanlar orada "Geçersiz" diye işaretlidir ve uygulanmaz. Ayrıntılı dünya yapısı `09_DUNYA_YENIDEN.md`, ülke paketi standardı `10_ULKE_STANDARDI.md`, tek ekonomi `11_TEK_EKONOMI.md`.

**04.10.2026 (M69) yeniden yazıldı.** Önceki sürüm 2011, Lüleburgaz, "babandan kalan dükkân", Nermin teyze, Cem, Selim, Bereket Market ve yedi hikâye bölümü anlatıyordu. Bunların hiçbiri oyunda yok; yeni işte bu eski anlatıma dönülmez.

Kural: **her sistem dünyadan bağımsız bir modüldür** (`Market*.h/.cpp`), otomasyon testi vardır ve oyuna tek bir yerden bağlanır (`MarketDirector`: gün açılışı, gün kapanışı, anlık çarpanlar). Rastgelelik kampanya tohumu + gün + nesneye bağlıdır: kaydı yüklemek sonucu değiştirmez. Motor ülke bilmez (M52): ülkeye özgü her şey `Config/ulkeler.json`'dadır.

## 1. Oyunun sözü

Football Manager gibi süren bir **işletme simülasyonu ve tycoon**. Oyuncu küçük bir marketle başlar ve onu kendi oyunuyla büyütür; dünya devi olmak mümkündür ama herkes olamaz. Oyunun sonu yoktur: şirket var olduğu sürece oyun sürer (M56, M69).

**Yeni oyun (M69):** oyuncu sırayla seçer:

1. başlangıç **ülkesi** (ülke paketi: para birimi, takvim, yasalar, rakipler, ekonomi),
2. **şehri** (il; varsayılan il yok),
3. **marketin adı** (zorunlu; tabelada, haberlerde, listelerde bu ad geçer; `Company.BrandName`),
4. **kendi adı** (isteğe bağlı; `PlayerName`),
5. **zorluk** (Rahat / Normal / Zor).

**Tek hikâye başlangıçtır:** ilçede tek şubeli küçük bir market (bakkal değil). Onu yıllarca bir aile işletti; aile yoruldu ve işi oyuncuya devretti. Bina marketle birlikte gelir (oyuncunun mülküdür, kira yoktur). Kasada bir aylık gider, bir kasiyer, iki reyon görevlisi, yarı dolu raflar ve toptancıya **hafif bir borç** vardır. Anne, baba ya da başka akraba yoktur; "aile dükkânı" sözü kullanılmaz.

Bundan sonraki hikâyeyi oyuncunun kararları yazar. Oyun bölümlere ayrılmaz, zorunlu hedef yoktur; dönüm noktaları **hatıra** ve **ilk** olarak kaydedilir (ilk şube, ilk il, ilk yurt dışı mağaza, dünya listesine giriş, dünya birinciliği…).

## 2. İlk mağaza ve borç

- İlk mağaza oyunda **kendi adıyla** geçer: market adı + şehir ("Yıldız Market Van"). Gerektiğinde yanında "ilk şuben" yazar (`MarketStart::FirstStoreName`).
- Bina bizimdir: kira ödenmez, bilançoda varlık olarak durur (`MarketFinance::BuildingValue`). Şubeler kiracıdır.
- **Devralınan borç:** ilk ayın sabit giderlerinin bir buçuk katı. Süresi, faizi, cezası yoktur; hiçbir şeyi kilitlemez, hedef ya da kutlama değildir. Oyuncu istediği zaman bir taksit (P) ya da tamamını (Finans) öder.
- İlk şube için yalnız oyunun kendi şartları vardır: birkaç kârlı gün, ilk mağazanın çevresinde yeterli pay ve açılış parası (`MarketBranches::CanOpen`).
- İlk mağaza da diğerleri gibi **kapatılabilecek** (karar M69; kodu G-110'da: kapatılınca gezilecek mağaza başka bir şubemize geçer).

## 3. Zaman

| Kavram | Kural | Modül |
|---|---|---|
| Oyun günü | Bir oynanan gün = bir takvim günü (mağaza açıkken gerçek 4 dakika) | `MarketGame` |
| Yıl | **Oyunda gerçek yıl yoktur** (M60): metinler "3. yıl" der. İçerideki sabit takvim tarihi yalnız hafta günü ve bayram hesabı içindir | `MarketCalendar` |
| Mevsim, hava | Deterministik; kategori talebini ve yaya sayısını etkiler (ülkenin iklimi pakette) | `MarketCalendar` |
| Bayram ve özel günler | Ülke paketinden; hicri bayramlar her yıl kayar | `MarketCalendar` |
| Maaş günleri | Ayın başı, ortası ve son iş günü sepet büyür; ay sonu bütçe daralır | `MarketCalendar` |
| Enflasyon ve ücret | Her ülkenin kendi eğrisi (paket `economy`), kampanya yılına göre; toptancı listesi her ay ±%2 oynar | `MarketPrices`, `MarketSuppliers` |
| Dönemler | Kur şoku, durgunluk, salgın, yüksek enflasyon, toparlanma: her kampanyada kayarak gelir (M60) | `MarketEras` |

Uzun oyun için **stratejik ilerletme** vardır: mağaza kapalıyken bir gün, hafta ya da ay aynı kurallarla, yürüyen insan olmadan oynanır; karar bekleyince, kasa eksiye düşünce ya da rapor gelince durur (`MarketSimulation`).

## 4. Müşteriler

Müşteri bir **segment** ile gelir (`MarketCustomers`): emekli, aile, iş çıkışı, öğrenci, esnaf, çocuk. Segment; geliş saatini, listeyi, adedi, bütçeyi, fiyat hoşgörüsünü, sabrı, yürüme hızını ve raf önünde bakma süresini belirler. Tanıdık müşterinin segmenti sabittir. Bütçe maaş gününde artar, ay sonunda düşer. Pahalı ürünü alma ihtimali rakip fiyatına göre değişir (`MarketProductDemand`). Her mağazanın günlük müşterisi tek formülden gelir (`MarketStoreDemand`): ilin alışverişi × pay × alışkanlık × yamyamlık.

**Hareket** (`MarketMotion`): kişiye sabit yürüyüş tarzı; listeyi en kısa yürüyüş sırasına koyma; kalabalıkta yol verme; uzun kuyrukta sepeti bırakma; iki tanıdığın kısa sohbeti.

## 5. Rakipler

Rakipler bütçesi, stratejisi ve hafızası olan **zincirlerdir** (`MarketChains`): il, bölge, ülke ve dünya kadrosu; aylık kararlar, fiyat savaşı, ezeli rakip, satılık zincirler ve satın alma. Rakipler oyuncunun verisini görmez; gözlenebilene (raf fiyatı, pay, açılan mağaza) tepki verir. Yerel küçük marketler ülkenin soyadlarından kurgu adlar alır (`MarketCast`). Listeye girmek bir başarıdır: dünya ilk 50, ülke listesi nüfusa göre 10–30 (M53).

## 6. Tedarik

- **Eski toptancı** (adı ülke paketinden): market yıllardır onunla çalıştığı için güven biraz yüksek başlar. Peşin başlar; düzenli ödeme 7, sonra 14 gün vade getirir; hacim %3–5 iskonto getirir. Nakit sıkıntısında "dükkânın eski hatırına" kampanya başına en çok üç kez üç günlük mal verir (M64).
- **Ucuz toptancı:** biraz sonra gelir, %4 ucuz ama eksik/kırık mal ve vade yok.
- **Tedarik ağı:** hat kademeleri, markalarla doğrudan anlaşma, depolar ve kamyonlar (`MarketSourcing`, `MarketDepots`, `MarketBrands`).
- Zam listesi ayın 1'inde gelir; zammı rafa yansıtmayanın marjı erir.

## 7. Mağaza işleri

- **Raf dizimi:** oyuncu elle dizer; görevliler kurala göre doldurur (`StaffPlanner`). Yeni şubeye otomatik planogram (`MarketLayout`).
- **Tazelik:** parti ve son kullanma günü, FEFO, son gün indirimi, fire (`MarketFreshness`).
- **Kampanyalar:** reyon indirimi, 3 al 2 öde, broşür, gondol başı, toptancı destekli teklif (`MarketPromotions`); şirket reklamı (`MarketAdvertising`).
- **Olaylar:** dolap arızası, elektrik kesintisi, denetim, düğün siparişi, taziye, kar, kamyon arızası…; her biri bir karar sunar, ritim koruyucusu üst üste kötü olayı tutar (`MarketEvents`, `MarketGoals`).
- **Kimlik:** ilk haftalardan sonra bir kez sorulur: Mahalle Marketi, Kaliteli Market ya da Hızlı İndirim. Bütün mağazalar bu kimliği taşır (`MarketStory`).

## 8. Şubeler ve şirket

- Şube açmak bir süreçtir: il ve tür seç → sözleşme ve depozito → tadilat → izin → işe alım → açılış stoğu → olgunlaşma (`MarketBranches`). Türler: ucuzcu, mahalle, yakın, süpermarket; büyük türler (hipermarket, toptan) şirket **8 mağaza ve 2 ile** ulaşınca açılır (M69: bölüm yerine büyüklük).
- Ziyaret edilmeyen mağazalar özet simülasyonla işler; ziyaret edilen mağaza aynı veriden kurulur (`MarketBranchVisit`).
- Yönetim kademeleri: mağaza, il, bölge, direktör, ülke, kıta müdürleri; doğrudan bağlı 5 kişi sınırı (`MarketManagers`).
- Yurt dışı şirketin büyüklüğüyle açılır (ana ülkede 25 mağaza, 5 il; M67); önce birkaç ay süren pazar araştırması ve "girmeye değer / zor / girmeyin" sonucu (M68); kendi mağazayla ya da ortaklıkla giriş; her ülkede bir alt şirket (M65).
- Strateji yolları ve il atağı (M48–M50, `MarketStrategy`).

## 9. Satış kanalları ve ödeme

İnternet mağazacılığı telefonla başlar; web ve platform kampanyanın ilerleyen yıllarında açılır. Kurye kapasitesi, depodan/raftan toplama, ikame kuralı, itibar; ilin online payı mağazalardan müşteri çeker (`MarketOnline`). Salgın dönemi her kampanyada farklı yaşanır, kapatılabilir. Ödeme: nakit, kart, yemek kartı; POS kirası ve komisyonu (`MarketPayments`).

## 10. Finans

- Muhasebe defteri: her para hareketi kayda geçer; gelir tablosu, bilanço (ilk mağazanın binası dahil), kasa denetimi (`MarketLedger`).
- Banka kredisi, kredi hattı, yabancı para kredisi (`MarketBanking`, `MarketFinance`); vergi ve mali müşavir (`MarketStaff`).
- Oyuncunun maaşı ve kişisel serveti şirketin kasasından ayrıdır (M37, `MarketOwner`).
- Ödeme sıkıntısında oyun bitmez: önce uyarı, sonra vadeler kapanır, seçenekler sunulur, en sonda bankanın kurtarma planı (M31).

## 11. Hedefler, ilkler, rekorlar

Kısa (hafta), orta (ay) ve uzun (yıl) ölçekte **öneri** hedefler oyuncunun kendi sayılarından türetilir; hiçbiri zorunlu değildir ve hiçbir şeyi açmaz. İlkler (ilk şube, 5/10/25… mağaza, 2/5/10/20 il, ilk depo, yurt dışı, ulusal pay, dünya ilk 10 / ilk 3 / birinci) ve rekorlar kutlama kartı ve hatıra olur (`MarketGoals`).

## 12. Ad

Oyunun satış adı henüz seçilmedi (aday: Supermarketing, Market Share, Chainmaker). Oyuncuya görünen ad tek yerden gelir (`Config/DefaultGame.ini` → `ProjectName`). İç kod adı satış adından bağımsızdır: **MarketSim** (M69). "Miras" adı hiçbir yerde kullanılmaz.
