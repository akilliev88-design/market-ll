# F — Sanat yönü ve stil rehberi ön tasarımı (G-106)

Mustafa'nın Claude'a (ya da başka bir tasarım ajanına) vereceği istek. Aşağıdaki çizginin altındaki her şeyi kopyalayıp yapıştır. İş 5 adım; her adımdan sonra bakıp "beğendim, sonraki adım" ya da düzeltmeni yaz. Başka oyun referansı yok (Mustafa 04.10.2026: "kendi kimliği olsun"). Ekran görüntüsü eklersen (menü, harita, dükkân içi) daha iyi sonuç alırsın: `Docs/Images/` altında ve Codex'in son ekran görüntülerinde var.

Sonuç gelince: seçilen yönü `Docs/STIL_REHBERI.md` olarak sakla; renk ve yazı tablolarını Codex koda geçirir (`MarketMenuWidget.cpp` → `SMarketMenu::Color`, `MarketTheme.cpp` → `Font`), ikonlar `Content/Slate/Icons/` altına SVG olarak girer, tabela ve etiketler 3B mağazaya (`MarketStoreKit`).

---

Merhaba. "MarketSim" adlı bir PC oyunu için **sanat yönü ve stil rehberi ön tasarımı** istiyorum. İşi adım adım yapacağız: her adımda yalnız o adımı yap, ben bakıp düzelteyim, sonra bir sonrakine geçelim. **Oyunun kendi kimliği olsun:** başka bir oyunu ya da markayı örnek alma, taklit etme.

## Oyun ne

- **Tür:** market zinciri tycoon'u (Unreal Engine 5). Oyuncu babasından kalan küçük bir mahalle bakkalı/marketiyle başlar; şubeler, iller, ülke, sonra 10 ülkeli bir dünya zinciri kurar. Oyunda gerçek yıl yok ("3. yıl").
- **Nasıl oynanır:** Asıl oyun ana ekranda: harita (il ve dünya haritası) + tıklanabilir yönetim menüsü (Özet, Sipariş, Fiyat, Kampanyalar, Raporlar, Rakipler, Personel, Finans, Satış kanalları, Şubeler). Ayrıca ilk dükkânın içinde birinci şahıs yürünür (raflar, kasa, müşteriler).
- **His:** sıcak, güven veren, "bir tur daha" dedirten; kalabalık ama boğmayan bir yönetim ekranı. Mahalle esnafı samimiyetinden dünya şirketine büyüme duygusu. Ciddi işletme ama karikatür değil; gerçekçi ama soğuk kurumsal da değil.
- **Kitle:** tycoon ve simülasyon oyuncuları; uzun süre tablo, para ve sıralama okuyan, ayrıntıyı seven ama boğulmak istemeyen oyuncu. İlk dil Türkçe, sonra İngilizce.
- **Kimliğin çekirdeği "miras":** babadan kalan mahalle dükkânı yıllar içinde dünya zincirine dönüşüyor. Görsel dil bu büyüme hikâyesini taşımalı (aşağıda "Büyüyen tabela").
- **Marka:** oyuncunun şirketinin varsayılan adı "Miras" (oyuncu değiştirebilir). Rakipler kurgu adlar (gerçek marka yok). Logo da değiştirilebilir olmalı; rehber varsayılan "Miras" logosu için bir öneri versin.

## Bugün ne var (koddan)

**Renkler** (iki tema: açık "kâğıt" ve koyu):

| Rol | Açık | Koyu |
|---|---|---|
| Sayfa arka planı | #F4F1EA | #101316 |
| Panel / kart | #FFFFFF | #1B1E22 |
| İç kutu / düğme | #EDE9E1 | #2C3137 |
| Metin | #1B1E22 | #F2F3F4 |
| Soluk metin | #6B7178 | #9AA3AC |
| Vurgu = iyi (yeşil) | #2F8A70 | #71C6AC |
| Kötü (kırmızı) | #C4453A | #F07F6E |
| Uyarı (turuncu) | #D08A1E | #E8A94E |
| Bilgi (mavi) | #2F5FA8 | #7FA7E0 |
| Çizgi | #E2DDD3 | #343A41 |
| Harita karası | #DEDAD1 | #23282E |
| Bizim iller (harita) | #9ED0BE | #2E6E5D |
| Birincil düğme | #1B1E22 (metin beyaz) | #F2F3F4 (metin koyu) |

Sorunlar: vurgu ile "iyi" aynı renk; marka rengi yok; ülke/durum renkleri (ana ülke, mağaza, ortaklık, araştırma) geçici seçildi.

**Yazı tipleri:** IBM Plex Sans (normal, orta, yarı kalın, kalın), IBM Plex Mono (sayılar), Bricolage Grotesque (başlık). Hepsi açık lisanslı (OFL); değişebilir ama öneri açık lisanslı olsun ve Türkçe karakterleri (ç, ğ, ı, İ, ö, ş, ü) iyi çizsin.

**İkonlar** (SVG, tek renk): bag, bars, bell, box, close, fast, faster, flag, house, map, more, pause, percent, person, play, sliders, store, tag, wallet. Tutarlı bir aile değiller.

**Ekran ölçüsü:** menü 1440×820 tasarım tahtası üzerinde çizilir, ekrana göre ölçeklenir; yazı boyu ayarı Küçük/Orta/Büyük. Yuvarlak köşeli kartlar, düz renk, hafif gölge.

## Senden istediklerim (adım adım)

