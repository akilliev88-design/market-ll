# Yönetim menüsü (G-059, G-074, G-075)

G-075: tam ekran, ekrana göre ölçekli (tasarım 1440×820), arkada bulanık dükkân; uzun açıklamalar "(i) Nasıl işler?" ipucunda.

Tasarım: claude.ai "Miras Market Menü Tasarımı" maketi (A1 oyun ekranı, A2 özet; açık/koyu tema).

## Kod

| Dosya | İçerik |
|---|---|
| `MarketMenuWidget.h` | `SMarketMenu`: 10 sayfa (`EPage`), yapı taşları, sayfa fonksiyonları |
| `MarketMenuInternal.h` | İki kaynak dosyanın ortak yardımcıları (`MarketMenuUi`: para yazımı, rakip fiyatı, alma şansı...) |
| `MarketMenuWidget.cpp` | Çerçeve (kenar çubuğu, başlık, onay katmanı), tema, yapı taşları, Özet, Sipariş, Raporlar |
| `MarketMenuPages.cpp` | Ürünler ve fiyat, Kampanyalar, Rakipler, Personel, Finans, Satış kanalları, Şubeler (harita, Lüleburgaz, şirket) |
| `MarketMenu.cpp` | Aç/kapat, oyunu durdurma, komut yönlendirme, `Todos()` (şimdi ne yapmalı listesi) |
| `MarketHudWidget.*` | Sade oyun ekranı (A1): gün/saat, kasa, borç, açık/kapalı, en çok 3 bildirim, kuyruk, ipucu, "Yönetim M" |
| `MarketMap.*` | Türkiye il haritası (`SMarketMap`, `Config/iller.json`) |
| `MarketRetail.*` | Ulusal zincir payları ve dünyanın en büyük perakendecileri (Rakipler sayfası) |

Menü yalnızca oyunu okur; her karar `AMarketGameMode::MenuCommand` (masadaki tuşların `Command()` kuralı) ya da `StaffCommand` (personel, vergi, `MarketDirector::Command`) üzerinden gider. Kurallar tek yerde kalır.

## Kullanım

| Tuş / düğme | İş |
|---|---|
| **M** (ya da masada E) | Menüyü tam ekran açar; dükkân arkada bulanık görünür ve **oyun sürer**. Fare imleci çıkar. |
| Space, 1 / 2 / 3 | Dükkânda: zamanı durdur/devam, oyun hızı 1x/2x/3x. Menüde: Space ve + / -, başlıkta düğmeler. |
| M, Esc, "Oyuna dön" | Kapatır. |
| 1–9, 0 | Özet, Sipariş, Ürünler ve fiyat, Kampanyalar, Rakipler, Personel, Finans, Satış kanalları, Şubeler, Raporlar |
| Kenar çubuğu | Yazı boyutu (Küçük/Orta/Büyük), zorluk (dükkân kapalıyken), salgın dönemi (10. yıldan önce), açık/koyu tema. Kırmızı nokta: o sayfada acil iş var. |
| "Emin misin?" | Riskli kararlar önce sorar: kovma, şube/mağaza kapatma ve açma, kredi, kampanya başlatma, web/platform, tahsilat. Enter = evet, Esc = vazgeç. |

## Sayfalar

1. **Özet** — sol: kasa, dün, bugün, kısayol daireleri, mağazayı aç/kapat, 1 gün / 1 hafta ilerlet. Sağ: bekleyen karar, **Şimdi ne yapmalı** (her satırda çözen sayfaya düğme), rakipler bugün, babanın borcu, ilk şube hedefleri, hikâye bölümü.
2. **Sipariş** — liste, öneri, toptancı koşulları, faturalar, zammı yansıt.
3. **Ürünler ve fiyat** — ürün resmi (Stüdyo etiketi: kutuda ön yüz, yuvarlakta bant ortası), fiyat, rakiplerin raf fiyatı, alma şansı. Kampanya yok; "Kampanyalar ›" düğmesi.
4. **Kampanyalar** — seçili ürün için indirim / 3 al 2 öde / gondol başı, broşür (reklam), toptancı teklifi, yürüyen her kampanyaya ayrı **Durdur**.
5. **Rakipler** — Yerel (ilçe payları, günlük haberler), Ulusal (`MarketRetail` yıl yıl pay ve mağaza + bizim ulusal payımız), Uluslararası.
6. **Personel** — ekip, İK, çalışanlar (Kov = sorar), başvurular. Vergi ve müşavir Finans'ta.
7. **Finans** — kasa/borç/alacak/eve giden, kredi 500/1.000/2.500 (bugünün parasıyla, sorar), veresiye limiti 4 kademe, tahsilat, vergi ve müşavir, taze ürün politikası (indirim/bağış/hiçbiri), nakit sıkıntısı merdiveni, ev harçlığı.
8. **Satış kanalları** — telefon/web/platform, kurye, ücretsiz teslimat, eksik ürün kuralı, POS ve yemek kartı.
9. **Şubeler** — Harita (katman: biz, BİM, A101, ŞOK, Migros, Onur; Türkiye/Trakya; ile tıkla), Lüleburgaz (şubeler, **Müdür seç** → `PromoteTo`, kapat, semt başına küçük/mahalle/büyük açılış ve açılamama nedeni), Şirket (şehirler +1/-1, yatırımlar).
0. **Raporlar** — gün sonu (en büyük 3 sorun, düğmeli), hafta grafiği. Gün kapanınca menü burada açılır; "Yeni güne başla" kapatır.

