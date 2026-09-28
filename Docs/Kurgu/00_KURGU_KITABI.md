# Miras Market — Kurgu kitabı

Sahibi: Claude (Mustafa'nın 28.09.2026 kararı: 3B model üretimi ve arayüz tasarımı dışında oyunun bütün kurgusu ve arka plan kodu Claude'da).
Bu belge oyunun **tek kaynak kurgusudur**. Kod bu belgeyi uygular. Sayılar kodda tek bir yerde durur (her modülün başındaki `constexpr` sabitleri) ve burada özetlenir. Tasarım paketi (`Docs/Planlama/`) hedef oyunu tarif eder; bu kitap onu oynanabilir kurallara çevirir ve sırayla gerçekleştirir.

Kural: **her sistem dünyadan bağımsız bir modüldür** (`Source/MirasMarket/Market*.h/.cpp`), otomasyon testi vardır ve oyuna tek bir yerden bağlanır (`MarketDirector`: gün açılışı, gün kapanışı, anlık çarpanlar). Rastgelelik kampanya tohumu + gün + nesneye bağlıdır: kaydı yüklemek sonucu değiştirmez.

## 1. Oyunun sözü

7 Mart 2011, Pazartesi. Lüleburgaz'da, İstasyon Caddesi'nin arka sokağında babandan kalan küçük bakkal-marketin kepengini açıyorsun. Kasada biraz bozuk para var. Depoda karışık koliler duruyor. Toptancıya 300 TL borç yazılı. Babanın hesap defterinin ilk sayfasında da tek bir not var: **"Bu dükkânın müşterisini tanı."**

Oyun, o dükkânın bir mahalle markası, sonra Trakya'nın, sonra Türkiye'nin ve sonunda başka ülkelerin tabelası olup olmayacağıdır. Her büyüme adımı öncekinin emeğine dayanır: müşterinin güveni, tedarikçinin güveni, çalışanın güveni, bankanın güveni.

## 2. Zaman

| Kavram | Kural | Modül |
|---|---|---|
| Oyun günü | Bir oynanan gün = bir takvim günü (dükkân açık kaldığı sürece gerçek 4 dakika) | `MarketGame` |
| Başlangıç | Gün 1 = 7 Mart 2011 Pazartesi. Hafta Pazartesi başlar, 6. ve 7. günler hafta sonudur | `MarketCalendar` |
| Mevsim | İlkbahar (Mar–May), yaz, sonbahar, kış | `MarketCalendar` |
| Bayram ve özel günler | 2011 için gerçek tarihler: 23 Nisan, 1 Mayıs, 19 Mayıs, Anneler Günü (8 Mayıs), Babalar Günü (19 Haziran), Ramazan (1–29 Ağustos), Ramazan Bayramı (30 Ağu–1 Eyl), 30 Ağustos, okul açılışı (19 Eylül), 29 Ekim, Kurban Bayramı (6–9 Kasım), yılbaşı. Sonraki yıllarda hicri bayramlar her yıl ~11 gün öne kayar | `MarketCalendar` |
| Maaş günleri | Ayın 1'i ve 15'i (memur/emekli), ayın son iş günü: trafik ve sepet büyür. Ay sonuna doğru bütçe daralır | `MarketCalendar` |
| Hava | Mevsime göre deterministik: sıcak gün içecek/dondurma/ayran, soğuk ve yağmurlu gün çay/çorba/makarna ve daha az yaya | `MarketCalendar` |
| Enflasyon | Yıllık TÜFE'nin yaklaşığı: 2011 %10,4 · 2012 %6,2 · 2013 %7,4 · 2014 %8,2 · 2015 %8,8 · 2016 %8,5 · 2017 %11,9 · 2018 %20,3 · 2019 %11,8 · 2020 %14,6 · 2021 %36,1 · 2022 %64,3 · 2023 %64,8 · 2024 %44,4 · 2025 sonrası kurgu senaryo (%30'dan yavaş düşüş). Günlük birikir; toptancı zamları ay başında duyurulur | `MarketSuppliers` |
| Asgari ücret | Net aylık yaklaşık: 2011/1 659, 2011/2 702, 2012 740–774, 2013 804–847, 2014 846–891, 2015 949–1000, 2016 1300, 2017 1404, 2018 1603, 2019 2020, 2020 2324, 2021 2826, 2022 4253–5500, 2023 8507–11402, 2024 17002, 2025 22104. Çalışan ücret beklentisi bunu izler | `MarketSuppliers` (endeksler) |

