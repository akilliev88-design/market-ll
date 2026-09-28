# AGENTS.md — Miras Market ortak çalışma kuralları

Bu projeyi üç taraf birlikte geliştirir: **Mustafa** (ürün sahibi, son karar), **Claude** ve **Codex**. Bir ajanın limiti bitince diğeri kaldığı yerden devam eder. Bu dosya ikisinin de ilk okuduğu yerdir; kuralları burada tut, ayrıntıyı bağlantılı belgelere koy.

## 1. Oturum başında (zorunlu, 2 dakika)

1. `Docs/Surec/DURUM.md` oku: şu an ne çalışıyor, ne yarım, sıradaki adım.
2. `Docs/Surec/GOREVLER.md` içinde **Devam ediyor** satırlarına bak. Başka ajanın üstündeki işi, DURUM'daki devam notu izin vermiyorsa ele alma.
3. `Docs/Surec/GUNLUK.md` son iki girişini oku.
4. Git kuruluysa `git status` ve `git log -5 --oneline`. Beklenmeyen değişiklik varsa silme; önce Mustafa'ya sor.

## 2. Oturum sonunda veya limit yaklaşınca (zorunlu)

1. `Docs/Surec/DURUM.md` dosyasını güncelle. Yarım iş varsa **Devam notu** bölümüne dosya adı, adım ve derleme/test durumunu yaz.
2. `Docs/Surec/GUNLUK.md` dosyasının **en üstüne** bir giriş ekle: tarih, ajan, yapılanlar, doğrulama, sıradaki adım.
3. `Docs/Surec/GOREVLER.md` içindeki durumları güncelle.
4. Git varsa anlamlı bir commit at. Mesaj Türkçe olabilir, ilk satır kısa olsun.

Limit aniden biterse sonraki ajan GUNLUK/DURUM ile dosyaların son hâlini karşılaştırarak eksiği tamamlar.

## 3. Proje haritası

| Yol | İçerik |
|---|---|
| `Source/MirasMarket/MarketEconomy.*` | Para, stok, sipariş, satış, gün sonu, kayıt uzlaştırma. Dünyadan bağımsız ve test edilebilir. |
| `Source/MirasMarket/ProductCatalog.*` | `Config/products.json` okuma/yazma, kutu atlas yerleşimi (`FBoxPackageLayout`). Oyun ve stüdyo ortak kullanır. |
| `Source/MirasMarket/MarketGame.*` | Prototip GameMode, karakter, sahne, müşteri, HUD. Tek dosyayı daha fazla büyütme; yeni sistemleri ayrı sınıflara ayır. |
| `Source/MirasMarket/MarketAutomation.cpp` | `-MirasSmoke` ve `-MirasCapture` otomatik oynanış/görüntü çalıştırmaları (`AMarketGameMode::TickAutomation`). |
| `Source/MirasMarket/MarketArrange.cpp` | Oyunda raf dizme modu (R): nişan, hayalet önizleme, RAF DÜZENİ paneli verisi. |
| `Source/MirasMarket/Planogram.*` | Raf planı verisi (`Config/planograms.json`, şema v3), ekipman ölçüleri, genişlik/derinlik/istif hesapları. |
| `Source/MirasMarket/StaffPlanner.*` | Reyon görevlisinin kararları (dünyadan bağımsız, test edilir): raf doldur, rafta olmayan ürünü kategorisinin reyonuna koy, bir koli almayan bloğu genişlet. |
| `Source/MirasMarket/MarketWorkers.cpp` | Oyundaki reyon görevlileri: yürüme, koli taşıma, rafa tek tek dizme (`AMarketGameMode::TickWorkers`). |
| `Source/MirasMarket/PlanogramEdit.*` | Blok blok elle dizme kuralları (`PlanBlock`, `AddBlock`, `AddToRowEnd`, taşı/kaldır/önde/yön/istif/aralık). Oyun ve editör ortak kullanır; reyon görevlileri de (`StaffPlanner`) bunu kullanır. |
| `Source/MirasMarket/MarketTests.cpp` | Unreal otomasyon testleri (`MirasMarket.*`). |
| `Source/MirasMarketStudio/` | Yalnızca editörde çalışan **Ürün Stüdyosu** ve **Raf Planı** modülü. Arayüz: `SProductStudio`, `SPlanogramStudio`, `SStudioViewport`, `StudioStyle`. Arka uç (`StudioBackend.h`) konulara bölünmüştür: `StudioBackend.cpp` (dosya/görsel/açılım), `StudioMeshes.cpp` (malzeme, kutu/şekil/model 3B), `StudioPresets.cpp` (hazır ambalajlar, parça renkleri), `StudioProducts.cpp` (kontrol, yayımla), `StudioPrompts.cpp` (promptlar, teslim klasörü). Ortak iç yardımcılar `StudioBackendInternal.h`. |
| `Config/products.json` | Ürün kataloğu, şema v2. Stüdyo yazar; elle de düzenlenebilir. |
| `Content/Products/` | Stüdyonun ürettiği varlıklar: `Packages/<ambalaj>/SM_*`, `Items/<ürün>/T_*_Label`, `MI_*`, `Materials/M_ProductLabel`. Elle düzenleme. |
| `AssetInbox/` | Kullanıcının getirdiği ham dosyaların kopyası ve manifestler. Stüdyo yazar; kaynak arşivi olarak saklanır. |
| `Docs/Planlama/` | v0.2 tasarım paketi (hedef oyun). |
| `Docs/Surec/` | Devir belgeleri: DURUM, GOREVLER, GUNLUK. |
| `Docs/URUN_STUDYOSU.md` | Stüdyonun kullanım kılavuzu ve dosya standardı. |
| `Docs/Uretim/` | Mustafa'nın dış ajanlara vereceği **kısa, kopyala-yapıştır promptlar** (A1/A2 kutu, B1/B2 şişe, C1/C2 poşet, D araştırma, E kontrol) ve marka/ürün listesi. Stüdyo davranışı değişirse bu promptları da güncelle. |
| `Uretim/` | Dış ajanların teslim klasörü: `Uretim/<ürün>/model/`, `Uretim/<ürün>/<yıl>/`. |
| `Docs/Uretim/Sablonlar/` | Stüdyonun ürün verisiyle doldurduğu prompt şablonları (`{{YER_TUTUCU}}`). Yeni yer tutucu eklersen `StudioPrompts.cpp` → `BuildPrompt` içine de ekle. |
| `Config/ambalajlar.json` | Hazır ambalaj kütüphanesi (stüdyo 3B şekli kendisi üretir). Ölçü değişirse id de değişir. |
| `Tools/Arsiv/katalog_olustur.py` | Arşiv. Katalogu ürün listesinden sıfırdan üretirdi; stüdyodaki değişiklikleri ve ambalaj atamalarını SİLER. Kullanma. |

