# Yönetim menüsü (G-059)

Tasarım: claude.ai "Miras Market Menü Tasarımı" maketi (açık/koyu tema). Kod: `Source/MirasMarket/MarketMenuWidget.*` (Slate arayüz), `MarketMenu.cpp` (aç/kapat, duraklatma, komut yönlendirme).

## Kullanım

| Tuş / düğme | İş |
|---|---|
| **M** | Menüyü her yerden açar. Oyun durur, fare imleci çıkar. |
| M, TAB, Esc | Menüyü kapatır (Esc menü açıkken oyundan çıkmaz). |
| 1–7 | Özet, Sipariş, Ürünler ve fiyat, Rakipler, Personel (G-060: çalışanlar, adaylar, İK, mali müşavir ve vergi; `Docs/PERSONEL_VE_MUHASEBE.md`), Şubeler, Raporlar |
| Kenar çubuğu "Açık tema / Koyu tema" | Tema; `GameUserSettings.ini` → `[MirasMarket.Menu] LightTheme` |

- Gün kapanınca menü **Raporlar** sayfasında açılır (7. günde Hafta sekmesi). "Yeni güne başla" kapatır.
- Menüdeki her düğme masadaki tuşlarla aynı `Command()` kuralını çalıştırır (`AMarketGameMode::MenuCommand`); menüden kararlar için masaya yürümek gerekmez. Masadaki tuşlar da çalışır.
- Smoke (`-MirasSmoke`) ve ekran görüntüsü (`-MirasCapture`) çalıştırmalarında menü kendiliğinden açılmaz; eski 30 saniyelik rapor kartı görünür.

## Logolar

`Content/Brands/<anahtar>/logo.png` dosyası varsa rozetin yerine o gösterilir: `bim`, `migros`, `a101`, `miras` (kendi logomuz). Dosya yoksa renkli baş harf rozeti. Kare, saydam arka planlı PNG önerilir (256×256). Oyun yayınlanacaksa logoların kullanım izni ayrıca kontrol edilmeli.

## Veri

- Fiyat sayfasındaki rakip fiyatları `MarketRivals::RivalFactor` (her rakip kendi kampanyası, boş reyon = "rafta yok"). Müşteri kararı yine `RivalPriceFactor` ile verilir (sayfada "müşterinin aklındaki rakip fiyatı").
- Hafta grafiği `FMarketState::History` (her kapanan gün: ciro, net, satış, kayıp, pay, kasa). İstatistik merkezi bu kayıttan beslenecek.

## Sonraki adımlar

1. HUD'u sadeleştirmek (maketteki A1: gün/saat, kasa, borç, en fazla 3 bildirim).
2. Ürün kartlarında gerçek ürün görseli (Stüdyo önizlemesi).
3. Rakipler: ulusal / uluslararası sekmeleri; Şubeler: il haritası (şube sistemiyle).
4. İstatistik merkezi (dönem, kapsam, karşılaştırma seçicileri).
