# Kararlar — oyunun bütün konuları

Oyundaki her konu burada bir satırdır: **ne karar verildi**, **hangi modülde**, **durum**. Yeni bir konu akla gelince önce buraya satır eklenir. Kurgu gerekçesi `00_KURGU_KITABI.md`'de, ayrıntılı tasarım `Docs/Planlama/` içindedir. Mustafa'nın doğrudan kararı **(M)** ile, Claude'un kurgu kararı **(C)** ile işaretlidir. (M) kararı değişmeden (C) kararı onu çiğneyemez.

Durum: **Uygulandı** · **Derlenmedi** (kod var, Unreal'de denenmedi) · **Sırada** · **Tasarım** (kararı var, kodu sonra) · **Açık** (Mustafa'ya sorulacak).

Sahiplik (M, 28.09.2026): 3B model üretimi, mağaza modeli ve arayüz tasarımı dışında her şey Claude'da. Mağaza **düzeni** (raf dizilimi, şube planı) Claude'da.

## A. Zaman ve dünya

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| A01 | Başlangıç tarihi | 7 Mart 2011 Pazartesi = gün 1. Hafta Pazartesi başlar (C; 2011 başlangıcı M) | `MarketCalendar` | Derlenmedi (G-061) |
| A02 | Gün uzunluğu | Açık dükkânda 4 gerçek dakika. Menüde zaman durur (mevcut) | `MarketGame` | Uygulandı |
| A03 | Bayram ve özel günler | 2011'in gerçek tarihleri; hicri bayramlar her yıl ~11 gün kayar (C) | `MarketCalendar` | Derlenmedi (G-061) |
| A04 | Mevsim ve hava | Deterministik; kategori talebini ve yayayı etkiler (C) | `MarketCalendar` | Derlenmedi (G-061) |
| A05 | Maaş günleri | Ayın 1'i, 15'i ve son iş günü trafik/sepet artar; ay sonu bütçe daralır (C) | `MarketCalendar` | Derlenmedi (G-061) |
| A06 | Enflasyon | Yıllık TÜFE yaklaşığı; 2025 sonrası kurgu senaryo (C) | Derlenmedi (G-063) |
| A07 | Asgari ücret | Tarihsel net yaklaşığı; ücret beklentisi bunu izler (C) | Derlenmedi (G-063) |
| A08 | Yer | Lüleburgaz esinli kurgu semtler (İstasyon başlangıç). Sonra Trakya, Türkiye, Bulgaristan pilotu (C) | `MarketBranches` | Tasarım |
| A09 | Stratejik ilerletme | Görevler devredildiyse gün/hafta/ay oynamadan simüle edilir; aynı kurallar (C; plan 01 §3) | `MarketDirector` | Tasarım |
| A10 | Zorluk | Ekonomi, rakip saldırganlığı, olay yoğunluğu ayrı ayar (plan) | `MarketDirector` | Tasarım |
| A11 | Gerçek marka adları | Ürünlerde gerçek marka görünür; F8 kurgu adlara çevirir (M) | `ProductCatalog` | Uygulandı |
| A12 | Gerçek zincirler | Rakip olarak gerçek zincirler, yalnızca olağan ticari davranış; olumsuz kurgu olaylar kurgu şirketlerle (M + plan 06) | `MarketCompetitors` | Kısmen (G-054) |

## B. Hikâye

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| B01 | Babanın borcu | 300 TL, süresiz, kapanmadan ikinci şube yok (M) | `MarketCampaign` | Uygulandı |
| B02 | Bölümler | 7 bölüm: Defter → Karşı Dükkân → İkinci Tabela → Trakya → Türkiye → Sınır Ötesi → Miras (C) | Derlenmedi (G-066) |
| B03 | Karakterler | Nermin teyze, Cem, Selim, Necati Bey, Kadir Bereketoğlu, Derya, banka müdürü (C; plan) | Derlenmedi (G-066) |
| B04 | Sat ya da devam | Bölüm 2'de Bereket Market teklifi; satmak bir sondur, oyun kaydı kalır (C; plan) | Derlenmedi (G-066) |
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
| C07 | İade ve şikâyet | Bozuk/yanlış ürün iadesi; iade kabulü memnuniyet, reddi itibar kaybı (plan 01 §11) | Derlenmedi (G-066) |
| C08 | Ödeme yöntemleri | Nakit, kredi kartı (POS komisyonu %1,5–2, ertesi gün hesaba), yemek kartı (2013 sonrası yaygın, komisyon yüksek). Kart kabulü bazı segmentlerin sepetini büyütür (C; plan 01 §11) | `MarketPayments` | Tasarım |
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
| D11 | **İnternet mağazacılığı** | Dönemle açılır. 2011–2013: telefonla sipariş ve mahalleye paket servis (bakkal geleneği, küçük sepet, sadakat artırır). 2014+: kendi web sitesi, toplama görevlisi, teslimat slotu. 2016+: pazar yeri/hızlı teslimat platformları (komisyonlu kanal, kurgu platform adı). 2020: salgın dönemi talep sıçraması (kurgu senaryo profili, gerçek olayın saygılı anılması). Online sipariş raftaki stoğu paylaşır (ayrılmış stok), toplama rotası, teslimat maliyeti, eksik ürün ikamesi, online müşteri memnuniyeti. Bölgesel aşamada karanlık mağaza (yalnızca sipariş toplayan depo) (C; plan 01 §11, 04 §2) | `MarketOnline` | Sırada |
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
| F05 | Şube müdürü | Hedef + yetki; sipariş/fiyat/personel sınırları içinde karar (plan 06 §4) | `MarketBranches` | Sırada |
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

