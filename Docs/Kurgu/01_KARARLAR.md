# Kararlar — oyunun bütün konuları

Oyundaki her konu burada bir satırdır: **ne karar verildi**, **hangi modülde**, **durum**. Yeni bir konu akla gelince önce buraya satır eklenir. Kurgu gerekçesi `00_KURGU_KITABI.md`'de, ayrıntılı tasarım `Docs/Planlama/` içindedir. Mustafa'nın doğrudan kararı **(M)** ile, Claude'un kurgu kararı **(C)** ile işaretlidir. (M) kararı değişmeden (C) kararı onu çiğneyemez.

Durum: **Uygulandı** · **Derlenmedi** (kod var, Unreal'de denenmedi) · **Sırada** · **Tasarım** (kararı var, kodu sonra) · **Açık** (Mustafa'ya sorulacak).

Sahiplik (M, 28.09.2026): 3B model üretimi, mağaza modeli ve arayüz tasarımı dışında her şey Claude'da. Mağaza **düzeni** (raf dizilimi, şube planı) Claude'da.

## A. Zaman ve dünya

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| A01 | Başlangıç tarihi | 7 Mart 2011 Pazartesi = gün 1. Hafta Pazartesi başlar (C; 2011 başlangıcı M) | `MarketCalendar` | Derlenmedi (G-061) |
| A02 | Gün uzunluğu | Açık dükkânda 4 gerçek dakika. **Menü açıkken zamanın akması ya da durması oyuncunun ayarıdır** (M 29.09.2026; varsayılan: akar, G-075 davranışı). Ayar kayda değil kullanıcı ayarlarına yazılır | `MarketGame`, `MarketMenu` | Derlenmedi (G-076) |
| A03 | Bayram ve özel günler | 2011'in gerçek tarihleri; hicri bayramlar her yıl ~11 gün kayar (C) | `MarketCalendar` | Derlenmedi (G-061) |
| A04 | Mevsim ve hava | Deterministik; kategori talebini ve yayayı etkiler (C) | `MarketCalendar` | Derlenmedi (G-061) |
| A05 | Maaş günleri | Ayın 1'i, 15'i ve son iş günü trafik/sepet artar; ay sonu bütçe daralır (C) | `MarketCalendar` | Derlenmedi (G-061) |
| A06 | Enflasyon | Oyunun kendi eğrisi: tanıdık biçim, yumuşak tepeler (2022 %30, 2023 %28); toptancı listesi her kampanya ve ayda ±%2 (M 29.09.2026: eğlence ve tahmin edilemezlik, birebir tarih değil) | `MarketPrices`, `MarketSuppliers` | Derlenmedi (G-073) |
| A07 | Asgari ücret | Yarı yıl sonu fiyat düzeyi × yılda %1,5 reel artış; ücret beklentisi bunu izler (M 29.09.2026) | `MarketPrices` | Derlenmedi (G-073) |
| A08 | Yer | Lüleburgaz esinli kurgu semtler (İstasyon başlangıç). Sonra Trakya, Türkiye, Bulgaristan pilotu (C) | Derlenmedi (G-068) |
| A09 | Stratejik ilerletme | Dükkân kapalıyken 1 gün / 1 hafta ilerlet: yürüyen insan olmadan aynı kurallarla gün; aile rutini (zam, vergi, borç taksiti, raf, önerilen sipariş); karar bekleyince, kasa eksiye düşünce ya da hafta bitince durur (C; plan 01 §3). Görev devriyle ay ilerletme şirket büyümesinde | `MarketSimulation` | Derlenmedi (G-071) |
| A10 | Zorluk | Rahat/Normal/Zor: müşteri sayısı (±%10/−%8) ve fiyat hoşgörüsü (+0,05/−0,04). Tarih (enflasyon, bayram, rakip açılışı) değişmez; rakip saldırganlığı ve olay yoğunluğu ayarı sonra (plan) | `MarketSimulation` | Derlenmedi (G-071) |
| A11 | Marka adları | L12 ile değişti: varsayılan kurgu ad (gerçeğe yakın); F8 geliştirme için gerçek adı gösterir (M 29.09.2026) | `ProductCatalog` | Derlenmedi |
| A12 | Gerçek zincirler | **Gerçek zincir adları kalır ve bütün rekabet davranışında (fiyat savaşı, açılış yarışı, kapanma, satın alınma) kullanılır** (M 29.09.2026). Her gerçek zincirin karşılığı olan kurgu ad ve logo rengi `Config/zincirler.json`'da hazır tutulur; yayın kararı verilirse tek ayarla kurgu adlara geçilir (F8 marka anahtarı gibi) | `MarketCompetitors`, `MarketRetail`, `Config/zincirler.json` | Kısmen: dosya hazır (G-077), koda bağlama G-079 |

## B. Hikâye

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| B01 | Babanın borcu | 300 TL, süresiz, kapanmadan ikinci şube yok (M) | `MarketCampaign` | Uygulandı |
| B02 | Bölümler | 7 bölüm: Defter → Karşı Dükkân → İkinci Tabela → Trakya → Türkiye → Sınır Ötesi → Miras (C) | Derlenmedi (G-066) |
| B03 | Karakterler | Nermin teyze, Cem, Selim, Necati Bey, Kadir Bereketoğlu, Derya, banka müdürü (C; plan) | Derlenmedi (G-066) |
| B04 | Sat ya da devam | Satmak "Sattın" sonunu gösterir ve hatıraya yazar; sonra "Rüyaymış: dükkâna dön" (varsayılan) ya da "Burada bitsin" (serbest oyun) (M 29.09.2026) | `MarketStory` | Derlenmedi (G-073) |
| B05 | Strateji kimliği | Mahallenin Bakkalı / Kaliteli Yerel / Hızlı İndirim Zinciri (C; plan) | Derlenmedi (G-066) |
| B06 | Başarısızlık | Oyun bitmez; küçülme ve toparlanma yolu var. Sonlar "kaybettin" değildir (M: borç ödenmezse tek şubede kalır) | Derlenmedi (G-067) |
| B07 | Dönüm noktası hatıraları | İlk kârlı gün, borcun kapanması, ilk şube… kalıcı liste (C) | Derlenmedi (G-066) |

## C. Müşteri

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| C01 | Alışveriş listesi | 1–4 ürün, ikame, boş sepetle çıkış (Codex G-053) | `MarketBasket` | Uygulandı |
| C02 | Segmentler | Emekli, ev, çalışan, öğrenci, esnaf, çocuk; semtle değişen pay (C) | Derlenmedi (G-062) |
| C03 | Fiyat kararı | Rakip reyon fiyatı + sadakat (G-051/054); segment duyarlılığı eklenecek | Derlenmedi (G-062) |
| C04 | Sadakat | 24 kişilik mahalle havuzu (G-053); veresiye ve sadakat kartı ile genişler | `MarketBasket`, `MarketCredit` | Kısmen |
| C05 | Veresiye | Tanınan müşteriye limitli; maaş gününde tahsilat, az sayıda batık (C; plan) | Derlenmedi (G-067) |
| C06 | Bekleme ve kalabalık | 90 sn sabır, 9 kişi sınırı (mevcut); segmente göre sabır (C) | Derlenmedi (G-062) |
| C10 | Yürüyüş ve hareket | Kişiye sabit yürüme tarzı (ağır adımlı, seri, oyalanan, telefona dalık, koşturan); kısa rota sırası (aile yazdığı sırayla, çocuk önce isteğine); raf önü süresi (tanıdık hızlı, boş raf arama, pahalıda karşılaştırma); kuyruğu görünce vazgeçme; kişisel alan, sağdan geçme, arkada yavaşlama; tanıdıklar sohbet eder (C) | `MarketMotion` | Derlenmedi (G-070) |
| C07 | İade ve şikâyet | Bozuk/yanlış ürün iadesi; iade kabulü memnuniyet, reddi itibar kaybı (plan 01 §11) | Derlenmedi (G-066) |
| C08 | Ödeme yöntemleri | Nakit, kredi kartı (POS komisyonu %1,5–2, ertesi gün hesaba), yemek kartı (2013 sonrası yaygın, komisyon yüksek). Kart kabulü bazı segmentlerin sepetini büyütür (C; plan 01 §11) | `MarketPayments` | Derlenmedi (G-069) |
| C09 | Sadakat kartı / uygulama | İlçe zinciri aşamasında; puan maliyeti karşılığında tekrar ziyaret (plan) | `MarketPromotions` | Tasarım |

