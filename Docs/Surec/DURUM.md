# Güncel durum

Son güncelleme: 06.10.2026 — Codex (G-118). Bu dosya 06.10.2026'da sadeleştirildi. 27.09–06.10 arasındaki bütün eski girişler, doğrulama tablosu ve eski "sıradaki adımlar" listesi `Docs/Surec/Arsiv/DURUM_20261006_oncesi.md` içindedir (tarihçe; güncel yön değildir). Yeni girişleri **"Son girişler" bölümünün en üstüne** ekle ve bu dosyayı kısa tut: bir iş kapanınca girişini 2–3 satıra indir, ayrıntı GUNLUK'te kalır.

## Kısaca: oyun nerede

- **Ne:** Steam'e çıkacak işletme simülasyonu + tycoon (iç kod adı MarketSim, satış adı açık). Kurgu `Docs/Kurgu/00_KURGU_KITABI.md`, kararlar `Docs/Kurgu/01_KARARLAR.md` (son yön M69).
- **Oyunun aklı (Claude):** ekonomi, ilk mağaza günü, şubeler, il/ülke yapısı ve 10 ülke paketi, rakip zincirler ve dünya ligi, personel/yönetim kademeleri, depolar, finans/banka/defter, internet satışı, kampanyalar, hedefler, strateji yolları (D9a) ve mağaza portföyü/kriz yanıtları (D9b), otomatik oyuncu (bot) ve denge raporu. M69 yeni başlangıç (ülke, şehir, market adı, oyuncu adı, zorluk; aile devri; bina bizim; süresiz borç; bölüm/final yok).
- **Dünya ve 3B (Codex):** 4 mağaza türü Blender binaları ve dünya örneklerine göre yerleşimleri (mahalle 20 ekipman/1 kasa, küçük 42/2, büyük 132/6, hiper 400/18), 132 yeni masaüstü ekipman modeli, yeni soğuk dolaplar, el yapımı 8×10 m mahalle marketi prototipi, sanat denemesi sahnesi, mağaza editörü, Ürün Stüdyosu, Raf Planı.
- **Son doğrulama (06.10.2026, G-118/M69c):** MarketSim DERLE, TEST **180/180**, Smoke geçti. Sanat, el yapımı mahalle marketi, üç büyük format ve normal gezi kontrolleri geçti; eski script yolu CoreRedirects ile açıldı. İki yıllık bot denetim hatası 0.
- **Git:** dal `akis-cc2`; M69 `2b47108`, D9b birleşimi `5dc33ea`, iç ad dönüşümü `2d2e495`. D9b kaynak uzlaşması uygulandı, kayıt sürümü 22. Proje `MarketSim.uproject`; runtime/stüdyo modülleri MarketSim/MarketSimStudio. M69, D9b ve M69c cloud/akis-cc2 dalına gönderildi; main değişmedi.

## Açık işler (öncelik sırasıyla)

| # | İş | Sahip | Not |
|---|---|---|---|
| 1 | **G-110 ilk mağazayı kapatma** | Claude (kural) + Codex (3B geçiş) | M69'un kalan parçası. G-109 ve G-118 tamamlandı. |
| 2 | **G-108 denge notları ve yeni bot koşusu** | Claude | D9a + D9b doğrulandı. Bu turdaki iki yıllık bot uzun dönem denge ölçümü değildir. |
| 3 | **Mustafa'nın değerlendirmesi** | Mustafa | Yeni mağaza yerleşimleri, el yapımı mahalle marketi, G-106 yönetim ekranı için dört tasarım yönü. |
| 4 | **Yeni reyonların satılabilir ürünleri** | Claude | Manav ve bazı gıda dışı reyonlar yalnız görüntü; katalog/stok/fiyat karşılığı gerekiyor. |
| 5 | **G-117 kalanı: Unreal şablon klasörleri** | Codex/Mustafa (editörde) | Reference Viewer ile kontrol edip kullanılmayanları editörden sil. Mannequins kullanılıyor, silinmez. |
| 6 | Sonra | — | G-104 öğretici, G-105 mağaza ziyareti modu, G-107 İngilizce, G-088 kalan 16 mağaza görünümü, G-011 dönem etiketleri. |

## Bilinen riskler / dikkat

- Eski POS ekran dokularında ve bazı varlık yollarında (`/Game/Materials/Miras/...`, `M_MirasSurface`, `MH_Teyze`) eski ad duruyor; oyuncuya görünenler M69 varlık cilasında kontrol edilmeli.
- Mağaza modellerinde bütün açılarda titreşimin giderildiği iddia edilmez (G-114/G-115 notları).
- Eski tasarım belgeleri (`Docs/OYUN_TASARIMI.md`, `GELISTIRME_PLANI.md`, `DOGRULAMA.md`, `Docs/Planlama/`, `Docs/Kurgu/02_DERIN_INCELEME.md`, `05_YOL_HARITASI.md`) M69 öncesi kurguyu anlatır; başlarına "ARŞİV" notu kondu. Kurgu için yalnız `00_KURGU_KITABI.md` ve `01_KARARLAR.md` esas alınır.
- World Labs API anahtarı Image-blaster klasörüyle silindi; platformda iptali Mustafa'da.

