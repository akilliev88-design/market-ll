# Miras Market insan üretim standardı

Bu sistemin amacı her müşteri için sıfırdan MetaHuman ve animasyon hazırlamayı önlemektir. Oyun,
`Content/MetaHumans/` altında adı `BP_MH_` ile başlayan bütün hazırlanmış karakterleri açılışta bulur
ve müşterilere tohumlu rastgelelikle dağıtır. C++ dosyasına karakter adı eklemek gerekmez.

## Yeni görünüş ekleme

1. MetaHuman Character içinde karakteri hazırla ve **Assemble** et.
2. Çıktıyı `Content/MetaHumans/MH_<KisaAd>/BP_MH_<KisaAd>` düzeninde tut.
3. Boy, kilo, yaş, ten, saç, kıyafet ve aksesuarı değiştir. Aynı yüzü yalnız kıyafet rengiyle çoğaltma;
   normal oyun kamerasında ayırt edilen siluet değişikliklerine öncelik ver.
4. Editörü kapat, `METAHUMAN_ANIMASYON_HAZIRLA.cmd` çalıştır, sonra `OYNA.cmd` ile kontrol et.

MetaHuman gövdeleri ortak iskeleti kullandığı için her karaktere ayrı yürüyüş kopyası gerekmez. Komut,
Quinn kaynak animasyonundan ortak `IK_Mannequin`, `IK_MarketMetaHuman`,
`RTG_Mannequin_MarketMetaHuman`, `MF_Unarmed_Walk_Fwd` ve `MM_Idle` varlıklarını üretir. Betik
tekrar çalıştırılabilir; aynı adları günceller.

## Hareket standardı

- Dünya hızı ile animasyon hızı eşlenir; hız değişince ayak kayması azalır.
- Her insanda küçük yürüme hızı, hızlanma, fren ve dönüş farkı vardır.
- Hedefe yaklaşırken fren yapılır; koridor köşelerinde hız düşer ve dönüş yumuşatılır.
- Animasyon başlangıç anı kişiye göre değişir; kalabalık aynı adımla yürümez.
- Müşteri ve reyon görevlisi aynı hareket çekirdeğini kullanır.

## Çeşitlilik planı

İlk oynanabilir hafta için 8–12 belirgin görünüş yeterlidir. Aynı karakter havuzundan farklı kıyafet
renkleri, saçlar ve hız profilleri üretilebilir. Daha kalabalık mağazalarda tam MetaHuman'ı yakındaki
insanlara ayır; uzaktaki insanlar için düşük LOD veya MetaHuman Crowd temsili kullan. Bu aşama,
oyunda aynı anda düzenli olarak 15'ten fazla insan görülmeye başladığında ele alınmalıdır.

Sonraki animasyon geçişi ortak bir Animation Blueprint'tir: başlama/durma, sağ-sol dönüş, raf ürünü
alma, sepete koyma, kasa bekleme ve ödeme klipleri tek durum makinesinde birleşir. Pose Warping ve
Stride Warping ancak bu klipler hazır olduğunda eklenmelidir; mevcut sabit hızlı kaymayı çözmek için
öğrenen bir yapay zekâ gerekmez.

## Kontrol listesi

- `Saved/Logs/METAHUMAN_ANIMASYON_son.log` içinde `MIRAS_METAHUMAN_ANIMATION_OK` var.
- Oyun günlüğünde `MirasMarket shoppers: N MetaHuman(s)` satırında `N` sıfırdan büyük.
- Ayaklar hızlanırken geriye kaymıyor, hedefte aniden sıçramıyor.
- Saç ve kıyafet gövdeyi takip ediyor; dizler ters bükülmüyor.
- Beş müşterinin adımları ve dönüş hızları birebir aynı görünmüyor.
