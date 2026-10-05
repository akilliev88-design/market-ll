# Mahalle marketi — Image-blaster denemesi

05.10.2026. Ortam üretildi. Collider Unreal'a ayrı klasöre aktarıldı; gerçek oyun karakteri zeminde duruyor (GENERATED_STORE_PHYSICS_PASSED). Dokulu HQ model World Labs'ta işleniyor. Görsel model aktarımı, ekran görüntülerinin incelenmesi ve kampanya bağlantısı tamamlanmadı.

## Son devam durumu

Codex yerel arka plan yardımcısını başlattı: `Tools/ImageBlaster/FinalizeGeneratedStore.ps1`. Mevcut HQ işi tamamlanınca indirir; Blender ile ölçü/doku/zemin kontrolünü yapar, Unreal'a aktarır ve üç kontrol görüntüsü alır. Durum: `Saved/ImageBlaster/finalize-status.json`. `review_ready` yalnız aktarım ve zemin kontrolünün geçtiğini söyler; görsel inceleme ve kusur düzeltmesi bekler. `failed` olursa mesaj/log incelenir.

Pencere kapanması World Labs işini iptal etmez. [Resmî mesh kılavuzu](https://docs.worldlabs.ai/marble/export/mesh) HQ işleminin bir saate kadar sürebileceğini belirtiyor. Yerel yardımcı durduysa `MARKET_AKTARIM_TAMAMLA.cmd` ile aynı işe devam edilir. Unreal açıkken aktarım yapmaz; kullanıcı editörü kapatıp yeniden çalıştırır.

Görsel model ve fizik ayrı: görünür HQ modelin çarpışması kapalı; ~38 bin üçgenli collider gizli fizik yüzeyi olarak kullanılır. Kaynak collider `AssetInbox/ImageBlaster/MahalleMarket/mahalle-market-collider.glb`, Unreal varlığı `/Game/Stores/Generated/MahalleCollision`. Giriş/cam kusurları henüz görsel olarak incelenmedi. Üretici mekânı ilk hedefteki 8×10 m dükkândan daha uzun yorumladı.

`MARKET_GEZI.cmd` ayrı Unreal gezi oturumunu açar (WASD/fare, Esc). Dokulu model yoksa başlamaz. Bu sahne satış, müşteri ve kampanya kaydı oluşturmaz; ana oyunun mağazası henüz değiştirilmedi.

Bu tur DERLE geçti; TEST 173 başarılı + 1 uyarı, 0 başarısız; Smoke geçti. Sonraki fizik kontrolü ve kaynak sürümü de derlendi, GENERATED_STORE_PHYSICS_PASSED. Son görsel aktarım/zemin kontrolü arka plan betiğinde; sonuç ve PNG'ler sonraki incelemede okunacak.

## Hazırlananlar

- `AssetInbox/ImageBlaster/MahalleMarket/mahalle-market-konsept.png`: cam cephe, açık sokak girişi, gün ışığı, düşük raflar, kasa ve dolaplarla görünüm hedefi.
- `AssetInbox/ImageBlaster/MahalleMarket/mahalle-market-bos.png`: aynı kamera ve mimari; raf/kasa/dolap/ürünler çıkarıldı. 3B üretime bu dosya verilir. Oyunun ekipmanları sonra yerleştirilir.
- İki görsel yerleşik imagegen ile üretildi; istemler aynı klasörde `istemler.json` içinde.
- Araç: `Saved/ImageBlaster/image-blaster`. Kaynak commit: `4acb43ba126a12358f71838d1b1a05e856b10eaf`.
- Windows çalıştırıcı: `Tools/ImageBlaster/market-world.mjs`. Aracın üretim fonksiyonunu doğrudan çağırır; bu deneme için Claude Code, Bun veya FAL hesabı gerekmez. Node.js bilgisayarda mevcut.

## İlk kurulum — Mustafa

1. [World Labs Platform](https://platform.worldlabs.ai) üzerinden hesap aç/giriş yap.
2. Platformdaki API anahtarı bölümünden anahtar oluştur. Anahtarı sohbet mesajına yazma.
3. Platformun Billing bölümüne API kredisi ekle. Marble web uygulamasının abonelik/kredileri API için geçmez. Minimum API kredi satın alımı şu anda 5 USD / 6250 kredi.
4. Projede `Saved/ImageBlaster/image-blaster/.env` dosyasını Not Defteri ile aç. `BURAYA_ANAHTARINI_YAZ` yerine API anahtarını yaz ve kaydet. Bu dosya Saved altında olduğundan proje Git kaydına girmez.

Güncel API fiyatı: standart Marble 1.1 + tek normal görüntü 1580 kredi (~1,264 USD); dokulu HQ GLB dışa aktarımı 3500 kredi (2,80 USD). Tek ortam + HQ model toplamı yaklaşık 5080 kredi / 4,064 USD. Yeniden üretimler ek ücretlidir. Çalıştırıcı mevcut ortamı kullanır ve otomatik yeni varyant üretmez. Ücretler: [World Labs API pricing](https://docs.worldlabs.ai/api/pricing).

## Çalıştırma

1. Proje kökündeki `MARKET_URET.cmd` dosyasına çift tıkla. Boş market görseli otomatik kullanılır. İş birkaç dakika sürebilir. Anahtar eksikse başlamadan durur. Pencereyi kapatma.
2. Üretim bitince `MARKET_ONIZLE.cmd` dosyasına çift tıkla; tarayıcıda `http://127.0.0.1:5173/mahalle-market` adresini aç. Önizleme penceresi açık kalmalı. Gerçek üretilmiş dünyayla gezinme henüz doğrulanmadı.
3. Unreal'a uygun dokulu model için `MARKET_MODEL_AKTAR.cmd` dosyasına çift tıkla. Bu adım ayrı HQ model dışa aktarım ücretini harcar. Sonuç `Saved/ImageBlaster/UnrealImport/mahalle-market-textured.glb` olur. Başarı/hata bilgileri pencereye yazılır.

Ortam dosyaları `Saved/ImageBlaster/image-blaster/worlds/mahalle-market/output/world` altında. Standart ortam GLB'si **collider** modelidir; güzel görünen dokulu modelle karıştırma. SPZ splatları ayrı dosyalardır. Windows giriş çağrısı doğrudan modül importu kullanır.

## Oyuna aktarımın kalan adımı

Dokulu HQ GLB elde edilince önce ayrı Unreal test sahnesine alınır. Ölçek/yön, camlar, giriş boşluğu, zeminin yürünebilirliği, malzemeler ve kamera hareketinde görüntü kontrol edilir. Mevcut oyunun rafları/kasası/ürünleri içine yerleştirilir; müşteri yolları ve mal kabul kapısı ayrıca düzenlenir. Sonra oyun bağlantısı, DERLE, TEST ve Smoke gerekir. Kaynak görseldeki 8×10 m hedefi üretilen modelde garanti değildir.

HQ mesh, splat görüntüsünün birebir aynısı olmayabilir. Üretilmiş dokulu mesh yerine SPZ kullanılırsa UE 5.8 ile uyumlu splat eklentisi ayrıca seçilip doğrulanmalıdır; mevcut projede böyle bir entegrasyon bulunamadı. API üretimi alınmadan sahneye varlık eklenmiş sayılmaz.

## Doğrulama

Görseller incelendi: cam cephe, sokak girişi ve boş sürüm korunuyor. Çalıştırıcı sözdizimi/yerel kaynak kontrolü ve eksik anahtarda durması kontrol edildi. Önizlemenin TypeScript/Vite derlemesi başarılı; yerel sunucu ve `/mahalle-market` HTTP 200 geçti. Gerçek üretilmiş 3B dünya henüz yok; bu kontrol yalnız sunucunun açıldığını doğrular. Depodaki npm kilit dosyası eskiydi; Saved altındaki araç kopyasında `npm install --ignore-scripts --workspaces=false --no-audit --no-fund` ile bağımlılıklar kuruldu. Ücretli servis çağrısı yapılmadı. Oyun kodu veya uasset değişmedi; DERLE/TEST/Smoke çalıştırılmadı.

Kaynaklar: [Image-blaster](https://github.com/neilsonnn/image-blaster), [HQ mesh export API](https://docs.worldlabs.ai/api/reference/worlds/export).
