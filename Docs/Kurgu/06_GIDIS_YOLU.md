# Miras Market: gidiş yolu

Sürüm 2: 02.10.2026 (ilk sürüm 30.09.2026) · Karar: Mustafa · Yazan: Claude
**Tek yol haritası budur.** `05_YOL_HARITASI.md` ve `02_DERIN_INCELEME.md` §4 ayrıntı ve geçmiştir. Sürüm 2'nin gerekçesi: `claude/durum_analizi_2026-10-02.md` (proje) ve aşağıdaki §3.

## 1. Yön (Mustafa, 30.09.2026)

1. **Önce oyunun aklı, sonra dükkân içi simülasyon.** Oyun bir **tycoon**: ana ekrandan (harita + menü) bütün şirket yönetilir. Ayrıntılıdır ama oyuncuyu boğmaz. Birinci şahıs dükkân, 3B mağazalar ve insanlar ikinci aşamada tamamlanır.
2. **Oyunu Mustafa uzun uzun oynamaz; Codex oynatır.** Oyunu yıllarca oynayan bir **otomatik oyuncu** yazılır. Codex her işten sonra onu çalıştırır, sonuç raporunu ve menü ekran görüntülerini günlüğe koyar. Mustafa rapora ve ekranlara bakıp karar verir.
3. **Toparlanma kuralı:** kaldırıldı (Mustafa, 02.10.2026). Derlenmemiş iş DURUM'da "derlenmedi" diye işaretlenir, tamamlanmış gibi gösterilmez.
4. **Dükkânlar hepsi aynı sistem (Mustafa, 02.10.2026).** İlk dükkânın yönetim açısından özel kuralı yok: kira, müdür, kampanya, stok eritme her mağazada aynı. Fark yalnız oyuncunun içinde yürüyebilmesi.

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

## 3. Durum (02.10.2026)

| Alan | Var | Eksik / risk |
|---|---|---|
| Akıl | M1–M38: finans, banka, kurtarma, zincirler, internet, reklam, komuta zinciri, markalar, reyonlar, tedarik, depolar, dönemler, salgın, patron maaşı ve servet. 150 test, bot doğrulama döngüsü | **Çekirdek döngü dengesiz:** tek dükkân yıllar içinde zarara dönüyor; dengeli botun sırası 30 yılda iyileşmiyor (35/33/31); büyüme tohuma bağlı (1 / 104 / 107). Yürünen dükkân ile şube iki ayrı ekonomi formülüyle çalışıyor |
| Simülasyon | Yürünen dükkân, raf dizme, reyon görevlisi, listeli müşteri, MetaHuman yürüyüşü, mağaza editörü, Ürün Stüdyosu, Blender kitleri | 3B dükkânın tycoon içindeki rolü belirsiz; 97 ürün, ülke başına yüzlercesine hat yok; kalabalık ve hipermarket performansı planlanmadı |
| Tasarım | Kimlik ve ilkeler net (§1, §2, §2b) | Öğretici, sanat yönü, ses, İngilizce, satılacak sürümde yalnız kurgu marka adları; kapsam küçük ekip için büyük |

## 4. Aşamalar

Sıra önemli. Her iş: yaz → Codex derler + test + otomatik oyuncu → Mustafa'ya kısa özet → sonraki iş. "Bitti" sütunu sağlanmadan sonraki satıra geçilmez.

### Aşama 1 — Oyunun aklı (sürüyor)

A1–A7 bitti (bot, denge hataları, zaman, mağaza ağı, il pazarı, şirket derinliği, dünya); A8 menü sadeleştirmenin iki turu yapıldı. Kalan:

