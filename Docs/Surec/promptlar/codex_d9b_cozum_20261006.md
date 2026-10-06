# Codex iş emri — G-118 devamı: D9b birleşimini Claude'un çözümüyle tamamla (06.10.2026)

Hazırlayan: Claude (Cowork). Onay: Mustafa. Önceki iş emri: `codex_m69_birlesim_20261006.md` (Aşama 1 bitti; Aşama 2 `MarketEconomy.h` çakışmasında doğru biçimde durdu).

## Claude ne yaptı

`cloud/akis-cc2` (df47d0d, D9b) ile yerel M69'u (2b47108 + belge commit'leri) kendi kopyasında birleştirdi ve D9b'yi M69'a uydurdu. Çözülmüş **27 dosyanın tam hâli** `Saved/Claude/d9b/cozum/` altında, proje içindeki yollarıyla. `manifest.json` her dosya için iki git blob kimliği tutar: `once_blob` (çözümün dayandığı yerel HEAD hâli), `cozum_blob` (çözüm).

Çözümde değişenler (D9b'nin kendisinin dışında):
- `MarketEconomy.h`: kayıt sürümü **22** (M69 ve D9b ayrı ayrı 21 yapmıştı), iki açıklama birlikte.
- `MarketPortfolio.cpp`: tür değiştirme M69'da kaldırılan `Kind.Chapter` / `MarketCompany::ChapterOpen` / `MarketStory::ChapterTitle` yerine `MarketBranches::FormatOpen` (8 mağaza + 2 il) kullanır. **Bu olmadan derleme kırılırdı.**
- "family shop" açıklama/test metni → "first store".
- `Test.ps1` alt sınırı **180** (175 + D9b'nin 5 yeni testi).
- Belgeler: GUNLUK'e D9b girişi tarih sırasına eklendi, DURUM'a kısa D9b satırı, GOREVLER D9 satırı, KARARLAR M46'da "bölüm"/"aile dükkânı" M69'a göre düzeltildi.

Claude derleyemedi; D9b'nin yeni dosyalarındaki bütün `Ad::Fonksiyon` ve alan adlarını mevcut başlıklara karşı taradı, eksik yalnız yukarıdaki bölüm kilidiydi.

## Adımlar

1. `akis-cc2` temiz olmalı (`git status`). `git fetch cloud`.
2. **Dayanak kontrolü:** `manifest.json`'daki her yol için `git rev-parse HEAD:<yol>` = `once_blob` olmalı (`once_blob` null olanlar D9b'nin yeni dosyalarıdır, HEAD'de yoktur). Farklı olan varsa `git diff` ile bak: yalnız satır sonu farkıysa devam; içerik farkıysa (Claude'dan sonra biri değiştirmiş) **dur**, RAPOR'a yaz.
3. `git switch -c birlesim-d9b`, `git merge --no-ff --no-commit cloud/akis-cc2` (üç çakışma beklenir: DURUM, GUNLUK, MarketEconomy.h).
4. `Saved/Claude/d9b/cozum/` altındaki 27 dosyayı (manifest hariç) aynı yollara kopyala, hepsini `git add`.
5. Kontrol: `git diff --cached --name-only --diff-filter=U` boş; her yol için `git rev-parse :<yol>` = `cozum_blob`. Uymayan varsa dur.
6. `git commit -m "D9b birlesimi: M46 portfoy + M47 cevaplar, M69 ile uyumlu (kayit surumu 22)"`.
7. DERLE → TEST (en az **180**; yeni testler `MirasMarket.Portfolio.*` ve `MirasMarket.Response.*`) → Smoke.
8. Ek kısa bot denemesi (D9b botta yenileme/taşıma/cevap kararları veriyor): `AUTOPLAY.cmd -Years=2 -Seeds=1 -Style=1`. Hata/çökme olmamalı; rapordaki "Portföy ve müdahaleler (D9b)" satırını GUNLUK'e kopyala. Uzun sürerse (30 dk+) durdurup not düş; bu adım engelleyici değil.
9. Hepsi geçerse: `git switch akis-cc2`, `git merge --ff-only birlesim-d9b`, `git push cloud akis-cc2`, `git branch -d birlesim-d9b`.
10. Sonra önceki iş emrinin **Aşama 3**'ü (iç ad MarketSim) aynen.

## Hata çıkarsa

- **Derleme hatası, Claude dosyasında:** yalnız açıkça derleme düzeyindeki hataları (eksik `#include`, yazım hatası, `const` uyumsuzluğu, yanlış tür dönüşümü) davranışı değiştirmeden düzelt ve her birini GUNLUK'e dosya:satır ile yaz. Mantık/kural gerektiren bir hata ise düzeltme: birleşim commit'i `birlesim-d9b` dalında kalsın, push yok, `Saved/Claude/d9b/RAPOR2.md` (hata satırları) yaz ve dur.
- **Test hatası:** testi ya da eşiği gevşetme; başarısız test adlarını ve mesajlarını RAPOR2'ye yaz, dur.
- Her durumda `akis-cc2` temiz ve son push'lanan hâliyle kalsın.

## Teslim

GOREVLER: G-109 Bitti, D9 (D9a + D9b) durumu, G-118. GUNLUK + DURUM: birleşim commit'i, DERLE/TEST/Smoke, bot özeti, yaptığın derleme düzeltmeleri, Aşama 3 sonucu.