## Son girişler

**06.10.2026 — Codex / G-118 tamamlandı:** Claude çözümü manifestle doğrulandı; D9b `5dc33ea`, iç ad MarketSim `2d2e495`, kayıt sürümü 22. Her iki aşamada DERLE/180 test/Smoke geçti; bot iki yıl, denetim hatası 0. Yeni ve eski yönlendirilen sınıf yollarıyla sanat/mahalle/büyük mağaza/normal gezi geçti. **Devam:** G-110 ve G-108; ayrıntı GUNLUK.


**06.10.2026 — Codex / G-118:** Belgeler `0b31528`, M69 kodu `2b47108` ile ayrı kaydedildi; son G-117 DERLE/TEST 175/175/Smoke doğrulaması geçerli. D9b mağaza portföyü ve kriz yanıtlarını getiriyor, ancak MarketEconomy.h kayıt sürümü satırı çakıştı (ortak ata 20; iki taraf 21). Birleşim geri alındı; temiz akis-cc2, push/iç ad yok. **Devam:** Claude kaynak uzlaşması; Saved/Claude/d9b/RAPOR.md ve üç dosyanın base/ours/theirs kopyaları hazır.


**06.10.2026 — Claude (Cowork) / belge düzeni:** Codex'in G-117 temizliği okundu. DURUM sadeleştirildi (eskisi `Arsiv/DURUM_20261006_oncesi.md`); G-109 "doğrulandı, commit ve iç ad bekliyor" yapıldı; AGENTS'ta olmayan dosyalar (`MarketRetail`, `MarketCredit`, `Tools/Arsiv/katalog_olustur.py`) ve eski test alt sınırı düzeltildi; M69 öncesi tasarım belgelerine arşiv notu eklendi. Kod değişmedi; derleme gerekmez.

**06.10.2026 — Codex / G-117 proje temizliği:** Image-blaster zinciri, 26 bozuk önbellek klasörü, geçici/arşiv dosyaları kaldırıldı (24.361 dosya, ~1,04 GB). M69 dosyaları korundu, temizlik commit'ine alınmadı. DERLE, TEST 175/175, Smoke geçti. Tam liste GUNLUK'te. **Devam:** şablon klasörleri için editör referans kontrolü (açık iş 7).

**05.10.2026 — Codex / G-113…G-116 (özet):** Image-blaster denemesi reddedildi (G-112, silindi). Sıfırdan Blender mahalle marketi (G-113, Mustafa değerlendirecek); küçük/büyük/hiper Blender binaları ve 132 model (G-114); titreyen eski soğuk dolap yeni kabinlerle değişti (G-115); dört format dünya marketi örneklerine göre yeniden yerleşti (G-116). Rehberler `Docs/Environment/BLENDER_MAHALLE_MARKETI.md`, `BLENDER_BUYUK_MARKETLER.md`, `DUNYA_MARKET_YERLESIMLERI.md`. Eski gezi taslakları `Saved/StoreBackups/` altında.

**05.10.2026 — Codex / G-106 (özet):** Sanat ve yönetim ekranı denemeleri. Önceki yönler reddedildi; 4 yönetim ekranı yönü ve gerçek Unreal sanat denemesi (`SANAT_DENEME.cmd`) Mustafa'nın değerlendirmesinde. Hiçbir yön seçilmedi, menü koduna uygulanmadı.

**05.10.2026 00:38 — CLAUDE_KOS:** M69'lu kod DERLE + TEST + Smoke geçti (G-109 doğrulaması).

**04.10.2026 — Claude Code / D9b (M46, M47):** mağaza portföyü (`MarketPortfolio`: yaşlanma, yıllık karne, yenile/taşı/tür değiştir) ve rakip hamlelerine/krizlere cevap (`MarketResponse`: müdür hiyerarşisi, son onay kartı, Mağazalar › Müdahaleler). GitHub `cloud/akis-cc2` df47d0d; derlenmeden yazıldı, 06.10.2026 M69 ile birleştirildi (G-118).

**04.10.2026 — Claude (Cowork) / G-109 M69:** yeni başlangıç ve eski hikâye temizliği yazıldı; kayıt sürümü 21. Ayrıntı GUNLUK 04.10.2026.

**04.10.2026 — Claude Code / D9a:** M48–M50 (il atağı, yol ayrımları, yollar) yeni yapıya taşındı ve doğrulandı.