| Sıra | İş | Kim | Bitti sayılır |
|---|---|---|---|
| **C8** | C8 + M36–M38 derleme, aile dükkânı tanı koşusu (aylık döküm, büyümesi kapalı koşu) | Codex | 150+ test; rapor dükkânın marjı hangi kalemde, hangi yılda kaybettiğini sayılarla söylüyor |
| **C9** | **Çekirdek denge:** C8 bulgularına göre düzeltme; §5 hedef eğrileri botta tutuyor | Claude → Codex | Üç tarz × üç tohum: iyi oyun hedef eğride, kötü oyun tökezliyor ama batmıyor; kurtarma 30 yılda bir elin parmaklarını geçmiyor; tohum farkı makul |
| **C10** | **Tek mağaza ekonomisi:** ilk dükkân ve şubeler aynı günlük modeli kullanır; yürünen dükkân onu gösterir, oyuncunun elle işi (raf, fiyat, sıra) küçük sapma ekler | Claude → Codex | Aynı koşullarda ilk dükkân ve bir şube aynı ay sonucunu veriyor (± küçük fark); testler |
| C11 | A8 kalanları (Codex C7 listesi), ilk saat için "Şimdi ne yapmalı" akışı | Claude → Codex | Codex ekran görüntüleri; Mustafa onayı |
| — | **Özellik dondurma:** C9 bitene kadar yeni sistem yok (yalnız düzeltme) | Herkes | — |

### Aşama 2 — Dikey dilim: ilk 3–5 saat (demo ve Steam sayfası)

| İş | Kim | Bitti sayılır |
|---|---|---|
| Öğretici: ilk gün → borç → ilk şube; her adım tek görev kartı | Claude (akış, metin) + Codex (bağlama) | Yeni bir oyuncu yardımsız ilk şubeyi açıyor (Mustafa denemesi) |
| Mağaza ziyareti modu: oyuncu bir mağazaya girer, sorun bulur (boş raf, kuyruk, yanlış fiyat, kirli reyon, kötü müdür), çözdüğü ekonomiye yazılır | Claude (kurallar) + Codex (3B) | Ziyaret başına 1–3 bulgu; botta etkisi ölçülüyor |
| Menü ve 3B sanat yönü: renk, tipografi, ikonlar, tabela; tek stil rehberi | Mustafa + Codex | Rehber belgesi; menü ve dükkân görüntüleri rehbere uyuyor |
| Yerelleştirme altyapısı: bütün metin String Table'a; İngilizce | Codex (altyapı) + Claude (çeviri) | Oyun İngilizce açılıp oynanıyor |
| Satılacak sürümde yalnız kurgu marka adları (F8 gerçek ad geliştirme içindir) | Codex | Paket derlemesinde gerçek ad yok (denetim betiği) |
| Temel ses: dükkân ortamı, kasa, kapı, menü | Mustafa (seçim) + Codex | Dikey dilimde sessiz an yok |
| Demo yapısı ve Steam sayfası metni, ekran görüntüleri | Mustafa + Claude | Sayfa taslağı hazır |

### Aşama 3 — İçerik hattı (ürünler, insanlar)

