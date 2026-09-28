# Sipariş ve mal kabul

## Yönetim masası

- `TAB / Q`: ürün seç.
- `B`: seçili üründen bir koliyi sipariş listesine ekle.
- `V`: seçili üründen bir koliyi listeden çıkar.
- `L`: önerilen siparişi listeye yaz. Öneri: dünkü satış + rafı boş bulan her müşteri için ~2 adet, %25 pay;
  raftaki, depodaki, arka kapıdaki ve yoldaki mal düşülür; ürün başına 120 adet depo sınırı ve 9 koli.
  Listede daha fazlası varsa azaltmaz. Rafta yeri olmayan ürüne öneri çıkmaz (önce R ile reyona koy).
- `N`: listedeki bütün ürünleri tek sipariş olarak onayla. Toptancı en az **50 TL**'lik siparişle gelir.

Masa kartında seçili ürün için raf, depo, kabul, yolda, dünkü satış, boş raf ve öneri tek satırda görünür.

Onay atomiktir: nakit veya ürünün depo sınırı yetmiyorsa hiçbir satır satın alınmaz. Ödeme onayda
yapılır. Stok tablosundaki **YOLDA** sütunu onaylanan, **KABUL** sütunu arka kapıya ulaşan ürünleri
gösterir.

## Ertesi sabah

Siparişler gün kapanınca arka kapıda karton koli olarak görünür. Ürün doğrudan depo stoğuna geçmez.
Kolinin yanına gidip `E` ile al, arka depodaki kabul noktasına götür ve yeniden `E` ile bırak.

Reyon görevlisi varsa sabah önce mal kabul kolilerini arka kapıdan depoya taşır, sonra raf işlerine
döner. Her seferinde ürünün bir kolisi taşınır.

Tedarikçi bazen bir ürünü eksik veya hasarlı getirir. Bu adetler gün sonu bildiriminde gösterilir ve
stoğa girmez. Sonuç gün ve ürün kimliğine bağlıdır; kaydı yeniden yüklemek sonucu değiştirmez.