Smoke (`-MirasSmoke`) ve ekran görüntüsü (`-MirasCapture`) çalıştırmalarında menü kendiliğinden açılmaz; HUD'daki eski rapor kartı görünür.

## Mağazalar sayfası (G-086b, G-089)

Üç sekme: **Mağazalar**, **Yönetim**, **Şirket**. Her kart sabit yükseklikte; seçiciler aynı kartın içinde katman olarak açılır (M15). Esc açık katmanı kapatır.

- **Mağazalar** — her şube: karne, özet satırı, **mal nereden geliyor** ("Depo: Tekirdağ · 120 km" / "Toptancıdan"; ayrıntı ipucunda, `MarketDepots::DescribeLink`), Kapat (sorar). Müdür satırı: Dükkândan (aile dükkânından kasiyer/görevli), Prim, Uyar, **Değiştir / Müdür al** (3 dış aday kartı, M22), il 3 mağazaya ulaşınca **İl müdürü yap**. Beceri İK müdürü ya da il müdürü yoksa gizli (`MarketManagers::SkillVisible`).
- **Yönetim** — kademe ağacı: en üstte **aile dükkânı** (ikinci mağaza açılana kadar kilitli, M19), her ülke için ülke müdürü (5 ilde mağaza olunca görünür, ikinci ülkede zorunlu, M20), ülkenin **depoları** (müdürsüz = yarı verim), ana bölge → alt bölge → il → mağaza müdürleri. Boş kademede **Ata**: 3 dış aday (beceri İK yoksa aralık, dürüstlük/tarz İK ile görünür, potansiyel ipucu, ücret; adaylar haftada bir yenilenir) ve terfi edebilecek mağaza müdürleri. Atanmış kişide Prim, Uyar, Görevden al. Sağda "Sana doğrudan bağlı" (5 kişi sınırı) ve öneriler.
- **Şirket** — şirket özeti, **Depolar** kartı (üç katman): depolar listesi (menzil 600 km, hizmet verdiği / kapasite 60, verim, müdür; "Haritada" ana ekranda deponun menzil halkasını gösterir, "Müdür ata"), **Depo kur › il seç** (önerilen il üstte, tahmini aylık kazanç, 600 km içindeki mağaza sayısı, kurulum ve kira; kurunca doğrudan depo müdürü adaylarına geçer), depo müdürü adayları. Kamyon sayısı / gereken. Yanında Yatırımlar (kamyon, merkezi satın alma, özel marka, karanlık mağaza).
- **Ana ekran haritası** — depolu illerde iğnenin yanında "D" kutusu; seçili depo ya da açık ilin deposu için 600 km menzil halkası. İl panelinde "En yakın depo" satırı.
- **Şimdi ne yapmalı** — müdürsüz depo, kapasitesini aşan depo, depoya uzak mağazalar (önerilen il ile), aile dükkânına müdür atanabilir (ikinci mağazadan sonra bir hafta).

Komutlar (`MarketDirector::Command`): `AppointCandidate` (EncodeArea × 10 + aday), `ManagerHireFor` / `ManagerReplaceWith` (şube × 10 + aday), `AppointPromote` (şube × 10 + kademe), `BuildDepotIn` (EncodeArea(Depot, ülke, il)), `BonusManager`, `WarnManager`, `DismissManager`.

## Logolar ve görseller

`Content/Brands/<anahtar>/logo.png` varsa rozetin yerine o gösterilir: `bim`, `migros`, `a101`, `sok`, `carrefoursa`, `kipa`, `metro`, `miras`. Yoksa renkli baş harf. Kare, saydam PNG (256×256) önerilir; yayın öncesi kullanım izni kontrol edilmeli.

Ürün resmi `/Game/Products/Items/<id>/T_<id>_Label` dokusundan kesilir (ürünün `MeshPath`'i boşsa baş harf rozeti).

## Veri

- Rakip raf fiyatı `MarketRivals::RivalFactor` × zincirin fiyat düzeyi; müşteri kararı `RivalPriceFactor`.
- Hafta grafiği `FMarketState::History`.
- Harita: `Config/iller.json` (81 il; turkey-map-react sınırlarından sadeleştirilip üçgenlenmiş, MIT). İl başına zincir mağaza sayıları **örnek veridir**; bizim mağazalarımız oyun durumundan gelir (Lüleburgaz = Kırklareli).
- Ulusal/uluslararası pazar: `MarketRetail.cpp` — oyun dünyasının verisi (M24: oyuncuya kaynak satırı, gerçek tarihçe ya da takvim yılı gösterilmez; yıl "N. yıl" olarak yazılır). Araştırma notu: proje belgesi `claude/rakipler_pazar_payi_arastirma.md`.

## Sonraki adımlar

1. Derleme ve oyunda deneme (G-074).
2. İstatistik merkezi (dönem, kapsam, karşılaştırma seçicileri).
3. Harita katmanlarında zincirlerin yıla göre büyümesi (şimdilik sabit örnek sayılar).