## H. Mağaza ve şube

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| H01 | Elle raf dizme | R modu, bloklar (G-045…047) | `PlanogramEdit` | Uygulandı |
| H02 | Görevli dizme kuralları | Boş raf, rafta olmayan ürün, dar blok (G-049); acemi yalnız doldurur (G-060) | `StaffPlanner` | Uygulandı / Derlenmedi |
| H03 | **Yeni şube otomatik dizilimi** | Kategori komşuluğu, talep ve marja orantılı yüz, göz hizası, ağır ürün alt raf, her ürüne en az bir koli; şablon kaydet/uygula (C; M: dizilim mantığı Claude'da) | `MarketLayout` | Sırada |
| H04 | Şube açma süreci | Semt → kira → tadilat → izin → işe alım → stok → açılış kampanyası → olgunlaşma (C; plan 01 §10) | `MarketBranches` | Sırada |
| H05 | Uzak şube simülasyonu | Özet talep, tek veri kaynağı, çift sayım yok (plan O05) | `MarketBranches` | Sırada |
| H06 | Yamyamlık | Yakın şubeler birbirinden müşteri çalar (plan) | `MarketBranches` | Sırada |
| H07 | Formatlar | Mahalle marketi, indirim, süpermarket, premium, hipermarket… güç seviyesi değil (plan 04 §2) | `MarketBranches` | Tasarım |
| H08 | Servis reyonları | Manav, kasap, fırın… reçete ve fire; dondu (G-021) | — | Tasarım |
| H09 | Olaylar | Arıza, kesinti, denetim, düğün, kar, geri çağırma; günde en çok 1 büyük olay (C) | Derlenmedi (G-066) |

## I. Büyüme

| No | Konu | Karar | Modül | Durum |
|---|---|---|---|---|
| I01 | İkinci şube | Borç kapalı + 950 TL + 3 kârlı gün + %35 pay (G-054); gerçek şube sistemiyle değişecek | `MarketCampaign` → `MarketBranches` | Kısmen |
| I02 | Bölge deposu ve kamyon | Trakya aşaması; rota, soğuk zincir (plan 01 §8) | `MarketCompany` | Tasarım |
| I03 | Ulusal aşama | Bölge müdürlükleri, merkezi satın alma, özel marka (plan) | `MarketCompany` | Tasarım |
| I04 | Uluslararası | İlk pilot Bulgaristan (C; plan A04 açık bırakmıştı) | `MarketCompany` | Tasarım |
| I05 | Sonlar | Sattın / Mahallenin dükkânı / Trakya / Türkiye / Sınır ötesi / Miras (C) | `MarketStory` | Tasarım |

## Açık sorular (Mustafa)

1. Sat ya da devam kararında "sattın" sonu gerçekten kampanyayı bitirsin mi, yoksa sadece bir sahne olup oyuncu yine devam edebilsin mi? Öneri: son sahnesi gösterilir, oyuncu isterse "aslında satmadım" diyerek geri döner.
2. 2020 salgın dönemi oyunda anılsın mı? Öneri: kurgu "salgın dönemi" senaryosu, kapanma günleri ve online sipariş patlaması, ölüm veya hastalık içeriği yok.
3. Enflasyonun 2021–2023 sertliği aynen mi, yoksa zorluk ayarıyla mı? Öneri: varsayılan tarihsel, "rahat" zorlukta yarıya indirilmiş.