**Öncelik:** okunurluk. Oyuncu saatlerce sayı okuyacak. Marka rengi durum renklerinden (iyi/kötü/uyarı/bilgi) ayrı olsun ve onlarla karışmasın; sayılar hizalı ve sabit genişlikte okunsun.

### Adım 1: 3 yön (her biri tek sayfa)

Her yön için: adı ve bir cümlelik fikri, ana renk paleti (marka rengi dahil), yazı tipi önerisi, ikon tarzı, örnek bir kart ve bir düğme, ana ekranın küçük bir taslağı, mağaza tabelasının küçük bir örneği. Yönler gerçekten farklı olsun ve üçü de "miras / büyüme" fikrinden kendi yorumunu çıkarsın.

### Adım 2: renk, yazı, ikon, bileşenler (seçilen yön)

1. **Renk sistemi:** rol tabanlı tokenlar (yukarıdaki tablonun yerine geçecek), açık ve koyu tema; ayrıca:
   - durum renkleri: iyi / kötü / uyarı / bilgi (marka renginden ayrı),
   - harita renkleri: kara, bizim iller, rakip yoğunluğu, fırsat,
   - dünya haritası durumları: ana ülke, mağazamız var, ortaklık, pazar araştırması, girilmedi,
   - grafik serileri için 6 renk, renk körlüğüne dayanıklı.
   Her renk için hex ve hangi öğede kullanıldığı. Metin/arka plan kontrastı okunur olsun.
2. **Yazı:** başlık, ara başlık, gövde, küçük not, sayı (tablo ve para) için boyut ve kalınlık ölçeği (1440 genişlik tasarım pikseli).
3. **İkon ailesi:** çizgi kalınlığı, köşe, boyut kuralı ve şu ikonların çizimi (SVG): dükkân, şube, depo, kamyon, harita, dünya, kişi, müdür, para, kredi/banka, sepet, etiket/fiyat, kampanya/yüzde, rapor/grafik, rakip, bildirim, ayarlar, oynat, duraklat, hızlı, daha hızlı, kapat, ekle, uyarı, tamam.
4. **Bileşenler:** kart, birincil/ikincil düğme, çip (sekme), tablo satırı (sıra, ad, çubuk, sayı), harita iğnesi (sayılı), bildirim satırı, karar kartı (başlık, metin, 2–3 seçenek), kutlama kartı, ilerleme çubuğu, ipucu kutusu.

### Adım 3: ekran taslakları (1440×820)

Ana ekran (il haritası, üstte ülke/bölge çipleri, solda hedefler, sağda il paneli, altta menü çubuğu), menü Özet sayfası, Şubeler sayfası, Rakipler (ülke listesi tablosu), bir karar kartı. Metinler İngilizcede daha uzun olabilir: yerleşim metin uzayınca bozulmasın.

### Adım 4: mağaza görsel dili (3B dükkânda görünen markalı şeyler)

3B dükkânın kendisi (modeller, ışık, malzeme) bu işin dışında; yalnız dükkânda görünen markalı öğeleri tasarla, menüyle aynı dilde:
- **Büyüyen tabela:** "Miras" tabelası şirket büyüdükçe değişir: (1) elle boyanmış mahalle esnafı tabelası, (2) şubelerle düzgün bir market logosu, (3) ülke çapında zincir, (4) dünya zincirinin sade logosu. Arayüzün geri kalanı sabit kalır; değişen yalnız logo, tabela ve bir iki renk vurgusu. Oyuncu şirketin adını değiştirebilir: düzen her adla çalışsın.
- **Dış tabela:** mahalle marketi, süpermarket, hipermarket, indirim marketi, yakın market, toptan perakende.
- **İç:** reyon/kategori levhaları (meyve-sebze, süt, içecek, temizlik...), raf fiyat etiketi (normal ve indirimli), kasa üstü levha, kampanya afişi ("3 al 2 öde", "%20"), poşet, çalışan önlüğü rengi.
- **Rakipler:** kurgu rakiplerin tabelaları bizimkinden ayrışsın (renk ailesi kuralları); oyunda 10 ülke var, rakip tabelaları o ülkenin dilinde yazılabilir: rehber bunun düzenini göstersin.

### Adım 5: logo ve tanıtım görseli

"MarketSim" oyun logosu (oyuncunun şirket logosundan ayrı) ve mağaza sayfası (Steam) için bir kapak görseli taslağı.

### Biçim (her adım için)

- Her adımı tek bir HTML sayfasında göster (renk tabloları, yazı örnekleri, ikonlar SVG olarak, bileşenler, taslaklar).
- Renk ve yazı ölçüsünü ayrıca düz bir tablo olarak da ver (rol adı, açık hex, koyu hex, kullanım); bir geliştirici bunu doğrudan koda geçirecek.
- İkonları ayrı SVG dosyaları olarak verebiliyorsan ver (24×24, tek renk, `currentColor`).
- Türkçe yaz; ekran metinleri Türkçe olsun.

### Kısıtlar

- Oyun Unreal Engine'in kendi arayüzüyle (Slate) çiziliyor: düz renkler, yuvarlak köşeli kutular, SVG ikonlar, açık lisanslı TTF yazılar. Karmaşık gradyan, bulanıklık ve doku olmasın; olacaksa kısa ve az.
- Sayı çok: tablolar, para, yüzdeler, sıralar. Sayılar hizalı ve okunaklı olmalı.
- Gerçek bir markaya (BİM, Migros, A101, Carrefour vb.) ya da başka bir oyuna benzeyen renk, logo veya düzen önerme.
- Önce yalnız Adım 1'i yap.