## D. Ticaret

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| D01 | Sipariş ve mal kabul | Çok ürünlü liste, ertesi sabah arka kapı, eksik/hasar (Codex G-052) | `MarketEconomy`, `MarketDelivery` | Uygulandı |
| D02 | Sipariş önerisi | Dünkü talep × 1,25; 50 TL asgari (G-058) | `MarketOrderAdvice` | Uygulandı |
| D03 | Tedarikçiler | Trakya Gıda (Selim), Özdemir Toptan (kurgu, ucuz/riskli), üretici doğrudan, kendi depo (C) | Derlenmedi (G-063) |
| D04 | Vade ve iskonto | Düzenli ödemeyle vade; hacimle iskonto (C) | Derlenmedi (G-063) |
| D05 | Zam duyurusu | Ayın 1'i liste fiyatı güncellenir, oyuncuya zam listesi gelir (C) | Derlenmedi (G-063) |
| D06 | Oyuncu kampanyaları | Reyon indirimi, 3 al 2 öde, broşür, gondol başı, tedarikçi destekli (C; plan) | Derlenmedi (G-064) |
| D07 | Özel marka | Ulusal aşamada "Miras" markası; üretici sözleşmesi, kalite riski (plan) | `MarketCompany` | Tasarım |
| D08 | Tazelik ve fire | Parti + son kullanma, FEFO, son gün indirimi, fire nedeni raporda (plan) | Derlenmedi (G-067) |
| D09 | Atık ve bağış | Son günü geçmemiş ama satılamayan ürün bağışlanabilir: fire yazılır, itibar artar (plan 01 §11) | Derlenmedi (G-067) |
| D10 | Depozito/iade ambalaj | Cam şişe depozitosu (ülke profiline bağlı, Türkiye 2011'de yaygın değil) (plan) | ülke profili | Tasarım |
| D11 | **İnternet mağazacılığı** (M32 ile yeniden kuruldu; aşağıdaki eski hâl) | Dönemle açılır. 2011–2013: telefonla sipariş ve mahalleye paket servis (bakkal geleneği, küçük sepet, sadakat artırır). 2014+: kendi web sitesi, toplama görevlisi, teslimat slotu. 2016+: pazar yeri/hızlı teslimat platformları (komisyonlu kanal, kurgu platform adı). 2020: salgın dönemi talep sıçraması (kurgu senaryo profili, gerçek olayın saygılı anılması). Online sipariş raftaki stoğu paylaşır (ayrılmış stok), toplama rotası, teslimat maliyeti, eksik ürün ikamesi, online müşteri memnuniyeti. Bölgesel aşamada karanlık mağaza (yalnızca sipariş toplayan depo) (C; plan 01 §11, 04 §2) | `MarketOnline` | Derlenmedi (G-069): telefon/web/platform, ayrılmış stok yerine kapanışta depo→raf toplama, ikame kuralı, kurye kapasitesi, itibar/yıldız, ilçe online payı ve dükkândan müşteri kaybı, 2020-21 profili. Karanlık mağaza şirket büyümesine kaldı; 2020-21 profili her kampanyada farklı (G-073) |
| D12 | Kasa önü ve dürtüsel ürün | Kasa önü rafı sepete küçük ek ürün getirir (plan 04) | `MarketCustomers` | Tasarım |

## E. Rekabet

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| E01 | Günlük rakip haberleri | BİM, Migros, 15. günden A101 (G-054) | `MarketRivals` | Uygulandı |
| E02 | Rakip şirket ajanları | Bütçe, strateji, hafıza, gözlenebilen veriye tepki (C; plan 06 §9) | Derlenmedi (G-065) |
| E03 | Bereket Market | Kurgu yerel rakip, kinci, nakdi az; satın alma teklifi; sonra satın alınabilir (C) | Derlenmedi (G-065) |
| E04 | Pazar payı modeli | Semtteki alışveriş gücü çekiciliğe göre bölünür (C) | Derlenmedi (G-065) |
| E05 | Personel ayartma | Rakip iyi çalışana teklif verir; moral ve ücret belirler (C; plan 06 §5) | Derlenmedi (G-065) |
| E06 | Satın alma / satılma | Tam oyuncu mülkiyetinde zorla alım yok (plan O06) | `MarketCompany` | Tasarım |

## F. İnsanlar

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| F01 | Kişi olarak çalışan | Beceri, hız, dayanıklılık, gizli dürüstlük, moral, yorgunluk (G-060) | `MarketStaff` | Derlenmedi |
| F02 | Kasiyer, reyon görevlisi, İK, mali müşavir | G-060 kuralları | `MarketStaff` | Derlenmedi |
| F03 | Cem | Babanın çırağı, özel aday; eğitimle müdür (C) | Derlenmedi (G-066) |
| F04 | Eğitim ve terfi | Eğitim günü = kapasite kaybı, sonrasında beceri artışı; iç terfi (plan 06 §5) | `MarketStaff` | Tasarım |
| F05 | Şube müdürü | Hedef + yetki; sipariş/fiyat/personel sınırları içinde karar (plan 06 §4) | Derlenmedi (G-068) |
| F06 | Vardiya | Sabah/akşam, hafta sonu; İK planlar (plan) | `MarketStaff` | Tasarım |
| F07 | İş kazası | Sigorta ve çalışan güveni (plan 01 §11) | `MarketEvents` | Tasarım |
| F08 | Suistimal | Küçük kasa farkı, şüphe kanıt değil (plan 06 §6, G-060) | `MarketStaff` | Derlenmedi |

## G. Para

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| G01 | Kuruş | Para her yerde int64 kuruş (AGENTS) | tümü | Uygulandı |
| G02 | Vergi | Haftalık KDV %8 + gelir vergisi %15 (oyun modeli); müşavir (G-060) | `MarketStaff` | Derlenmedi |
| G03 | Kredi | Trakya Bankası (kurgu); faiz yıla göre; aylık taksit; tapu teminatı (C) | Derlenmedi (G-067) |
| G04 | Kira, elektrik, bakım, sigorta | Yeni şubelerde kira; dolap sayısı ve mevsime göre elektrik; arıza/bakım (plan 01 §9) | Derlenmedi (G-067) |
| G05 | Ödeme sıkıntısı | Uyarı → sevkiyat durur → yapılandırma → stok eritme → şube kapatma → ipotek (C; plan 06 §8) | Derlenmedi (G-067) |
| G06 | Ay sonu raporu | Kâr-zarar, nakit akışı, basit bilanço (plan 06 §7) | Derlenmedi (G-067) |
| G07 | Yatırımcı ve halka arz | Ulusal aşamada; kontrol kaybı görünür (plan 06 §1) | `MarketCompany` | Tasarım |
| G08 | Kur | Uluslararası aşamada ülke para birimi; grup raporu dönüşümü (plan 06 §13) | `MarketCompany` | Tasarım |
| G09 | Ev harçlığı | Aile dükkândan geçinir: günde 30 TL × asgari ücret endeksi eve; kasa darsa yarısı, boşsa hiç. İşletme gideri değil, yalnız nakit; ay sonu raporunda "eve" (C) | `MarketFinance` | Derlenmedi (G-071) |
| G10 | Katalog marjı | Toptancı fiyatı katalog maliyetinin 1,10 katı: brüt marj ~%34 yerine ~%25; hacim iskontosu ve vade değer kazanır (M 29.09.2026: gerçek bakkal marjı değil, oynanış için orta yol) | `MarketSuppliers` | Derlenmedi (G-073) |

## H. Mağaza ve şube

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| H01 | Elle raf dizme | R modu, bloklar (G-045…047) | `PlanogramEdit` | Uygulandı |
| H02 | Görevli dizme kuralları | Boş raf, rafta olmayan ürün, dar blok (G-049); acemi yalnız doldurur (G-060) | `StaffPlanner` | Uygulandı / Derlenmedi |
| H03 | **Yeni şube otomatik dizilimi** | Kategori komşuluğu, talep ve marja orantılı yüz, göz hizası, ağır ürün alt raf, her ürüne en az bir koli; şablon kaydet/uygula (C; M: dizilim mantığı Claude'da) | Derlenmedi (G-068) |
| H04 | Şube açma süreci | Semt → kira → tadilat → izin → işe alım → stok → açılış kampanyası → olgunlaşma (C; plan 01 §10) | Derlenmedi (G-068) |
| H05 | Uzak şube simülasyonu | Özet talep, tek veri kaynağı, çift sayım yok (plan O05) | Derlenmedi (G-068) |
| H06 | Yamyamlık | Yakın şubeler birbirinden müşteri çalar (plan) | Derlenmedi (G-068) |
| H07 | Formatlar | Mahalle marketi, indirim, süpermarket, premium, hipermarket… güç seviyesi değil (plan 04 §2) | `MarketBranches` | Tasarım |
| H08 | Servis reyonları | Manav, kasap, fırın… reçete ve fire; dondu (G-021) | — | Tasarım |
| H09 | Olaylar | Arıza, kesinti, denetim, düğün, kar, geri çağırma; günde en çok 1 büyük olay (C) | Derlenmedi (G-066) |

## I. Büyüme

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| I01 | İkinci şube | Borç kapalı + 950 TL + 3 kârlı gün + %35 pay (G-054); gerçek şube sistemiyle değişecek | Derlenmedi (G-068) |
| I02 | Bölge deposu ve kamyon | Depo ≥4 mağaza (marj +%1,5, uzak mağaza lojistik kaybı %3 → 0), 8 uzak mağazaya bir kamyon (yetmezse %1,5); merkezi satın alma depo + ≥8 mağaza (+%2). Rota ve soğuk zincir sonraki aşama | `MarketCompany` | Derlenmedi (G-072) |
| I03 | Ulusal aşama | İstanbul, Bursa, İzmir, Ankara, Kocaeli toplu mağaza modeli; "Miras" özel markası (≥20 mağaza, +%1,5 marj, +%3 müşteri); 10 mağazaya bir bölge müdürü; ulusal pay mağaza başı ~%0,04; karanlık mağaza. Yatırımcı/halka arz sonraki aşama | `MarketCompany` | Derlenmedi (G-072) |
| I04 | Uluslararası | Pilot Kırcaali (Bulgaristan), sonra Filibe; ikinci ülke Romanya (Köstence); yeni ülkede 90 gün öğrenme (−%3 marj), gümrük %1. Kur sonraki aşama (C) | `MarketCompany` | Derlenmedi (G-072) |
| I05 | Sonlar (eski) | Sattın (G-066); Miras: 7. bölümde bir yıl her ölçüde önde (pay %40, 60 mağaza, kâr, memnuniyet %60), oyun serbest sürer. Mahallenin dükkânı / Trakya / Türkiye / Sınır ötesi şimdilik bölüm hatıraları olarak kalır (C) | `MarketStory`, `MarketCompany` | Kısmen (G-072) |

## J. Dünya ve oyun sonu (derin inceleme sonrası, `02_DERIN_INCELEME.md`)

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| J01 | Yeniden tasarım | `Docs/Kurgu/02_DERIN_INCELEME.md` §3 ve §4 yol haritası uygulanacak (M 29.09.2026: "önerini uygula") | tümü | Devam ediyor (G-076…) |
| J02 | **Oyun sonu (Game Dev Tycoon gibi)** | Tek büyük son vardır: "Miras" — Küresel Perakende Ligi'nde 2 mali yıl üst üste ciroda 1. olmak (pozitif faaliyet nakdi, borç/FAVÖK < 3) **ya da** kampanyanın son tarihi (31.12.2040) gelince, hangisi önce olursa. Son ekranı bir kez gösterilir (özet: ciro sırası, mağaza, ülke, hatıralar). Sonra **"Oynamaya devam et"**: simülasyon sürer ama yeni bölüm, hikâye sahnesi, dönem olayı, yeni ülke açılışı gelmez; son bir daha gösterilmez (M 29.09.2026; tarih ve ölçüt C). Bölüm numarası 99 hilesi kaldırılır; ayrı `bEnded` bayrağı | `MarketStory`, `MarketCompany` | Derlenmedi (G-076); lig ölçütü G-082 |
| J03 | Sattın sonu | Satış sonu yine gösterilir, iki seçenek: **"Rüyaymış"** satışı tamamen geri alır (satış parası ayrı tutulur, kasaya karışmaz; harcanamaz) ve oyun kaldığı yerden sürer. **"Burada bitsin"** kampanyayı bitirir: kayıt "bitti" diye işaretlenir, oyuncu yeni oyun açar. Satılmış dükkânla serbest oyun yok; eski "bölüm 99" davranışı kalkar (C) | `MarketStory` | Derlenmedi (G-076) |
| J04 | Dünya yapısı | Dünya → bölge → ülke → pazar hücresi; 25–35 ülke, modern perakende payı S eğrisi, giriş yolları (kendi mağaza, franchise, ortaklık, satın alma, toptan, online) (C) | yeni `MarketWorld` | Tasarım |
| J05 | Küresel Perakende Ligi | Her yıl "Küresel 50" ciro tablosu; gerçek devler kendi eğrisiyle (A12'ye göre gerçek adlar), "dünya ölçeği" ayarı ×0,5–×1,0 (C) | yeni `MarketWorld`, `MarketRetail` | Tasarım |
| J06 | Yerel rekabet ekosistemi | Bakkal, semt pazarı, fırın, aile marketi, indirim, süpermarket, hipermarket, online; logit müşteri seçimi; rakip bilançosu, kişilik, hafıza (C) | `MarketCompetitors` | Tasarım |
| J07 | Kampanya kapsamı | Tek ürün / marka / alt kategori / kategori / raf / tüm mağaza / kart sahipleri; mekanik, teşhir, toptancı fonu (C) | `MarketPromotions` | Derlenmedi (G-078): kapsam + mekanik + yüzde + gün; toptancı fonu ve takvim şablonları sonra |

## K. Tedarik ağı (Mustafa 29.09.2026 + Claude eklemeleri)

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| K01 | **Ölçekle ucuzlayan alım** | Rakip zincirler aynı malı bizden ucuza alır (indirim zincirleri ~%8–12, ulusal süpermarket ~%5 daha ucuz). Bizim alım gücümüz son 12 ayın alım hacmiyle artar: `maliyet × (1 − 0,03 × log2(hacim / başlangıç hacmi))`, tavan −%12; depo, merkezi alım ve üreticiden doğrudan alım ayrıca düşürür (M) | `MarketSuppliers`, `MarketCompetitors` | Tasarım (G-083) |
| K02 | **Toptancı katmanları** | Yerel (ilçe toptancısı, bayi), bölgesel (Trakya distribütörü), ulusal distribütör, üreticiden doğrudan, uluslararası (ithalatçı, yurt dışı mağazalar için yerel tedarikçi). Üst katman ölçek ve kurum ister (depo, satınalma müdürü, kamyon) (M) | `MarketSuppliers` | Tasarım (G-083) |
| K03 | **Her toptancı her malı taşımaz** | Uzman tedarikçiler: içecek bayisi (kola/su), süt ürünleri bayisi (soğuk zincir), kuru gıda toptancısı, temizlik-kozmetik toptancısı, fırın (ekmek), hal/manav (meyve-sebze), ileride tekel. Sipariş ekranı ürünü taşıyan toptancılara göre bölünür (M) | `MarketSuppliers`, `Config/toptancilar.json` | Tasarım (G-083) |
| K04 | Teslim günleri | Her toptancı haftanın belli günleri gelir (ör. içecek Salı-Cuma, süt her gün, kuru gıda Pazartesi); teslim süresi ve asgari sipariş toptancıya göre. Sipariş bir planlama işi olur (C) | `MarketSuppliers`, `MarketDelivery` | Tasarım |
| K05 | Fiyat listeleri ve karşılaştırma | Aynı ürün farklı toptancıda farklı fiyat, vade ve iskontoyla; sipariş ekranında karşılaştırma (C) | `MarketSuppliers`, menü | Tasarım |
| K06 | Marka anlaşmaları | İçecek bayisi dolap verir, karşılığında raf/dolap payı ister (dolapta rakip marka yok); üretici raf giriş ücreti ve hedef primi öder (çeyreklik hedef tutarsa % iade); üretici destekli kampanyalar (C) | `MarketSuppliers`, `MarketPromotions` | Tasarım |
| K07 | Toptancının stoğu ve kıtlık | Toptancı da mal bitirir; olaylar: ayçiçek yağı kıtlığı (2021 esinli), şeker/çay zammı öncesi stok yapma; zam duyurusundan önce stok yapan kazanır, depo ve nakit riskiyle (C) | `MarketSuppliers`, `MarketEvents` | Tasarım |
| K08 | Ödeme biçimleri | Peşin iskonto (%2), vade, çek/senet (2011–2018 yaygın; karşılıksız çek riski), toptancıya göre kredi limiti (C) | `MarketSuppliers`, `MarketFinance` | Tasarım |
| K09 | Toptancı ilişkisi | Toptancılar kişiliklidir (Selim gibi). Rakip zincirle anlaşan toptancı mal kısabilir, sadık müşteriye kıtlıkta öncelik verir; toptancı değiştirmek eski toptancının güvenini düşürür (C) | `MarketSuppliers` | Tasarım |
| K10 | Satınalma müdürü | Mağaza sayısı büyüyünce personel rolü: pazarlık gücü ve iskonto artar, sipariş hatası azalır (C) | `MarketStaff` | Tasarım |
| K11 | Kalite ve soğuk zincir | Zayıf soğuk zincirli bayiden gelen sütün raf ömrü kısalır; ucuz tedarikçide kalite olayı riski (C) | `MarketFreshness` | Tasarım |
| K12 | Özel marka üreticisi | Fason üretici anlaşması: asgari parti, kalite denetimi, ihracat fırsatı (yurt dışı mağazalar) (C) | `MarketCompany` | Tasarım |
| K13 | Uluslararası tedarik | İthal ürün kur riski taşır; yurt dışı mağaza yerel tedarikçi bulmak zorunda (ya da Türkiye'den ihracat, gümrük) (C) | `MarketWorld` | Tasarım |

## L. Yayın için kapsayıcılık (Mustafa 29.09.2026, üçüncü tur)

Mustafa'nın kararları **(M)**; Claude'un önerileri **(öneri)** — Mustafa onaylayınca (C/M) olur.

| No | Konu | Karar | Durum |
|---|---|---|---|
| L01 | Dükkân dışı yok | Oyuncu dükkândan dışarı çıkmaz. Oyun = yönetim paneli + harita + gidilebilen dükkânlar. Dükkânlar kesit/maket (izometrik, "dollhouse") görünümüyle gösterilir; ışık gün saatine göre değişir (Mustafa'nın gönderdiği moodboard görseli) (M) | Tasarım |
| L02 | Başlangıç yeri seçimi | ETS2 gibi: oyuncu dünya haritasından ülke ve o ülkedeki il/şehri seçer (M) | Tasarım |
| L03 | Hikâye çerçevesi | Aileden kalan (babadan değil) küçük market; içinde 1 kasiyer, 2 reyon görevlisi; oyuncu müdür; işletmenin kendi borcu; raflar belli düzende ama tam dolu değil. Hedef: dev zincir (M) | Tasarım |
| L04 | Kendi ekonomisi | Oyun ekonomisi gerçek hayattan farklı, kendine özgü (M) | Tasarım |
| L05 | Para birimleri | Dolar, TL, euro, sterlin gibi para birimleri var; ülkeye göre (M) | Tasarım |
| L06 | Belirsiz yıl | Gerçek takvim yılı yerine daha belirsiz zaman (M) | Tasarım |
| L07 | Birinci şahıs kalır | Oyun birinci şahısla başlar (kendi dükkânında kasa, raf, mal kabul). Oyuncu istediği zaman kendi dükkânlarından birine girip dolaşır, raf dizer. Yönetim paneli, harita ve dükkân maketi (L01) birinci şahısla yan yana; büyüdükçe işler çalışanlara geçer ama birinci şahıs hiç kapanmaz (M 29.09.2026) | Tasarım |
| L08 | Kurlar (öneri) | Gerçek para birimi adları, kurgu kurlar: her ülkenin "ekonomi karakteri" (istikrarlı / oynak / yüksek enflasyonlu), kur rastgele yürüyüş + ara sıra şok; oyuncu raporlama para birimini seçer; lig tablosu tek ortak birimde | Açık |
| L09 | Zaman (öneri) | "1. yıl · İlkbahar" gibi göreli takvim; mevsimler ve o ülkenin kültürel günleri (Ramazan/Noel/Şükran günü…) ülke profilinden; dönemler (kartlı ödeme, internet satışı, hızlı teslimat) gerçek yıl yerine oyun içi ilerlemeyle açılır; olaylar adsız (ör. "kur şoku", "salgın") | Açık |
| L10 | Ülke profilleri (öneri) | Veriyle tanımlı ülke paketi: para birimi, ekonomi karakteri, alışveriş alışkanlığı (günlük/haftalık), geleneksel rakipler (bakkal→corner shop, kiosk; pazar→Wochenmarkt, farmers market), çalışma saatleri ve pazar kapalı yasası, ödeme yöntemleri, isim havuzu, bayram takvimi, ürün/marka havuzu. İleride mod desteği (ETS2 gibi) | Açık |
| L11 | Başlangıç zorluğu (öneri) | Haritada her şehir: rekabet, alım gücü, kira, büyüme potansiyeli, kur istikrarı yıldızları. Küçük kasaba kolay, başkent zor | Açık |
| L12 | Kurgu ama gerçeğe yakın markalar | Game Dev Tycoon gibi (Sony → Vonny): ürün markaları ve zincirler gerçek firmaya yakın kurgu adla; her ülkenin kendi firmalarından. Pazar payları ve güçleri gerçek firmalardan esinli. Ayrıca satılacak **Marka Editörü** ile oyuncu istediği ad ve logoyu yükler (M 29.09.2026). İlk liste: `Config/markalar.json` (77 marka), `Config/zincirler.json` (`useFictional: true`), `products.json` `fictionalName`. Not (C): adlar gerçek markayla karıştırılmayacak kadar farklı tutuldu; yayın öncesi bir hukukçuya gösterilmesi önerilir | Kısmen: veriler hazır, oyun varsayılanı kurgu ad (G-084/G-085) |
| L13 | Maket görünümünde oynanış (öneri) | Dükkâna tıkla: raf, kasa, depo, çalışan; baloncuklar sorunları gösterir ("süt rafı boş", "kuyruk uzun"); iç tasarım (zemin, duvar, ışık tonu: sıcak/aydınlık/loş) para ister ve müşteri gruplarını farklı çeker; gün saati kaydırıcısı ışığı ve müşteri yoğunluğunu gösterir | Açık |
| L14 | Dil (öneri) | Türkçe + İngilizce arayüz baştan; metinler tek tablodan | Açık |
| L15 | Başlangıç kasası (öneri) | Aileden kalan dükkânın kasasında 3 çalışanın 1 haftalık maaşı bulunur; ilk günler yalnız maaşla eksiye düşmesin. Akraba tohumla seçilir (teyze, dayı, hala, amca, büyükanne); şehir listesinde rekabet çarpanı (L11'in ilk adımı) | Kodda (G-084 2. parça), onay bekliyor |

**Etkisi (Claude notu):** Türkiye'ye ve 2011'e bağlı kodlar (`MarketCalendar` gerçek tarihler, `MarketPrices` yıl eğrisi, `iller.json`, hikâye karakter adları, `products.json` markaları, `MarketRetail` gerçek veriler) ülke profiline taşınacak. Yeni görev G-084 bunu yapar; G-080…G-082 sırası buna göre güncellenir.

## M. Mağaza ağı ve yönetim (Mustafa 29.09.2026, dördüncü tur; ayrıntı `03_MAGAZA_AGI.md`)

| # | Konu | Karar | Durum |
|---|---|---|---|
| M01 | Yalnız il | Lüleburgaz ve semtler kalktı. Her ülkede tek yer düzeyi il (TR 81 il, DE 16 eyalet, GB 12 bölge, ABD 50 eyalet) (M) | Kodda (G-086a) |
| M02 | Bütün ülke açık | Ev ilinin dışına ilk mağaza için İK müdürü + mali müşavir; yurt dışı 6. bölümde (M) | Kodda (G-086a) |
| M03 | Varsayılan başlangıç ili yok | Oyuncu ülke ve ili kendisi seçer; seçmeden oyun başlamaz. Kırklareli yalnız eski kayıtlar ve otomatik koşular için (M) | Kodda (G-086a) |
| M04 | Market türleri | Ucuzcu, mahalle, süpermarket, hipermarket (hiper: 5. bölüm, bölge deposu, 500 bin+ nüfus) (M) | Kodda (G-086a) |
| M05 | Yönetim kademeleri | Mağaza müdürü → il müdürü (ilde 3+ mağaza, ildeki bütün mağazalar; yardımcı yok) → bölge müdürü (alt bölge) → bölge direktörü (ana bölge) → ülke müdürü (ikinci ülkeye girince her ülkede, Türkiye dahil) (M) | Tasarımda (G-086b) |
| M06 | Kişi sınırı yalnız oyuncuda | Oyuncu en çok 5 kişiyle doğrudan ilgilenir; aşınca gözetim eksikliği. Müdürlerin sınırı yok; büyük il iyi il müdürü ister (gereken beceri 40 + mağaza/3, Claude önerisi) (M) | Yük göstergesi kodda, ceza G-086b |
| M07 | Bölgeler iki kat | TR: 7 coğrafi bölge + 20 alt bölge (Trakya, Doğu Karadeniz…); DE 4; GB 2+6; ABD 4+9 (M) | Kodda (veri) |
| M08 | Gezilebilir mağaza | Her ilde her türden 1, yurt dışında her ülkede her türden 1 (M) | G-087 |
| M09 | Ana ekran harita | Harita ana ekrandır; açık ve koyu tema; tasarım Claude'da. İlk hâl (üç sütun, dönen kartlar) M12 ile değişti (M) | Kodda (G-086a → G-086c) |
| M10 | Eski yurt dışı mağazaları | Kırcaali, Filibe, Köstence kapanır, depozito geri; Bulgaristan/Romanya dünya aşamasında (M) | Kodda (göç) |
| M11 | Bölge deposu alt bölge başına | Tek depo yerine her alt bölgeye bir depo (Claude, 03 §6) | M23 ile kalktı (G-089) |
| M12 | Sade ana ekran | Harita bütün ekran; il seçilince sağdan tek panel; sayılar üst haplarda, sayfalar alt dokta; tek asistan satırı; kararlar katmanda; az renk. Dönen kartlar ve yan sütunlar kalktı (Mustafa 30.09.2026, Claude tasarımı, 03 §12) | Kodda (G-086c) |
| M13 | Bölge çipleri | "Trakya" düğmesi kalktı; ülkenin bütün ana bölgeleri çip, tıklayınca harita o bölgeye yakınlaşır (M, 30.09.2026) | Kodda (G-086c) |
| M14 | Oyun içi görünüm = tuval | Menü ve dükkân içi HUD, tasarım tuvalindeki 4–5 numaralı çizimlerin birebir diliyle: IBM Plex Sans (metin), IBM Plex Mono (sayılar), Bricolage Grotesque (başlıklar), çizgi ikonlar, hap biçimli yüzeyler, açık temada yumuşak gölge, kâğıt zemin. Dokta tahtadaki altı sayfa + "Diğer" (Rakipler, Finans, Satış); ayarlar tarih hapının sonunda (M, 30.09.2026) | Kodda (G-086d) |
| M15 | Ekran zıplamaz | Alt dok her zaman aynı: ilk öğe "Ana ekran" hep görünür ("Harita" adı kalktı). Üst haplar, dok, zil yerinden oynamaz; il paneli haritanın üstünde yüzen kart olarak sağdan kayarak gelir, harita yumuşakça yana kayar; "Diğer" kartı solarak yükselir. İçeriği değişen kartların yüksekliği sabit (ör. Sipariş'te "Öneriyi yaz" listeyi oynatmaz). Hareket olursa yumuşak olur (M, 30.09.2026) | Kodda (G-086e) |
| M16 | Haritada il adı ve sipariş penceresi | Haritada yalnız seçili ilin adı yazar; ad ilin en geniş yerine oturur, sığmazsa küçülür (8 px'e kadar), yine sığmazsa kâğıt renkli hafif zemin alır; iğne de aynı noktada, ad iğnenin altında. Sipariş listesi pencere olarak açılır ("Listeyi aç", "Öneriyi yaz"): toptancı seçimi, satırlarda miktar, satır silme, "Değiştir" ile ürünü başka ürünle değiştirme (koliler taşınır), sağda ürün ekleme listesi. Sekmeler arası geçiş solarak ve hafif yükselerek (M, 30.09.2026) | Kodda (G-086f) |
| M17 | Hazır mağaza görünümleri | Şube açınca türüne göre hazır bir mağaza görünümü atanır: 4 tür × 5 = 20 mağaza (sayı ileride artabilir). Bir ilde her türün tek gezilebilir mağazası olur; aynı ildeki aynı türden şubeler o görünümle gezilir; farklı illerde aynı görünüm tekrar edebilir. Mağazalar kabuk + yerleşim + tema ile kurulur, ürünler otomatik dizilir. Görünüm Codex'te, oyun hesabı ve atama Claude'da. Sözleşme `04_MAGAZA_KITI.md` (M, 30.09.2026) | Sırada (G-088) |
| M18 | Raf kategorisi oyuncunun | Her mağazada (aile dükkânı dahil) oyuncu rafın üstündeki kategori tabelasına bakıp T ile rafın kategorisini değiştirebilir; gondolun iki yüzü ayrı. Görevliler yeni kategoriye göre dizer, raftaki eski ürünler silinmez, uyarı çıkar. Tabelalar ve 3B yazılar Türkçe harfli, Türkçe büyük harf kuralıyla (İÇECEK, KURU GIDA). `04_MAGAZA_KITI.md` §7 (M, 30.09.2026) | Sırada (G-088) |
| M19 | Aile dükkânına müdür | Aile dükkânına müdür atama, ikinci şube açıldıktan sonra açılır (Claude yorumu: aile dükkânı dışında ilk şube açılınca; aile dükkânı + 1 şube = iki mağaza). Müdür atanınca dükkân onun kararlarıyla işler, oyuncu 5 kişi sayımına 1 kişi olarak girer (M, 30.09.2026) | Sırada (G-086b ek) |
| M20 | Ülke müdürü ne zaman | Bir ülkede **5 ilde** mağazamız olunca o ülkenin ülke müdürü atanabilir (Türkiye dahil); öncesinde kademe ağacında görünmez. Şirket ikinci ülkeye girince her ülkede zorunlu kuralı sürer (M, 30.09.2026) | Sırada (G-086b ek) |
| M21 | Deneyimle gelişen beceri, sınırlı | Müdürlerin becerisi deneyimle (iyi yönetilen haftalar, kıdem) artar ama her kişinin gizli bir **tavanı (potansiyel)** vardır; tavana yaklaştıkça artış yavaşlar; genel üst sınır 95. Yeni müdürün ilk haftası alışma (−20) kalır (M, 30.09.2026) | Sırada (G-086b ek) |
| M22 | Rastgele, seçilebilir müdür adayları | Her atamada (mağaza, il, bölge, direktör, ülke, depo, aile dükkânı) rastgele üretilmiş 3 dış aday gösterilir; oyuncu birini seçer. Aday: ad, beceri (İK yoksa aralık), dürüstlük (İK varsa), tarz, potansiyel ipucu, ücret isteği. Aynı kampanyada ad tekrar etmez; adaylar haftada bir yenilenir (M, 30.09.2026) | Sırada (G-086b ek) |
| M23 | Depolar ve depo müdürü | Büyük depo bir **ile** kurulur (oyuncu seçer; oyun şubelere göre en uygun ili önerir) ve çevresindeki birden çok ile hizmet verir. Her şube kendi ülkesindeki en yakın depoya bağlanır; uzaklık arttıkça lojistik maliyeti artar, çok uzakta depo hizmet vermez (şube toptancıdan alır). Her deponun bir **depo müdürü** olur: becerisi fireyi, eksik/kırık teslimatı ve raf bulunurluğunu etkiler; müdürsüz depo verimsiz çalışır. Eski alt bölge depoları o alt bölgedeki en uygun ile taşınır. M11'in yerini alır (M, 30.09.2026) | Kodda, oyun mantığı (G-089; menü sonra; derlenmedi) |
| M24 | "2011" kavramı kalkar | Oyunun kendi ekonomisi var; oyuncu hiçbir yerde takvim yılı görmez ("N. yıl"). Koddaki 2011 adları (`Kurus2011`, `CatalogBase`, `StartYear`) yalnız "oyun başı fiyat düzeyi" ve takvimin iç çapasıdır. Oyuncuya görünen gerçek dünya metinleri (zincir tarihçeleri, kaynak satırı, "2014+", "2020-2021") kaldırıldı ya da "N. yıl" oldu. Belgelerde "2011 fiyatı" yerine "oyun başı fiyat düzeyi" (M, 30.09.2026) | Kodda (derlenmedi) |
| M25 | Markalar reyonda yer ister | Markalar (kurgu adlı) reyonda yer için yarışır: daha çok yüz (facing), göz hizası, gondol başı için raf parası, ciro primi, ortak kampanya ve bedava mal teklif eder; karşılığında reyon payı ister. Her reyonda markaların pazar payı oyuncunun raf kararıyla değişir; yer verilmeyen marka şartlarını kötüleştirir ya da kampanyasını çeker. "Miras" özel markası da bu yarışa girer. Ayrıntı Claude tasarlayacak (M, 30.09.2026) | Yazıldı, derlenmedi (Akış C, `MarketBrands`) |
| M26 | Hipermarkette satılan her şey: reyonlar | Paketli katalog (products.json) her mağazanın rafı olarak kalır; üstüne reyonlar gelir, her biri ayrı bir iş (tek tek ürün değil): taze (manav, kasap, şarküteri, fırın-pastane, balık) ve gıda dışı (elektronik-beyaz eşya, giyim-ev tekstili, ev-mutfak, oyuncak, kırtasiye-kitap, bebek, evcil hayvan, bahçe-oto, mevsimlik). Karar mağaza türü başına (o türdeki bütün şubelerde açılır) ve reyon başına fiyat duruşu (ucuz/normal/pahalı); alan sınırlı (mahalle %10, süpermarket %22, hiper %70). Kasap, fırın, balık usta ister; mevsimler (oyuncak yılbaşı, kırtasiye eylül, kasap Kurban öncesi, giyimde ocak/temmuz indirimi). Temsilci ürünler simülasyon aşamasında (M, 01.10.2026) | Yazıldı, derlenmedi (Akış C, `MarketDepartments`) |
| M27 | Yayına kadar eski kayıt uyumu yok | Oyun yayımlanmadı; eski kayıtların önemi yok. Yeni kodda eski kayıt çevirisi (`Migrate`) ve testi yazılmaz. Kayıt biçimi değişince kayıt sürümü artar, eski kayıt yüklenmez ("yeni oyun başlat"). Mevcut çevirme kodu ve ölü alanlar silinir. Yayından sonra kural yeniden konuşulur (M, 01.10.2026) | Kodda: A6 sürüm kapısı, B7 ve C3 temizlik (sürüm 3; derlenmedi) |
| M28 | Şirket finansı | Aile dükkânının küçük kredisi kalır; ilk şubeden sonra şirket bankalarla çalışır: aylık **kredi notu** (A+…D: borç/FAVÖK, faizi karşılama, büyüklük, defter yaşı, geciken taksit, nakit sıkıntısı; her parça bir satırda), dört banka (yerel: küçük ve pahalı; ticari; yatırım: büyük ve seçici; kalkınma: en ucuz, yılda bir yatırım kredisi) ve A notu + büyüklükle **tahvil** (ana para sonda). Yatırım kredisi 24/36/60 ay, isteğe bağlı 6 ay yalnız faiz. **Kredi limiti** (B notundan): cironun %15’i, eksi kasayı kendiliğinden kapatır. **Borç sınırı** borç/FAVÖK 3,5: aşılınca faiz +2 puan ve yeni kredi yok, 3 ay sürerse en büyük banka borcun dörtte birini geri ister. **Yapılandırma**: tek uzun kredi, %2 masraf, not 6 ay bir basamak aşağı (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketBanking`) |
| M29 | Rakip satın alma ile büyüme | Satılık zinciri almak vardı (8 aylık ciro); artık satılık olmayan zincire de **teklif** verilir: fiyat bir yıllık ciro (sağlıklıysa %20 fazla), sahibi sağlığına, büyüklüğüne ve bize kızgınlığına göre kabul eder ya da reddeder; ezeli rakip satmaz, yabancı devlerin kolları satılmaz. Ret: danışmanlara %0,5, zincir daha kızgın, 180 gün bekleme. Kasada yetmeyen kısım bankadan **satın alma kredisiyle** gelir (banka hedefin kazancını da sayar). Alınan zincirin mağazalarının ne olacağını M30 belirler (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketChains`, `MarketBanking`) |
| M30 | Bağlı şirket, devlerin çıkışı, her ülkeye kendi adları | Satın alınan zincir artık satılmaz: 20 mağazaya kadarsa şubemiz olur; büyükse **bağlı şirket** olarak kendi adıyla çalışır, kârı bize gelir, pazar payı ve sıralamada bizden sayılır. İstenirse ayda en çok 20 mağazası bizim adımıza çevrilir (açılış masrafının yarısı) ya da bağlı şirket 8 aylık ciroya satılır. Yabancı devler zararda ya da küçük kaldıkları ülkeden **çıkabilir** (yılda %6, zararda +%35): kolu 6 aylık ciroya satılığa çıkar; girmediğimiz ülkedeki bir kol, şirket bölümü açıldıysa **o ülkeye giriş kapısıdır**. 8. yıldan sonra ara sıra kimsenin olmadığı bir ülkede de satılık kol çıkar. **Adlar ülkeden gelir**: bankalar `Config/ulkeler.json` `banks`, toptancı, satıcı, komşu aile marketi ve sahibi o ülkenin ad/soyadlarından (`MarketCast`); Trakya Bankası, Trakya Gıda/Selim, Bereket Market/Kadir Bey kalktı (oyun her yerden başlar) (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketChains`, `MarketCast`) |
| M31 | Zombi şirket yok: bankanın kurtarma planı | Kasa 45 gün eksi kalırsa banka uyarır; 60. günde plan uygulanır: bağlı şirketler satılır, zarar eden (ve henüz açılmamış) şubeler en kötüsünden başlayarak kapanır, hiç şube kalmadıysa yöneticiler ücret alamadığı için ayrılır ve aile dükkânında en iyi 2 çalışan kalır. Kalan açık + rafları doldurmaya yetecek para (başlangıçta 1.000 TL) 36 aylık kurtarma kredisine çevrilir (yılın faizi + 8 puan, her yeni kurtarmada +4). Oyun bitmez; aile dükkânından yeniden başlanır. Şube açarken aylık sabit gider gösterilir (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketFinance::Rescue`) |
| M32 | İnternetten satış şirketin ve ilin işi | Eski telefon siparişi, aile dükkânında kurye tutma, tek karanlık mağaza ve "salgın var/yok" ayarı kalktı (salgın her oyunda var). Zaman çizelgesi takvim yılına değil oyunun salgınına bağlı: web ~6 yıl, ülkenin hızlı teslimat platformu ~4 yıl, telefon uygulaması 1,5 yıl önce, kendi 30 dk teslimatımız salgından 6 ay sonra. Başta hiçbiri görünmez; asistan söylentiyi birkaç ay önce, rakiplerin internete çıkışını da olunca söyler. İnternet payı salgına kadar yavaş, salgında sıçrar, sonra kaymaya devam eder, mağazaların işi de büyür, dengeye oturur (ülkenin tavanı `ulkeler.json` `online.plateau`). Koşullar: web 2 mağaza, uygulama web + 8 mağaza (yazılım firması karar kartı: ucuz/sağlam/seçkin), platforma aile dükkânı bile girebilir, hızlı teslimat uygulama + ilde 4 mağaza + karanlık depo. Yeni il şirket kuralıyla başlar. İl müdürü kanalları kendi kafasına göre açıp kapatmaz: ay sonunda son aylara bakar (bir kanal üç ay üst üste zarar ettiyse kapatmayı, şirketin olup ilin kullanmadığı bir kanalı açmayı, karanlık depo kurmayı) ve öneri yazar; öneri ülke müdürü varsa ondan, yoksa bölge direktörü ya da bölge müdüründen geçip karar kartı olarak bize gelir, onaylamadıkça değişmez (ret: üç ay aynı konu açılmaz). Müdür yoksa şirket kuralı; oyuncu ili kendisi ayarlayıp sonra bırakabilir. Kurye sipariş başı ödenir; uygulama siparişi en pahalı teslimat ama en büyük sepet. E-ticaret müdürü (yıldız, toplama, isterse politika), politika: teslimat ücreti, en az sepet, internet fiyat farkı, reklam, eksik ürün kuralı. Kartlar: uygulama firması, salgında internete geçiş, platformun kendi marketi (özel anlaşma), komisyon artışı. Kaçırılan teknoloji batırmaz, tökezletir (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketOnline`) |
| M33 | Komuta zinciri: gündelik iş aşağıda, kritik iş yukarıda | Fiyat, sipariş ve stok eritme oyuncuya sorulmaz. Şube müdürü yavaş satan malı haftalık indirimle eritir (temkinli %15, fiyatçı %30, diğerleri %20); %20'yi aşan indirim il müdürünün onayını ister, il müdürü yoksa %20'de kalır; zayıf şube müdürünün iyi satan ürüne yanlış indirimini becerikli il müdürü (50+) geri alır; hepsi haftalık şube satırında görünür. Kritik kararlar yukarı çıkar: il müdürü üç ay üst üste zarar eden (ilk üç ayını geçmiş) şubeyi kapatmayı, bütün mağazaları kâr eden ve yeri olan ilde (kasada açılış bedelinin üç katı varsa) yeni mahalle marketi açmayı önerir; öneri ülke müdürü, yoksa bölge direktörü, yoksa bölge müdürü üzerinden karar kartı olarak gelir. Ret: kapatma 90, açma 180 gün susar. İl müdürü yoksa kimse önermez (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketCommand`, `MarketBranches::Clearance`) |
| M34 | Şirket reklamı | Ülke başına altı kanal, her biri 0–3 seviye: televizyon (ülke çapında, pahalı, en güçlü, en geç unutulan), radyo, açık hava (ilde), gazete ve broşür (mağaza başına; yıllarla zayıflar), sosyal medya (ucuz, yıllarla güçlenir, internet siparişini de artırır), arama ve uygulama reklamı (yalnız internet; M32'nin internet reklamı artık bu). Her kanalın akıllarda kalanı kendi hızıyla söner; mağaza müşterisi en çok %12, internet siparişi en çok %25 artar, azalan getiriyle. Üç kanal birlikte %20, beşi birden %30 daha etkili ("hepsi bir arada"). 20 mağazadan sonra reklam müdürü: beceri her lirayı güçlendirir, istenirse karışımı cironun bir payıyla (binde 10–50) kendisi kurar, aralıkta artırır. Aile dükkânının mahalle broşürü kampanya olarak kalır (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketAdvertising`) |
| M35 | Eski oyunun Türkiye kalıntıları | Aile dükkânının sokağındaki dört zincir mağazası artık ev ilindeki gerçek zincirlerden seçilir (önce o ilde mağazası olan, türüne uyan: indirimci, hızlı açan indirimci, ikinci indirimci, süpermarket); adları ve günlük haberleri (MarketRivals) onlardan; zincir kapanır, satılır ya da biz alırsak sokaktaki mağazası da kapanır. Hava durumu ülkenin iklimiyle (`ulkeler.json` `climate`), kasabın zirvesi Kurban Bayramı yalnız ay takvimli ülkelerde, başka yerde yıl sonu. Mali müşavir "Necati Bey" yerine ülkenin adlarından. Kullanılmayan gerçek zincir tablosu (`MarketRetail`) silindi (M, 01.10.2026) | Yazıldı, derlenmedi (`MarketCompetitors::Bind`) |
| M36 | Aile dükkânı sıradan bir mağaza (Mustafa 02.10.2026) | Hikâye: annemiz ve babamız emekli oldu, marketi bize bıraktı (teyze/amca/dayı yok; hikâyeler babadan söz eder). Dükkânın diğer mağazalardan farkı yok: bina annemle babamın, ilin mahalle marketi kirasını onlara öder (emekli gelirleri, günün gideri); kasadan eve para çekme, tapu ipoteği, veresiye defteri ve dükkâna özel mahalle broşürü kalktı (broşür şirket reklamının kanalı, bütün mağazalar için). Babanın toptancıya borcu kalır, ilk hedef yine onu kapatmak. | Yazıldı, derlenmedi (`MarketFinance::RentToday`; `MarketCredit` silindi) |
| M37 | Patronun maaşı ve kişisel servet; başlangıç parası (Mustafa 02.10.2026) | Şirket bize her ay maaş öder (asgari ücretin 1–40 katı, biz seçeriz; şirketin gideri, vergi ve sigorta kesilir, net kişisel servete). Geçen yılın net kârından kâr payı (%15 stopaj; kasada bir aylık sabit gider kalır; kurtarma planında yok). Kişisel paradan şirkete sermaye konabilir. Ailenin aylık yaşam gideri servetten düşer; ileride kişisel hayatta alınacak şeyler. Kurtarma planı maaşı asgariye indirir. Bütün maaşlar oyunda aylık gösterilir. Başlangıç: kasada dükkânın bir aylık sabit gideri (maaşımız dahil), babanın toptancı borcu bunun 1,5 katı; ülkeye, ile ve yıla göre. | Yazıldı, derlenmedi (`MarketOwner`) |
| M38 | Her mağazada aynı kampanya sistemi; kimlik şirketin (Mustafa 02.10.2026) | İndirim, 3 al 2 öde, gondol başı ve kapsamlı kampanyalar her mağazada: oyuncu ilk dükkânda, bir şubede ya da bütün mağazalarda başlatır (Kampanyalar › Nerede?, Mağazalar › Kampanya). Mağaza müdürü kendi mağazasında sebebiyle uygular (yavaş satan malı bir haftalık indirimle eritir; ilk dükkânın müdürü de aynısını yapar); %20 üstü il müdürüne sorulur. Müdürü olmayan mağaza yalnız oyuncunun kararlarıyla işler. Dükkân kimliği bakkal değil, küçük bir marketin şirket kimliği: Mahalle Marketi / Kaliteli Market / Hızlı İndirim; bütün mağazalar taşır. | Yazıldı, derlenmedi (`MarketPromotions::StoreEffect`, `ManagerClearance`) |
| M39 | Hedef eğriler (Mustafa 02.10.2026) | Dengenin ölçütü `06_GIDIS_YOLU.md` §5: iyi oyunda ilk şube 4–8. ay, 3. yıl 10 mağaza, 10. yıl 60–120 mağaza ve ulusal ilk 10, 20. yıl ilk 3; temkinli ilk şube 8–14. ay, 10. yıl 15–40 mağaza; 30 yılda kurtarma 0–1 (kötü oyunda en çok 3–5, borç birikmez); ilk dükkânın faaliyet kârı enflasyonla büyür; 30 günden uzun olaysız dönem yok. Kötü oyun tökezletir, batırmaz. | Onaylandı; bot raporu her turda doldurur |
| M40 | Ülke müdürünün maaşı ağın büyüklüğüne göre; merkez giderleri merkezin defterinde (Claude C11, 02.10.2026; Mustafa değiştirebilir) | Codex C10 verisi: temkinli ve dengeli oyuncu 5 ile yayılan 7 mağazada ülke müdürü atıyor, müdürün maliyeti (yılda ~130 bin TL, 2011 parası) ilk dükkânla altı şubenin kârını yiyor; şirket 7 mağazada takılıyor, 9–10. yılda tek dükkâna düşüyor. Karar: ülke müdürünün bandı ülkedeki mağaza sayısıyla ölçeklenir (az mağazada %30, 30 mağazada tam bant); şirket büyüdükçe maaşı haftalık olarak banda yükselir, hiç düşmez. İK müdürü ve mali müşavirin ücreti merkezin defterine yazılır; kurtarma planının ve kasada tutulacak "bir aylık sabit gider"in hesabına merkezin kalan giderleri (web, uygulama, karanlık depo, reklam, e-ticaret/reklam müdürü, depo/kamyon, POS ve yemek kartı) girer. Asgari ücretin reel artışı yılda %1,5 yerine %0,5 (Codex C10 D2). | Yazıldı (C11) |
| M41 | Hedef eğriler güncellendi; zorluk seviyeleri (Mustafa 02.10.2026: "dengeli oynayan da büyümeli, ne çok zor ne çok kolay; zorlanmak isteyen Zor oynar") | Normal: dengeli ilk şube 4–8. ay, 3. yıl 8–12, 10. yıl 80–150 mağaza, ulusal ilk 10 (20. yıl ilk 3); temkinli 8–14. ay, 3. yıl 4–8, 10. yıl 40–80, ilk 20; atak 10. yıl 120–220 ama hızın riski olmalı (30 yılda 0–2 kurtarma). Büyüme döneminde kasada uzun süre 6 aydan fazla sabit gider boşta durmamalı (para her aşamada biraz sıkışık). İlk dükkânın reel kârı yatay kalabilir (0,8–1,3). Rahat: dengeli 10. yıl 120–200; Zor: dengeli 40–80, temkinli 15–40 (eski hedef). Zor modda rakipler, kredi, yönetici maliyeti ve ölçek kârı sertleşir. Ayrıntı `06_GIDIS_YOLU.md` §5. | Hedefler güncellendi (Mustafa izin verdi, Claude yazdı; Mustafa değiştirebilir); zorluk sertlikleri yazılacak |
| M42 | Orta oyunun kolay parası; ilk şube; zorluk sertliği (Claude C12, 02.10.2026; Mustafa değiştirebilir) | C11 bot verisi: süpermarket tadilatını 1–2 ayda, hipermarket 4 ayda çıkarıyor (net marj %16 / %11); 15 mağazadan sonra para sınır olmaktan çıkıyor. Karar (C12b): küçük mağazaların kira ve tadilatı değişmez (ilk deneme ×1,5 kira mahalle şubesini %1 net kâra düşürdü, oyun 3 mağazada kaldı); süpermarket kira ×1,5 ve tadilat 30.000, hipermarket kira ×1,5 ve tadilat 70.000 TL (C12c) (2011 parası; yaklaşık yarım yıllık kâr); süpermarket fiyatı rakip düzeyinde (1,04 → 1,00). Ailenin dükkân kirası değişmez. Şirketin ilk şubesi (ev ilinde mahalle marketi) komşu esnafın boşalan dükkânı: tadilatın %40'ı. Zorluk: yeni mağaza tadilatı ve kiralar Rahat ×0,75, Zor ×1,15 (C12c) (müşteri, fiyat hoşgörüsü ve rakip sertliği zaten zorluğa bağlı). Bot `-Difficulty=0/1/2`. | Yazıldı, derlenmedi (C12) |
| M43 | Kredi başvurusu, teklif süreci, piyasa kulisi, büyüyen depo (Mustafa 02.10.2026: "bankaya gidip kredi çekebilelim, asistan teklifimize cevap geldi diye sunsun; söylentiler: 1 güvenilir, 3 güvenilir 1 güvensiz kaynak; bazen güvensiz doğru çıksın, bazen 3 güvenilirin söylediği olmasın") | Kredi: bankaya başvuru (bugünkü teklifin %25–100'ü ya da 1,5 katı), cevap 2–4 gün (tahvil 5): tamamı, bir kısmı (gerekçesiyle) ya da ret (gerekçesiyle); faiz o günün faizi ± bankanın havası (−0,5 … +1 puan); teklif 7 gün geçerli, birkaç bankaya aynı anda başvurulabilir (bankada bir açık başvuru). Satın alma: teklif zincirin yönetim kuruluna gider, cevap 2–4 gün; kabul edilirse 14 gün içinde ödenir, kasa yetmezse bütün uygun bankalara satın alma kredisi başvurusu yapılır, bir teklif kabul edilince anlaşma kendiliğinden kapanır; süre geçerse anlaşma düşer. Kulis: şirketin şubesi olunca 50–90 günde bir söylenti (C13b; önce 25–45, rakip birleşmesini çok artırıyordu) (en çok 3 açık): satışa çıkma, bir ile girme, başka zinciri alma, fiyat kırma. Doğruluğu baştan gizli belirlenir (yaklaşık %42; zincirin sağlığına ve türe göre %30–60). Kaynaklar 4–8 günde bir gelir (en çok 5): güvenilir kaynak %74, güvensiz %55 haklı; ülke müdürü ve mali müşavir daha çok güvenilir kaynak getirir. Asistan kaynaklardan dürüst olasılığı söyler (zayıf/belirsiz/güçlü/çok güçlü): yalnız güvensizlerin konuştuğu söylenti ~%43 doğru çıkar, 3 güvenilir onay ~%5 boşa çıkar. Gününde doğru olan gerçekleşir, yanlış olan yalanlanır. Depo: kirası ve müdür maaşı hizmet ettiği şube sayısıyla büyür (az şubede %30, 20 şubede tam). Otomatik oyuncu anlık kredi/satın almayı kullanır; dengeli/atak depoyu krediyle kurar. | Yazıldı, derlenmedi (C13) |
| M44 | Zorluk şubelere de işler (Claude C14e, 03.10.2026; C14d botu Zor'da atak oyuncuyla 293–294 mağazaya ulaştı: zorluğun müşteri çarpanı yalnız aile dükkânına uygulanıyordu) | Rahat/Normal/Zor müşteri çarpanı uzak şubelerin günlük alışveriş sayısına yarı gücüyle uygulanır (1,05 / 1,00 / 0,96; C15b: tam güç Zor'da temkinli/dengeliyi 1–15 mağazaya düşürdü, Rahat'ta ikiye katladı). Kira/tadilat çarpanı (M42) aynen. | Yazıldı, derlenmedi (C14e) |
| M45 | Hızlı büyüme kumardır (Mustafa 03.10.2026: "hızlı oynayan büyüyebilir ama riske girme ihtimali de yüksek olur, bu dengeyi tutturalım") | Yönetimin bir yılda takip edebileceği kira sözleşmesi: 6 + açık mağazanın %40'ı + her il/bölge/ülke müdürü için 2 (müdür payı en çok mağazanın %30'u; C15b: ilk hâli 8 + yarısı + 3 hiç devreye girmedi). Son 12 ayın sözleşmeleri bunu aşınca zorlanma (0–1) = sözleşme / kapasite − 1. Yeni şube %60 × zorlanma ihtimalle aceleyle seçilmiş yer olur: müşteri kalıcı olarak dörtte bir az; açılışta haberle öğrenilir. 180 günden genç şubelerde hizmet %12 × zorlanma düşer. Ana ekranda "Büyüme yönetimin önünde" uyarısı (sözleşme, kapasite, risk). Temkinli/dengeli bot uyarıda bekler, atak bot devam eder. Kayıt sürümü 8. | Yazıldı, derlenmedi (C15) |

## Mustafa'nın kararları (29.09.2026, ikinci tur)

1. Derin incelemedeki önerinin tamamı uygulanacak (J01).
2. Oyun sonu Game Dev Tycoon gibi: son bir kez gelir, sonra oynamaya devam edilir ama yeni içerik gelmez (J02).
3. Menü açıkken zamanın akması ya da durması oyuncunun tercihi (A02).
4. Gerçek zincir adları kalır; kurgu karşılıkları hazır tutulur, yayın kararında onlara geçilir (A12).

## Mustafa'nın kararları (29.09.2026)

Mustafa: "Kararı sen ver, oyun eğlenceli olsun; gerçek hayatla birebir olması şart değil, birebir yaparsak tahmin edilebilirlik artar."

1. **Sattın sonu:** son gösterilir, sonra "Rüyaymış: dükkâna dön" ya da "Burada bitsin" (B04).
2. **Salgın dönemi:** kalır (varsayılan açık, kapatılabilir), ama her kampanyada başlangıcı, süresi ve kapanan hafta sonları farklıdır (D11).
3. **Enflasyon:** tarihsel rakamlar yerine oyunun kendi eğrisi, ay ay ±%2 kampanya farkı (A06, A07).
4. **Marj:** toptancı fiyatı katalog maliyetinin 1,10 katı, brüt marj ~%25 (G10).

Açık soru yok.
