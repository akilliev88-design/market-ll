> **29.09.2026 güncel durum (Claude, Cowork):** Aşağıdaki kalan işler 1-5 yapıldı ve dosyalar `Source/MirasMarket`'e kondu
> (`MarketMenuWidget.*`, `MarketMenuPages.cpp`, `MarketMenuInternal.h`, `MarketMenu.cpp`, `MarketHudWidget.*`, `MarketMap.*`,
> `MarketRetail.*`, `MarketDirector.*` → `PromoteTo`). **Derlenmedi.** Bu klasör artık yalnız arşiv; güncel kaynak Source'ta.
> Ayrıntı: `Docs/Surec/GUNLUK.md` ve `Docs/MENU.md`.

# G-074 devam notu (menü entegrasyonu yarım kaldı)

Tarih: 2026-09-29. Hazırlayan: Claude (Cowork). **Hiçbiri derlenmedi.**

## Oyuna konanlar (Source/MirasMarket, derlenmedi)
Hata düzeltmeleri (G-074 inceleme): MarketEconomy.h/.cpp (PendingLoss, Received, LateSince, ReconcileWith kampanya eşlemesi),
MarketSuppliers.cpp (geç fatura bir kez işaretlenir), MarketBranches.h/.cpp (KDV'li şube satışı, kasa sınırlı sipariş,
şube kapanınca mal depoya / yarı fiyata), MarketDirector.cpp (yeni Close imzası, tedarikçi sırası), MarketFinance.cpp,
MarketStaff.cpp (vergi cezası), MarketCredit.cpp (tahsil zarı müşteriye bağlı), MarketFreshness.cpp (partiler Received ile),
MarketGame.h/.cpp (3 al 2 öde bütçesi, kapanışta kuyruk, DropCarriedDelivery, Migrate, kapasite iadesi, masa ipuçları M menüye,
FMarketTodo + Todos() bildirimi), MarketDelivery.cpp, MarketVisuals.cpp (SurfaceSpecOf), MarketStoryTests.cpp,
MarketSimulation.cpp, MirasMarket.Build.cs (Config/iller.json), Config/iller.json (81 il haritası).

Not: MarketGame.h'de `Todos()` bildirildi ama tanımı yeni MarketMenu.cpp'de (bu klasörde). Kimse çağırmadığı için
eski menüyle de bağlanır.

## Bu klasördeki yarım işler (menü, henüz Source'a konmadı)
- `Source/` : yeni sürümler — MarketMenuWidget.h (10 sayfa, yeni bloklar), MarketMenuInternal.h, MarketMenu.cpp (Todos),
  MarketHudWidget.h/.cpp (sade HUD, A1), MarketMap.h/.cpp (Türkiye haritası, SLeafWidget), MarketRetail.h/.cpp (ulusal/uluslararası zincir payları).
  Bunlar birlikte Source/MirasMarket'e konmalı; tek başına HUD veya MarketMenu.cpp eski menüyle derlenmez.
- `parcalar/` : eski MarketMenuWidget.cpp'nin parçaları ve yeni yazılan çerçeve:
  - pA.cpp (MarketMenuUi yardımcıları, include'lar), pB.cpp (tema ve bloklar), pNew1.cpp (PageName, Ask, RiskyButton, Choice,
    Section, Why, PictureBrush, ProductPicture, Construct 10 sayfa + ConfirmLayer, OnKeyDown, OnMouseButtonDown, NavItem,
    Sidebar (zorluk/salgın/tema), Header), pGoalDecStory.cpp, pMoneyTimeOnline.cpp, pOrders.cpp, pPagesOld.cpp (Fiyat,
    Rakipler, Personel, Şubeler eski), pReports.cpp, menu_old.cpp (tam eski dosya).
  - pNew1.cpp'de Türkçe harfler gerçek harf; C++'ta \uXXXX'e çevrilmeli (AGENTS.md). ed.py'deki esc() bunu yapar.

## Kalan işler
1. MarketMenuWidget.cpp = pA + pB + pNew1 + GoalList/DecisionCard/StoryCard + yeni SummaryPage (A2: sol istatistik/kısayol
   daireleri/aç-kapa/zaman, sağ DecisionCard + TodoList + rakip kartı + borç/hedef + StoryCard) + TodoList + Orders (indeks
   korumaları) + Reports. Header'da çağrılan `SMarketMenu::` bloklarının (Label, LabelBy, Fixed, Card, Button, Badge...) pB'de olduğu kontrol edilmeli.
2. Yeni MarketMenuPages.cpp: Fiyat (ürün resmi, kampanyasız), Kampanyalar (her kampanyaya Durdur, teklif, reklam),
   Rakipler sekmeleri (yerel/ulusal/uluslararası), Personel (Kov = RiskyButton, vergi Finans'a), Finans (kredi 500/1000/2500
   sorulu, veresiye 0–3, tahsil, vergi/muhasebeci, taze ürün politikası 0/1/2, TroubleStage, ev harcaması), Satış kanalları,
   Şubeler sekmeleri (MapTab, LocalBranchesTab: kapat=RiskyButton, müdür seçici, 3 format, CanOpen nedeni; CompanyTab).
3. Director'a "PromoteTo" eylemi: Arg = BranchIndex*1000000 + EmployeeId. CloseStore, PandemicProfile, FreshPolicy 0,
   CreditLimit 3, TakeLoan 2, OpenBranch format 2 menüden erişilebilir olmalı.
4. Tüm yeni dosyalarda ASCII kontrolü, gölgeleme (C4456–C4459) kontrolü, unity build için adlandırılmış namespace.
5. Docs/MENU.md, AGENTS.md haritası, DURUM/GOREVLER/GUNLUK ("derlenmedi"), sonra SON_KONTROL.cmd.

Derleme riski olan yerler: FSlateVertex alanları, GetResourceHandle, MakeCustomVerts/MakeLines imzaları (MarketMap.cpp),
FSlateBrush::SetUVRegion, FSlateLayoutTransform, FBoxPackageLayout kullanımı (pNew1 PictureBrush).
