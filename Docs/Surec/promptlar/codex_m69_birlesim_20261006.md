# Codex iş emri — M69 kaydı, D9b birleştirmesi ve iç ad (06.10.2026)

Hazırlayan: Claude (Cowork). Onay: Mustafa (06.10.2026, "iş emrini verebilirsin"). Görev: **G-118** (GOREVLER'e ekle).

Üç aşama, sırayla. Her aşama kendi commit'i. Bir aşama başarısız olursa **sonrakine geçme**, aşağıdaki "Durunca" kuralını uygula.

## Aşama 0 — Ön kontrol (değişiklik yok)

1. `git status`, `git branch -vv`, `git remote -v`, `git fetch --all`.
2. Dal `akis-cc2`, uzak takip `cloud/akis-cc2` olmalı. Yerel ile uzak arasındaki ayrımı yaz: `git log --oneline cloud/akis-cc2..akis-cc2` ve `git log --oneline akis-cc2..cloud/akis-cc2`. 04.10.2026'da uzakta yerelde olmayan D9b commit'i vardı; hangi commit(ler) olduğunu ve `git diff --stat` ile dokunduğu dosyaları raporla.
3. `main` (origin ve cloud) ile `akis-cc2` ilişkisini de yaz; **main'e bu iş emrinde dokunma.**
4. Commit edilmemiş değişiklikler şunlardan oluşmalı:
   - **M69 kodu ve testleri** (Claude, 04.10.2026; liste GUNLUK 04.10 "G-109 M69" girişinde): `Source/`, `Config/ulkeler.json`, `Config/zincirler.json`, `Config/DefaultGame.ini`, `Test.ps1` vb.
   - **Belgeler** (Claude, 06.10.2026): `AGENTS.md`, `README.md`, `Docs/Surec/DURUM.md`, `GOREVLER.md`, `GUNLUK.md`, yeni `Docs/Surec/Arsiv/DURUM_20261006_oncesi.md`, ARŞİV notu eklenen 15 belge (`Docs/OYUN_TASARIMI.md`, `GELISTIRME_PLANI.md`, `DOGRULAMA.md`, `Docs/Planlama/*`, `Docs/Kurgu/02_DERIN_INCELEME.md`, `05_YOL_HARITASI.md`), `Docs/Kurgu/00_KURGU_KITABI.md`, `01_KARARLAR.md`, `Docs/Surec/promptlar/` içindeki yeni iş emirleri.
   Bunların dışında beklenmeyen bir değişiklik varsa dur ve raporla.

## Aşama 1 — M69'u kaydet

1. Belge değişikliklerini ayrı commit: `Belgeler: DURUM sadelesti, M69 durumu, eski tasarim belgeleri arsiv notu`.
2. Kalan M69 kod/ayar/test değişiklikleri: `git commit -F Saved/Claude/m69a_mesaj.txt`.
3. Commit'ten önce son doğrulama zaten var (G-117: DERLE, TEST 175/175, Smoke bu çalışma ağacıyla geçti). Arada bir şey değiştiyse yeniden DERLE/TEST/Smoke.
4. Henüz push yok.

## Aşama 2 — D9b ile birleştir

1. `git switch -c birlesim-d9b` (akis-cc2'den), sonra `git merge cloud/akis-cc2`.
2. **Çakışma yalnız `Docs/Surec/` (DURUM/GOREVLER/GUNLUK) ise** sen çöz: bizim hâlimizi koru, D9b'nin girişlerini kaybetme (GUNLUK'te tarih sırasıyla ekle, DURUM'da "Son girişler"e 2–3 satır özet, GOREVLER'de D9 satırını D9b bilgisiyle güncelle).
3. **Çakışma `Source/`, `Config/`, test ya da `Test.ps1` içindeyse kendin çözme** (Claude'un alanı): `git merge --abort`, sonra "Durunca" kuralı.
4. Temiz birleşirse: DERLE → TEST (en az 174) → Smoke. Dikkat: D9b, M69'un kaldırdığı adları kullanıyor olabilir (`FamilyShop`, `ESupplier::Family`, `Story.Chapter`, `EStop::Chapter`, `RentToday`, `LeadershipDays`, borç kilidi/hedefi, "aile dükkânı" metinleri). Derleme ya da test bu yüzden kırılırsa Claude dosyalarını düzeltme; birleştirme commit'i `birlesim-d9b` dalında kalsın, push etme, "Durunca" kuralı. Kayıt sürümü (`FMarketState::CurrentVersion`) iki tarafta da arttıysa bunu raporla.
5. Hepsi geçerse: `git switch akis-cc2`, `git merge --ff-only birlesim-d9b`, `git push cloud akis-cc2`, `git branch -d birlesim-d9b`.

## Aşama 3 — İç ad MirasMarket → MarketSim (M69c)

Yalnız Aşama 2 push edildiyse.

1. Unreal Editor kapalı olsun. `python Saved/Claude/m69_ic_ad.py`. Betik 04.10'da yazıldı; senin sonradan eklediğin dosyalar (`MarketArtTrial`, `MarketLargeStoreTrial`, `MarketStoreDressing`, `MarketStoreWalkAudit`, gezi CMD'leri, `Tools/*.py`, `*.ps1`) da `MirasMarket` → `MarketSim` değişimine girer. Çıktıdaki `KALAN` satırlarını oku:
   - `/Game/Materials/Miras/...` ve `M_MirasSurface/M_MirasAcrylic` bilerek kalır (varlık yolu).
   - Komut satırı bayrağı ya da kodda kalan başka `Miras...` adı varsa (ör. senin eklediğin yeni bayraklar) tutarlı biçimde `Sim...`'e çevir; çağıran CMD/PS1 ile birlikte.
2. `Binaries/Win64/UnrealEditor-MirasMarket*` ve `UnrealEditor-MirasMarketStudio*` dosyalarını sil, `Intermediate/` gerekirse temizle; `.sln` varsa yeniden üret.
3. `CLAUDE_KOS.cmd`, `Saved/Claude/is.cmd`, `adim.cmd` betik tarafından atlanır; `MirasMarket` geçiyorsa elle güncelle (çalışırken değil).
4. DERLE → TEST (testler artık `MarketSim.*`; `Test.ps1` filtresi de değişmiş olmalı, en az 174) → Smoke.
5. Ek kontrol: içerikteki eski sınıf yolları `[CoreRedirects]` ile açılıyor mu: `SANAT_DENEME`/`MAHALLE_MARKET_GEZI` için `Tools/ArtTrialReview.ps1` ve bir büyük mağaza için `Tools/LargeStoreReview.ps1` (GameMode yolu `/Script/MarketSim...`), `MAGAZA_GEZI` için `Tools/StoreTourTest.ps1`. Logda "failed to load"/"Can't find" sınıf hatası olmamalı.
6. Hepsi geçerse `git add -A` (yalnız takip edilen ve taşınan dosyalar; `Saved/` girmez), `git commit -F Saved/Claude/m69c_mesaj.txt`, `git push cloud akis-cc2`.
7. Proje dosyası artık `MarketSim.uproject`. README, AGENTS ve CMD'lerde eski `.uproject` adı kalmadığını doğrula.

## Durunca

Bir aşama durursa:
- Çalışma ağacını temiz ve tutarlı bırak (yarım merge ya da yarım taşıma kalmasın; Aşama 3 yarıda kaldıysa `git reset --hard HEAD` ile Aşama 2 sonuna dön, `Saved/` korunur).
- `Saved/Claude/d9b/RAPOR.md` yaz: hangi aşama, hangi komut, hata çıktısının ilgili kısmı. Çakışan her dosya için `git show :1:<yol>`, `:2:`, `:3:` hâllerini `Saved/Claude/d9b/<dosya adı>.base/.ours/.theirs` olarak kaydet (merge abort'tan **önce**). Derleme hatasıysa `Saved/Logs/DERLE_son.log` hata satırları; test hatasıysa başarısız test adları.
- DURUM "Son girişler" ve GUNLUK'e kısa not; Claude çözer.

## Teslim

- GOREVLER: G-109 → Bitti (Aşama 1–2 geçtiyse), G-118 satırı (bu iş), G-117'nin şablon kontrolü ayrı kalır.
- GUNLUK + DURUM: commit kimlikleri, D9b'de ne geldiği (1–2 cümle), DERLE/TEST/Smoke sonuçları, iç ad sonrası kalan `Miras` satırları.
