# Claude Code işi: D3 + D5 (dünya yeniden tasarımı, M51)

Miras Market (UE 5.8 C++ tycoon), proje: C:\Users\mtass\Desktop\market-ll. Ben Mustafa. Claude (Cowork) ile işi bölüşüyoruz: sen D3 ve D5'i yapacaksın, Cowork aynı anda D1, D2, D4'ü yapıyor ve sonunda ikisini birleştirecek.

## Önce oku

- AGENTS.md
- Docs/Kurgu/09_DUNYA_YENIDEN.md (tamamı; özellikle §1 yön, §3 sorunlar, §4 tablo, §7 ülkeler, §8 kararlar, §9 iş bölümü)
- Docs/Kurgu/01_KARARLAR.md: M51, L02–L11 (ülke paketleri), F8 (gerçek marka adları yalnız geliştirmede)
- Docs/Surec/DURUM.md (ilk 10 satır)
- Docs/Surec/GUNLUK.md (ilk 2 giriş)

Kod kuralları (AGENTS.md §5):
- Kaynaklar ASCII. Türkçe metni yaz, sonra `python Tools/escape_unicode.py`.
- Para int64 kuruş.
- FString::Printf'e yalnız sabit biçim metni ver.
- Yeni kural = yeni test.

## Çalışma yeri

```
git worktree add ..\market-ll-cc -b akis-cc
```

- Bütün iş C:\Users\mtass\Desktop\market-ll-cc içinde. Ana klasörde dosya değiştirme.
- Derleme ve test yalnız worktree'de: DERLE.cmd /q, TEST.cmd /q. Ana klasörde CLAUDE_KOS.cmd çalışıyorsa önce bana sor.
- main'e birleştirme, push yapma.

## Hedef

Oyun dünya çapında olacak: oyuncu 10 ülkenin herhangi birinde başlar, hepsinde oynar. Türkiye ülkelerden biri. Bugün kodda Türkiye'ye özel dallar var (`== TEXT("tr")`, yaklaşık 45 yer). Bunlar ülke paketine (Config/ulkeler.json) taşınacak. 6 yeni ülkenin paketi yazılacak.

## İŞ D3 — Türkiye dallarını ülke paketine taşı

