Miras Market, **Akış B — ikinci tur**. Aynı dal (`akis-b`) ve aynı klasörde (`C:\Users\mtass\Desktop\market-ll-B`) devam ediyorsun.

Başlamadan: B1–B4 (ve yaptıysan B5) bitmiş, `Docs/Surec/akislar/B.md` güncel, son commit push edilmiş olmalı. Değilse önce onları bitir. Sonra `Docs/Kurgu/07_AKIL_ISBOLUMU.md`'yi yeniden oku (değişti): §4 "Sonraki işler" altındaki **B6** senin işin; kurallar §2–§3 aynen geçerli. Mutlaka `Docs/Kurgu/06_GIDIS_YOLU.md` §2b'yi (zevk ve akış: "bir tur daha" hissi) oku; B6 bu belgenin koda dökülmüş hâli. `00_KURGU_KITABI.md` §5 (bölümler) ve `MarketStory`, `MarketEvents` mevcut yapısına bak.

## Yapacağın: B6 · Hedefler, kilometre taşları ve kutlamalar
Yeni `MarketGoals.*` (`namespace MarketGoals`, dünyadan bağımsız, tohumlu, testli):
1. **Üç ölçekte hedef** oyuncunun aşamasına göre (aile dükkânı → ilk şube → il → ülke → yurt dışı) üretilir; ekranda **hep en az bir yakında bitecek hedef** olur. Her hedef: başlık, ilerleme (0–1), kalan süre, tek cümle "neden önemli", tamamlanınca ne olur. Kısa (bu hafta), orta (bu ay), uzun (bu yıl / bölüm). Hedefler oyuncunun yaptığına uyar: aynı hedefi üst üste vermez, imkânsızı vermez, çok kolayı da vermez.
2. **Kilometre taşları ve rekorlar:** ilkler ve rekorlar kayda yazılır; her biri bir **kutlama** kaydı üretir (başlık, bir cümle, önem 0–2). Ödüller küçük ve anlamlı: hatıra (`MarketStory::AddMemory`), ekibe moral, toptancı güveni; büyük para ödülü yok.
3. **Ritim koruyucusu:** 20 günden uzun süre ne olay ne karar ne kilometre taşı varsa `MarketEvents`'e olumlu ya da ilginç bir olay önerir; 7 günde 3'ten çok kötü olay yığılırsa yeni kötü olayı erteler. Eşikler zorluğa bağlı.
4. Günlük kapanış `MarketDirector.cpp`'deki kendi bloğuna (`// ===== Akış B =====`), durum `FMarketState` sonundaki kendi bloğuna (tek alan, kendi `USTRUCT`'ın).
5. Testler: hedef hep var ve ulaşılabilir; aynı gün iki kez kapanışta çift kutlama yok; sistem oyunun ortasında devreye girse bile geçmiş için kutlama yağmuru olmaz (eski kayıt uyumu gerekmez: karar M27); ritim koruyucusu sıkıcı dönemde olay önerir, felaket yığılmasında erteler.
6. B.md'ye "C'ye istekler": üst şeritte hedef şeridi, kısa kutlama kartı, Raporlar'da rekorlar sekmesi, "Şimdi ne yapmalı"ya hedef satırı — hangi işlev, hangi metin.

## Kurallar
07 §2–§3 aynen: yalnız kendi dosyaların (B satırı + yeni `MarketGoals.*`); menüye, A ve C dosyalarına dokunma; metin sade Türkçe, yılsız (M24); her alt iş ayrı commit + push; kendi klasöründe DERLE + TEST geçmeden bitti deme; limit azalırsa `[yarım]` commit ve B.md başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
Ne bitti, testler kaç/kaç, örnek 5 hedef ve 5 kutlama cümlesi (oyuncu ne görecek), C'nin bağlaması gerekenler.