| İş | Kim | Bitti sayılır |
|---|---|---|
| **Ambalaj arketipleri:** 56 → ~120 şekil; ürün = arketip + etiket | Codex (Blender/Studio) | Kategori başına en az bir doğru şekil |
| **Etiket üretim hattı:** marka kimliği (renk, logo yazısı) + ürün hattı şablonu + çeşit adı/rengi + ülke dili, veri dosyasından toplu PNG; özel çizim yalnız ülke başına 30–50 öne çıkan ürüne | Claude (şablon kuralları, veri) + Codex (betik, Studio toplu içe alma) | Bir ülkenin 300 etiketi tek komutla; gözle inceleme galerisi |
| **Raf çizimi:** arketip başına tek materyal + etiket doku dizisi, toplu örnek (instancing) ve örnek başına etiket indisi; 4–6 m ötesinde ürün bloğu tek "raf kartı"; etiket 256–512 px (öne çıkan 1024) | Codex | Hipermarket (binlerce ürün) hedef kare hızında; çizim çağrısı ölçümü |
| **Ülke kataloğu şeması:** ortak ürün tipleri (süt 1 L, kola 1 L) + ülke markası; manav/kasap/fırın etiketsiz, kasada örnekli meyve/et | Claude (şema, veri) + Codex | Yeni ülke = marka listesi + etiket üretimi; yeni mesh yok |
| **İnsanlar:** 12–20 temel MetaHuman, parametreli kıyafet/saç/aksesuar, ülkeye göre görünüş havuzu (veri) | Mustafa (karakterler) + Codex | Kalabalıkta tekrar göze batmıyor |
| Animasyon: ortak Anim BP + Motion Matching (Epic Game Animation Sample), raftan alma IK ile, sepet/araba, kasa bekleme, ödeme | Codex | Ayak kayması yok; alma hareketi her raf yüksekliğinde doğru |
| Kalabalık: yakın 10–15 kişi tam MetaHuman, uzak düşük ayrıntı ya da pişirilmiş animasyon; ekonomi sayar, ekran temsil eder | Codex | 40 kişilik hipermarket hedef kare hızında |
| Portreler: aynı MetaHuman'lardan menü yüzleri (müdür, rakip sahibi, hikâye karakterleri); personel üniforması şirket kimliğinin renginde | Codex | Müdür kartlarında yüz var |

### Aşama 4 — Genişleme ve topluluk

| İş | Kim | Bitti sayılır |
|---|---|---|
| Kalan 16 mağaza şablonu, kalan ülkeler (veriyle) | Codex + Claude | Her ülke botla 10 yıl oynanıyor, Türkçe kalıntı yok |
| Editörler (mağaza, ürün, raf) oyuncu için cilalı ve belgeli; Workshop'a paylaşma | Codex | Oyuncu yaptığı mağazayı/ürünü paylaşıp başkası yükleyebiliyor |
| Performans ve kayıt sağlamlığı | Codex | Uzun koşu ve büyük ağda kare hızı, kayıt boyutu hedefte |

### Aşama 5 — Yayın ve DLC

**Karar önerisi:** editörler ana oyunda kalır (topluluk içeriği oyunun ömrünü uzatır ve ana oyunu satar; ücretli araçlar tepki çeker). Para içerikten gelir:

