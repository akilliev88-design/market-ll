"""Builds Config/products.json from the product list below (27.09.2026).
All sizes are ESTIMATES (estimated=true); the external agent is told to report real sizes.
Prices are fictional game-balance values. Run: python Tools/katalog_olustur.py"""
import json, pathlib

# id | realName | brand | category | type | dims | parts | notes | cost | price | case | color | active
# dims: kutu "G,D,Y"; poset "G,D,Y" (D = dolu kalinlik); yuvarlak "cap,Y,etiketY"
ROWS = r"""
sutas_sut_1l|Sütaş Süt 1 L|Sütaş|süt|kutu|95,64,195|||1.70|2.50|12|2E8B57|1
pinar_sut_1l|Pınar Süt 1 L|Pınar|süt|kutu|95,64,195|||1.70|2.45|12|E30613|0
icim_sut_1l|İçim Süt 1 L|İçim|süt|kutu|95,64,195|||1.65|2.40|12|1F5FA8|0
torku_sut_1l|Torku Süt 1 L|Torku|süt|kutu|95,64,195|||1.55|2.25|12|D71920|0
sutas_ayran_200ml|Sütaş Ayran 200 ml|Sütaş|süt|kase|70,75,75|Etiket,Kapak|beyaz plastik bardak, alüminyum folyo kapak|0.45|0.75|24|2E8B57|0
sutas_yogurt_1kg|Sütaş Yoğurt 1 kg|Sütaş|süt|kase|140,90,90|Etiket,Kapak|beyaz plastik kase, baskılı folyo/plastik kapak|2.40|3.50|6|2E8B57|0
pinar_labne_200g|Pınar Labne 200 g|Pınar|süt|kase|95,55,55|Etiket,Kapak|plastik kase, baskılı folyo kapak|2.10|3.10|12|E30613|0
pinar_beyaz_peynir_500g|Pınar Beyaz Peynir 500 g|Pınar|süt|kutu|120,90,60|||5.50|7.90|12|E30613|0
danone_danino|Danone Danino 4'lü|Danone|süt|kase|55,55,55|Etiket,Kapak|küçük plastik kap, folyo kapak|1.60|2.40|12|1C6FB7|0
coca_cola_1l|Coca-Cola 1 L PET|Coca-Cola|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, içi koyu kola (Cam yuvası koyu kahve, opacity 0.9), kırmızı plastik kapak|1.80|2.75|12|E41E26|1
coca_cola_zero_1l|Coca-Cola Zero 1 L PET|Coca-Cola|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, içi koyu kola, siyah kapak; 2011 "Coca-Cola Zero", 2018+ "Zero Sugar / Şekersiz"|1.80|2.75|12|111111|0
coca_cola_kutu_330ml|Coca-Cola 330 ml Kutu|Coca-Cola|içecek|teneke|66,115,115|Etiket,Kapak|alüminyum kutu, etiket tüm gövdeyi sarar; Kapak = gümüş metal üst|0.90|1.40|24|E41E26|0
fanta_portakal_1l|Fanta Portakal 1 L PET|Fanta|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, içi turuncu, turuncu/mavi kapak|1.70|2.60|12|F7941D|0
sprite_1l|Sprite 1 L PET|Sprite|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|yeşil şeffaf PET, yeşil kapak|1.70|2.60|12|00A651|0
pepsi_1l|Pepsi 1 L PET|Pepsi|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, içi koyu kola, mavi kapak; logo 2023-24'te yenilendi|1.70|2.60|12|004B93|0
yedigun_1l|Yedigün 1 L PET|Yedigün|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, içi turuncu|1.50|2.30|12|F58220|0
cola_turka_1l|Cola Turka 1 L PET|Cola Turka|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, içi koyu kola; 2015'e kadar Ülker/Yıldız, sonra DyDo|1.50|2.30|12|C8102E|0
camlica_gazoz_1l|Çamlıca Gazoz 1 L PET|Çamlıca|içecek|pet_sise|80,300,70|Etiket,Cam,Kapak|şeffaf PET, renksiz gazoz, yeşil kapak|1.30|2.00|12|00A651|0
uludag_gazoz_250ml|Uludağ Gazoz 250 ml Cam|Uludağ|içecek|cam_sise|60,225,60|Etiket,Cam,Kapak|şeffaf cam şişe, renksiz gazoz, metal taç kapak|0.60|1.00|24|0072BC|0
beypazari_maden_suyu_200ml|Beypazarı Maden Suyu 200 ml|Beypazarı|içecek|cam_sise|55,185,50|Etiket,Cam,Kapak|şeffaf/hafif yeşil cam, metal kapak|0.35|0.60|24|1B75BB|0
kizilay_maden_suyu_200ml|Kızılay Maden Suyu 200 ml|Kızılay|içecek|cam_sise|55,185,50|Etiket,Cam,Kapak|şeffaf cam, metal kapak|0.35|0.60|24|E31E24|0
erikli_su_1_5l|Erikli Su 1,5 L|Erikli|içecek|pet_sise|88,330,90|Etiket,Cam,Kapak|şeffaf PET, renksiz su, mavi kapak|0.55|0.90|6|0071BC|0
hayat_su_1_5l|Hayat Su 1,5 L|Hayat|içecek|pet_sise|88,330,90|Etiket,Cam,Kapak|şeffaf PET, renksiz su, mavi kapak|0.50|0.85|6|00A0E3|0
cappy_visne_1l|Cappy Vişne 1 L|Cappy|içecek|kutu|95,64,195|||1.90|2.90|12|8B1538|0
dimes_portakal_1l|Dimes Portakal 1 L|Dimes|içecek|kutu|95,64,195|||1.85|2.80|12|F39200|0
tamek_seftali_1l|Tamek Şeftali 1 L|Tamek|içecek|kutu|95,64,195|||1.75|2.60|12|F6A01A|0
lipton_ice_tea_seftali_1_5l|Lipton Ice Tea Şeftali 1,5 L|Lipton|içecek|pet_sise|88,330,90|Etiket,Cam,Kapak|şeffaf PET, içi açık kahve/şeftali, sarı kapak|1.90|2.90|6|FFD200|0
red_bull_250ml|Red Bull 250 ml|Red Bull|içecek|teneke|53,135,135|Etiket,Kapak|ince alüminyum kutu; Kapak = gümüş metal üst|2.20|3.50|24|1E3A73|0
caykur_rize_turist_500g|Çaykur Rize Turist 500 g|Çaykur|çay-kahve|kutu|115,65,190|||4.80|6.75|12|0A7A3B|1
caykur_tiryaki_1kg|Çaykur Tiryaki 1 kg|Çaykur|çay-kahve|poset|190,70,300|||8.50|12.00|6|0A7A3B|0
lipton_yellow_label_100|Lipton Yellow Label 100'lü Demlik Poşet|Lipton|çay-kahve|kutu|170,115,90|||7.50|10.50|12|FFD200|0
dogus_cay_1kg|Doğuş Çay 1 kg|Doğuş|çay-kahve|poset|190,70,300|||8.00|11.50|6|B31B1B|0
dogadan_yesil_cay_20|Doğadan Yeşil Çay 20'li|Doğadan|çay-kahve|kutu|125,70,65|||1.80|2.75|12|6AB023|0
kurukahveci_mehmet_efendi_100g|Kurukahveci Mehmet Efendi Türk Kahvesi 100 g|Mehmet Efendi|çay-kahve|poset|100,40,170|||2.60|3.75|24|7A4A2A|0
nescafe_3u1_arada_10|Nescafé 3ü1 Arada 10'lu|Nescafé|çay-kahve|kutu|130,55,150|||2.40|3.50|12|C8102E|0
jacobs_monarch_200g|Jacobs Monarch 200 g|Jacobs|çay-kahve|kavanoz|95,170,80|Etiket,Cam,Kapak|şeffaf cam kavanoz, içi koyu kahve granül, altın/kahve kapak|11.00|15.50|6|5B3A29|0
ulker_cikolatali_gofret|Ülker Çikolatalı Gofret|Ülker|bisküvi-çikolata|poset|60,25,160|||0.20|0.35|36|8B0000|0
ulker_albeni|Ülker Albeni|Ülker|bisküvi-çikolata|poset|45,25,150|||0.30|0.50|36|E4002B|0
ulker_halley|Ülker Halley 10'lu|Ülker|bisküvi-çikolata|kutu|190,105,45|||2.20|3.25|12|0055A4|0
ulker_biskrem|Ülker Biskrem|Ülker|bisküvi-çikolata|poset|70,45,190|||0.90|1.40|24|3E2723|0
ulker_potibor|Ülker Pötibör|Ülker|bisküvi-çikolata|poset|60,45,180|||1.00|1.75|12|F1C44F|1
eti_canga|Eti Canga|Eti|bisküvi-çikolata|poset|50,25,150|||0.25|0.40|36|D2232A|0
eti_burcak|Eti Burçak|Eti|bisküvi-çikolata|poset|70,50,200|||0.90|1.40|24|8B5A2B|0
eti_puf|Eti Puf|Eti|bisküvi-çikolata|poset|60,35,160|||0.35|0.55|36|E4007C|0
eti_browni|Eti Browni|Eti|bisküvi-çikolata|poset|70,30,100|||0.45|0.70|24|5C3317|0
torku_banada|Torku Banada 400 g|Torku|bisküvi-çikolata|kavanoz|80,110,60|Etiket,Cam,Kapak|şeffaf cam kavanoz, içi kakaolu krema, kapak|3.20|4.75|12|D71920|0
nestle_damak|Nestlé Damak|Nestlé|bisküvi-çikolata|poset|80,20,160|||0.70|1.10|24|6B3A2A|0
milka_sutlu_80g|Milka Sütlü 80 g|Milka|bisküvi-çikolata|poset|90,12,180|||1.10|1.65|24|7D69AC|0
falim_sakiz|Falım Sakız|Falım|bisküvi-çikolata|poset|30,15,80|||0.08|0.15|100|009FE3|0
lays_klasik|Lay's Klasik|Lay's|atıştırmalık|poset|200,60,280|||1.30|2.00|12|FFD100|0
ruffles_originals|Ruffles Originals|Ruffles|atıştırmalık|poset|200,60,280|||1.40|2.10|12|0055A4|0
doritos_nacho|Doritos Nacho|Doritos|atıştırmalık|poset|200,60,280|||1.40|2.10|12|E4002B|0
cerezza|Çerezza|Çerezza|atıştırmalık|poset|170,50,250|||0.90|1.40|12|F39200|0
peyman_karisik|Peyman Karışık Kuruyemiş|Peyman|atıştırmalık|poset|150,40,220|||3.00|4.50|12|8B1D1D|0
tadim_kavrulmus_findik|Tadım Kavrulmuş Fındık|Tadım|atıştırmalık|poset|150,40,220|||3.20|4.75|12|E30613|0
filiz_spagetti_500g|Filiz Spagetti 500 g|Filiz|makarna-bakliyat|poset|80,35,270|||0.90|1.35|20|D52B1E|0
barilla_spagetti_500g|Barilla Spaghetti n.5 500 g|Barilla|makarna-bakliyat|kutu|72,30,265|||2.10|3.25|12|0055A4|1
nuhun_ankara_spagetti_500g|Nuh'un Ankara Spagetti 500 g|Nuh'un Ankara|makarna-bakliyat|poset|80,35,270|||0.85|1.25|20|E30613|0
pastavilla_spagetti_500g|Pastavilla Spagetti 500 g|Pastavilla|makarna-bakliyat|poset|80,35,270|||0.85|1.25|20|0B5AA6|0
soke_un_2kg|Söke Un 2 kg|Söke|makarna-bakliyat|poset|170,90,300|||2.20|3.25|10|00843D|0
duru_bulgur_1kg|Duru Pilavlık Bulgur 1 kg|Duru|makarna-bakliyat|poset|140,60,230|||1.60|2.40|12|D2232A|0
reis_kirmizi_mercimek_1kg|Reis Kırmızı Mercimek 1 kg|Reis|makarna-bakliyat|poset|140,60,230|||2.40|3.50|12|E30613|0
komili_zeytinyagi_1l|Komili Zeytinyağı 1 L|Komili|yağ-salça|pet_sise|85,290,110|Etiket,Cam,Kapak|şeffaf şişe, içi yeşil-sarı zeytinyağı, kapak|8.00|11.50|12|2E7D32|0
kirlangic_zeytinyagi_1l|Kırlangıç Zeytinyağı 1 L|Kırlangıç|yağ-salça|pet_sise|85,290,110|Etiket,Cam,Kapak|şeffaf şişe, içi zeytinyağı|7.50|10.75|12|00563F|0
yudum_aycicek_1l|Yudum Ayçiçek Yağı 1 L|Yudum|yağ-salça|pet_sise|85,290,110|Etiket,Cam,Kapak|şeffaf PET, içi sarı yağ, sarı/kırmızı kapak|3.20|4.50|12|FFC20E|0
orkide_aycicek_1l|Orkide Ayçiçek Yağı 1 L|Orkide|yağ-salça|pet_sise|85,290,110|Etiket,Cam,Kapak|şeffaf PET, içi sarı yağ|3.10|4.40|12|E4007C|0
tat_domates_salcasi_830g|Tat Domates Salçası 830 g|Tat|yağ-salça|teneke|99,118,118|Etiket,Kapak|teneke kutu, etiket gövdeyi sarar; Kapak = metal üst|3.00|4.25|12|E30613|0
tukas_biber_salcasi_650g|Tukaş Biber Salçası 650 g|Tukaş|yağ-salça|kavanoz|85,125,80|Etiket,Cam,Kapak|cam kavanoz, içi kırmızı salça, metal kapak|3.40|4.90|12|D52B1E|0
tamek_bezelye_konserve|Tamek Bezelye Konservesi|Tamek|yağ-salça|teneke|74,112,112|Etiket,Kapak|teneke kutu; Kapak = metal üst|1.40|2.10|24|00843D|0
bizim_mutfak_mercimek_corbasi|Bizim Mutfak Mercimek Çorbası|Bizim Mutfak|yağ-salça|poset|130,15,170|||0.55|0.85|24|F39200|0
knorr_domates_corbasi|Knorr Domates Çorbası|Knorr|yağ-salça|poset|130,15,170|||0.60|0.90|24|00843D|0
kemal_kukrer_ketcap|Kemal Kükrer Ketçap|Kemal Kükrer|yağ-salça|pet_sise|60,190,90|Etiket,Govde,Kapak|opak kırmızı sıkılabilir plastik şişe, kapak|1.80|2.75|12|D71920|0
calve_mayonez|Calvé Mayonez|Calvé|yağ-salça|pet_sise|60,190,90|Etiket,Govde,Kapak|opak beyaz sıkılabilir şişe, mavi kapak|2.20|3.25|12|0055A4|0
algida_magnum_klasik|Algida Magnum Klasik|Algida|dondurma|poset|100,35,200|||1.60|2.50|24|3E2723|0
algida_cornetto|Algida Cornetto|Algida|dondurma|poset|80,80,200|||1.00|1.60|24|0072BC|0
algida_carte_dor_1l|Algida Carte d'Or 1 L|Algida|dondurma|kutu|160,110,90|||4.20|6.25|6|C8A165|0
ariel_toz_4kg|Ariel Toz Deterjan 4 kg|Ariel|temizlik|kutu|280,110,330|||14.00|19.90|4|00843D|1
alo_toz_4kg|Alo Toz Deterjan 4 kg|Alo|temizlik|kutu|280,110,330|||11.50|16.50|4|E30613|0
omo_toz_4kg|Omo Toz Deterjan 4 kg|Omo|temizlik|kutu|280,110,330|||13.00|18.50|4|0055A4|0
persil_jel_1l|Persil Jel 1 L|Persil|temizlik|pet_sise|95,240,120|Etiket,Govde,Kapak|opak plastik şişe, yeşil/beyaz kapak|6.00|8.75|12|00843D|0
bingo_toz_4kg|Bingo Toz Deterjan 4 kg|Bingo|temizlik|kutu|280,110,330|||10.50|15.00|4|E30613|0
fairy_bulasik_650ml|Fairy Bulaşık Deterjanı 650 ml|Fairy|temizlik|pet_sise|70,230,120|Etiket,Govde,Kapak|yarı saydam plastik şişe (Govde), içi yeşil, beyaz kapak|2.40|3.50|12|00A651|0
pril_bulasik_675ml|Pril Bulaşık Deterjanı 675 ml|Pril|temizlik|pet_sise|70,230,120|Etiket,Govde,Kapak|plastik şişe, kapak|2.20|3.25|12|00A0E3|0
yumos_yumusatici_1_5l|Yumoş Yumuşatıcı 1,5 L|Yumoş|temizlik|pet_sise|110,250,140|Etiket,Govde,Kapak|opak plastik şişe, renkli kapak|3.40|4.90|12|7AC143|0
domestos_750ml|Domestos 750 ml|Domestos|temizlik|pet_sise|80,250,120|Etiket,Govde,Kapak|opak plastik şişe, eğik boyun, kapak|2.60|3.75|12|0055A4|0
cif_krem_500ml|Cif Krem 500 ml|Cif|temizlik|pet_sise|70,190,100|Etiket,Govde,Kapak|opak beyaz plastik şişe, kapak|2.40|3.50|12|FFD100|0
colgate_dis_macunu_100ml|Colgate Diş Macunu 100 ml|Colgate|kişisel bakım|kutu|190,40,38|||2.40|3.50|24|E4002B|0
signal_dis_macunu_100ml|Signal Diş Macunu 100 ml|Signal|kişisel bakım|kutu|190,40,38|||2.20|3.25|24|0072BC|0
ipana_dis_macunu_100ml|Ipana Diş Macunu 100 ml|Ipana|kişisel bakım|kutu|190,40,38|||2.00|2.95|24|00A0E3|0
duru_sabun_4lu|Duru Sabun 4'lü|Duru|kişisel bakım|kutu|190,65,55|||2.00|2.95|12|00A651|0
arko_tiras_kopugu|Arko Tıraş Köpüğü 200 ml|Arko|kişisel bakım|teneke|53,190,170|Etiket,Kapak|aerosol kutu; Kapak = plastik başlık|3.20|4.75|12|0055A4|0
elidor_sampuan_500ml|Elidor Şampuan 500 ml|Elidor|kişisel bakım|pet_sise|75,220,120|Etiket,Govde,Kapak|opak plastik şişe, kapak|4.20|6.25|12|E4007C|0
head_shoulders_500ml|Head & Shoulders 500 ml|Head & Shoulders|kişisel bakım|pet_sise|75,220,120|Etiket,Govde,Kapak|opak plastik şişe, kapak|6.50|9.50|12|0055A4|0
rexona_deodorant|Rexona Deodorant 150 ml|Rexona|kişisel bakım|teneke|50,150,130|Etiket,Kapak|aerosol kutu; Kapak = plastik başlık|3.80|5.50|12|0055A4|0
selpak_tuvalet_kagidi_32|Selpak Tuvalet Kâğıdı 32'li|Selpak|kağıt|poset|400,300,380|||12.00|17.50|2|E4002B|0
solo_havlu_kagit_8|Solo Havlu Kâğıt 8'li|Solo|kağıt|poset|440,250,260|||6.50|9.50|4|0055A4|0
familia_pecete|Familia Kâğıt Peçete|Familia|kağıt|poset|200,100,200|||1.20|1.80|24|E4007C|0
"""
TR = str.maketrans("çğıöşüÇĞİÖŞÜâîû", "cgiosuCGIOSUaiu")