## 4. Komutlar (Windows, proje kökü)

| Komut | Ne yapar |
|---|---|
| `TEST.cmd /q` | `Test.ps1`'i çalıştırır; çıktı `Saved/Logs/TEST_son.log`. |
| `DERLE.cmd /q` | Editor hedefini derler. Çıktı: `Saved/Logs/DERLE_son.log`. **Unreal Editor açıkken derleme yapma.** |
| `powershell -File Test.ps1` | Otomasyon testleri; rapor `Saved/TestReports/index.json`. En az 7 test başarılı olmalı. |
| `powershell -File SmokeTest.ps1` | Gerçek oyun dünyasında otomatik oynanış kontrolü. |
| `OYNA.cmd` | Oyunu pencerede açar. |
| `STUDYO.cmd` | Editörü açar ve Ürün Stüdyosu'nu getirir (Tools > Ürün Stüdyosu ile de açılır). |
| `python Tools/escape_unicode.py` | C++ kaynaklarındaki Türkçe karakterleri `\uXXXX` kaçışına çevirir. |

Motor yolu: `C:\Program Files\Epic Games\UE_5.8` (UE 5.8.3). Farklıysa betiklere `-EngineRoot` ver.

## 5. Kod kuralları

- **Para** her yerde `int64` kuruş. JSON'da TL ve iki ondalık.
- **Ürün kimliği (`id`) yayımlandıktan sonra değiştirilmez.** Kayıtlar stoğu id ile eşleştirir; id değişirse o ürünün stoğu sıfırlanır.
- `products.json` şema değişikliği geriye uyumlu olmalı (yeni alanlar isteğe bağlı). Şemayı değiştiren, `MarketCatalog::Parse/Serialize`, testleri ve `Docs/URUN_STUDYOSU.md` dosyasını birlikte günceller.
- C++ kaynakları ASCII kalır. Türkçe metni yaz, sonra `python Tools/escape_unicode.py` çalıştır. Oyun HUD metinleri şimdilik ASCII Türkçedir.
- `.uasset` dosyalarını metin gibi düzenleme. Varlıkları stüdyo veya editör Python betikleri üretir.
- Yeni oyun kuralı = yeni test. Para/stok korunumu, iki kez uygulama ve kayıt uyumluluğu testleri öncelikli.
- Kutu ambalajında yüz ↔ UV eşlemesi yalnızca `FBoxPackageLayout` üzerinden yapılır; başka yerde koordinat kopyalama.

## 6. "Bitti" demenin şartı

Değişiklik ancak şunlardan sonra bitmiş sayılır: `DERLE.cmd` başarılı, `Test.ps1` başarılı. Oyun akışına dokunduysan `SmokeTest.ps1` de başarılı olmalı. Derleyemediysen DURUM'a **"derlenmedi"** yaz; derlenmemiş işi tamamlanmış gibi gösterme.

## 7. Ajanlara özel notlar

- **Codex:** Aynı klasörde yerel çalışır. Kabuk erişimin varsa derleme ve testleri sen çalıştır, sonuçları GUNLUK'e yaz.
- **Claude (Cowork):** Klasöre köprü üzerinden erişir. Kabuk yoksa dosyaları hazırlar, derlemeyi Mustafa'dan (`DERLE.cmd`) ister, logu `Saved/Logs/DERLE_son.log` dosyasından okur. **Dosya yazdıktan sonra geri okuyup karşılaştır:** 27.09.2026'da bir aktarım iki dosyayı eski hâliyle yazdı ve derleme bu yüzden kırıldı.
- Mustafa'ya teknik terimi az, sonucu net anlat. Bir karar onun tercihine bağlıysa sor. Varsayım yaptıysan GUNLUK'e yaz.