Tarihsel değerler oyunun atmosferi içindir. Denge gerekirse zorluk ayarıyla yumuşatılır ama tarih gibi sunulan sayı uydurulmaz. 2025 sonrası açıkça **kurgu senaryo**dur.

Uzun oyun için **stratejik ilerletme** vardır: işler devredildiyse oyuncu bir günü, haftayı ya da ayı mağazada oynamadan simüle eder. Mağaza aynı kurallarla, fiziksel müşteri yerine özet talepten işler (bkz. §10).

## 3. Yer: Lüleburgaz ve ötesi

İlk harita dükkân ve sokağıdır. Ekonomik harita semtlerden oluşur. Semt adları gerçek ilçeden esinlenir ama sokaklar ve işletmeler kurgudur.

| Semt (kurgu) | Nüfus | Gelir | Kira (aylık) | Not |
|---|---|---|---|---|
| **İstasyon** (başlangıç) | orta | orta-düşük | yok (aile mülkü) | Eski esnaf, emekliler, tren yolu |
| **Çarşı** | yüksek yaya | orta | yüksek | Esnaf, öğle arası, toplu alım |
| **Kocasinan** | yüksek | orta | orta | Aileler, haftalık alışveriş |
| **Yeni Mahalle / Toki** | artıyor (yıllar içinde büyür) | orta-düşük | düşük | Genç aileler, fiyat duyarlı |
| **Üniversite yolu** | orta, dönemlik | düşük | orta | Öğrenci, gece geç saat, hazır yiyecek |
| **Sanayi** | az konut | orta | düşük | İşçi, sabah erken, toplu kahvaltılık |
| **Villa / Evrensekiz yolu** | düşük | yüksek | yüksek | Premium, taze, marka sadakati |

