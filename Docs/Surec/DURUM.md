# Güncel durum

Son güncelleme: 06.10.2026 — Claude (Cowork). Bu dosya 06.10.2026'da sadeleştirildi. 27.09–06.10 arasındaki bütün eski girişler, doğrulama tablosu ve eski "sıradaki adımlar" listesi `Docs/Surec/Arsiv/DURUM_20261006_oncesi.md` içindedir (tarihçe; güncel yön değildir). Yeni girişleri **"Son girişler" bölümünün en üstüne** ekle ve bu dosyayı kısa tut: bir iş kapanınca girişini 2–3 satıra indir, ayrıntı GUNLUK'te kalır.

## Kısaca: oyun nerede

- **Ne:** Steam'e çıkacak işletme simülasyonu + tycoon (iç kod adı MarketSim, satış adı açık). Kurgu `Docs/Kurgu/00_KURGU_KITABI.md`, kararlar `Docs/Kurgu/01_KARARLAR.md` (son yön M69).
- **Oyunun aklı (Claude):** ekonomi, ilk mağaza günü, şubeler, il/ülke yapısı ve 10 ülke paketi, rakip zincirler ve dünya ligi, personel/yönetim kademeleri, depolar, finans/banka/defter, internet satışı, kampanyalar, hedefler, strateji yolları (D9a), otomatik oyuncu (bot) ve denge raporu. M69 yeni başlangıç (ülke, şehir, market adı, oyuncu adı, zorluk; aile devri; bina bizim; süresiz borç; bölüm/final yok).
- **Dünya ve 3B (Codex):** 4 mağaza türü Blender binaları ve dünya örneklerine göre yerleşimleri (mahalle 20 ekipman/1 kasa, küçük 42/2, büyük 132/6, hiper 400/18), 132 yeni masaüstü ekipman modeli, yeni soğuk dolaplar, el yapımı 8×10 m mahalle marketi prototipi, sanat denemesi sahnesi, mağaza editörü, Ürün Stüdyosu, Raf Planı.
- **Son doğrulama (06.10.2026, Codex G-117 sonrası):** DERLE geçti, TEST **175/175**, Smoke geçti. Bu koşu çalışma ağacındaki M69 değişiklikleriyle birlikte yapıldı.
- **Git:** dal `akis-cc2`. M69 değişiklikleri hâlâ **commit edilmedi**; GitHub'daki D9b ile yerel M69 aynı dosyalara dokunduğu için birleştirme bekliyor.

## Açık işler (öncelik sırasıyla)

| # | İş | Sahip | Not |
|---|---|---|---|
| 1 | **M69'u commit et + GitHub D9b ile birleştir** | Mustafa onayı; Codex (git) | M69 doğrulandı (G-109). Birleştirmede çakışan dosyalar D9b ve M69'un ortak dokunduğu Claude dosyaları; çakışma çözümü Claude'a gelir. |
| 2 | **İç ad MirasMarket → MarketSim** (M69c) | Claude hazırladı; Mustafa çalıştırır | Betik `Saved/Claude/m69_ic_ad.py` (git mv + metin + `[CoreRedirects]`). 1. madde bitmeden çalıştırma. |
| 3 | **G-110 ilk mağazayı kapatma** | Claude (kural) + Codex (3B geçiş) | M69'un kalan parçası. |
| 4 | **D9 kalanı:** M46 mağaza portföyü, M47 krizlere/rakip hamlelerine cevap | Claude | Sonra G-108 denge notları ve yeni bot koşusu. |
| 5 | **Mustafa'nın değerlendirmesi** | Mustafa | Yeni mağaza yerleşimleri (`MAGAZA_GEZI.cmd`, `KUCUK_/BUYUK_MARKET_GEZI.cmd`, `HIPERMARKET_GEZI.cmd`), el yapımı mahalle marketi (`MAHALLE_MARKET_GEZI.cmd`), yönetim ekranı için 4 tasarım yönü (G-106, `Docs/Environment/YONETIM_PANELI_ON_TASARIM.md`). |
| 6 | **Yeni reyonların satılabilir ürünleri** | Claude (katalog/stok/fiyat) | Manav meyveleri ve bazı gıda dışı reyonlar şu an yalnız görüntü; katalogda karşılıkları yok. |
| 7 | **G-117 kalanı:** Unreal şablon klasörleri (`Content/ThirdPerson`, `LevelPrototyping`, `__ExternalActors__/ThirdPerson`, `__ExternalObjects__/ThirdPerson`, `Developers`, `Input`) | Codex/Mustafa (editörde) | Kod taraması referans bulmadı; editörde Reference Viewer ile bakıp editörden silinmeli. `Content/Characters/Mannequins` kullanılıyor, silinmez. |
| 8 | Sonra | — | G-104 öğretici, G-105 mağaza ziyareti modu, G-107 İngilizce, G-088 kalan 16 hazır mağaza görünümü, G-011 dönem etiketleri (M69 ile "gerçek yıl" kalktı; klasör adları 2011… yeniden düşünülmeli). |

