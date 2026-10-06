# Personel ve muhasebe (G-060)

Kod: `Source/MarketSim/MarketStaff.h/.cpp` (dünyadan bağımsız), testler `MarketStaffTests.cpp` (`MarketSim.Staff.*`).
Oyunda: menü (M) → **Personel** sayfası; masada H (kasiyer) ve J/K (reyon görevlisi) tuşları çalışmaya devam eder.

## Kurgu

Babanın dükkânında artık "kasiyer var/yok" diye bir düğme yok; **insanlar** var. Her çalışanın adı, ücreti, morali, yorgunluğu ve becerisi var. İnsanlar iyi çalışır, yorulur, hata yapar, bazen küser, zam ister, istifa eder. Oyuncu hepsini tek tek izlemek zorunda değildir: dükkân büyüyünce **İK müdürü** insanlarla, babanın eski müşaviri **Necati Bey** de defterlerle ilgilenir. Kararlar yine oyuncunundur. Bu iki kişi sorunları önüne getirir.

Kural: hiçbir şey birdenbire olmaz. İstifa önce dilekçeyle gelir, hırsızlık tek günde kanıtlanmaz, vergi cezası önce uyarıyla gelir.

## Roller

### Kasiyer (en fazla 2)
- Ödeme hızı kişiye bağlıdır. Sepet başına süre: `(1,6 + 0,35 × adet) × rutin / hız`. Hızlı ve deneyimli biri 6 adetlik sepeti ~3 sn'de, yorgun ve acemi biri ~6 sn'de geçirir. Eskiden sabit 4 sn'ydi.
- **Kasa farkı:** her müşteride küçük bir hata ihtimali vardır. Bu ihtimal beceri düşükse ve yorgunluk yüksekse artar. Hata çoğunlukla fazla para üstüdür. Hatalar günde birkaç kuruştan birkaç liraya kadar tutar.
- **Dürüst olmayan kasiyer** (her 8 adaydan ~1'i, görünmez): bazı günler cironun %1–3'ü kadar (en çok 20 TL) eksik çıkar. Uyarılırsa bir hafta boyunca bu çok azalır.
- İki kasiyer varsa gün içindeki müşteriyi paylaşırlar ve daha az yorulurlar. Kasiyer izinliyse kasayı oyuncu alır (E).

### Reyon görevlisi (en fazla 3)
- Kararları eskisi gibi `StaffPlanner` verir: boş rafı doldurur, rafta olmayan ürünü reyonuna dizer, dar bloğu genişletir.
- **Hız** yürümeyi ve dizmeyi değiştirir (×0,75–1,25, yorgunlukta ×0,7'ye kadar düşer).
- **Beceri** tek seferde taşıdığı adedi belirler (12–24). Becerisi 45'in altındaki **acemi görevli yalnızca raf doldurur**; yeni blok açmayı ve blok genişletmeyi deneyimliler yapar.
- Gün içinde dizdiği her ürün yorgunluğa eklenir.

### İK müdürü (1 kişi; 3 kasiyer/reyon görevlisinden sonra aday listesine gelir)
- İşe alma 200 TL, ücret ~35 TL/gün (beceriye göre).
- Her gün **en mutsuz kişiyle konuşur** (moral +8).
- **Yorgunu izne çıkarır:** yorgunluğu 70'i geçen biri varsa ve yerine bakacak bir meslektaşı varsa ertesi gün izin verir. Yoksa "ikinci bir kasiyer düşün" diye uyarır.
- **Ayrılanın yerine aday alır:** eski ücretten en çok %15 fazla isteyen ve referansı kötü olmayan en becerikli adayı seçer. Menüden kapatılabilir.
- **Ücret pazarlığı:** onun döneminde işe alınanlar %8 daha düşük ücretle başlar.
- **Aday listesi:** İK yokken 3 aday gelir, liste haftada bir yenilenir ve yalnızca kaba bilgi görünür ("deneyim orta"). İK varken 6 aday gelir, liste 3 günde bir yenilenir; beceri, hız ve dayanıklılık rakamla görünür, bir de referans notu çıkar. Kötü niyetli adayın referansı %70 ihtimalle "zayıf, dikkat" çıkar. Kesin değildir.
- İstifa eden biri için "%10 zam" önerir.

### Mali müşavir (Necati Bey, dış hizmet, 4 TL/gün)
- Babanın eski müşaviridir. Menüden "Müşavirle anlaş" ile başlar; işe alma ücreti yoktur, sözleşme istenince biter.
- **Haftalık vergiyi hesaplar ve zamanında öder.** Belgeli giderleri topladığı için vergi %10 daha az çıkar.
- **Vergi incelemesi gelmez.** Müşavir yokken haftaların ~1/6'sında inceleme olur ve vergi tutarının %25'i kadar (en az 20 TL) ceza çıkar.
- **Kasayı kişi kişi sayar.** Son 7 günün en az 4'ünde eksik ve toplam −30 TL'den kötü bir kasiyer varsa bunu bildirir: "Sayım hatası da olabilir; uyarabilir ya da izleyebilirsin." Suçlamaz, işaret eder. Müşavir yokken oyuncu yalnızca toplam kasa farkını görür.
- **Nakit uyarısı:** kasa, iki günlük gider ve ödenecek vergi için yetmiyorsa haber verir.

## Moral, yorgunluk, istifa

- **Yorgunluk (0–100):** çalışılan gün eklenir (kasiyer: müşteri sayısı, görevli: dizdiği adet, diğerleri sabit), her gece 20 düşer. İzin günü 45 düşer. Dayanıklılığı yüksek olan daha az yorulur.
- **Moral (0–100):** her gün hedefe doğru beşte bir yol alır. Hedef = 60 + ücret farkı (piyasa ücretinin %10 üstü → +10) − fazla yorgunluk (50'nin üstü × 0,6) + İK varsa 5.
- **Piyasa ücreti** beceriyle artar ve çalışan işte öğrenir (3 günde +1 beceri). Zamanla zam beklentisi doğar.
- **İstifa:** üç gün üst üste moral 30'un altındaysa dilekçe verir ve iki gün sonra ayrılır. Moral 45'e çıkarsa (zam, izin, İK konuşması) dilekçesini geri alır.
- **İşten çıkarma:** 3 günlük ücret ihbar tazminatı ödenir, diğerlerinin morali 3 düşer.
- **Uyarı:** dürüst biri için moral −15, dürüst olmayan için −8. Uyarı yalnızca kasiyere verilir.

## Vergi (oyun modeli, gerçek mevzuat değil)

- Dönem bir haftadır. 7., 14., … günün sonunda vergi çıkar:
  - **KDV:** (satış − toptancıdan alış) × %8. Eksiye düşerse fark sonraki haftaya devreder.
  - **Gelir vergisi:** haftanın pozitif net sonucu × %15.
- **3 gün içinde** ödenmelidir (7. günün vergisi en geç 10. günün sonunda). Gecikirse önce %5, sonra her gün %1 ceza işler (en çok %50).
- Müşavir yoksa oyuncu **Menü → Personel → Vergiyi öde** ile öder. Son günden bir gün önce hatırlatma gelir.

## Oyuncunun kararları (menü → Personel)

| Düğme | Ne olur |
|---|---|
| İşe al (aday) | 120 TL (İK müdürü 200 TL); aday kadroya geçer |
| Zam %10 | Ücret +%10, moral +12, istifa dilekçesini geri alabilir |
| İzin ver | Kapalıyken bir sonraki gün, açıkken yarın izinli (ücretli). Moral +4 |
| Uyar | Yalnız kasiyer. Eksik ondan geliyorsa bir hafta azalır |
| Çıkar / Sözleşmeyi bitir | 3 günlük tazminat (müşavirde yok) |
| Vergiyi öde | Kasadan öder |
| Ayrılanın yerine al: açık/kapalı | İK'nın kendiliğinden işe alması |

Gün sonu raporunda (Raporlar) **PERSONEL VE VERGİ** bölümü o günün olaylarını yazar.

## Kayıt uyumu

- Eski kayıtlar: `bCashier`/`Stockers` yüklenince aynı ücretle kişilere dönüşür (`MarketStaff::Migrate`). Kasiyer "Emine Kaya", görevliler eski adlarıyla gelir.
- `bCashier`/`Stockers` artık "bugün işte olan" sayısıdır; dünya (kasa, yürüyen görevliler) bunlara bakar.
- Tüm rastgelelik kampanya tohumu + gün + kişiye bağlıdır: kaydı yüklemek sonucu değiştirmez.

## Denge adayları (G-055'te ayarlanacak)

`MarketStaff.h` başındaki sabitler: vergi oranları (%8 KDV, %15 gelir), müşavir ücreti (4 TL/gün), inceleme sıklığı (1/6), ceza oranları, havuz büyüklüğü, istifa eşiği. En hassas nokta şu: vergi, borcu ödeme hızını yavaşlatır. İlk hafta çok zorlaşırsa ilk vergi dönemi 2. haftaya kaydırılabilir.

## Sonraki adımlar (öneri)

1. Şube müdürü: hedef + bütçe verilir, sipariş ve fiyatı sınırlar içinde yönetir (tasarım paketi 06, §4).
2. Satın almacı ve depo sorumlusu: sipariş önerisini kendisi onaylar, mal kabulde sayım yapar (eksik/hasarlı olayını yakalar).
3. Vardiya: sabah/akşam ve hafta sonu. İzin planı İK'ya geçer.
4. Müşavirden ay sonu kâr-zarar ve nakit akışı raporu; banka kredisi ve toptancı vadesi.
5. Temizlikçi: mağaza temizliği müşteri memnuniyetine eklenir.
