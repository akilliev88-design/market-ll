# Canlılık hedefi — referans mağaza görünümü

Referans: `Docs/Images/Referans/referans_dokme_reyon.png` (Mustafa, 28.09.2026).
Karşılaştırılan oyun görüntüsü: `Saved/Screenshots/MirasMarket.png` (28.09.2026, Blender mağaza kiti sonrası).

Amaç sınırlamak değil: oyun bu fotoğraf kadar **dolu, aydınlık ve dokulu** görünmeli. "Yapay" hissin kaynağı tek bir ayar değil, aşağıdaki sekiz farkın toplamı.

## Fark tablosu

| # | Konu | Referans | Şu an | Hedef (ölçülebilir) |
|---|---|---|---|---|
| 1 | Raf doluluğu | Rafta boş yer yok; yüzlerce ürün, renkli tekrar | 7 ürün; raf alanının çoğu boş, gondollar siyah boşluk | Doluluk kuralı yok (Mustafa, 28.09): elde kaç ürün varsa raf o kadarıyla dolar — önde adedi boş genişliğe yayılır, derinlik raf derinliğince |
| 2 | Parlaklık | Yüksek anahtar ışık, yumuşak gölge; ortalama parlaklık **0,45** | Koyu; ortalama parlaklık **0,18** (HUD dahil) | Aynı ölçümde **0,40–0,50**; siyah tavan yok |
| 3 | Zemin | Açık bej, cilalı; ışıklar ve raflar zeminde yansıyor | Mat, koyu kahve karo | Roughness 0,08–0,15, gerçek yansıma; 60×60 cm açık karo/terrazzo |
| 4 | Ahşap | Koyu ceviz damarlı kaplama, siyah alt süpürgelik | Düz turuncu renk | Albedo + normal + roughness doku seti; Blender'da gerçek ölçekli UV |
| 5 | Dökme reyon | Temiz şeffaf akrilik hazneler, eğik kapak; içinde tane tane gıda (fındık, leblebi, mercimek, kuru meyve) | Buzlu/gürültülü cam, içinde düz renkli küpler | Gürültüsüz saydam materyal; içerik = yükseklik haritalı tane dokusu, en az 8 farklı gıda |
| 6 | Tabela ve etiket | Kırmızı büyük tabela, her hazne/ürün altında beyaz fiyat etiketi | Küçük yazı, fiyat etiketi yok | Her planogram bloğunun altında fiyat etiketi; reyon başına okunur kategori tabelası |
| 7 | Derinlik | Arka planda başka reyonlar ve dolu duvar rafları | Arka plan karanlık ve boş | Kameradan en az 3 dolu reyon katmanı görünür |
| 8 | Arayüz | Yok (fotoğraf) | HUD ekranın ~%45'ini kaplıyor | Oynarken ekranın **≤ %12'si**; ayrıntı paneli tuşla açılır |

## Öncelik (etki / emek)

1. **G-029 HUD sadeleştirme** — en ucuz, en büyük "oyun ekranı" etkisi.
2. ~~G-030 Dekor stoğu~~ — iptal (Mustafa). Yerine **G-034**: 24 sınırları kalktı, raf kapasitesi raf planından gelir, otomatik dolum ve test modu.
3. **G-031 Işık + zemin + tavan** — açık gri tavan, lineer armatür sıraları, sabit pozlama, cilalı açık zemin, yansımalar (Lumen/SSR ayarı kontrol edilecek).
4. **G-021 PBR doku seti** (mevcut görev) — ceviz ahşap, boyalı metal, zemin.
5. **G-032 Dökme reyon içeriği** — saydam akrilik + tane dokulu gıda yüzeyleri.
6. **G-033 Tabela ve fiyat etiketleri** — planogramdan otomatik.

## Kabul yöntemi

Her görevin sonunda aynı kamera noktasından 1280×720 yakalama alınır ve referansla yan yana `Docs/Images/Karsilastirma/<tarih>_<gorev>.png` olarak kaydedilir. Ortalama parlaklık ve doluluk tahmini GUNLUK girişine yazılır. Görsel yineleme ekran görüntüsü gerektirdiği için ışık/materyal işleri kabuğu olan ajana (Codex) daha uygundur; kod ağırlıklı işler (G-029, G-030, G-033) iki ajan da yapabilir.