Sonraki haritalar: Kırklareli (merkez, Babaeski, Pınarhisar), Tekirdağ (Çorlu, Çerkezköy), Edirne; sonra Türkiye bölgeleri; ilk yurt dışı pilotu **Bulgaristan** (Trakya'ya komşu, lev, farklı ürün boyları).

## 4. Karakterler

| Karakter | Kim | Oyundaki işlevi | İlk görünüş |
|---|---|---|---|
| **Nermin teyze** | Mahallenin hafızası, babanın 30 yıllık müşterisi | Veresiye, dedikodu (müşteri memnuniyeti ipuçları), aile itibarı. Dükkâna küserse yarım mahalle küser | 1. gün sabah |
| **Cem** | Babanın 17 yaşından beri yanında çalışan çırağı, şimdi 26 | İlk aday: dürüst, deneyimli, yavaş. Eğitimle şube müdürü olur (3. bölüm) | 2. gün, aday listesinde özel aday |
| **Selim** | Trakya Gıda Dağıtım'ın satış temsilcisi | Tedarik koşulları: vade, iskonto, kampanya desteği. Düzenli ödeme ve hacimle koşullar iyileşir. Geç ödenirse soğur | İlk sipariş |
| **Necati Bey** | Babanın mali müşaviri | Vergi, defter, kasa farkı, nakit uyarısı (G-060) | Masa, istenince |
| **Kadir Bereketoğlu** | Karşı sokaktaki **Bereket Market**'in sahibi (kurgu) | İlk rakip: fiyat savaşı, dedikodu, 2. haftada dükkânı satın alma teklifi, yıllar sonra ortaklık ya da satış teklifi | 1. hafta |
| **Derya** | Operasyon yöneticisi adayı (İstanbul'da zincirde çalışmış) | Şube raporları, görev devri, merkezi depo | 3. bölüm sonu |
| **Banka şube müdürü (Ziraat/Halk… kurgu: "Trakya Bankası")** | Kredi | İlk kredi, faiz, teminat | Nakit sıkıntısı veya şube kararı |
| **Belediye / zabıta** | Denetim, ruhsat | Yeni şube izin süreci, tabela, hijyen | Olay |

Gerçek zincirler (BİM, A101, Migros, Şok, CarrefourSA) yalnızca **olağan ticari davranış** gösterir: fiyat, kampanya, mağaza açma. Suistimal ya da suç gibi olumsuz kurgu olaylar hiçbir zaman gerçek şirkete atfedilmez; bunlar kurgu şirketlerle anlatılır (Bereket Market, "Özdemir Toptan", "Anadolu Perakende A.Ş.").

## 5. Bölümler (ana hikâye)

Başarısızlık hikâyeyi bitirmez. Her bölümün **hedefleri** state'ten ölçülür, **dönüm noktaları** hatıra olarak kaydedilir.

| # | Bölüm | Gerilim | Hedefler (hepsi) | Açtığı şey |
|---|---|---|---|---|
| 1 | **Defter** (İlk hafta) | Dükkânı ayakta tutmak | İlk sipariş · ilk kârlı gün · 7 gün tamamla | Hafta raporu, Cem adaylığı |
| 2 | **Karşı Dükkân** (Mahalle) | Bereket Market ve zincirlerle rekabet | Babanın borcunu kapat · yerel pay %35 · 15 sadık müşteri · sat/devam kararı | Strateji kimliği seçimi, kampanyalar, veresiye |
| 3 | **İkinci Tabela** (İlçe) | Tek kişi her işe yetişemez | 2. ve 3. şube · ilk müdür · oyuncusuz geçen bir gün | Şube yönetimi, stratejik ilerletme, İK müdürü |
| 4 | **Trakya** (Bölge) | Hacim artar, sevkiyat yetişmez | 8 şube, 2 il · bölge deposu · ilk kamyon | Merkezi satın alma, doğrudan üretici anlaşmaları |
| 5 | **Tabela Türkiye'de** (Ulusal) | Büyüme ile kontrol | 50 şube · özel marka · ulusal pazar payı %2 | Yatırımcı/borç/halka arz yolları, bölge müdürlükleri |
| 6 | **Sınırın Ötesi** (Uluslararası) | Başka pazarda yeniden öğrenmek | Bulgaristan pilotu kârlı · 2. ülke | Ülke profilleri, kur |
| 7 | **Miras** (Liderlik) | Büyükken dayanıklı kalmak | Birden çok ölçütte birkaç yıl liderlik | Serbest oyun, alternatif sonlar |

**Sat ya da devam et (Bölüm 2):** 10. gün civarında (en geç borç kapanınca) Kadir Bereketoğlu dükkânı ister. Teklif, dükkânın o günkü değerine göre hesaplanır. Oyuncu satarsa kısa bir "başka hayat" sonu gelir ve kampanya kaydı korunur. Devam ederse **strateji kimliğini** seçer:

| Kimlik | Güçlü yan | Bedeli |
|---|---|---|
| **Mahallenin Bakkalı** (uygun fiyat + samimiyet) | Sadakat ve veresiye güçlü, müşteri fiyatı daha az sorgular | Marj düşük, büyüme yavaş |
| **Kaliteli Yerel** (taze, yöresel, Trakya ürünleri) | Yüksek marj, premium semtlerde güçlü | Fire riski, pahalı tedarik |
| **Hızlı İndirim Zinciri** (az çeşit, koli teşhiri, ölçek) | Şube açmak ucuz, alış iskontosu | Sadakat zayıf, zincirlerle doğrudan savaş |

Hiçbiri her koşulda üstün değildir. Kimlik, rakiplerin oyuncuya nasıl tepki verdiğini de değiştirir.

## 6. Müşteriler

Müşteri bir **segment** ve bir **alışveriş amacı** ile gelir (`MarketCustomers`).

| Segment | Pay (İstasyon) | Liste | Bütçe | Fiyat duyarlılığı | Sevdiği | Saat |
|---|---|---|---|---|---|---|
| Emekli | %22 | 1–3 | düşük | yüksek | süt, çay, temel gıda | sabah |
| Ev (haftalık aile) | %28 | 3–6 | yüksek | orta | süt, makarna, temizlik, içecek | öğleden sonra, hafta sonu |
| Çalışan (iş çıkışı) | %24 | 1–3 | orta | düşük | içecek, hazır, atıştırmalık | akşam |
| Öğrenci | %12 | 1–2 | çok düşük | çok yüksek | içecek, bisküvi, makarna | öğle, akşam |
| Esnaf (toplu alım) | %6 | 2–4 çok adet | yüksek | orta | çay, içecek, temizlik | sabah |
| Çocuk / hızlı eksik | %8 | 1 | çok düşük | düşük | bisküvi, gazoz, ayran | okul çıkışı |

Semt değişince segment payları değişir (Üniversite yolunda öğrenci %40). Bütçe maaş gününde artar, ay sonunda düşer. Müşteri listesini semt + mevsim + hava + bayram ağırlıklarıyla seçer. Pahalı gelen ürünü alma ihtimali segmentin duyarlılığına göre değişir. Sadakat (G-053) ve rakip fiyatı (G-054) korunur.

**Veresiye:** tanınan müşteriler (sadakat havuzu) ödeme gününe kadar deftere yazdırabilir. Oyuncu kişi başı limit koyar. Ödemeler maaş günlerinde gelir; bazıları gecikir, çok azı hiç ödemez. Veresiye sadakati ve Nermin teyzenin gözündeki itibarı artırır ama nakdi bağlar (`MarketCredit`).

## 7. Rakipler

Rakipler bütçesi, stratejisi ve hafızası olan **şirketlerdir** (`MarketCompetitors`). Oyuncunun verisini görmezler; yalnızca gözlenebilen şeye tepki verirler: raf fiyatlarımız, pazar payımız, açtığımız şube.

| Şirket | Format | Strateji | Tepkisi |
|---|---|---|---|
| **Bereket Market** (kurgu, yerel) | mahalle marketi | Duygusal, kinci, nakdi az | Payımız artınca 2–3 gün sonra en çok sattığımız reyonda indirim; nakdi biterse pes eder; 2. bölümde satın alma teklifi; yıllar sonra satılık olur (satın alınabilir) |
| **BİM** | indirim | Düşük maliyet, sabit çeşit, "aktüel" günleri | Haftalık aktüel ürün kampanyası; fiyat savaşına girmez, sadece her zaman ucuzdur |
| **Migros** | süpermarket | Geniş çeşit, kart kampanyaları | Hafta sonu kampanyası, premium semtlerde güçlü |
| **A101** | indirim | Agresif açılış | 15. gün ilçeye girer (G-054), payımız yüksek semtlere şube açar |
| **Şok** (2011 ortasından) | indirim | Yeni sahiple hızlı büyüme | 2011 yazından sonra yeni şubeler |

Pazar payı modeli: bir semtteki alışveriş gücü, dükkânlar arasında çekiciliğe göre bölünür. Çekicilik = fiyat algısı + bulunurluk + hizmet (bekleme) + mesafe + sadakat + kampanya. Yerel payımız (şu anki `MarketShare`) bu modelin İstasyon semtindeki sonucudur.

## 8. Tedarik

| Kaynak | Açılış | Koşul |
|---|---|---|
| **Trakya Gıda Dağıtım (Selim)** | Başlangıç | Liste fiyatı, 50 TL asgari, peşin. 4 hafta düzenli ödeme → 7 gün vade. Aylık hacim 1.500 TL → %3 iskonto |
| **Özdemir Toptan** (kurgu, ucuz) | 2. bölüm | %4 ucuz, ama eksik/hasar 3 kat, bazen gecikir |
| **Üretici doğrudan** (Sütaş, Pınar, Coca-Cola bayisi…) | Bölge hacmi | Kategori bazında %8–12 ucuz, büyük asgari sipariş, raf/kampanya şartı |
| **Bölge deposu** (kendi) | 4. bölüm | Kendi dağıtımın; kamyon, rota, soğuk zincir |

Zamlar: enflasyon maliyeti her gün biraz artırır; toptancı liste fiyatını ayın 1'inde günceller ve oyuncuya "zam listesi" gelir. Raf fiyatını güncellemeyen oyuncunun marjı erir. Zamdan önce stok yapmak meşru bir stratejidir ama depo ve nakit sınırlıdır.

## 9. Mağaza işleri

- **Raf dizimi:** oyuncunun elle dizmesi (G-045…047) ve görevlilerin kuralları (G-049) korunur. Yeni şubeler için **otomatik planogram** (`MarketLayout`): reyonlar kategori komşuluğuna göre sıralanır (süt ile kahvaltılık, içecek ile atıştırmalık, temizlik ayrı ve gıdadan uzak). Ön yüz sayısı talep ve marjla orantılıdır. Yüksek marjlı ürün göz hizasına, ağır ürün alt rafa konur. Her ürün en az bir koli alır. Oyuncu bu planı şablon olarak kaydedip başka şubeye uygulayabilir.
- **Tazelik:** süt ürünleri ve ekmek gibi ürünlerde parti ve son kullanma günü vardır. Görevli önce eskisini öne koyar (FEFO). Son gün %30 indirim rafı açılır, süresi geçen ürün fire olur. Fire, gün raporunda "neden" olarak görünür (`MarketFreshness`).
- **Kampanyalar (oyuncu):** reyon indirimi, "3 al 2 öde", haftalık broşür (bedelli, trafik artırır), gondol başı teşhir (görünürlük), tedarikçi destekli kampanya (Selim bedelin bir kısmını karşılar). Her birinin maliyeti, süresi ve rakibin tepkisi vardır (`MarketPromotions`).
- **Olaylar:** dolap arızası, elektrik kesintisi, zabıta denetimi, mahalle düğünü (toplu alım), cenaze evi (veresiye/ikram), öğrenci gecesi, kar yağışı, su baskını, tedarikçi grevi, geri çağırma. Her olay bir karar sunar. Günde en çok bir büyük olay gelir (olay bütçesi). Aynı olay 2 haftadan önce tekrar etmez (`MarketEvents`).

## 10. Şubeler

Şube açmak bir **süreçtir** (`MarketBranches`): semt seç → kira sözleşmesi (depozito) → tadilat ve ekipman (format bütçesi) → izin (3–10 gün, zabıta olayı olabilir) → işe alım (İK) → açılış stoğu (merkezden transfer ya da sipariş) → açılış günü kampanyası → olgunlaşma (ilk 30 günde müşteri alışkanlığı oluşur).

Ziyaret edilmeyen şubeler **özet simülasyonla** işler. Talebi semt, segment, takvim, rakipler, stok bulunurluğu, personel ve fiyat belirler. Aynı `MarketDemand` kuralları müşteri başına değil, grup halinde uygulanır. Stok, para ve fire tek kaynaktan yürür; ziyaret edilen şube aynı veriden kurulur. Yeni şube yakındaki kendi şubemizden müşteri çalar (**yamyamlık**).

Müdür: şubeye **hedef ve yetki** verilir (asgari bulunurluk, fiyat bandı, sipariş bütçesi, personel sayısı). Müdür bunlar içinde sipariş verir, fiyatı rakibe göre ayarlar ve istisnaları raporlar. Müdürün becerisi ve dürüstlüğü G-060 kişi modelinden gelir.

### İnternet mağazacılığı ve ödeme (G-069, uygulandı, derlenmedi)

Kanallar dönemle açılır (`MarketOnline`):

- **Telefon siparişi (2011+):** bakkal geleneği. Tanıdık müşteri (3+ ziyaret, memnun) arar; en çok emekliler ve aileler. Kurye yoksa günde 4 siparişi kapanıştan sonra oyuncu götürür. Evde hizmet almak sadakat sayılır.
- **Web sitesi (2014+):** kurulum ve barındırma masrafı var, en az bir kurye ister, kartla ödenir (%1,8). Bütün semtten büyük sepetler gelir. İnsanların siteyi öğrenmesi iki ay sürer.
- **Platform (2016+, kurgu ad "Getirsin"):** kuryeyi platform sağlar, komisyonu %18'dir. Sipariş sayısını yıldız belirler; yıldız, online itibardan gelir.

Online sipariş dükkânın stoğunu paylaşır: kapanışta önce depodan, sonra raftan toplanır. Eksik ürün için oyuncu bir kural seçer: müşteriyi arayıp sorar, aynı reyondan benzerini koyar ya da ürünü çıkarır. Kuryenin taşıyabileceğinden fazla sipariş gelirse bir kısmı geç kalır, bir kısmı iptal olur. Her iki durumda itibar düşer. Toplama işi reyon görevlisini yorar.

İlçedeki market alışverişinin bir kısmı her yıl internete kayar: 2016'da ~%1,2, 2023'te ~%5. Bu müşteriler **her dükkânın** kapısından eksilir. Yalnızca online olan dükkân bir kısmını sipariş olarak geri kazanır. Online olmamanın bedeli yavaş yavaş azalan müşteridir; online olmanın bedeli kurye, komisyon ve toplama işidir. Kurye sabit maliyettir, bu yüzden hacim yoksa zarar ettirir (taklitli simülasyon: 2024'te 4 sipariş/gün için 2 kurye zarar ettirir).

**2020–2021 profili (açık soru 2, varsayılan açık, `PandemicProfile` komutuyla kapatılır):**

- Mart 2020'de on günlük panik alışverişi: temel gıda, temizlik ve kâğıt talebi ×2.
- 2020 baharında ve 2020-21 kışında hafta sonu kısıtlamaları: dükkân kısa saat açık, müşteri ×0,4.
- 29 Nisan – 17 Mayıs 2021 tam kapanma.
- Online payı ×3,5, sonra ×2,5, 2022 sonuna kadar ×1,4.
- Hastalık ya da ölüm içeriği yoktur.

**Ödeme (`MarketPayments`):**

- **Kartla ödemek isteyenlerin payı yıllara göre artar:** 2011'de %25, 2021'de %75. Emekliler nakit öder; beyaz yakalılar ve öğrenciler kart kullanır; çocuklar hep nakit öder.
- **POS yoksa:** kart isteyen müşterinin %35'i sepeti kasada bırakır, kalanı söylenerek nakit öder.
- **POS varsa:** aylık kira ve %1,8 komisyon ödenir; para ertesi gün hesaba geçer. Kartla ödeyen müşteri sepete biraz daha fazla koyar.
- **Yemek kartı:** POS ister. Komisyonu %6'dır ve aylık aidatı vardır; karşılığında öğlen işçiler uğrar (müşteri +%3).
- **Veresiyeye yazılan sepet** o gün ödenmez.

## 11. Finans

- **Nakit, borç ve varlık** ayrı izlenir. Ay sonunda (Necati Bey varsa) kâr-zarar, nakit akışı ve basit bilanço hazırlanır.
- **Kira** (yeni şubeler), **elektrik** (dolap sayısı ve mevsim), **bakım**.
- **Banka kredisi:** limit, nakit akışı ve teminata (dükkân tapusu) bağlıdır. Faiz yılın faiz ortamını izler: 2011 %15 dolayı, 2018–19 %25+, 2021–23 dalgalı. Taksitler aylıktır.
- **Tedarikçi vadesi:** ücretsiz kısa vadeli finansmandır.
- **Ödeme sıkıntısı:** otomatik iflas yoktur. Önce uyarı gelir, sonra toptancı sevkiyatı durdurur, sonra borç yapılandırma, stok eritme, şube kapatma, varlık satışı. En son aile dükkânı ipoteklenir. Oyuncu her aşamayı önceden görür.

## 12. Büyüme ve şirket

Aile dükkânı → ilçe zinciri → bölge (depo, kamyon) → ulusal (özel marka "Miras", bölge müdürlükleri, merkezi satın alma) → uluslararası (ülke profili: para birimi, ürün boyları, rakipler, kira). Her aşamanın açılma koşulu **birden çok ölçüttür** (nakit, hizmet, ekip, borç yapısı); tek başına para yetmez. Şirket adı ve tabela kimliği oyuncunun seçimidir. İlk dükkânın aile tabelası korunabilir.

Sonlar: **Sattın** (2. bölüm), **Mahallenin dükkânı** (tek dükkânda kalıp sağlam yaşamak da bir sondur), **Trakya'nın markası**, **Türkiye'nin markası**, **Sınır ötesi**, **Miras** (birden çok ölçütte liderlik). Hiçbiri "kaybettin" değildir.

## 13. Uygulama sırası ve durum

| Adım | Modül | Durum |
|---|---|---|
| Personel ve muhasebe | `MarketStaff` | G-060, derlenmedi |
| Takvim, mevsim, bayram, hava, maaş günü | `MarketCalendar` | G-061, derlenmedi (trafik, sipariş öngörüsü, akşam raporu bağlı) |
| Müşteri segmentleri | `MarketCustomers` | G-062 |
| Tedarik, enflasyon, zam | `MarketSuppliers` | G-063 |
| Oyuncu kampanyaları | `MarketPromotions` | G-064 |
| Rakip şirketler ve pazar payı | `MarketCompetitors` | G-065 |
| Hikâye bölümleri ve olaylar | `MarketStory`, `MarketEvents` | G-066 |
| Tazelik, veresiye, finans | `MarketFreshness`, `MarketCredit`, `MarketFinance` | G-067 |
| Şubeler ve otomatik raf dizimi | `MarketBranches`, `MarketLayout` | G-068 |
| İnternet mağazacılığı ve ödeme | `MarketOnline`, `MarketPayments` | G-069, derlenmedi |
| İnsan hareketi zekâsı | `MarketPeople` | G-070 |
| Stratejik ilerletme ve zorluk | `MarketDirector` | G-071 |
| Şirket büyümesi | `MarketCompany` | G-072 |
| Oyuna bağlama noktası | `MarketDirector` | her adımda büyür |

Her adım: modül + test + `MarketDirector` bağlantısı + bu kitapta ilgili bölümün "uygulandı" notu + GUNLUK girişi + commit.
