# Miras Market: gidiş yolu

Tarih: 30.09.2026 · Karar: Mustafa · Yazan: Claude
**Tek yol haritası budur.** `05_YOL_HARITASI.md` ve `02_DERIN_INCELEME.md` §4 bunun ayrıntı ve geçmişidir; hata numaraları (#1–#51) orada.

## 1. Yön (Mustafa, 30.09.2026)

1. **Önce oyunun aklı, sonra dükkân içi simülasyon.** Oyun bir **tycoon**: ana ekrandan (harita + menü) bütün şirket yönetilir. Ayrıntılıdır ama oyuncuyu boğmaz. Birinci şahıs dükkân, 3B mağazalar ve insanlar ikinci aşamada tamamlanır.
2. **Oyunu Mustafa uzun uzun oynamaz; Codex oynatır.** Oyunu yıllarca oynayan bir **otomatik oyuncu** yazılır. Codex her işten sonra onu çalıştırır, sonuç raporunu ve menü ekran görüntülerini günlüğe koyar. Mustafa rapora ve ekranlara bakıp karar verir.
3. **Toparlanma kuralı:** aynı anda en fazla **bir** derlenmemiş Claude işi. Derlenip test ve otomatik oyuncu raporu gelmeden yenisine başlanmaz.

## 2. Tycoon arayüz ilkeleri ("boğmayan ayrıntı")

Her yeni sistem ve menü sayfası bu ilkelere göre kontrol edilir.

| İlke | Anlamı |
|---|---|
| Sonuç önce, ayrıntı sonra | Kartta bir cümle ve bir sayı; hesabın ayrıntısı ipucunda ya da "Nasıl işler?"te |
| Yalnız gerektiğinde görünür | Bir sistem, oyuncu ona ulaşınca açılır (ör. ülke müdürü 5 ilde mağaza olunca, depo 4 mağazadan sonra) |
| Kararı oyun getirir | Oyuncu sayfa sayfa aramaz: "Şimdi ne yapmalı" ve Kararlar listesi yapılacak işi, çözen düğmeyle birlikte getirir |
| Devredilebilir | Her tekrar eden iş bir müdüre bırakılabilir (sipariş, fiyat, raf, işe alma). Müdür iyiyse oyuncu stratejiye bakar |
| Her kararın bedeli ve etkisi söylenir | "Emin misin?" penceresi: ne kadar para, ne değişir, ne zaman görülür |
| Hızlı zaman | Gün / hafta / ay ilerletme; ilerletme karar gerekince, kasa eksiye düşünce ya da önemli bir olayda kendiliğinden durur |
| Sabit ekran | Kartlar sabit yükseklikte, düğmeler yer değiştirmez (M15) |

## 2b. Zevk ve akış: "bir tur daha" hissi (Mustafa, 30.09.2026)

Oyuncunun zamanı su gibi akmalı. Her sistem yazılırken şu soruya cevap verilir: **bu, oyuncuya bir sonraki dakikayı oynatmak için ne veriyor?**

| İlke | Anlamı | Örnek |
|---|---|---|
| Hep yarım kalmış bir şey | Ekranda her an yakında bitecek bir hedef ya da gelişme durur | "Şube 3 gün sonra açılıyor", "ilde birinciliğe %2 kaldı", "son taksit ay sonunda" |
| Üç ölçekte hedef | Kısa (gün), orta (ay), uzun (yıl/bölüm) hedefler aynı anda | Günlük satış rekoru · aylık kâr hedefi · "ülkede ilk 3" |
| Görünür ilerleme ve kutlama | Eşik geçilince kısa bir kutlama kartı, haritada yeni iğne, rekor satırı | "İlk 10 mağaza!", "İlk kez il birincisi" |
| Kararın sonucunu görmek | Her kararın etkisi birkaç gün içinde bir sayıda ya da haberde görünür | "İndirim sayesinde bu hafta +120 müşteri" |
| Anlamlı takas | Kararlarda açık tek doğru yok: ucuz mu kaliteli mi, hızlı mı güvenli mi | Deneyimli ama pahalı müdür / ucuz ama gelişmeye açık aday |
| Hikâye ve yüzler | Rakiplerin ve karakterlerin adı, kişiliği, hafızası var; oyuncunun "ezeli rakibi" olur | Yıllardır aynı ilde çekiştiğin zincirin sahibi seni tebrik eder ya da intikam alır |
| Ritim | Sakin dönemlerle yoğun dönemler (bayram, kriz, rakip saldırısı, açılış) sırayla gelir | Uzun sessizlik olmaz, üst üste felaket de olmaz |
| Ceza değil ders | Kötü gidenin nedeni tek cümleyle söylenir ve toparlanma yolu vardır; oyun affedicidir ama tembelliği ödüllendirmez | "Kasa eksi: toptancı borcu vadesi geldi. Seçenekler: kredi, stok indirimi, tahsilat" |
| Sıkmadan devret | Tekrar eden iş sıkmaya başladığı anda devredilebilir hâle gelir | Üç kez elle verilen siparişten sonra "bunu müdüre bırak" önerisi |
| Merak | Açılacak bir sonraki şey hep görünür ama kilitli | Haritada "5 ilde mağaza olunca ülke müdürü" rozeti |

**Ölçülür:** otomatik oyuncu raporu "sıkıcı dönemleri" (30 günden uzun süre ne karar ne olay ne eşik) ve "felaket yığılmasını" (7 gün içinde 3'ten çok kötü olay) işaretler.

## 3. Aşamalar

### Aşama 0 — Doğrulama borcu (hemen)

| Kim | İş | Bitti sayılır |
|---|---|---|
| Codex | Bugünkü hâli derle; TEST; Smoke (G-086f, G-089, M24 dahil). Kırılanı Claude'a bildir | Derleme ve bütün testler geçti, smoke geçti, git commit |
| Claude | Derleme/test hatalarını düzeltir | Aynı |

### Aşama 1 — Oyunun aklı (Claude yazar, Codex doğrular)

Sıra önemli. Her iş: yaz → Codex derler + test + otomatik oyuncu raporu → Mustafa'ya kısa özet → sonraki iş.

| Sıra | İş | Bitti sayılır |
|---|---|---|
| **A1** | **Otomatik oyuncu ve denge raporu.** Dünyasız, oyunu menü komutlarıyla (`MarketDirector::Command`) oynayan bir bot: temkinli / dengeli / atak üç tarz, birkaç tohum. Aile dükkânı (`MarketSimulation::PlayDay`) + şubeler + şirket yıllarca ilerler. Çıktı: `Saved/AutoPlay/<tarih>/rapor.md` + `.csv` (kasa, borç, mağaza sayısı, pay, iflas, ilk şube / ilk depo / 5 il gibi eşiklere ulaşma günü, en çok para kaybettiren şey). Otomasyon testi olarak da koşar (kısa sürüm) | Codex tek komutla çalıştırıyor (`TEST.cmd` içinde kısa, ayrı komutla uzun); rapor GUNLUK'e bağlanıyor |
| A2 | **Açık denge hataları:** #19 kampanya adedi istismarı, #24 raftaki fiyat kararı, #27 maliyet altı satış/destekli kampanya raporu, #30 online kampanya, #41 gecikme faizi tavanı, #43 zarar devri, #45 ulusal pay ciroya bağlı, #21/#25/#31 kontrolü | Her biri için test; botun raporunda istismar yok |
| A3 | **Zaman ve tur:** 1 gün / 1 hafta / 1 ay ilerletme (#12), ilerletme işi yapan kişinin becerisine bağlı (#8), hafta ve ay raporu ritmi, durma koşulları | Bot aylık turla oynuyor; menüde üç düğme |
| A4 | **Mağaza ağının aklı tamam:** G-086b/G-089 doğrulanmış hâli; G-088 Aşama C'nin ekonomi kısmı (mağaza görünümü ataması ve kayıt, ölçülerden çeşit/kuyruk/taze/kira/tadilat/çalışan çarpanları). "Mağazayı gez" düğmesi Aşama 2'ye | Testler; botta şube geri dönüş süresi makul (#47) |
| A5 | **Yaşayan il pazarı + tedarik ağı + markalar** (G-079 kalanı, G-083, M25): ilde rakip havuzu, yerel/bölgesel/ulusal toptancı, ölçekle ucuzlayan alım; markaların reyonda yer yarışı (raf parası, ciro primi, ortak kampanya) ve reyondaki marka payları | Testler; botta rakipler tepki veriyor |
| A6 | **Şirket derinliği** (G-080 kalanı, G-081): muhasebe defteri (tek kaynak), ücret alt sınırı ve sigorta, ülke içi büyüme, dönem olayları | Testler; bot raporunda bilanço tutarlı |
| A7 | **Dünya ve son** (G-082): yeni ülkeler `ulkeler.json`'da, kurgu kurlar (L08), lig, oyun sonu (J02) | Testler; bot zirveye 40–60 saatlik denk süre içinde ulaşıyor ya da ulaşamıyor, rapor söylüyor |
| A8 | **Menü sadeleştirme turu:** her sayfa §2'ye göre gözden geçirilir; gereksiz sayı ve düğme kalkar, açılma koşulları konur | Codex'in ekran görüntüleri; Mustafa onayı |

### Aşama 2 — Dükkân içi simülasyon (Aşama 1'den sonra)

Birinci şahıs dükkân, G-088 kalan 16 mağaza ve "Mağazayı gez" (Codex'ten test modu açmayan gezi girişi), müşteri ve çalışan hareketleri, MetaHuman, raf dizme ayrıntıları, dükkân içi HUD'un menüyle aynı dili konuşması, elle oyun testi (G-055).

## 4. Şeritler

| Kim | Aşama 1 boyunca |
|---|---|
| **Claude** | A1…A8 sırayla, tek seferde bir iş. Belgeler (DURUM/GOREVLER/GUNLUK, kararlar) güncel |
| **Codex** | Her Claude işini derler, test eder, otomatik oyuncuyu çalıştırır, menü ekran görüntüsü alır (`-MirasCapture` menü sayfaları), sonucu GUNLUK'e yazar, commit atar. Elinde kalan G-088 editör işini kapatır; kalan 16 mağaza Aşama 2'ye kalır |
| **Mustafa** | Karar verir; bot raporuna ve ekranlara bakar; istediği zaman kısa deneme |

## 5. Aşama 1 için beklenen kararlar

Aşağıdakiler ilgili iş gelmeden sorulacak; şimdilik Claude önerisiyle ilerlenir (`05_YOL_HARITASI.md` §5).

| Karar | Gerektiği iş | Claude önerisi |
|---|---|---|
| L08 kurgu kurlar | A7 | Para birimi adı gerçek, kur kurgu; ülkeye göre istikrarlı/oynak |
| J02 oyun sonu | A7 | "30. yılın sonu" ya da lig birinciliği |
| Dönem olayları | A6 | Sıra sabit, zamanı her oyunda kayar |
| Zirveye süre | A1, A7 | 40–60 saat; "dünya ölçeği" ayarı |
| L12 zincir adları | A5 | Varsayılan kurgu ad |
