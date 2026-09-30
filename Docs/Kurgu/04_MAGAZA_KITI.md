# Mağaza kiti — hazır mağaza görünümleri (G-088)

Tarih: 30.09.2026 · Karar: M17 (`01_KARARLAR.md`) · Sahip: **Codex** (görünüm, varlık, kurucu kod), **Claude** (oyun hesabı, atama, kayıt, menü)

## 1. Amaç

Oyuncu bir ildeki mağazasını gezmek istediğinde, o mağazanın **türüne** uygun hazır bir mağaza görünümü yüklenir ve birinci şahısla gezilir.

- Dört tür: `kucuk` (ucuzcu), `mahalle`, `buyuk` (süpermarket), `hiper` (hipermarket). Kimlikler koddaki `MarketBranches::FormatIds()` ile aynıdır, değiştirilmez.
- Her türe **5 hazır mağaza**, toplam 20. Sayı ileride artabilir; kod sayıya bağlı olmamalı.
- **Atama:** bir ilde her türün tek bir gezilebilir mağazası vardır. Görünüm `(ülke, il, tür)` üçlüsünden tohumla seçilir ve kayda yazılır. Aynı ilde aynı türden birden çok şube olsa da hepsi o tek görünümle gezilir. Farklı illere aynı görünüm düşebilir, bu kabul edilmiştir.
- Aile dükkânı (başlangıç ili, `mahalle`) bugünkü elle dizilen dükkân olarak kalır; bu kitin parçası değildir.

## 2. Bir mağaza üç katmandan kurulur

| Katman | İçerik | Kim üretir |
|---|---|---|
| **Kabuk** | Zemin, duvarlar, tavan, cephe/vitrin, giriş kapısı, arka kapı (mal kabul), depo odası, ışık düzeni | Codex (Blender + Unreal) |
| **Yerleşim** | Ekipmanların (gondol, duvar rafı, soğutucu, dondurucu, manav, kasa…) yeri, yönü ve reyon kategorisi | Codex (`Config/magazalar.json`) |
| **Tema** | Zemin/duvar malzemesi, ışık tonu, tabela rengi | Codex (tema kimliği + malzeme örnekleri) |

Ürünler rafa **elle konmaz**. Mağaza yüklenince raflar `MarketLayout::Plan` ile otomatik dizilir (müşteri yolu, talep, marj, marka bloğu kuralları).

## 3. Türlere göre ölçü bantları

| Tür | Satış alanı | Kasa | Raf (m, ön yüz) | Soğutucu/dondurucu (m) | Zorunlu bölümler | Karakter |
|---|---|---|---|---|---|---|
| `kucuk` ucuzcu | 250–400 m² | 2–3 | 60–110 | 8–16 | kuru gıda, içecek, temizlik, süt soğutucusu | Dar çeşit, palet/koli üstü teşhir, sade ve aydınlık, az dekor |
| `mahalle` | 80–200 m² | 1–2 | 25–60 | 4–10 | kuru gıda, içecek, süt, küçük manav, sigara/kasa arkası | Sıcak, sıkışık, tanıdık; kasa arkası rafı |
| `buyuk` süpermarket | 600–1.500 m² | 4–8 | 180–400 | 25–60 | yukarıdakiler + geniş manav, şarküteri tezgâhı, fırın/ekmek, dondurucu koridoru, kişisel bakım | Geniş koridor, kategori tabelaları, reyon adaları |
| `hiper` hipermarket | 3.000–6.000 m² | 15–30 | 700–1.500 | 100–200 | yukarıdakiler + kasap, balık, ev/elektronik/tekstil alanı (dekor olabilir), müşteri hizmetleri | Çok uzun koridorlar, yüksek tavan, palet raflar |

Aynı türün 5 mağazası birbirinden belirgin farklı olsun: bina biçimi (dar-uzun, köşe, L, kare), giriş yönü, kasa hattının yeri, tema. Bant içinde küçükten büyüğe dağılsın (ör. mahalle: 85, 110, 140, 170, 200 m²).

## 4. Sözleşme dosyası: `Config/magazalar.json`

Codex yazar ve günceller; Claude yalnız okur. Uzunluklar santimetre, açılar derece, eksenler Unreal'dakiyle aynı (giriş `-Y` yönünde, zemin `Z=0`, orijin satış alanının ortası).