1. Bütün `TEXT("tr")` / `== "tr"` / `bTurkey` / `bHome` kullanımlarını bul ve listele: Calendar, Chains, Competitors, Country, Departments, Eras, Managers, Payments, Staff, Start, MarketMenuPages ve diğerleri.
2. Her biri için davranışı pakete taşı; paket şemasını (`MarketCountry::FProfile`, `MarketCountry::Parse`) genişlet:
   - resmî tatiller ve okul açılış/kapanışı (MarketCalendar'daki Türkiye listesi);
   - ulusal zincir kadrosu: `MarketChains::NationalRoster` içindeki BİM, A101, Şok, Migros… satırları, büyüklükleri, arketipleri, patron adları ve bölgeleri;
   - ödeme alışkanlıkları (yemek kartı vb.);
   - kasaplık ve bayram zirvesi (MarketDepartments);
   - isim havuzları;
   - zincir adı gösterimi (MarketCompetitors).
3. **Türkiye'de davranış birebir aynı kalmalı.** Mevcut testlerin hepsi geçmeli; gerekiyorsa Türkiye için "önce / sonra aynı" testi ekle.
4. Sonunda kodda ülke kodu yalnız "paket yoksa varsayılan" seçiminde kalsın (`MarketCountry::Find` ve `MarketStart`'taki varsayılan).

## İŞ D5 — 6 yeni ülke paketi

Ülkeler: Fransa (fr), İspanya (es), Polonya (pl), Brezilya (br), Meksika (mx), Japonya (jp). Mevcut "de", "gb", "us" paketlerini şablon al.

Her paket şunları içerir:
- Para birimi.
- displayScale: oyunun iç birimine göre. Başka bir ülkenin aynı sepeti, mevcut paketlerle tutarlı bir iç fiyatla almalı.
- fxPerWorld.
- Ekonomi karakteri:
  - Brezilya: yüksek enflasyon.
  - Meksika, İspanya, Polonya: oynak.
  - Fransa: istikrarlı.
  - Japonya: istikrarlı, çok düşük enflasyon.
- Ekonominin sayıları: inflationMean/Vol, loanSpread, wageFactor, groceryPerPersonDay, employerSocialRate, severanceDaysPerYear, rentFactor.
- Alışkanlıklar: haftalık alışveriş payı, pazar kapalı mı, kart payı.
- Geleneksel ticaret adları.
- Zincir kadrosu: ülkenin gerçek pazar yapısına benzeyen ama **kurgu adlı** zincirler (F8: gerçek marka adı yok).
  - Fransa hipermarket ağırlıklı.
  - Brezilya atacarejo.
  - Japonya kombini.
  - Polonya indirim marketi.
- Tatiller.
- 4 kurgu banka.
- Online platform adı ve tavanı.
- İklim: aylık sıcaklık ve yağmur.
- İsim havuzları: en az 40 ad, 40 soyad.
- Akraba sözcükleri.
- referencePopK.
- Bölgeler ve alt bölgeler.
- İller: ülkenin birinci düzey idari birimleri, nüfus (bin), varsa gelir/kira; mümkünse harita koordinatı (cx, cy) ve kmPerMapUnit.
  - Japonya'da 47 prefektör.
  - Fransa'da bölgeler ya da büyük departmanlar, tutarlı bir seçimle; tercihini teslim notunda açıkla.

Kurallar:
- Sayılar oyun değeridir, kesin gerçek olması gerekmez; mantıklı ve birbirine göre tutarlı olsun.
- Kaynak gerekiyorsa web'den bak. Gerçek şirket adlarını yalnız teslim notunda "benzer olduğu gerçek yapı" olarak yaz, pakete koyma.

Testler:
- MarketCountryTests'e her yeni ülke için ayrıştırma testi: il sayısı, bölgeler, tatiller, zincirler, para.
- Her ülkede bir kampanya başlatıp 30 gün ilerleten basit bir duman testi (MarketStart + MarketSimulation), çökme ve para/stok farkı olmamalı.
- Test.ps1'deki alt sınırı yeni test sayısına göre artır.

## Dokunma (Cowork aynı anda değiştiriyor)

MarketPrices.*, MarketLedger.*, MarketBanking.*, MarketBranches.*, MarketEconomy.*, MarketSimulation.*, MarketDirector.*, MarketEras.*, MarketStory.*, MarketCompany.*, MarketAutoPlay*.*, MarketGame.*, MarketMenu.cpp, MarketMenuWidget.cpp.

- Bu dosyalarda Türkiye dalı varsa düzeltme; teslim notuna dosya ve satırla yaz, Cowork yapacak.
- MarketManagers.cpp ve MarketStaff.cpp'de yalnız isim havuzu kısmına dokunabilirsin. Değişikliği `// D3-CC BEGIN ... // D3-CC END` blokları içine al.
- MarketChains.cpp'de yalnız NationalRoster/GiantRoster ve pakete taşıma kısmına dokunabilirsin; aynı blok işaretleriyle.
- MarketMenuPages.cpp'deki "tr" dalları sende, aynı blok işaretleriyle.

## Teslim

1. DERLE + TEST worktree'de geçsin.
2. Belgeler:
   - Docs/Kurgu/01_KARARLAR.md'ye D3 ve D5 için satır ya da M51 satırına not.
   - GUNLUK.md en üstüne giriş.
   - DURUM.md "Kısaca" en üstüne bir satır ("Claude Code / D3-D5").
   - Paket şeması değiştiyse Docs/URUN_STUDYOSU.md değil, ulkeler.json'un `note` alanı ve 09_DUNYA_YENIDEN.md §4'e kısa ek.
3. akis-cc dalına anlamlı commit'ler.
4. Docs/Surec/akislar/D3_D5_cc_teslim.md:
   - değişen/yeni dosyalar;
   - taşınan her Türkiye dalı (önce nerede, şimdi paketin hangi alanında);
   - paket şemasına eklenen alanlar;
   - dokunmadığın dosyalarda kalan Türkiye dalları (dosya:satır);
   - her yeni ülkenin özeti (il sayısı, ekonomi karakteri, zincir kadrosu ve benzediği gerçek yapı);
   - derleme/test sonucu;
   - bilinen eksikler.
5. Bitince bana kısa özet ver, teknik terim az.
