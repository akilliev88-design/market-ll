Miras Market projesindesin (Unreal 5.8, C++). Proje kökü: C:\Users\mtass\Desktop\market-ll. Oyunun aklı (Aşama 1) üç ajana bölündü; sen **Akış A**'sın: doğrulama, otomatik oyuncu, zaman. Aynı anda Claude Code (Akış B) ve Claude Cowork (Akış C) başka dosyalarda çalışıyor.

## Önce oku (sırayla, atlamadan)
1. `AGENTS.md` (kurallar, kod kuralları, "bitti" şartı)
2. `Docs/Kurgu/07_AKIL_ISBOLUMU.md` — **bu işin sözleşmesi.** Dosya sahipliği, ortak dosyalardaki işaretli bloklar, kalite kuralları ve senin işinin tam tarifi (§4 Akış A) orada. Bu prompt ile çelişirse o belge geçerlidir.
3. `Docs/Kurgu/06_GIDIS_YOLU.md` (yön: önce akıl, sonra simülasyon; tycoon ilkeleri)
4. `Docs/Surec/DURUM.md` devam notunun en üstü, `GUNLUK.md` son iki giriş
5. İşe başlamadan: `MarketSimulation.h/.cpp`, `MarketDirector.h/.cpp`, `MarketStart.h`, `MarketAutomation.cpp`, `Test.ps1`

## Yapacakların (sırayla)
**A0 — Aşama 0, `main` üzerinde, önce bu.** `DERLE.cmd /q` → `TEST.cmd /q` → `powershell -File SmokeTest.ps1`. Derlenmemiş Claude işleri var (G-086f, G-089/M19–M24, MarketStoreAssign). Bu adımda derleme/test hatalarını **hangi dosyada olursa olsun** en küçük düzeltmeyle düzeltme iznin var (Mustafa verdi); mantığı değiştirme, mantık hatası görürsen not et. Her düzeltmeyi GUNLUK'e dosya:satır ile yaz, DURUM'u güncelle, `Test.ps1` alt sınırını güncel test sayısına çek. Hepsi yeşilse commit (`Aşama 0: ...`) + `git push`. Diğer akışlar bu commit'i bekliyor; bunu hızlı ve temiz bitir.

**Sonra kendi worktree'ne geç:** `git worktree add ..\market-ll-A -b akis-a`, orada `git lfs pull`, ilk `DERLE.cmd /q`. Bundan sonra yalnız orada çalış; `main` klasörüne dokunma (orada Claude Cowork çalışıyor).

**A1 — Otomatik oyuncu ve denge raporu (G-090).** 07 §4 A1'deki tarif: `MarketAutoPlay.*`, üç tarz (temkinli/dengeli/atak), dünyasız, `MarketSimulation::PlayDay` + yalnız oyuncunun da kullandığı komutlar; ölçümler, eşik günleri, tutarlılık denetimleri, istismar şüphesi; `Saved/AutoPlay/<tarih>/rapor.md` (Türkçe, Mustafa okuyacak — teknik terim az, sonuç önce) + csv; kısa otomasyon testi `MirasMarket.AutoPlay.Short` (<60 sn), uzun commandlet + `AUTOPLAY.cmd`.

**A3 — Zaman ve tur.** 07 §4 A3: 1 hafta = 7 gün (#12), 1 ay, `EStop` durma nedenleri + oyuncuya bir cümle, ilerletilen günlerde aile dükkânını yürütenin becerisine göre kusur (#8), hafta/ay özet işlevleri. Menüye dokunma; `AMarketGameMode` tarafını hazırla, menü bağlantısını akış notuna "C'ye istek" olarak yaz.

**Son:** uzun bot koşusu (10 yıl × 3 tarz × 3 tohum), raporu `Docs/Surec/akislar/A_ilk_rapor.md` olarak kopyala.

## Kurallar (kısaca; ayrıntısı 07 §2–§3)
- Yalnız kendi dosyaların: `MarketSimulation.*`, `MarketAutoPlay.*`, commandlet, `MarketAutomation.cpp`, `MarketGame.*`, `Test.ps1`, `SmokeTest.ps1`, `AUTOPLAY.cmd`. `MarketEconomy.h` ve `MarketDirector.cpp`'de yalnız `// ===== Akış A =====` bloğuna ekleme. Menü dosyalarına ve başka akışın dosyalarına dokunma; gerekeni akış notuna yaz.
- Dünyadan bağımsız, belirlenimci (tohumlu) mantık; her yeni kurala test; eski kayıt uyumu; ASCII + `\uXXXX`; C4456–C4459 gölgelemesi yok; unity build için adlı namespace.
- Her alt iş ayrı commit (`A1: ...`), düzenli `git push -u origin akis-a`.
- Akış boyunca DURUM/GOREVLER/GUNLUK/AGENTS'a dokunma (A0 hariç); günlüğün `Docs/Surec/akislar/A.md` (biçim 07 §5).
- Bitti = kendi klasöründe DERLE + TEST (+ smoke) geçti. Derlenmemiş işi bitmiş gösterme.
- Limit yaklaşırsa: `[yarım]` commit + push, A.md'nin başına "Kaldığım yer".

## Bitince bana (Mustafa'ya) kısa Türkçe özet ver
Ne bitti, testler kaç/kaç, botun ilk raporundan en önemli 5 bulgu, C'nin bağlaması gerekenler.