def main():
    products = []
    for line in ROWS.strip().splitlines():
        (pid, name, brand, cat, typ, dims, parts, notes, cost, price, case, color, active) = line.split("|")
        d = [int(x) for x in dims.split(",")]
        pack = {"type": typ}
        if typ in ("kutu", "poset"):
            pack.update(widthMm=d[0], depthMm=d[1], heightMm=d[2])
        else:
            pack.update(diameterMm=d[0], heightMm=d[1], labelHeightMm=d[2])
        if parts: pack["parts"] = parts
        elif typ == "poset": pack["parts"] = "Etiket"
        if notes: pack["notes"] = notes
        pack["estimated"] = True
        p = {"id": pid, "realName": name, "fictionalName": name, "category": cat,
             "cost": float(cost), "price": float(price), "caseUnits": int(case), "color": color}
        if active != "1": p["active"] = False
        p["brand"] = brand
        p["package"] = pack
        products.append(p)
    ids = [p["id"] for p in products]
    assert len(ids) == len(set(ids)), "duplicate id"
    assert sum(1 for p in products if p.get("active", True)) <= 24
    out = {"schemaVersion": 2,
           "note": "Fiyatlar oyun dengesi icin kurgusaldir. Olculer tahminidir (estimated). Urun Studyosu yonetir; active:false = hazirlik listesi.",
           "products": products}
    # Same line layout as the studio serializer: one product per line.
    lines = ["{", '  "schemaVersion": 2,', '  "note": ' + json.dumps(out["note"], ensure_ascii=False) + ",", '  "products": [']
    for i, p in enumerate(products):
        lines.append("    " + json.dumps(p, ensure_ascii=False, separators=(",", ":")) + ("," if i + 1 < len(products) else ""))
    lines += ["  ]", "}"]
    path = pathlib.Path(__file__).resolve().parent.parent / "Config" / "products.json"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(len(products), "urun;", sum(1 for p in products if p.get("active", True)), "oyunda ->", path)

if __name__ == "__main__":
    main()
