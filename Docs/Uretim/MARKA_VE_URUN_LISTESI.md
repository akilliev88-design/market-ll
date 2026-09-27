# Marka ve ürün listesi (Türkiye, 2011 → 2026)

**Bu ürünlerin hepsi Ürün Stüdyosu'nda hazır** (97 ürün; 6'sı oyunda, gerisi Hazırlık listesinde). Ambalaj türü, tahmini ölçüler, parçalar ve renk notları katalogda (`Config/products.json`) kayıtlı; katalog `Tools/katalog_olustur.py` ile bu listeden üretildi.

Oyunun başlangıcı olan 2011'den bugüne Türkiye marketlerinde bulunan başlıca firmalar, markalar ve örnek ürünler. **Ambalaj** sütunu hangi promptun kullanılacağını söyler:

- **K** = dikdörtgen kutu → A1 (veya A2)
- **Ş** = şişe / kavanoz / teneke / kase → B1 + B2
- **P** = poşet / yumuşak paket → C1 + C2

**Güvenilirlik:** "Önemli olaylar" bölümündeki satın almalar ve isim/logo değişiklikleri kaynaklarla doğrulandı (en altta). Ürün satırları, 2011'den beri piyasada olduğu genel olarak bilinen markalardır; **her ürünün her dönemde rafta olup olmadığını ve dönem ambalajını D promptuyla doğrulat**. Ürün kimlikleri önerilir; stüdyoda değiştirilebilir (yayımladıktan sonra değiştirme).

2033 bütün ürünler için **kurgu gelecektir**: gerçek bir 2033 ambalajı yoktur; promptlar markanın 2025 hâlinden makul bir evrim üretir.

## Önemli olaylar (dönem etiketlerini etkiler)

