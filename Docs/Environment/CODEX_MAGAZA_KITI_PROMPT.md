# Codex görev metni — G-088 Mağaza kiti

Aşağıdaki metni Codex'e olduğu gibi yapıştır.

```text
Miras Market projesinde yeni bir görevin var: G-088 Mağaza kiti. Oyunda şube açıldığında oyuncunun gezebileceği hazır mağaza görünümlerini yapacaksın. Dört market türü var (kucuk = ucuzcu, mahalle, buyuk = süpermarket, hiper = hipermarket); her türe 5 mağaza, toplam 20.

ÖNCE OKU (bu sırayla)
1. AGENTS.md (özellikle 1, 2, 6 ve 8. bölümler: oturum kuralları, "bitti" şartı, dosya sahipliği)
2. Docs/Kurgu/04_MAGAZA_KITI.md  ← bu görevin sözleşmesi, her şey burada
3. Docs/Surec/DURUM.md ve GOREVLER.md (G-088 satırı)
4. Docs/Environment/BLENDER_STANDARDI.md, CLAUDE_BLENDER_PROMPT.md, equipment_template.json
5. Referans kod: Tools/Blender/create_store_kit.py, Source/MirasMarket/Planogram.h/.cpp, MarketLayout.h, MarketAutomation.cpp

NE YAPACAKSIN
- Config/magazalar.json: 04_MAGAZA_KITI.md §4 şemasıyla. Her mağazada kabuk, giriş/arka kapı/depo/oyuncu başlangıç noktaları, ekipman listesi (id, equipment, category, konum, yön), dekor ve tema.
- Blender: mağaza kabukları ve eksik ekipmanlar (tek ve çok kasa bandı, dondurucu, şarküteri tezgâhı, fırın rafı, palet teşhir, manav tezgâhları, sepet/araba alanı vb.). Mevcut Blender standardına uy (metre, -Y ön, zemin merkezi orijin, UCX, equipment.json).
- Source/MirasMarket/MarketStoreKit.h/.cpp: Load, TemplatesFor, Find, ToPlanogram, Build, Clear (§5). Build mağazayı dünyaya kurar; raflara ürünleri MarketLayout::Plan'ın doldurduğu planogramdan dizer. Ürün ve tekrarlı ekipman instanced çizilir.
- Planogram: yeni ekipmanlar equipment.json'dan okunsun, sabit kodlanmasın. Var olan gondola_double_1200 ve wall_shelf_2400 davranışı değişmesin.
- Tools/validate_stores.py: magazalar.json'u doğrular ve "stats" alanını ekipmanlardan HESAPLAR (elle yazma). Tür bantlarının ve zorunlu bölümlerin dışına çıkan mağaza hata verir.
- MarketStoreKitTests.cpp: json okuma, bant/zorunlu bölüm kontrolü, ToPlanogram + MarketLayout::Plan ile her mağazanın doldurulabildiği, benzersiz id.
- Raf kategorisi seçimi (04_MAGAZA_KITI.md §7, bütün mağazalarda, aile dükkânı dahil): rafın üstündeki tabelaya bakınca "Kategoriyi değiştir [T]"; T ile products.json'daki kategorilerin listesi (Türkçe adlarla) + "Kategorisiz"; çift yüzlü gondolda her yüz ayrı. Seçince tabela hemen değişir, reyon görevlileri yeni kategoriye göre çalışır, raftaki eski ürünler silinmez ama "bu reyona ait değil" uyarısı çıkar. Aile dükkânında planograma yazılır; şube mağazaları için MarketStoreKit'te "fikstür id -> kategori" eşlemesi alan bir giriş bırak (kaydını Claude bağlayacak).
- Türkçe tabela: 3B yazılarda FoldTurkish/ASCII kalksın. Plex Sans SemiBold'dan Türkçe harfli önbellekli UFont (A-Z, a-z, 0-9, ÇĞİÖŞÜçğıöşü, ₺ ve noktalama) editör Python betiğiyle üret; tabela, fiyat etiketi ve öteki 3B yazılar bunu kullansın. Büyük harf Türkçe kuralıyla (i -> İ, ı -> I): MarketCatalog::UpperTurkish yaz ve testle.
- MarketAutomation.cpp: -MirasStorePreview=<id> (mağazayı kur, otomatik diz, 5 açıdan 1280x720 görüntü al, Saved/Screenshots/Stores/<id>/).

SIRA
- Aşama A: Türkçe tabela + raf kategorisi seçimi (önce aile dükkânında), sonra okuyucu + kurucu + doğrulama + her türden BİR mağaza (mahalle_01, kucuk_01, buyuk_01, hiper_01). Bitince dur, önizleme görüntülerini Mustafa'ya göster, onay al.
- Aşama B: onaydan sonra kalan 16 mağaza. Aynı türün 5 mağazası bina biçimi, giriş yönü, kasa hattı ve tema bakımından belirgin farklı; alanları bant içinde küçükten büyüğe dağılsın.

DOKUNMA (Claude'un alanı)
MarketBranches.*, MarketCompany.*, MarketEconomy.* (FMarketState), MarketLayout.cpp, MarketMenu*.*, MarketHudWidget.*, MarketStory.*, MarketDirector.*, MarketSimulation.*, Config/ulkeler.json, Config/products.json.
Mağaza atama, kayıt, menüdeki "gez" düğmesi ve stats sayılarının oyun hesabına bağlanması Claude'da. Bu dosyalarda değişiklik gerekirse yapma; GOREVLER.md G-088 notuna ve GUNLUK.md'ye yaz.

KALİTE
- 1080p'de 60 fps (hipermarket en az 45). Işık mevcut Lumen ayarıyla, tema başına bir ışık önayarı.
- Logo/marka uydurma yok; tabelalarda reyon adları.
- C++ kaynakları ASCII (Türkçe metinden sonra python Tools/escape_unicode.py).
- Aile dükkânının (mevcut elle dizilen dükkân) düzeni değişmez; ona yalnız raf kategorisi seçimi ve Türkçe tabela eklenir. Smoke testi eskisi gibi geçmeli.

BİTTİ ŞARTI
DERLE.cmd /q, TEST.cmd /q ve SmokeTest.ps1 geçmeli; her mağaza için -MirasStorePreview görüntüleri alınmış ve gözle incelenmiş olmalı. Sonra DURUM.md devam notu, GUNLUK.md (en üste) ve GOREVLER.md G-088 satırını güncelle, anlamlı bir commit at.
```