```json
{
  "schemaVersion": 1,
  "stores": [
    {
      "id": "mahalle_01",
      "format": "mahalle",
      "name": "Köşe dükkânı",
      "theme": "sicak_ahsap",
      "shell": "/Game/Stores/Shells/SM_Shell_Mahalle01",
      "footprintCm": [1400, 900],
      "salesAreaM2": 110,
      "backroomM2": 18,
      "ceilingCm": 290,
      "points": {
        "entrance":  {"at": [0, -470, 0], "yaw": 0},
        "receiving": {"at": [650, 420, 0], "yaw": 180},
        "playerStart": {"at": [0, -380, 0], "yaw": 0},
        "customerSpawn": [[0, -600, 0]],
        "backroom": {"min": [450, 250, 0], "max": [690, 440, 0]}
      },
      "fixtures": [
        {"id": "gondol_1", "equipment": "gondola_double_1200", "category": "kuru gıda", "at": [-200, 0, 0], "yaw": 0},
        {"id": "duvar_1", "equipment": "wall_shelf_2400", "category": "içecek", "at": [-640, 100, 0], "yaw": 90},
        {"id": "kasa_1", "equipment": "checkout_single", "category": "", "at": [300, -380, 0], "yaw": 0}
      ],
      "props": [
        {"mesh": "/Game/Stores/Props/SM_BasketStack", "at": [120, -430, 0], "yaw": 0}
      ],
      "stats": {
        "shelfFrontM": 38.4,
        "coolerM": 6.0,
        "freezerM": 0,
        "produceM2": 3.5,
        "counters": [],
        "checkouts": 1,
        "selfCheckouts": 0,
        "backroomPallets": 6
      }
    }
  ]
}
```

Kurallar:

- `id` = `<format>_<iki hane>`; yayımlandıktan sonra **değişmez** (kayıtlar bu kimliği tutar).
- `fixtures[].id` mağaza içinde benzersiz; `MarketCatalog::IsValidId` kuralına uyar. `category` ürün kategorisiyle (`products.json`) harfi harfine aynı, **Türkçe harflerle** yazılır (`içecek`, `kuru gıda`, `süt`); dosya UTF-8. Reyon tabelası, otomatik dizim ve reyon görevlileri bunu kullanır. Bu yalnız başlangıç değeridir; oyuncu oyunda değiştirebilir (§8); kasa, masa gibi ürün almayan ekipmanda boş.
- `equipment` mutlaka `Planogram` tarafından tanınan bir ekipman kimliği olur. Yeni ekipman eklenirse `MarketPlanogram::Equipment()` onu `equipment.json`'dan okur (sabit kodlama yok).
- `stats` **elle yazılmaz**, doğrulama aracı `fixtures` ve ekipman verisinden hesaplar. Oyun hesabı bu sayıları kullanır, o yüzden görünümle birebir tutarlı olmalı.
- Yeni alanlar isteğe bağlı eklenir; `schemaVersion` yalnız geriye uyumsuz değişiklikte artar.

## 5. Kod sınırı

**Codex yazar** (yeni dosyalar):

- `Source/MirasMarket/MarketStoreKit.h/.cpp` + `MarketStoreKitTests.cpp`
  - `MarketStoreKit::Load()`: `magazalar.json` okur, doğrular (bantlar, zorunlu bölümler, benzersiz id, ekipman bilinir mi).
  - `MarketStoreKit::TemplatesFor(Format)`: o türün mağaza kimlikleri, sıralı.
  - `MarketStoreKit::Find(Id)`: tek mağaza verisi (`FStoreTemplate`).
  - `MarketStoreKit::ToPlanogram(Template)`: mağazanın ekipmanlarını `FMarketPlanogram` olarak verir (ürünsüz).
  - `MarketStoreKit::Build(UWorld*, const FStoreTemplate&, const FMarketPlanogram& Filled)`: kabuğu, ekipmanları, ürünleri (instanced), dekoru ve ışığı dünyaya kurar; `Clear()` kaldırır.
