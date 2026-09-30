# Akış A — Codex

## Yapılanlar

- A0 main: daefd97; DERLE, TEST 84/84 ve Smoke geçti; GitHub'a gönderildi. Kaynak düzeltmesi gerekmedi. Test alt sınırı ve uyarıyla başarılı sayımı düzeltildi.
- akis-a / market-ll-A oluşturuldu; git lfs pull ve ilk DERLE geçti (77,94 sn).
- A1: MarketAutoPlay.h/.cpp/Tests.cpp, MirasAutoPlayCommandlet.h/.cpp, AUTOPLAY.cmd. Gerçek aktif katalog + planogram kapasiteleriyle MarketStart kurulumu; üç görünür karar tarzı, yalnız normal komutlar ve ortak fiyat adımı. Kayıt dosyalarına dokunmaz.
- MarketSimulation FDay: satış/sipariş/ana kapanış nakit eşitliği ve mal kabul korunum kontrolleri. AdjustPrice normal oyuncu ve bot için ortak; MarketGame eski fiyat hesabı bu işleve taşındı, kural aynı.

## Doğrulama

30.09.2026: A1 DERLE geçti. İlk 120 gün × 3 tarz × 3 tohum: 9 koşu, denetim hatası 0; 11,93 sn. Tüm tarzlar borç/nakit sıkıntısına girdi; ekonomi kuralı değiştirilmedi. Tam TEST 86/86 geçti. Son sipariş/İK/rapor ekleri DERLE + AutoPlay 2/2 ile doğrulandı; kısa bot 3×120 gün 1,5 saniye (60 sn altı). İlk pasif sipariş davranışı düzeltildi: büyük öneri bütçeye göre normal koli siparişlerine bölünür. Son 120. gün kasaları 7.534,55 / 14.259,81 / 20.657,32 TL; denetim hatası 0. Smoke GEÇTİ: 1 satış, sipariş, mal kabul, işe alma, gün kapama ve disk kayıt/yükleme.

## Yeni açık işlevler

- MarketAutoPlay::Run(Options, Base, Capacities): dünyasız belirlenimci kampanyalar; rapor bellekte.
- LoadInputs, WriteReport, Validate; commandlet -run=MirasAutoPlay -Years=10 -Seeds=3 -Country=tr -Province=kirklareli. Dosya tarihi yalnız çıktı adında.
- MarketSimulation::AdjustPrice(State, Products, Index, bUp): oyunun mevcut ± fiyat adımı ve sınırları.

## C'ye istekler

- B defteri hazır olunca Simulation::Shopper SellBasket, PlayDay SubmitOrder ve ana CloseDay para hareketleri B'nin Post çağrılarıyla bağlanmalı. Ana kapanıştan sonraki Director farkı ölçülür; sistemlerin her birinin korunumu şu an bağımsız denetlenmiyor. Rapor bunu açık yazar.
- Bot komut tablosunu yeni markalar, tedarik, lig ve dönem olaylarıyla genişletin. Mevcut kararların başarısızlığını sessizce atlamaz: reddedilen komut sayısı raporda.
- Run aktif ülkeyi eski kimliğiyle geri kurar, ekonomi tohumu 0 olur. Commandlet için sorun yok; menüden bot başlatılırsa oyuncunun CountryId/RivalSeed ikilisini koşu sonrası yeniden SetActive yapın. Global ülke/ekonomi nedeniyle koşular paralel çalıştırılmamalı.

## Kararlar ve varsayımlar

Tarzlar: temkinli tampon 30 günlük maaş+temel gider, açılış bedeli ×2,5, 30 günlük büyüme ritmi, fiyat ×1,05, kredi yok; dengeli 14 gün/×1,5/14 gün/×1,00, kredi yok; atak 7 gün/×1,1/7 gün/×0,95, kredi var. Depo değerlendirme eşiği 12/8/4 mağaza. Adaylarda gizli beceri/dürüstlük okunmaz: listedeki ilk adayı alır. Kurgu kimliği 0/1/2; satış teklifini tüm tarzlar reddeder.

## Bilinen sorunlar

Tam defter denetimi B/C entegrasyonuna bağlı. Gider sıralaması ölçülebilen kalemleri kapsar; stok yatırımı ve nakit olmayan fire etiketli. Lig henüz bulunmadığından ilk 3/ilk 10 ölçülemez. İlk rapor aile dükkânının giderlerine karşı satışının yetersiz olduğunu gösteriyor; uzun koşuda ayrıntı verilecek.
