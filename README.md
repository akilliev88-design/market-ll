# MarketSim (çalışma adı)

Unreal Engine 5.8.3 ile yapılan, **küçük bir marketten dünya devine** uzanan bir işletme simülasyonu ve tycoon. Football Manager gibi sürer: zorunlu bölüm ya da oyun sonu yoktur, hikâyeyi oyuncunun kararları yazar. Satış adı henüz seçilmedi; oyuncuya görünen ad `Config/DefaultGame.ini` → `ProjectName` satırından gelir. İç kod adı **MarketSim**'dir ve satış adından bağımsızdır.

## Başlangıç

Yeni oyunda oyuncu sırayla seçer: **ülke**, **şehir**, **marketin adı**, **kendi adı** ve **zorluk** (Rahat / Normal / Zor).

İlçede tek şubeli küçük bir market vardır. Onu yıllarca işleten aile yoruldu ve işi sana devretti. Bina artık senin; bir kasiyer, iki reyon görevlisi, yarı dolu raflar, kasada bir aylık gider ve toptancıya hafif bir borç gelir. Borcun süresi ve faizi yoktur, hiçbir şeyi kilitlemez; ne zaman ödeyeceğine sen karar verirsin. Sonrası senin hikâyen: şubeler, iller, ülkeler, rakip zincirler ve dünya listesi.

## Oyna

`OYNA.cmd` dosyasına çift tıkla (bu makinedeki Unreal kurulumunu kullanır) ya da PowerShell'de:

```powershell
cd C:\Users\mtass\Desktop\market-ll
.\Play.ps1
```

İlk derleme gerekiyorsa `DERLE.cmd` (veya `Build.ps1`). Kaynak kod değişince yeniden derle. Motor başka klasördeyse betiklere `-EngineRoot 'D:\Epic\UE_5.8'` verilebilir. Proje henüz bağımsız bir Windows oyunu olarak paketlenmedi; Unreal kurulumu gerekir.

## Temel tuşlar

| Tuş | İşlev |
|---|---|
| WASD / fare | Hareket / bakış |
| E | Yakındaki rafı doldur, kasada ödeme al, koliyi taşı |
| O | Mağazayı aç / erken kapat |
| M | Yönetim menüsü (sipariş, fiyat, personel, finans, şubeler, raporlar) |
| R | Raf dizme modu |
| P | Devralınan borçtan bir taksit öde (tamamı: menü › Kararlar) |
| Space, 1 / 2 / 3 | Zamanı durdur, oyun hızı |
| F5 / F9 | Kaydet / yükle |

## Ürün Stüdyosu

Dışarıda hazırlanan kutu/etiket görsellerini ve modelleri oyuna eklemek için `STUDYO.cmd` (veya Unreal Editor'da **Tools > Ürün Stüdyosu**). Ayrıntı: [Docs/URUN_STUDYOSU.md](Docs/URUN_STUDYOSU.md).

## Birlikte geliştirme

Claude ve Codex aynı klasörde sırayla çalışır. Kurallar [AGENTS.md](AGENTS.md); güncel durum [Docs/Surec/DURUM.md](Docs/Surec/DURUM.md), görevler [Docs/Surec/GOREVLER.md](Docs/Surec/GOREVLER.md), oturum geçmişi [Docs/Surec/GUNLUK.md](Docs/Surec/GUNLUK.md). Güncel kurgu [Docs/Kurgu/00_KURGU_KITABI.md](Docs/Kurgu/00_KURGU_KITABI.md), bütün kararlar [Docs/Kurgu/01_KARARLAR.md](Docs/Kurgu/01_KARARLAR.md).

Testler: önce derle, sonra `Test.ps1` (rapor `Saved/TestReports`). Gerçek oyun dünyasında otomatik kontrol: `SmokeTest.ps1`. Otomatik oyuncu ve denge raporu: `AUTOPLAY.cmd`.