| DLC | İçerik |
|---|---|
| Ülke paketleri | Markalar, zincirler, iklim, bayramlar, mağaza kiti, dil |
| Dönem / senaryo | 1990'larda başlangıç, kriz senaryoları, meydan okumalar |
| Mağaza formatları | AVM hipermarketi, fırın/şarküteri zinciri, kozmetik, cash & carry |
| Kişisel hayat | Servetle ev, araba, aile, itibar (M37'nin devamı) |
| Üretim ve tedarik | Özel marka fabrikası, lojistik filosu, franchise |
| Kozmetik | Dekor, üniforma, tabela temaları; destekçi paketi |

Editörden gelir istenirse: temel editör ücretsiz, ek ekipman ve dekor kütüphaneleri ücretli.

## 5. Hedef eğriler (C9'un ölçütü; C11'de güncellendi)

**İlke (Mustafa + Claude, 02.10.2026):** Dengeli oynayan da büyür; zevk, büyümenin ritminden gelir. Her aşamada para biraz sıkışık olmalı, her yeni mağaza bir karar olmalı. Bir mağaza "kendiliğinden" açılıyorsa ya da kasada milyonlar boşta duruyorsa oyun sıkılmaya başlamıştır. Zorluk her aşamada yeni bir sorunla gelir: önce para, sonra yönetim (müdür, kadro), sonra lojistik ve rakipler, en sonda doymuş pazar. Hedefler **Normal** zorluk içindir; Rahat ve Zor ayrı satırlardadır.

### Normal zorluk

| Ölçü | Dengeli (iyi oyun) | Temkinli | Atak | Kötü oyun |
|---|---|---|---|---|
| İlk şube | 4–8. ay | 8–14. ay | 3–6. ay | Gecikir, olmayabilir |
| 3. yıl mağaza | 8–12 | 4–8 | 12–25 | — |
| 10. yıl mağaza | 80–150 | 40–80 | 120–220 | Tökezler, toparlanır |
| Ulusal sıra | 10. yıl ilk 10, 20. yıl ilk 3 | 10. yıl ilk 20, 20. yıl ilk 5 | 10. yıl ilk 10 | — |
| Kurtarma planı (30 yıl) | 0–1 | 0 | 0–2 (hız riskli olmalı) | En çok 3–5; borç birikmez |
| Boşta para (büyüme dönemi, 1–15. yıl) | Kasa 6 aylık sabit gideri aşan dönemler kısa | Kısa | — | — |
| İlk dükkânın yıllık faaliyet kârı (reel, 10. yıl / 1. yıl) | 0,8–1,3 (yatay ya da hafif artış; dönem şoku yılında düşebilir) | 0,8–1,3 | 0,7–1,3 | Düşer ama eksiye kalıcı geçmez |
| Sıkıcı dönem (30 gün olaysız) | 0 | 0 | 0 | 0 |

Temkinli ile dengeli arasındaki fark oyundan gelmeli: temkinli daha az risk alır, daha az büyür ama hiç batmaz; dengeli daha hızlı büyür, arada bir sıkışır. İlk dükkân bir mahalle marketidir; indirim zincirleri yayılırken onun reel kârının yatay kalması başarıdır, "enflasyonla birlikte büyür" beklentisi kaldırıldı.

### Rahat ve Zor

| Ölçü (dengeli bot) | Rahat | Zor |
|---|---|---|
| İlk şube | 3–6. ay | 8–14. ay |
| 10. yıl mağaza | 120–200 | 40–80 (temkinli 15–40) |
| Ulusal sıra | 10. yıl ilk 5 | 10. yıl ilk 20, 20. yıl ilk 10 |
| Kurtarma (30 yıl) | 0 | 1–3 olabilir; borç birikmez |

Zor: daha az müşteri ve daha sert fiyat duyarlılığı (bugün var), saldırgan rakipler ve fiyat savaşı, pahalı kredi, daha pahalı yöneticiler; büyük mağaza ve ölçek kârı daha düşük. Rahat: tersi. Zorluk oyunun kurallarını değil, sayıların sertliğini değiştirir.

**Onaylandı (Mustafa, 02.10.2026; C11 güncellemesi Mustafa'nın isteğiyle Claude, 02.10.2026).** Bot raporu her turda Normal tablosunu doldurur; Rahat ve Zor, zorluk ayarı bota bağlandığında ölçülür. `AutoPlay.LateCarefulGrowth` temkinli ilk şube en geç 14. ay.

## 6. Şeritler

| Kim | Ne yapar |
|---|---|
| **Claude** | Oyun kuralları, ekonomi, kurgu, veri şemaları, şablon kuralları, metinler ve çeviri; tek seferde bir paket; belgeler güncel |
| **Codex** | Derleme, test, otomatik oyuncu ve rapor, menü ve dükkân ekran görüntüleri; 3B, Blender, Studio, animasyon, performans; commit |
| **Mustafa** | Kararlar, sanat yönü ve karakterler, kısa denemeler, rapor ve ekranlara bakış |

## 7. Beklenen kararlar

| Karar | Gerektiği iş | Claude önerisi |
|---|---|---|
| ~~Hedef eğriler (§5)~~ | C9 | Onaylandı 02.10.2026 |
| Oyun sonu (J02) | Aşama 2 | 30. yılın sonu ya da lig birinciliği; sonra serbest oyun |
| Demo kapsamı | Aşama 2 | İlk dükkân + ilk iki şube, ~2 saat |
| İngilizceden sonraki diller | Aşama 2–4 | Almanca, İspanyolca, Portekizce (Brezilya) |
| DLC sırası | Aşama 5 | Önce bir ülke paketi ve kişisel hayat |
