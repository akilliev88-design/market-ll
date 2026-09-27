# Görev panosu

Durumlar: **Sırada**, **Devam ediyor**, **Doğrulama bekliyor**, **Bitti**. Sahip: Mustafa / Claude / Codex / Açık.
Yeni görev en alta eklenir; biten görev silinmez, durumu değişir. Kimlikler tekrar kullanılmaz.

| ID | Görev | Sahip | Durum | Not |
|---|---|---|---|---|
| G-001 | Ortak devir sistemi (AGENTS.md, CLAUDE.md, Docs/Surec) | Claude | Bitti | 27.09.2026 |
| G-002 | Katalog v2: 6 ürün sınırını kaldır, raf/mağaza ürüne göre büyüsün, kayıt id ile uzlaşsın, koli adedi | Claude / Codex | Bitti | 27.09.2026; derleme, 7/7 test ve smoke geçti |
| G-003 | Ürün Stüdyosu v1 (Unreal Editor paneli): kutu şablonu, model içe alma, yüz/UV etiketi, 3B önizleme, Oyuna ekle | Claude | Doğrulama bekliyor | v1 derlendi; v1.1 değişiklikleriyle yeniden derleme + elle deneme gerekli |
| G-004 | Git deposu kur (`git init`, ilk commit, `.gitattributes` ile uasset/png/fbx için LFS kararı) | Codex | Bitti | 27.09.2026; Unreal çıktıları hariç, ikili varlıklar Git LFS ile izleniyor |
| G-005 | Kutu yüzlerinin dışa doğru baktığını ve etiketlerin ayna olmadığını editörde gözle doğrula | Mustafa / Codex | Bitti | 27.09.2026; milk_1l ekran görüntüsünde ön/yan/üst doğru, ayna yok |
| G-006 | Özel modeller için UV şablonu dışa aktarma (etiket hazırlarken kılavuz PNG) | Codex | Bitti | 27.09.2026; Etiket yuvası UV0 → 2048 px PNG, derleme ve UvTemplateExport testi geçti |
| G-007 | Ölçek/pivot düzeltme alanı (içe alınan model yanlış birimle gelirse) | Codex | Bitti | 27.09.2026; ölçek + Pitch/Yaw/Roll + pivot/raf XYZ, katalog/önizleme/oyun bağlantısı ve test geçti |
| G-008 | Oyunda rakip indiriminin 5 günün 4'ünde açık olması (Day % 5 <= 3) — tasarım kararı | Mustafa | Sırada | İlk incelemede bulundu |
| G-009 | Müşterisiz gün yerel payı düşürüyor (memnuniyet 0 sayılıyor) | Codex | Bitti | 27.09.2026; ziyaretçi yoksa pay korunuyor, otomasyon testi eklendi |
| G-010 | Kuyruk sırası dizi sırasına göre; yoldaki müşteri önce bekleyenin önüne geçebiliyor | Codex | Bitti | 27.09.2026; varış bileti/rütbesi ve QueueArrivalOrder testi eklendi |
| G-011 | Dönem etiketleri: ürün başına 2011/2018/2025/2033 etiket seti; oyun yılı ilerledikçe raftaki etiket değişsin | Açık | Sırada | Dosya düzeni hazır: `Uretim/<ürün>/<yıl>/`; stüdyo + katalog + oyun takvimi gerekiyor |
| G-012 | Dış üretim prompt paketi (`Docs/Uretim/`) + marka/ürün listesi | Claude | Bitti | 27.09.2026; Planlama/07 arşive alındı |
| G-013 | Stüdyo v1.1: tek görsel kutu açılımı (otomatik bölme), model yuvaları Etiket/Cam/Kapak/Govde, malzeme.json, cam saydam ana materyal, katalogda yuva bazlı `materials` | Claude | Doğrulama bekliyor | güncel toplam derleme ve 9/9 test geçti; cam şişe ile elle deneme bekliyor |
| G-014 | Stüdyo v1.2: acilim.json panel sınırları, %15 oturtma, önizleme ışığı; A3 promptu | Claude / Codex | Bitti | 27.09.2026; derlendi, milk_1l yeniden yayımlandı ve oyunda yüz yerleşimi doğrulandı |
| G-015 | Stüdyo v1.3: 97 ürünlük katalog, hazırlık listesi, ürüne göre doldurulan dış üretim promptları | Claude | Doğrulama bekliyor | Derleme + 9/9 test geçti; bir ürünle elle uçtan uca deneme bekliyor |
| G-016 | Ürün ölçülerini doğrula (hepsi tahmini); D promptu gerçek ölçüleri raporlar | Mustafa / Açık | Sırada | Stüdyoda "Ölçü: doğrulanmış" işareti |
| G-017 | Stüdyo v1.5: hazır ambalaj kütüphanesi, üretilen şişe/teneke/kavanoz/kase şekilleri ve parça renkleri | Claude / Codex | Doğrulama bekliyor | Derleme + 9/9 test + smoke geçti; dört şeklin yönü elle incelenecek |
| G-018 | Smoke testini katalogdaki değişken koli adedi ve maliyetle uyumlu hâle getir | Codex | Bitti | 27.09.2026; smoke geçti |
| G-019 | Hazır ambalajlar için ajana verilecek gerçek ölçülü PNG şablonları ve şablona bağlı prompt | Codex | Bitti | 27.09.2026; kutu/etiket/kapak kılavuzları, prompt kuralları, görsel kontrol ve otomasyon testi geçti |
| G-020 | İlk görsel gerçekçilik geçişi: etiket parlama kontrolü, dengeli mağaza ışığı, renk düzeni, metal raf ve zemin derzleri | Codex | Bitti | 28.09.2026; derleme, 10/10 test, smoke ve 1280×720 sahne karşılaştırması geçti |
| G-021 | Çevre PBR varlık geçişi: raf/zemin/duvar/tavan/kasa doku setleri ve ayrıntılı prop modelleri | Açık | Sırada | Teknik ışık temeli hazır; Blender/asset üretim standardı ve çevre içe aktarma hattı kurulacak |
| G-022 | Referans market canlılığı: koyu açık tavan, sıcak zemin, raf önü dolgu, marka başlıkları ve dengeli dolu raflar | Codex | Bitti | 28.09.2026; toplam başlangıç stoğu korunarak 16 raf/16 depo, derleme + 10/10 test + smoke + görsel kontrol geçti |
| G-023 | Blender çevre varlığı hattı ve ilk gerçek çift yüz gondol rafı | Codex | Bitti | 28.09.2026; `.blend`/FBX/metadata/önizleme, Unreal otomatik import, 120×90×160 cm + 5 materyal + 3 UCX doğrulaması, oyuna entegrasyon geçti |
| G-024 | Tek yüz 1200 mm duvar rafı ve yan duvar kategori dizilimi | Codex | Sırada | Gondol standardını kullanacak; mağazadaki boş yan hacimleri dolduracak |
