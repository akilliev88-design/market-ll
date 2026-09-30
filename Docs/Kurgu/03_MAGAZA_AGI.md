# Mağaza ağı ve yönetim hiyerarşisi (taslak 2, Mustafa'nın cevaplarıyla)

29.09.2026 — Claude, Mustafa'nın şu kararlarından çıkarıldı:
- Lüleburgaz ve semtler kalkıyor; her ülkede yalnız **il** düzeyi var.
- Bütün ülke baştan açık.
- İlk şubeden sonra şubeleri **müdürler** yönetir. Müdürler bize bağlıdır ve iyi ya da kötü çalışabilir.
- Şirket büyüdükçe müdürlerin üstüne **il müdürü** ve **bölge müdürü** (ör. Trakya bölge müdürü) gelir.
- Gezilebilir mağaza: **her ilde her market türünden 1 tane**. Yurt dışında **her ülkede her türden 1 tane**.

Bu belge kod yazılmadan önce mantığı kontrol etmek için yazıldı.

**Taslak 2 (Mustafa'nın cevapları, 29.09.2026):**
- Hipermarket de bir tür olarak eklenir.
- Harita ana ekran olur (§12).
- Kırcaali, Filibe ve Köstence eski kayıtta kapanır, depozito geri gelir; Bulgaristan ve Romanya dünya aşamasında eklenir.
- **Taslak 3:** İl müdür yardımcısı yok; il müdürü ildeki bütün mağazalara bakar. Kişi sınırı yalnız sende: en çok **5** doğrudan bağlı (§4.5).
- Bölgeler 7 coğrafi bölge + alt bölgeler (Trakya, Doğu Karadeniz…) olarak ikiye ayrılır; diğer ülkeler de böyle bölünür (§1).
- **Varsayılan başlangıç ili yok:** oyuncu ili kendisi seçer.
- **Türkiye'nin de ülke müdürü olur:** Şirket ikinci ülkeye girince her ülke, Türkiye dahil, bir ülke müdürüne bağlanır.

---

## 1. Coğrafya: yalnız il

İki bölge düzeyi var:
- **Ana bölge:** coğrafi bölge; başında **bölge direktörü** durur.
- **Alt bölge:** başında **bölge müdürü** durur.

Her il tek bir alt bölgeye bağlıdır. Oyun için sade tutuldu: sınırda kalan bir il, çoğunluğunun olduğu bölümde sayılır.

| Ülke | İl karşılığı | Sayı | Ana bölge | Alt bölge |
|---|---|---|---|---|
| Türkiye | il | 81 (`iller.json`, haritalı) | 7 coğrafi bölge | 20 (aşağıda) |
| Almanya | eyalet (Land) | 16 | 4: Nord, Ost, West, Süd | aynı 4 (ayrı bir üst düzey gerekmez) |
| Birleşik Krallık | bölge / ülke | 12 | 2: İngiltere, İskoçya–Galler–K. İrlanda | 6: Kuzey, Midlands, Güney İngiltere, İskoçya, Galler, K. İrlanda |
| ABD | eyalet (state) | 50 | 4 nüfus sayımı bölgesi: Northeast, Midwest, South, West | 9 alt bölge (division): New England, Mid-Atlantic, East North Central, West North Central, South Atlantic, East South Central, West South Central, Mountain, Pacific |

**Türkiye'nin alt bölgeleri (81 il):**

| Ana bölge | Alt bölge | İller |
|---|---|---|
| Marmara | **Trakya** | Edirne, Kırklareli, Tekirdağ |
| Marmara | İstanbul–Kocaeli | İstanbul, Kocaeli, Sakarya, Yalova |
| Marmara | Güney Marmara | Bursa, Balıkesir, Çanakkale, Bilecik |
| Ege | Ege Kıyısı | İzmir, Aydın, Muğla, Manisa |
| Ege | İç Ege | Denizli, Uşak, Kütahya, Afyonkarahisar |
| Akdeniz | Batı Akdeniz | Antalya, Isparta, Burdur |
| Akdeniz | Çukurova | Adana, Mersin, Hatay, Osmaniye, Kahramanmaraş |
| İç Anadolu | Ankara | Ankara, Kırıkkale, Çankırı, Eskişehir |
| İç Anadolu | Konya | Konya, Karaman, Aksaray, Niğde |
| İç Anadolu | Orta Kızılırmak | Kayseri, Nevşehir, Kırşehir, Yozgat, Sivas |
| Karadeniz | Batı Karadeniz | Zonguldak, Bartın, Karabük, Bolu, Düzce, Kastamonu, Sinop |
| Karadeniz | Orta Karadeniz | Samsun, Amasya, Çorum, Tokat, Ordu |
| Karadeniz | **Doğu Karadeniz** | Giresun, Trabzon, Rize, Artvin, Gümüşhane, Bayburt |
| Doğu Anadolu | Erzurum–Kars | Erzurum, Kars, Ardahan, Iğdır, Ağrı, Erzincan |
| Doğu Anadolu | Yukarı Fırat | Malatya, Elazığ, Tunceli, Bingöl |
| Doğu Anadolu | Van Gölü | Van, Muş, Bitlis, Hakkari |
| Güneydoğu Anadolu | Orta Fırat | Gaziantep, Şanlıurfa, Adıyaman, Kilis |
| Güneydoğu Anadolu | Dicle | Diyarbakır, Mardin, Batman, Siirt, Şırnak |

Almanya'nın 4 bölgesi:
- **Nord:** Schleswig-Holstein, Hamburg, Bremen, Niedersachsen, Mecklenburg-Vorpommern.
- **Ost:** Berlin, Brandenburg, Sachsen, Sachsen-Anhalt, Thüringen.
- **West:** Nordrhein-Westfalen, Hessen, Rheinland-Pfalz, Saarland.
- **Süd:** Baden-Württemberg, Bayern.
- **Her ilin verisi:**
  - nüfus;
  - alım gücü (müşterinin fiyata toleransı ve sepet büyüklüğü);
  - kira çarpanı;
  - rekabet (zincirlerin yoğunluğu; Türkiye'de `iller.json`'daki BİM/A101/ŞOK/Migros sayıları);
  - bölge.
- **Özel değerler:** Öne çıkan illere (İstanbul, Ankara, İzmir, Van…) elle değer verilir. Diğerlerinin değeri nüfustan türetilir: büyük il = pahalı kira, kalabalık rekabet, biraz yüksek alım gücü.
- **Oyun başı:** Oyuncu haritadan ülke ve il seçer. **Varsayılan il yok**; seçmeden oyun başlamaz.
- **Aile dükkânının yeri:** Aile dükkânı bu **ev ilinde**dir. Oyunda il altında hiçbir yer adı geçmez; dükkânın tabelasında "MİRAS MARKET / <İL>" yazar.

## 2. Market türleri

| Tür | Kimin için | Raf / ürün | Çalışan | Fiyat hedefi (rakibe göre) | Hizmet | Kira | Günlük alışveriş (olgun) |
|---|---|---|---|---|---|---|---|
| **Ucuzcu** (indirim marketi) | fiyata duyarlı aileler, öğrenciler | dar çeşit, koliden satış | 3 | %6 ucuz | düşük | düşük | çok |
| **Mahalle marketi** | yakındaki herkes, emekliler | orta çeşit | 3 | eşit | orta | orta | orta |
| **Süpermarket** | haftalık alışveriş, yüksek gelir | geniş çeşit, marka | 6 | %4 pahalı | yüksek | yüksek | çok, büyük sepet |
| **Hipermarket** | ayda birkaç kez büyük alışveriş, arabayla gelen | en geniş çeşit, gıda dışı, toplu paket | 20 | %2 ucuz (hacimle) | yüksek | çok yüksek, şehir dışı arsa | en çok, en büyük sepet |

- **Tür, ilin halkıyla eşleşir:**
  - Alım gücü düşük ilde ucuzcu, yüksek ilde süpermarket daha iyi çalışır.
  - Ucuzcuyu fiyat savaşı, süpermarketi tazelik ve çeşit bozar.
- **Hipermarket (Mustafa: evet):**
  - Ne zaman: 5. bölümde ("Ülke Çapında") ve en az 1 bölge deposu varken açılır.
  - Nerede: Yalnız nüfusu 500 binin üstündeki illerde.
  - Etkisi: O ildeki kendi küçük mağazalarımızdan da müşteri alır.

## 3. Mağaza açmak

- **Nerede:** Ülkenin her ili baştan açık.
- **İlk şube için:**
  - işletmenin borcu kapanmış olmalı;
  - belli sayıda kârlı gün;
  - yerel payda hedef.

  (Bunlar bugünkü şartlar, aynen kalır.)
- **Üçüncü mağazadan itibaren:** İK müdürü gerekir.
- **Ev ili dışında ilk mağaza için:**
  - İK müdürü;
  - mali müşavir;
  - o mağazayı yönetecek **bir müdür**, terfi ile ya da dışarıdan.

  Senin şirketin artık "dükkân" değil.
- **İlde yer sınırı:** Bir ildeki mağaza sayısı nüfusla sınırlı: her 40 bin kişiye 1 mağaza, en az 2. Kırklareli ~9, İstanbul ~390. Sınıra yaklaştıkça kendi mağazalarımız birbirinin müşterisini alır (bkz. §7).
- **Açılış süreci (bugünkü gibi):**
  1. Kira sözleşmesi ve depozito.
  2. Tadilat, 5 gün.
  3. Ruhsat, 3 gün; mali müşavir yoksa +3 gün.
  4. İşe alım.
  5. Açılış stoğu.
  6. Açılış.
- **Açılış bedeli:** 2 aylık kira (ilin kira çarpanıyla) + türün tadilat bedeli + açılış stoğu.

## 4. Yönetim hiyerarşisi

```
SEN (genel müdür)
 └─ Ülke müdürü            (şirket ikinci ülkeye girince her ülkede zorunlu, Türkiye dahil)
     └─ Bölge direktörü    (ana bölge: Marmara, Karadeniz...)
         └─ Bölge müdürü   (alt bölge: Trakya, Doğu Karadeniz...)
             └─ İl müdürü  (ilde 3+ mağaza)
                 └─ Mağaza müdürü  (her şubede)
```

- **Her düzey gerektiğinde gelir.** Küçük bir şirkette mağaza müdürleri doğrudan sana bağlıdır.
- **Atanmamış bir düzeyin kişileri bir üstteki kişiye bağlanır.** Örnek: İl müdürü yoksa o ilin mağaza müdürleri, bölge müdürüne ya da sana bağlanır.
- **Kişi sınırı yalnız sende (§4.5).** Senin 5 kişilik sınırın dolunca bir üst düzey atayıp altındakileri ona bağlarsın.

### 4.1 Mağaza müdürü (her şubede bir)

- **Nasıl gelir:** İki yol var.
  - Aile dükkânından terfi. Ailenin işini bilir; becerisi +5, ücreti %40 artar.
  - Dışarıdan işe alım. Rastgele beceri; İK müdürü varsa aday görülerek seçilir.
- **Gizli özellikler:** beceri, dürüstlük ve **tarz**.
  - Temkinli: az stok, az fire, bazen raf boş.
  - Cömert: çok stok, raf dolu, fire çok.
  - Fiyatçı: rakibi izler, marjı ezebilir.

  Beceri ve dürüstlük İK müdürüyle (ve il müdürüyle) görünür hâle gelir.
- **Performans (her hafta karne A–D):**
  - kâr / beklenen kâr;
  - raf doluluğu;
  - fire;
  - müşteri memnuniyeti;
  - kasa tutarlılığı (dürüst olmayan müdür kasadan az az alır).
- **Senin kararların:**
  - prim ver (moral ve beceri gelişir, maliyet);
  - uyar;
  - değiştir (yeni müdür 1 hafta alışma dönemi, eskisi tazminat alır);
  - terfi ettir (iyi mağaza müdürü → il müdürü adayı).
- **Kendiliğinden gelişme:**
  - İyi yönetilen mağazada müdür zamanla gelişir.
  - Kötü karne üst üste gelirse moral düşer ve müdür ayrılabilir.

### 4.2 İl müdürü

- **Ne zaman:** Bir ilde **3 mağaza** olunca atanabilir.
- **Kaç mağaza olursa olsun tek il müdürü:** İl müdürü ildeki bütün mağazalara bakar; yardımcı ya da ikinci il müdürü yok.
- **Büyük il, iyi il müdürü ister (Claude önerisi):** İl müdürünün denetim gücü becerisine bağlı. Gereken beceri = 40 + mağaza sayısı / 3.
  - İzmir'de 30 mağaza varsa 50 beceri yeter.
  - İstanbul'da 150 mağaza için 90 beceri gerekir.
  - Beceri yetmezse denetim eksik kalır: sipariş hatası artar, hırsız daha geç yakalanır.

  Böylece ek kademe açmadan büyük il yine bir karar olur: kimi il müdürü yaparsın, ne kadar ücret verirsin.
- **Ev ilinde:** Aile dükkânı hariç sayılır.
- **Ne yapar:**
  - O ildeki mağaza müdürlerini denetler; sipariş hataları azalır, karneler yükselir.
  - **Kasadan çalanı yakalar:** haber gelir, müdür değiştirilir.
  - Kötü müdürü önce uyarır, sonra sana değiştirme önerisi getirir.
  - O ilde yeni şube açılışını 2 gün kısaltır.
  - Kendi becerisi ve dürüstlüğü var. Kötü il müdürü iyi mağaza müdürlerini de yorar.
- **Kimden:** en iyi mağaza müdüründen terfi ya da dışarıdan.

### 4.3 Bölge müdürü (alt bölge) ve bölge direktörü (ana bölge)

- **Bölge müdürü ne zaman:** Bir alt bölgede **2 il müdürü** olunca atanabilir.
- **Ne yapar:**
  - İl müdürlerini denetler.
  - Bölge deposu ve kamyonlarla birlikte lojistik kaybını azaltır.
  - Bölgedeki fiyat ve kampanya kararlarını senin kurallarına göre uygular.
- **Örnek:** "Trakya bölge müdürü" Edirne, Kırklareli ve Tekirdağ il müdürlerinin üstündedir.
- **Bölge direktörü:** Bir ana bölgede (ör. Karadeniz) 2 bölge müdürü olunca atanabilir. Bölge müdürlerini denetler; bölge depolarının ortak planlamasıyla +%0,5 marj sağlar.

### 4.4 Ülke müdürü (her ülkede, Türkiye dahil)

- **Ne zaman zorunlu:** Şirket **ikinci bir ülkeye girdiğinde**, her ülkenin bir ülke müdürü olur; Türkiye dahil. Hiçbir ülkenin ağı doğrudan sana bağlanmaz.
- **Tek ülkedeyken:** İstersen daha önce de atayabilirsin. Ülke müdürü senin 5 kişilik sınırını tek başına rahatlatır, ama pahalıdır.
- **Yabancı ülkede ek:** O ülkenin dilini, yasasını (ör. Almanya'da pazar kapalı) ve toptancılarını bilir.
- **Kimler bağlanır:** O ülkedeki bölge direktörleri ve bölge müdürleri. Onlar yoksa il ve mağaza müdürleri.

### 4.5 Kişi sınırı: yalnız sende

- **Sınır:** Sen en çok **5** kişiyle doğrudan ilgilenebilirsin. İK müdürü bu sayıyı artırmaz.
- **Senden aşağısında sınır yok:** İl müdürü ildeki bütün mağazalara, bölge müdürü bütün il müdürlerine bakar. Onların sınırını beceri belirler (§4.2).
- **Doğrudan bağlı sayılanlar:** Bir üst düzeyi atanmamış herkes. Örnek: il müdürü olmayan ildeki mağaza müdürleri, bölge müdürü olmayan il müdürleri, ülke müdürü yoksa bölge direktörleri.
- **Aile dükkânı:** Müdür atanmadıkça sayılmaz, çünkü orada zaten sen varsın.
- **5'i aşınca "gözetim eksikliği" başlar, ağın sana doğrudan bağlı kısmında:**
  - Fazladan her kişi için o kişilerin becerisi −4 (en çok −25).
  - Kasadan çalan görünmez olur.
  - Memnuniyet yavaşça düşer.
- **Örnek büyüme:**
  - Tek ilde 5 şube: 5 mağaza müdürü sana bağlı; sınırdasın.
  - 6. şube için il müdürü atarsın. İl müdürü 1 kişi sayılır, altındaki bütün mağazalar onun.
  - 5 ilde il müdürü olunca yine sınırdasın; bölge müdürü ya da ülke müdürü atarsın.
- **Sonuç:** Küçükken hiyerarşiye gerek yok; büyüdükçe kademeler zorunlu olur. Yönetim maliyeti de gerçek bir karardır.

### 4.6 Ücretler (günlük, oyun başı fiyat düzeyinde; enflasyonla artar)

| Rol | Ücret |
|---|---|
| Mağaza müdürü | 30–45 TL (beceriye göre) |
| İl müdürü | 60–80 TL |
| Bölge müdürü | 120–150 TL |
| Bölge direktörü | 180–220 TL |
| Ülke müdürü | 250 TL + ülkenin ücret düzeyi |

### 4.7 Aile dükkânı ve sen

- Başta aile dükkânında durursun: birinci şahıs, kasa, raf.
- İstersen aile dükkânına da **müdür atarsın**. Dükkân onun kararlarıyla işler; sen yönetim paneli, harita ve istediğin mağazayı ziyaret etmekle uğraşırsın.
- **Aile dükkânına müdür atayınca:** Oyun **harita ana ekranından** açılır (§12). Herhangi bir mağazaya "içeri gir" diyerek birinci şahısa geçersin.

## 5. Gezilebilir mağazalar (birinci şahıs)

- **Kaç tane:** Her **il × tür** için bir gezilebilir mağaza var; ör. "İzmir – Ucuzcu", "İzmir – Süpermarket". Yurt dışında her **ülke × tür** için bir tane.
- **Diğer mağazalar:** Aynı ildeki aynı türden öteki mağazalar o gezilebilir mağazanın raf düzenini ve ürün seçimini paylaşır. Simülasyonları ayrıdır; kira, müşteri ve müdür ayrıdır.
- **Raf düzeni:** Gezdiğin mağazada rafı değiştirirsen soru gelir: "Bu düzen İzmir'deki bütün ucuzcularına uygulansın mı?" Ülke şablonu da mümkün: bütün ucuzcular.
- **Aile dükkânı:** Kendi özel düzeniyle ayrı kalır.
- **Ne zaman yapılır:** Ziyaretin kendisi, yani sahne ve dükkân maketi (karar L01/L13), ayrı bir iş (G-087). Bu belgedeki ağ modeli ona hazır kurulur.

## 6. Lojistik

G-089 (karar M23) ile depolar **ile** kurulur. Kod: `MarketDepots.*`.

- **Depo yoksa:** Ev ili dışındaki şube malı toptancının arabasıyla alır: maliyet +%3. Ev ilindeki şube ev ilinin toptancısından alır (ek maliyet yok).
- **Depo (bir ilde):**
  - Oyuncu ili seçer. Oyun, şubelerin ciro ağırlıklı uzaklığını en aza indiren ili önerir ve tahmini aylık kazancı yazar.
  - Kurma bedeli 30.000 TL, kira ayda 6.000 TL (oyun başı fiyat düzeyi; ilin kira çarpanı ve enflasyonla artar). En az 4 mağaza ve 600 km içinde bir şube gerekir. Bir ilde tek depo.
  - Kapasite 60 şube; fazlası depoyu yavaşlatır.
- **Bağlantı:** Her açık şube kendi ülkesindeki **en yakın** depodan alır. Menzil 600 km; ötesinde depo yok sayılır. Ev ilindeki şube depoyu yalnız 200 km içindeyse kullanır.
- **Mesafe:** İl merkezleri arası kuş uçuşu. Türkiye'de `iller.json` harita koordinatı × 1,61 km (İstanbul–Ankara ≈ 350 km, Edirne–Kars ≈ 1.380 km). Haritası olmayan ülkede: aynı alt bölge 150 km, aynı ana bölge 350 km, değilse 700 km.
- **Maliyet:** Depo iskontosu %1,5 × depo verimi. Yol: ilk 100 km bedava, sonra her 100 km için mal maliyetine +%0,6.
- **Kamyon:** Her bağlı şube 1 yük, her 300 km için +1 yük; bir kamyon günde 8 yük taşır. Eksik kamyon en çok +%1,5 maliyet ve yolda %2'ye kadar kayıp getirir.
- **Depo müdürü** (günlük 80–120 TL, oyun başı fiyat düzeyi):
  - Verim: müdürsüz %50; müdürlü %60–100 (beceriyle). İlk hafta alışma −20.
  - Düşük verim: fire artar (günde en çok +%0,4), teslimatın %4'üne kadarı eksik/kırık gelir; raf boşalır.
  - Dürüst olmayan müdür teslimatın ‰8'ini kaçırır. Onu yalnız ülke müdürü ya da oyuncu yakalar (il/bölge müdürü değil). Uyarı bir süre durdurur.
  - Ülke müdürü varsa ona, yoksa oyuncuya bağlıdır ve 5 kişi sayımına girer.
  - Müdürsüz depo her hafta uyarı verir.
- **Merkezi satın alma:** 8 mağazadan sonra, +%2 (bir depo gerekir).
- **"Miras" özel markası:** 20 mağazadan sonra, +%1,5 marj ve biraz daha müşteri.
- **Yurt dışı:**
  - gümrük ve evrak: +%1;
  - ilk 90 gün "öğrenme": −%3 marj;
  - o ülkenin toptancıları (G-083 ile gelecek).
- **Eski kayıtlar:** Alt bölge deposu, o alt bölgede mağazamız en çok olan ile (yoksa en kalabalık iline) taşınır; müdürsüz başlar ve bir kez "depona müdür ata" haberi gelir.

## 7. Kendi mağazalarımızın birbirinden müşteri alması

- **İlin "yeri":** Nüfus / 80 bin (en az 1). Kırklareli ~4,5, İstanbul ~195.
- **Kayıp:** Aynı ilde yer sayısını aşan her kendi mağazamız, öteki mağazaların müşterisini azaltır.
- **Ev ili:** Aile dükkânı da ev ilinde sayılır. Ev iline çok şube açmak aile dükkânını da biraz boşaltır.

## 8. Hikâye bölümleri (il bazına uyarlanmış)

| Bölüm | Ad | Hedefler |
|---|---|---|
| 1–3 | (aynı) | defter, karşı dükkân, ikinci tabela |
| 4 | **İller** | 2 ilde 8 mağaza · ilk il müdürü · ilk bölge deposu |
| 5 | **Ülke Çapında** | 50 mağaza · ilk bölge müdürü · "Miras" markası · ulusal pay %2 |
| 6 | **Sınırın Ötesi** | ülke müdürü ile ilk yabancı mağaza 30 günde kârlı · ikinci yabancı ülke |
| 7 | **Miras** | bir yıl her ölçütte önde (değişmedi; G-082'de dünya ligi) |

## 9. Eski kayıtlar

- **Lüleburgaz şubeleri:** Kırklareli şubeleri olur.
- **Eski "şehir mağazaları":**

  | Eski şehir | Yeni il |
  |---|---|
  | Babaeski, Kırklareli | Kırklareli |
  | Çorlu, Tekirdağ | Tekirdağ |
  | Edirne, Keşan | Edirne |
  | İstanbul (iki yaka) | İstanbul |
  | Bursa, İzmir, Ankara, Kocaeli | aynı il |

  Hepsi müdürlü, olgun mahalle marketine dönüşür.
- **Kırcaali, Filibe, Köstence:** Kapanır, depozito geri gelir. Bulgaristan ve Romanya dünya aşamasında (G-082) ülke paketi olarak eklenir.
- **Eski kaydın başlangıç ili:** Eski kayıtlarda il seçimi yoktu. Eski kayıt Kırklareli'de başlamış sayılır (Lüleburgaz oradaydı). Bu yalnız eski kayıtlar için geçerli; yeni oyunda varsayılan il yok.

## 10. Şubeler sayfası (menü; harita ana ekran olunca §12'ye taşınır)

1. **Harita:** Türkiye'de il haritası; diğer ülkelerde il listesi.
   - Katmanlar: bizim mağazalarımız, rakip yoğunluğu, **müdür kapsaması** (il müdürü olan iller).
   - İle tıklanınca: nüfus, kira, rekabet, bizim mağazalarımız, **tür başına "aç" düğmesi**.
2. **Mağazalar:** Bütün mağazalarımız il il gruplanır.
   - Her satırda müdür, karne (A–D) ve dün/hafta kârı yazar.
   - Düğmeler: müdür değiştir / terfi / uyar / prim, kapat.
3. **Yönetim:** Hiyerarşi ağacı.
   - Senin doğrudan bağlıların ve sınırın ("5/5 — sınırdasın").
   - İl, bölge ve ülke müdürü atama.
   - Haftanın en iyi ve en kötü 3 mağazası.
4. **Şirket:** Özet, depolar, kamyonlar, merkezi alım, özel marka, karanlık mağaza.

## 11. Uygulama sırası (öneri)

1. **G-086a:**
   - İl verisi (4 ülke), ana ve alt bölgeler.
   - Başlangıçta il seçimi zorunlu, varsayılan yok.
   - Şubeler il + türle; 4 tür, hipermarket dahil.
   - Şehir mağazaları şubeye göç eder.
   - Semtler ve Lüleburgaz kalkar.
   - Bölge deposu alt bölge başına olur.
   - Harita ana ekranının sade hâli (§12).
2. **G-086b:**
   - Müdür tarzı ve karne.
   - İl müdürü, bölge müdürü, bölge direktörü, ülke müdürü (her ülkede).
   - Denetim sınırı.
   - Yönetim sekmesi.
3. **G-087:**
   - Gezilebilir mağazalar (il × tür).
   - Aile dükkânına müdür.
   - Raf şablonu paylaşımı.

## 12. Ana ekran: harita (Mustafa'nın önerisi, Claude'un yerleşimi)

Yönetimin ana ekranı haritadır. Bugünkü menünün "Özet" sayfasının yerini alır.

**Sade hâl (30.09.2026, Mustafa onayladı).** İlk tam tasarım (üç kalıcı sütun, dönen kartlar, eşit ağırlıkta paneller) "her şey iç içe" bulundu; baştan sadeleştirildi. Tasarım tuvali "Miras Market Ana Ekran", 4 ve 5 numaralı çizimler.

```
┌──────────────────────────────────────────────────────────────────────┐
│ (12 Nisan · 08:40 ❚❚ 1x 2x 3x) (Kasa · Dün · Mağaza)  (Kararlar ③)(Ayarlar)│  üst haplar
│ (Türkiye | Marmara | Ege | Akdeniz | İç Anadolu | Karadeniz | …)      │  bölge çipleri
│                                                                      │
│                         HARİTA (bütün ekran)                          │
│                                                                      │
│ (! Edirne mağazası iki haftadır D alıyor   [Bak])   (Mağazalarımız|Rakip) │
│        (1 Harita 2 Sipariş 3 Ürünler … 0 Raporlar | Dükkâna gir)       │  alt dok
└──────────────────────────────────────────────────────────────────────┘
```

İlkeler:

1. **Harita sahnedir.** Kalıcı yan sütun yok; harita ekranı doldurur.
2. **Bir anda tek odak.** Bir ile tıklayınca sağdan tek panel açılır (420 px); harita ve haplar ona yer açar. Aynı ile ikinci tıklama ya da Esc paneli kapatır.
3. **Sayılar üstte, eylemler altta.** Sol üst: tarih, saat, yer, hız. Yanında kasa, dünkü net, mağaza sayısı. Sağ üst: kararlar (bekleyen sayısıyla) ve ayarlar. Alt: bütün sayfalar tek dokta (1–0 tuşları sürer) ve "Dükkâna gir".
4. **Bilgi istenince gelir.** Kararlar, yapılacaklar, bölüm hedefleri ve borç "Kararlar" katmanında. Ekranda yalnız bir asistan satırı: en acil yapılacak iş, yoksa günün notu (her gün değişir). "Bak" onu çözen sayfayı açar.
5. **Az renk.** Yeşil bizim iller (ev ili koyu), turuncu sorunlu (karnesi D), gerisi kâğıt ve gri. İkinci katman yalnız "Rakip yoğunluğu".

- **Bölge çipleri (Mustafa, 30.09.2026):** Eski "Trakya" düğmesi kalktı. Yerine ülkenin bütün ana bölgeleri çip olarak durur (TR 7 bölge; öteki ülkelerde kendi bölgeleri). Çipe tıklayınca harita o bölgeye yumuşakça yakınlaşır; bölge dışı iller kâğıda karışır, bölgedeki illerin adları yazılır. Aynı çipe ya da ülke adına tıklamak, Esc de bütün ülkeye döndürür. Öteki ülkeler 6. bölümden (ya da yurt dışında mağaza varsa) aynı sırada çıkar; haritası olmayan ülkelerde seçili bölgenin illeri düğme olarak listelenir.
- **İl paneli:** alt bölge · ana bölge, il adı; dört kutu (nüfus, alım gücü, kira, rekabet) ve bir satırlık beklenti; mağazalarımız (aile dükkânı, şubeler: karne harfi, müdür, son 30 gün); 3+ mağazada "il müdürü istiyor" uyarısı (atama G-086b); "Yeni mağaza aç · N yer daha var" altında dört tür kartı (bedel, çalışan, açılamıyorsa nedeni). İle uyan tür "öneri" ile çerçevelenir (alım gücü düşükse ucuzcu, yüksek ve il 300 bin+ ise süpermarket, yoksa mahalle). Kart tıklanınca "Emin misin?" sorulur.
- **Öteki sayfalar** aynı üst hapların ve alt dokun arasında açılır; başlıkları üstte durur.
- **Görsel dil (M14, G-086d):** oyun içi ekran tuvaldeki çizimlerin aynısıdır. Yazı: IBM Plex Sans (metin), IBM Plex Mono (para ve sayılar), Bricolage Grotesque (il adı, sayfa başlıkları). Renk: kâğıt F4F1EA / koyu 101316; il zemini DEDAD1 / 23282E; bizim il 9ED0BE / 2E6E5D; ev ili 3F9A80; vurgu 2F8A70 / 71C6AC; uyarı D08A1E / E8A94E. Haplar yarım yükseklik köşeli, açık temada yumuşak gölge. Dosyalar `Content/Slate` (Fonts — SIL OFL lisanslarıyla, Icons — beyaz SVG, Shadow.png); paketlemede `DirectoriesToAlwaysStageAsNonUFS=Slate`.
- **Tahtadan küçük farklar:** hız düğmelerine 3x eklendi; ayarlar tarih hapının sonunda; dokta tahtadaki altı sayfa + "Diğer" (Rakipler, Finans, Satış); bölge çipleri katman çiplerinin altında (M13); harita dışındaki sayfalarda dokun başında "Harita".
- **Akış (M15, G-086e):** ekranda hiçbir şey zıplamaz. Dok sabit (Ana ekran, Mağazalar, Sipariş, Fiyat, Kampanya, Personel, Raporlar, Diğer, Dükkâna gir); tarih hapı solda, kasa hapı sağda, zil sağ altta hep aynı yerde. İl paneli haritanın üstünde, sağda yüzen kart (üstte bölge çiplerinin altından, altta dokun üstüne kadar); sağdan kayarak girer, harita aynı anda sola kayar. "Diğer" kartı dokun üstünde solarak belirir. Gölge görseli kaldırıldı (gri kutu gibi görünüyordu); yüzeylerde ince kenar çizgisi var.
- **Dükkân içi HUD** aynı dili kullanır: sol üstte tarih hapı (açık/kapalı, hız) ve kasa hapı (kasa, bugün, borç çubuğu), sağ üstte yuvarlak işaretli uyarı kartları, altta kuyruk/satış hapları, ipucu hapı ve vurgu renkli "Yönetim M" hapı.
- **Oyun başı:**
  1. Yeni oyunda harita açılır: ülke seç → il seç.
  2. Kısa giriş metni gelir.
  3. **Birinci şahısla aile dükkânına girilir** (karar L07).
- **Sonradan:**
  - M tuşu ya da dükkândaki masa haritaya döndürür.
  - Aile dükkânına müdür atandıktan sonra oyun doğrudan haritadan açılır.
- **Ne zaman yapılır:** İlk hâl G-086a'da kodlandı; sade hâl G-086c'de (30.09.2026).

## Kalan sorular

- Yok. Sade ana ekran (§12) onaylandı ve G-086c'de koda geçti.