| Yıl | Olay | Oyunda etkisi |
|---|---|---|
| 2007 (başlangıç öncesi) | Coca-Cola, Doğadan'ı satın aldı | 2011'de Doğadan zaten Coca-Cola bünyesinde |
| 2012 | Yıldız Holding ŞOK marketlerini satın aldı | Rakip zincir büyümesi için referans |
| 2013 | Ajinomoto, Kükre Gıda'nın (Kemal Kükrer) %50'sini aldı; 2017'de tamamını | Marka adı sürdü; kurumsal bilgi arka yüzde değişebilir |
| 2014 | OYAK, Tukaş'ın çoğunluk hissesini Okullu ailesine sattı | Marka sürdü |
| 2015 (Mayıs) | Yıldız Holding, Ak Gıda'yı (İçim) Fransız Lactalis'e sattı | İçim sürdü; üretici bilgisi değişir |
| 2015 | Cola Turka, Çamlıca Gazoz ve Saka Su Japon DyDo Drinco'ya satıldı | Marka sürdü; 2023 sonrası boykot döneminde satışları arttı |
| 2016 | Ülker, Godiva, McVitie's ve DeMet's "pladis" çatısında birleşti | Ülker markası sürdü; kurumsal kimlikte pladis görünür |
| 2016 (Aralık) | Komili ve Kırlangıç yağ markaları Bunge'ye satıldı | Marka sürdü |
| 2017 (Nisan) | Ajinomoto, Bizim Mutfak'ın üreticisi Örgen Gıda'yı Yıldız'dan aldı; 2018'de Ajinomoto İstanbul çatısında birleşti | Marka sürdü |
| 2017 (Ağustos) | Coke Zero'nun adı dünyada "Coca-Cola Zero Sugar" oldu; Türkiye'de 2018 itibarıyla ambalajda "Şekersiz" öne çıktı | 2011 etiketi "Coca-Cola Zero", 2018+ "Zero Sugar / Şekersiz" |
| 2017 | Banvit, BRF ve Katar Yatırım Otoritesi ortaklığına satıldı | Marka sürdü |
| 2021 | Unilever çay işi (Lipton) CVC'ye satıldı (ekaterra); sonra "Lipton Teas and Infusions" | Lipton markası sürdü |
| 2023–2024 | Pepsi, 14 yıl sonra logosunu yeniledi (2023 tanıtım, 2024'ten itibaren dünya geneli) | 2025 etiketi yeni logo; 2011/2018 eski logo |
| 2023–2024 | Boykot dalgası bazı küresel markaların satışını düşürdü, yerli alternatifler (ör. Cola Turka) öne çıktı | Rakip/talep olayları için fikir |
| 2024 (Ocak) | Koç Holding, Tat Gıda'daki %49 hissesini Memişoğlu'na sattı | Tat markası sürdü |
| 2025 (Temmuz) | Algida ve Magnum, Türkiye'de de Unilever'den ayrılan "The Magnum Ice Cream Company" çatısına geçti | Algida markası sürdü |
| 2025 (Aralık) | Lipton, Rize'deki yaş çay fabrikalarını Öz-Gür Çay'a devretti; paketleme Sakarya'da sürüyor | Lipton rafta sürüyor |

## Ürünler

### Süt ve süt ürünleri

| Ürün kimliği | Ürün | Firma (sahiplik notu) | Ambalaj |
|---|---|---|---|
| sutas_sut_1l | Sütaş Süt 1 L | Sütaş | K |
| pinar_sut_1l | Pınar Süt 1 L | Yaşar Holding | K |
| icim_sut_1l | İçim Süt 1 L | Ak Gıda (2015'e kadar Yıldız, sonra Lactalis) | K |
| torku_sut_1l | Torku Süt 1 L | Konya Şeker | K |
| sutas_ayran_200ml | Sütaş Ayran 200 ml | Sütaş | Ş |
| sutas_yogurt_1kg | Sütaş Yoğurt 1 kg | Sütaş | Ş (kase: Govde + Kapak) |
| pinar_labne_200g | Pınar Labne 200 g | Yaşar Holding | Ş (kase) |
| pinar_beyaz_peynir_500g | Pınar Beyaz Peynir 500 g | Yaşar Holding | K |
| danone_danino | Danone Danino | Danone Tikveşli | Ş (kase) |

### Soğuk içecekler

| Ürün kimliği | Ürün | Firma (sahiplik notu) | Ambalaj |
|---|---|---|---|
| coca_cola_1l | Coca-Cola 1 L PET | Coca-Cola İçecek | Ş |
| coca_cola_zero_1l | Coca-Cola Zero / Zero Sugar 1 L | Coca-Cola İçecek (2017 isim değişikliği) | Ş |
| coca_cola_kutu_330ml | Coca-Cola 330 ml kutu | Coca-Cola İçecek | Ş (teneke) |
| fanta_portakal_1l | Fanta Portakal 1 L | Coca-Cola İçecek | Ş |
| sprite_1l | Sprite 1 L | Coca-Cola İçecek | Ş |
| pepsi_1l | Pepsi 1 L PET | PepsiCo (2023–24 yeni logo) | Ş |
| yedigun_1l | Yedigün 1 L | PepsiCo | Ş |
| cola_turka_1l | Cola Turka 1 L | 2015'e kadar Yıldız/Ülker, sonra DyDo Drinco | Ş |
| camlica_gazoz_1l | Çamlıca Gazoz 1 L | 2015'e kadar Yıldız/Ülker, sonra DyDo Drinco | Ş |
| uludag_gazoz_250ml | Uludağ Gazoz 250 ml cam | Uludağ İçecek | Ş (Etiket + Cam + Kapak) |
| beypazari_maden_suyu_200ml | Beypazarı Maden Suyu 200 ml cam | Beypazarı | Ş (Etiket + Cam + Kapak) |
| kizilay_maden_suyu_200ml | Kızılay Maden Suyu 200 ml cam | Kızılay | Ş (Etiket + Cam + Kapak) |
| erikli_su_1_5l | Erikli Su 1,5 L | Nestlé Waters (2006'dan beri) | Ş |
| hayat_su_1_5l | Hayat Su 1,5 L | Danone Hayat | Ş |
| cappy_visne_1l | Cappy Vişne 1 L | Coca-Cola İçecek | K |
| dimes_portakal_1l | Dimes Portakal 1 L | Dimes | K |
| tamek_seftali_1l | Tamek Şeftali 1 L | Tamek | K |
| lipton_ice_tea_seftali_1_5l | Lipton Ice Tea Şeftali 1,5 L | PepsiCo–Lipton ortaklığı | Ş |
| red_bull_250ml | Red Bull 250 ml | Red Bull | Ş (teneke) |

### Çay ve kahve

| Ürün kimliği | Ürün | Firma (sahiplik notu) | Ambalaj |
|---|---|---|---|
| caykur_rize_turist_500g | Çaykur Rize Turist 500 g | Çaykur | K |
| caykur_tiryaki_1kg | Çaykur Tiryaki 1 kg | Çaykur | P |
| lipton_yellow_label_100 | Lipton Yellow Label 100'lü demlik poşet | Unilever → 2021 CVC (ekaterra) | K |
| dogus_cay_1kg | Doğuş Çay 1 kg | Doğuş Çay | P |
| dogadan_yesil_cay_20 | Doğadan Yeşil Çay 20'li | Coca-Cola (2007'den beri) | K |
| kurukahveci_mehmet_efendi_100g | Kurukahveci Mehmet Efendi Türk Kahvesi 100 g | Mehmet Efendi | P |
| nescafe_3u1_arada_10 | Nescafé 3ü1 Arada 10'lu | Nestlé | K |
| jacobs_monarch_200g | Jacobs Monarch 200 g | JDE | Ş (kavanoz: Etiket + Cam + Kapak) |

### Bisküvi, çikolata, şekerleme

| Ürün kimliği | Ürün | Firma (sahiplik notu) | Ambalaj |
|---|---|---|---|
| ulker_cikolatali_gofret | Ülker Çikolatalı Gofret | Ülker (2016'dan beri pladis çatısı) | P |
| ulker_albeni | Ülker Albeni | Ülker / pladis | P |
| ulker_halley | Ülker Halley | Ülker / pladis | K |
| ulker_biskrem | Ülker Biskrem | Ülker / pladis | P |
| ulker_potibor | Ülker Pötibör | Ülker / pladis | P |
| eti_canga | Eti Canga | Eti | P |
| eti_burcak | Eti Burçak | Eti | P |
| eti_puf | Eti Puf | Eti | P |
| eti_browni | Eti Browni | Eti | P |
| torku_banada | Torku Banada | Konya Şeker | Ş (kavanoz) |
| nestle_damak | Nestlé Damak | Nestlé | P |
| milka_sutlu_80g | Milka Sütlü 80 g | Mondelez | P |
| falim_sakiz | Falım Sakız | Mondelez (Kent) | P |

### Atıştırmalık ve kuruyemiş

| Ürün kimliği | Ürün | Firma | Ambalaj |
|---|---|---|---|
| lays_klasik | Lay's Klasik | PepsiCo | P |
| ruffles_originals | Ruffles Originals | PepsiCo | P |
| doritos_nacho | Doritos Nacho | PepsiCo | P |
| cerezza | Çerezza | PepsiCo | P |
| peyman_karisik | Peyman Karışık Kuruyemiş | Peyman | P |
| tadim_kavrulmus_findik | Tadım Kavrulmuş Fındık | Tadım | P |

### Makarna, un, bakliyat

| Ürün kimliği | Ürün | Firma | Ambalaj |
|---|---|---|---|
| filiz_spagetti_500g | Filiz Spagetti 500 g | Barilla (2003'ten beri) | P |
| barilla_spagetti_500g | Barilla Spaghetti n.5 500 g | Barilla | K |
| nuhun_ankara_spagetti_500g | Nuh'un Ankara Spagetti 500 g | Nuh'un Ankara | P |
| pastavilla_spagetti_500g | Pastavilla Spagetti 500 g | Pastavilla | P |
| soke_un_2kg | Söke Un 2 kg | Söke Un | P |
| duru_bulgur_1kg | Duru Pilavlık Bulgur 1 kg | Duru Bulgur | P |
| reis_kirmizi_mercimek_1kg | Reis Kırmızı Mercimek 1 kg | Reis Gıda | P |

### Yağ, salça, konserve, hazır gıda

| Ürün kimliği | Ürün | Firma (sahiplik notu) | Ambalaj |
|---|---|---|---|
| komili_zeytinyagi_1l | Komili Zeytinyağı 1 L | 2016'ya kadar Anadolu Endüstri Holding, sonra Bunge | Ş |
| kirlangic_zeytinyagi_1l | Kırlangıç Zeytinyağı 1 L | 2016'dan sonra Bunge | Ş |
| yudum_aycicek_1l | Yudum Ayçiçek Yağı 1 L | Yudum | Ş |
| orkide_aycicek_1l | Orkide Ayçiçek Yağı 1 L | Orkide | Ş |
| tat_domates_salcasi_830g | Tat Domates Salçası 830 g | Tat Gıda (2024'te Koç payını Memişoğlu'na sattı) | Ş (teneke) |
| tukas_biber_salcasi_650g | Tukaş Biber Salçası 650 g | Tukaş (2014'te OYAK → Okullu) | Ş (cam kavanoz) |
| tamek_bezelye_konserve | Tamek Bezelye Konservesi | Tamek | Ş (teneke) |
| bizim_mutfak_mercimek_corbasi | Bizim Mutfak Mercimek Çorbası | Örgen Gıda (2017'den beri Ajinomoto) | P |
| knorr_domates_corbasi | Knorr Domates Çorbası | Unilever | P |
| kemal_kukrer_ketcap | Kemal Kükrer Ketçap | Kükre (2013/2017 Ajinomoto) | Ş |
| calve_mayonez | Calvé Mayonez | Unilever | Ş |

### Dondurma

| Ürün kimliği | Ürün | Firma (sahiplik notu) | Ambalaj |
|---|---|---|---|
| algida_magnum_klasik | Algida Magnum Klasik | Unilever → 2025 The Magnum Ice Cream Company | P |
| algida_cornetto | Algida Cornetto | aynı | P |
| algida_carte_dor_1l | Algida Carte d'Or 1 L | aynı | K |

### Temizlik

| Ürün kimliği | Ürün | Firma | Ambalaj |
|---|---|---|---|
| ariel_toz_4kg | Ariel Toz Deterjan 4 kg | P&G | K |
| alo_toz_4kg | Alo Toz Deterjan 4 kg | P&G | K |
| omo_toz_4kg | Omo Toz Deterjan 4 kg | Unilever | K |
| persil_jel_1l | Persil Jel 1 L | Henkel | Ş |
| bingo_toz_4kg | Bingo Toz Deterjan 4 kg | Evyap (sahiplik geçmişini D ile doğrulat) | K |
| fairy_bulasik_650ml | Fairy Bulaşık Deterjanı 650 ml | P&G | Ş |
| pril_bulasik_675ml | Pril Bulaşık Deterjanı | Henkel | Ş |
| yumos_yumusatici_1_5l | Yumoş Yumuşatıcı 1,5 L | Unilever | Ş |
| domestos_750ml | Domestos 750 ml | Unilever | Ş |
| cif_krem_500ml | Cif Krem 500 ml | Unilever | Ş |

### Kişisel bakım ve kâğıt

| Ürün kimliği | Ürün | Firma | Ambalaj |
|---|---|---|---|
| colgate_dis_macunu_100ml | Colgate Diş Macunu 100 ml | Colgate-Palmolive | K |
| signal_dis_macunu_100ml | Signal Diş Macunu 100 ml | Unilever | K |
| ipana_dis_macunu_100ml | Ipana Diş Macunu 100 ml | P&G | K |
| duru_sabun_4lu | Duru Sabun 4'lü | Evyap | K |
| arko_tiras_kopugu | Arko Tıraş Köpüğü | Evyap | Ş (teneke) |
| elidor_sampuan_500ml | Elidor Şampuan 500 ml | Unilever | Ş |
| head_shoulders_500ml | Head & Shoulders 500 ml | P&G | Ş |
| rexona_deodorant | Rexona Deodorant | Unilever | Ş (teneke) |
| selpak_tuvalet_kagidi_32 | Selpak Tuvalet Kâğıdı 32'li | Selpak | P |
| solo_havlu_kagit_8 | Solo Havlu Kâğıt 8'li | Hayat Kimya | P |
| familia_pecete | Familia Kâğıt Peçete | Hayat Kimya | P |

## Kaynaklar

- [Ak Gıda'nın Lactalis'e satışı (T24, 2015)](https://t24.com.tr/haber/yildiz-holding-ak-gidayi-fransiz-groupe-lactalise-satti,295718)
- [Cola Turka 2015'te DyDo'ya satıldı, boykot sonrası satışlar (Patronlar Dünyası, 2024)](https://www.patronlardunyasi.com/2015te-japonlarin-aldigi-cola-turka-satislari-artirdi-yeniden-reklama-basladi)
- [Coca-Cola'nın Doğadan'ı alması (Hürriyet, 2007)](https://www.hurriyet.com.tr/ekonomi/coca-cola-dogadan-i-50-milyon-dolara-alip-bitki-cayina-giriyor-7295376)
- [Yıldız Holding olayları: ŞOK 2012, pladis 2016 (Wikipedia)](https://en.wikipedia.org/wiki/Y%C4%B1ld%C4%B1z_Holding)
- [Kükre ve Örgen Gıda'nın Ajinomoto çatısında birleşmesi (Gıda Türk, 2018)](https://www.gidaturk.com.tr/2018/03/kukre-gida-ve-orgen-gida-ajinomoto-istanbul-catisi-altinda-birleserek-hedef-buyuttu/)
- [Tukaş'ın Okullu ailesine satışı (Hürriyet, 2014)](https://www.hurriyet.com.tr/ekonomi/iste-yarim-asirlik-tukas-i-alan-aile-27010671)
- [Komili ve Kırlangıç'ın Bunge'ye satışı (Hürriyet, 2016)](https://www.hurriyet.com.tr/ekonomi/komili-ve-kirlangic-yag-bunge-gidaya-satiliyor-40307944)
- [Coke Zero → Coca-Cola Zero Sugar (MediaCat, 2017)](https://mediacat.com/coke-zero-artik-coca-cola-zero-sugar/)
- [Türkiye'de "Zero"dan "Şekersiz"e (Marketing Türkiye, 2018)](https://www.marketingturkiye.com.tr/soylesiler/zero-gitti-coca-cola-sekersize-odaklandi/)
- [Pepsi'nin yeni logosu (MediaCat)](https://mediacat.com/pepsi-logosunu-ve-gorsel-kimligini-yeniledi/)
- [Tat Gıda hisse satışı (Bloomberg HT, 2024)](https://www.bloomberght.com/koc-holding-tat-gida-daki-hisselerini-satti-2345306)
- [Algida'nın The Magnum Ice Cream Company'ye geçişi (CNBC-e, 2025)](https://www.cnbce.com/sirket-haberleri/unilever-dondurma-isini-bagimsizlastirdi-algida-ve-magnum-artik-yeni-cati-altinda-h14738)
- [Unilever çay işinin CVC'ye satışı (Unilever, 2021)](https://www.unilever.com.tr/news/2021/unilever-ekaterray-45-milyar-karslgnda-cvc-capital-partners-fund-viiie-satyor/)
- [Lipton'un Rize fabrikalarını devri (Euronews, Aralık 2025)](https://tr.euronews.com/business/2025/12/03/lipton-turkiyedeki-39-yillik-cay-uretimini-sonlandirma-karari-aldi)
- [Yabancılara satılan Türk markaları derlemesi — Banvit/BRF 2017 (ilhamipektas.com; ikincil kaynak)](https://www.ilhamipektas.com/yabancilara-satilan-unlu-turk-markalari/)
