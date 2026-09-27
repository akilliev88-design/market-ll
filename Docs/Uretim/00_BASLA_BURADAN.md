# Dış üretim — buradan başla

Dışarıda (başka bir yapay zekâ ajanında) üretilecek her şeyin metni **Ürün Stüdyosu'nda, seçtiğin ürüne göre hazırlanır**. Elle doldurman gereken bir şey yok.

## Akış

1. `STUDYO.cmd` → soldan **Hazırlık** listesinden ürünü seç (97 ürün hazır; bkz. `MARKA_VE_URUN_LISTESI.md`).
2. Sağda **AMBALAJ** bölümünde hazır ambalajlardan birini seç (her ürüne bir tane önceden atandı; istersen değiştir). Stüdyo 3B şeklini kendisi yapar: kutu/poşet için kutu, şişe/teneke/kavanoz/kase için gerçek kapaklı şekil. Ölçü girmen gerekmez.
   - Karton kutuda kapak varsa **"Üstte kapak var"** açık olsun: kapak etiket görselinin üst yüzüne çizilir.
   - Şişe/kavanoz/kasede **PARÇA RENKLERİ**: cam/içerik rengi, saydamlık, kapak rengi (HEX).
   - Listede olmayan ambalaj: **"Listede yok: özel ambalaj"** (ölçü + kutu şablonu ya da kendi 3B modelin).
3. **DIŞ ÜRETİM** bölümünde dönemi (2011 / 2018 / 2025 / 2033) ve promptu seç:
   - **Kutu ve poşet:** A1 (tek görsel açılım, önerilen) · A2 (6 ayrı yüz) · A3 (sınır düzeltme; normalde gerekmez)
   - **Şişe, teneke, kavanoz, kase:** B2 (açılmış etiket + kapağın üstten görseli). Model promptu gerekmez; B1 yalnız özel model içindir.
   - Hepsi için: D (dönem araştırması), E (teslim kontrolü)
4. Hazır ambalaj kullanıyorsan **Şablonları oluştur**. Açılan klasördeki `acilim_sablonu.png` ya da `label_sablonu.png` (+ gerekiyorsa `kapak_sablonu.png`) dosyalarını ajana yükle.
5. **Promptu kopyala** → ajana yapıştır. Ürün adı, marka, ölçüler, kesin piksel ölçüleri, yüklenen şablonun kullanım kuralı, kapak kuralı ve teslim klasörü hazır yazılı.
6. **Teslim klasörünü aç** → ajanın dosyalarını oraya koy (`Uretim/<ürün>/<yıl>/`).
7. (İsteğe bağlı) E promptunu çalıştır; "YÜKLEMEYE HAZIR" görene kadar düzelttir.
8. Stüdyoda alttaki kutucuklara yükle:
   - Kutu/poşet: **Tek görsel (açılım)** → `acilim.png` (stüdyo panelleri kendisi bulur).
   - Şişe vb.: **Etiket** → `label.png`, **Kapak (üstten)** → `kapak.png`.
9. **Oyuna ekle** (ya da şimdilik **Kaydet**, hazırlıkta kalsın). Oyunda en fazla 24 ürün olabilir; yer açmak için bir ürünü **Oyundan çıkar**.

Oyun şu an tek etiket gösterir: başlangıç yılı için **2011** setini yükle. Diğer dönem klasörleri G-011 tamamlanınca kullanılacak.

## Şablonlar

Prompt metinleri `Docs/Uretim/Sablonlar/*.txt` dosyalarındadır (`{{URUN_ADI}}` gibi yer tutucularla). Stüdyo bunları doldurur. Metni değiştirmek istersen bu dosyaları düzenle; stüdyo bir sonraki seçimde yeni metni kullanır. Ortak bölümler: `_urun.txt` (ürün bilgisi), `_genel.txt` (genel kurallar), `_donem.txt` (dönem kuralları).

## Klasör düzeni

```text
Uretim/
  <urun_kimligi>/
    model/            ← yalnız özel model (B1): model.fbx, malzeme.json, uv_sablon.png, model_rapor.md
    2011/             ← stüdyo kılavuzları + A1/A2 veya B2 çıktısı: acilim.png | 6 yüz | label.png (+kapak.png); kaynak.md
    2018/ …  2025/ …  2033/ …
```
