# Doğrulama — 27 Eylül 2026

- Unreal Engine 5.8.3 / Win64 Development Editor hedefi başarıyla derlendi.
- Unreal Automation: 4 test başarılı, 0 başarısız, 0 atlanmış test.
- Stok ve sipariş: stok korunumu, ön ödeme, ertesi gün teslimat, yetersiz nakit ve depo kapasitesi doğrulandı.
- Satış: stoktan fazla satışın reddi, sepetteki fiyatın korunması, ciro/maliyet/nakit ve gün sonu ayrımı doğrulandı.
- Kayıt: bellekte serileştirme/yükleme, personel ve marka tercihi, bozuk stok ve değişen ürün kimliğinin reddi doğrulandı.
- Oyun dünyası kontrolü: oyuncu oluştu; raf ve yönetim masası etkileşimleri çalıştı; sipariş ve kasiyer alımı tamamlandı; 4 müşteri satışı gerçekleşti; gün kapandı ve disk kaydı yeniden yüklendi. Süreç çıkış kodu 0.
- 1280×720 oyun görüntüsü gerçek Unreal render çıktısından alındı ve incelendi. Raf yazıları ve tavan düzeltildikten sonra görüntü yeniden kontrol edildi.

Otomasyon raporu: `Saved/TestReports/index.json`.
Oyun oturumu günlüğü: `Saved/Logs/GameplaySmoke.log`.
Görsel kontrol günlüğü: `Saved/Logs/VisualCheck.log`.

Bu doğrulamalar elle tüm gün oynama, uzun kampanya dengelemesi, donanımlar arası performans testi veya paketlenmiş Windows dağıtımı testi içermez. İkinci şubeye ulaşma süresi henüz uzun oyun oturumlarıyla ayarlanmadı.

![İlk prototipin Unreal içindeki görünümü](Images/prototip.png)