## Bilinen riskler / dikkat

- Eski POS ekran dokularında ve bazı varlık yollarında (`/Game/Materials/Miras/...`, `M_MirasSurface`, `MH_Teyze`) eski ad duruyor; oyuncuya görünenler M69 varlık cilasında kontrol edilmeli.
- Mağaza modellerinde bütün açılarda titreşimin giderildiği iddia edilmez (G-114/G-115 notları).
- Eski tasarım belgeleri (`Docs/OYUN_TASARIMI.md`, `GELISTIRME_PLANI.md`, `DOGRULAMA.md`, `Docs/Planlama/`, `Docs/Kurgu/02_DERIN_INCELEME.md`, `05_YOL_HARITASI.md`) M69 öncesi kurguyu anlatır; başlarına "ARŞİV" notu kondu. Kurgu için yalnız `00_KURGU_KITABI.md` ve `01_KARARLAR.md` esas alınır.
- World Labs API anahtarı Image-blaster klasörüyle silindi; platformda iptali Mustafa'da.

## Son girişler

**06.10.2026 — Claude (Cowork) / belge düzeni:** Codex'in G-117 temizliği okundu. DURUM sadeleştirildi (eskisi `Arsiv/DURUM_20261006_oncesi.md`); G-109 "doğrulandı, commit ve iç ad bekliyor" yapıldı; AGENTS'ta olmayan dosyalar (`MarketRetail`, `MarketCredit`, `Tools/Arsiv/katalog_olustur.py`) ve eski test alt sınırı düzeltildi; M69 öncesi tasarım belgelerine arşiv notu eklendi. Kod değişmedi; derleme gerekmez.

**06.10.2026 — Codex / G-117 proje temizliği:** Image-blaster zinciri, 26 bozuk önbellek klasörü, geçici/arşiv dosyaları kaldırıldı (24.361 dosya, ~1,04 GB). M69 dosyaları korundu, temizlik commit'ine alınmadı. DERLE, TEST 175/175, Smoke geçti. Tam liste GUNLUK'te. **Devam:** şablon klasörleri için editör referans kontrolü (açık iş 7).

**05.10.2026 — Codex / G-113…G-116 (özet):** Image-blaster denemesi reddedildi (G-112, silindi). Sıfırdan Blender mahalle marketi (G-113, Mustafa değerlendirecek); küçük/büyük/hiper Blender binaları ve 132 model (G-114); titreyen eski soğuk dolap yeni kabinlerle değişti (G-115); dört format dünya marketi örneklerine göre yeniden yerleşti (G-116). Rehberler `Docs/Environment/BLENDER_MAHALLE_MARKETI.md`, `BLENDER_BUYUK_MARKETLER.md`, `DUNYA_MARKET_YERLESIMLERI.md`. Eski gezi taslakları `Saved/StoreBackups/` altında.

**05.10.2026 — Codex / G-106 (özet):** Sanat ve yönetim ekranı denemeleri. Önceki yönler reddedildi; 4 yönetim ekranı yönü ve gerçek Unreal sanat denemesi (`SANAT_DENEME.cmd`) Mustafa'nın değerlendirmesinde. Hiçbir yön seçilmedi, menü koduna uygulanmadı.

**05.10.2026 00:38 — CLAUDE_KOS:** M69'lu kod DERLE + TEST + Smoke geçti (G-109 doğrulaması).

**04.10.2026 — Claude (Cowork) / G-109 M69:** yeni başlangıç ve eski hikâye temizliği yazıldı; kayıt sürümü 21. Ayrıntı GUNLUK 04.10.2026.

**04.10.2026 — Claude Code / D9a:** M48–M50 (il atağı, yol ayrımları, yollar) yeni yapıya taşındı ve doğrulandı.