- `Planogram.*` içinde yalnız yeni ekipman tanımları (ölçüler `equipment.json`'dan).
- `Tools/Blender/create_store_*.py`, `Tools/validate_stores.py`, `AssetInbox/Environment/Stores/`, `Content/Stores/`.
- `MarketAutomation.cpp` içine `-MirasStorePreview=<id>` (mağazayı kur, otomatik diz, 5 açıdan 1280×720 görüntü al).

**Claude yazar** (Codex dokunmaz):

- Atama ve kayıt: `FMarketState` içinde il+tür → mağaza kimliği; tohumlu seçim; eski kayıtlar.
- `MarketBranches`, `MarketCompany`, `MarketLayout::Fixtures` (magazalar.json'a bağlanır), `MarketMenu*` ("Mağazayı gez" düğmesi), ekonomi ve simülasyon.
- `stats` sayılarının oyun hesabına bağlanması: çeşit kapasitesi (raf metresi), kuyruk (kasa), tazelik (soğutucu/manav), depo, açılış bedeli ve kira.

Codex, Claude'un dosyalarında bir değişiklik gerektiğini görürse yapmaz; `GOREVLER.md` G-088 notuna ve `GUNLUK.md`'ye yazar.

## 6. Performans ve görsel kalite

- Hedef: mevcut bilgisayarda 1080p'de 60 fps; hipermarkette en az 45 fps.
- Ürünler ve tekrarlı ekipman instanced (HISM) çizilir. Kabuk tek parça ya da az parça; Nanite uygunsa açık.
- Işık: mevcut Lumen ayarı; her tema bir ışık önayarı (sıcak / aydınlık / soğuk-beyaz).
- Görsel dil mevcut dükkânla aynı ailede (Blender standardı, PBR, logo uydurma yok; tabela yazıları kurgu zincir adıyla değil, reyon adıyla).

## 7. Raf kategorisi seçimi ve Türkçe tabela (karar M18)

Oyuncu her mağazada (aile dükkânı dahil) rafın üstündeki kategori tabelasına bakıp o rafın kategorisini değiştirebilir.

**Oynanış**
- Oyuncu bir rafın tabelasına (ya da raf dizme modunda rafa) nişan alınca ipucu çıkar: "Kategoriyi değiştir [T]".
- T ile küçük bir seçim listesi açılır: `products.json`'daki bütün kategoriler (Türkçe adlarıyla, alfabetik), en üstte şimdiki kategori işaretli, en altta "Kategorisiz". Fare tekerleği / oklar ile seçilir, E ya da tık ile onaylanır, Esc kapatır.
- Çift yüzlü gondolda her yüzün kategorisi ayrı seçilebilir (ön ve arka yüz farklı reyon olabilir); tek yüzlü duvar rafında tek kategori.
- Değişince: tabela hemen yeni adı gösterir; reyon görevlileri o raf için yeni kategoriye göre çalışır (`StaffPlanner` zaten `Fixture.Category` okuyor); rafta duran eski kategorideki ürünler **kendiliğinden kaldırılmaz** (oyuncunun blokları taşınmaz/silinmez kuralı), ama rafın üstünde "3 ürün bu reyona ait değil" uyarısı görünür. "Kategorisiz" rafa görevli hiçbir şey dizmez.
- Kasa, masa gibi ürün almayan ekipmanda seçim çıkmaz.

**Kayıt**
- Aile dükkânı: seçim planogramın `Fixture.Category`/`Label` alanına yazılır; planogram zaten kampanya kaydında (G-078 #5).
- Şubelerin gezilebilir mağazası: seçim (ülke, il, tür) mağazasına özel kayda yazılır (`FMarketState`, Claude) ve o mağaza otomatik dizilirken kullanılır. Codex bunun için `MarketStoreKit` içinde "fikstür id → kategori" eşlemesi alan bir giriş bırakır; kaydı Claude bağlar.

**Türkçe tabela**
- Bugün tabelalar `FoldTurkish` ile ASCII'ye çevriliyor (`MarketGame.cpp` ~436), çünkü 3B yazı (`UTextRenderComponent`) motorun varsayılan yazı tipinde Türkçe harf taşımıyor. Bu kalkar.
- 3B yazı için Türkçe harfli bir yazı tipi varlığı: `Content/Slate/Fonts` içindeki IBM Plex Sans SemiBold'dan önbellekli (offline) `UFont`; karakter seti en az A–Z, a–z, 0–9, `ÇĞİÖŞÜçğıöşü`, `₺ % . , : - / ×` ve boşluk. Editör Python betiğiyle üretilir, elle `.uasset` düzenlenmez.
- Büyük harf Türkçe kuralıyla: `i → İ`, `ı → I` (`içecek` → `İÇECEK`, `kuru gıda` → `KURU GIDA`). `FString::ToUpper` bunu yanlış yapar; ortak bir `MarketCatalog::UpperTurkish` yardımcısı yazılır ve testlenir.
- Aynı yazı tipi fiyat etiketlerine ve rafta görünen öteki 3B yazılara da uygulanır.

## 8. Teslim sırası

1. **Aşama A:** sözleşme okuyucu + kurucu + doğrulama aracı + her türden **bir** mağaza (4 mağaza). Mustafa önizleme görüntülerine bakıp onaylar.
2. **Aşama B:** kalan 16 mağaza.
3. **Aşama C (Claude):** atama, kayıt, menüden gezme, `stats` → oyun hesabı.
