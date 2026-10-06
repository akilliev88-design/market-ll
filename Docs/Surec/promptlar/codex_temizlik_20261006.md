# Codex iş emri — proje temizliği (06.10.2026)

Hazırlayan: Claude (Cowork), Mustafa'nın isteğiyle. Onay: Mustafa.

## Önce oku (güvenlik kuralları)

1. **M69'a dokunma.** Çalışma ağacındaki Claude M69 değişiklikleri (commit dışında duranlar) geri alınmaz, silinmez, bu temizlik commit'ine karıştırılmaz. Temizliği ayrı bir commit olarak at.
2. Git'te izlenen dosyaları `git rm` (klasör için `git rm -r`) ile sil; `Saved/`, `Claude outputs/`, `__pycache__`, `*.bak` git dışıdır, normal sil.
3. **`MAHALLE_MARKET_GEZI.cmd` silinmeyecek.** O Blender ile yaptığımız el yapımı mahalle marketidir (G-113). Silinecek olan Image-blaster'ın `MARKET_GEZI.cmd` dosyasıdır; adlar benzer, karıştırma.
4. Silmeden önce arka planda Image-blaster'a ait bir `node` ya da `powershell` (FinalizeGeneratedStore) süreci çalışıyorsa kapat.
5. Bitince: `DERLE.cmd /q` → `TEST.cmd /q` (en az 174 test) → `SmokeTest.ps1`. Sonuçları GUNLUK/DURUM/GOREVLER'e yaz.

---

## Bölüm 1 — Image-blaster ve "market üret" zinciri (Mustafa: tamamen sil)

Image-blaster denemesi (G-112) reddedildi, kampanyaya bağlanmadı. Yerine Blender mağazaları geldi (G-113/G-114/G-116). Aşağıdakilerin hepsi yalnız bu denemeye ait.

**Kök dizindeki CMD'ler**
- `MARKET_URET.cmd`
- `MARKET_ONIZLE.cmd`
- `MARKET_MODEL_AKTAR.cmd`
- `MARKET_AKTARIM_TAMAMLA.cmd`
- `MARKET_GEZI.cmd`

**Kaynak kod** (ayrı GameMode; başka hiçbir kaynak dosya kullanmıyor, testi yok)
- `Source/MirasMarket/MarketGeneratedStore.h`
- `Source/MirasMarket/MarketGeneratedStore.cpp`

**Araçlar**
- `Tools/ImageBlaster/` (klasörün tamamı: `FinalizeGeneratedStore.ps1`, `market-world.mjs`)
- `Tools/import_generated_market.py`
- `Tools/import_generated_collision.py`
- `Tools/Blender/inspect_generated_market.py`
- `Tools/Blender/render_generated_market.py`
- Bunların `__pycache__` kopyaları (`import_generated_*.pyc`, `inspect_generated_market.*.pyc`)

**Unreal varlıkları** (~123 MB, LFS)
- `Content/Stores/Generated/` (klasörün tamamı: `MahalleMarket` ve `MahalleCollision`)

**Ham dosyalar** (~158 MB)
- `AssetInbox/ImageBlaster/` (klasörün tamamı; `Raw/` içindeki 152 MB GLB dahil)

**Saved altı** (git dışı, ~350 MB)
- `Saved/ImageBlaster/` (klasörün tamamı: indirilen image-blaster kopyası, `.env`, `UnrealImport/` GLB, durum JSON'ları)
- Varsa `Saved/Screenshots/GeneratedStore/` ve `Saved/Logs/GeneratedStoreFinalize.log`

**Belgeler ve görseller**
- `Docs/Environment/IMAGE_BLASTER_MAHALLE_MARKETI.md`
- `Docs/Images/GeneratedMarket/`

**Belgelerde düzeltme (silme değil)**
- `Docs/Surec/DURUM.md` 11. satırdaki "G-112 aktarım devam ediyor … Devam: review_ready olunca…" notu eskidi: başına "(kapandı, deneme silindi 06.10.2026)" yaz. GUNLUK'teki eski girişler tarihçe olarak kalır.
- Son kontrol: `grep -rn -i "ImageBlaster\|image-blaster\|GeneratedStore\|Stores/Generated" Source Tools Config *.cmd *.ps1` boş dönmeli.

**Mustafa'ya not (Codex yapmaz):** World Labs API anahtarı `Saved/ImageBlaster/image-blaster/.env` içindeydi. Artık kullanılmayacaksa World Labs platformunda anahtarı iptal et.

---

## Bölüm 2 — Açık çöp (güvenle silinir)

- **Proje kökündeki ~30 bozuk adlı klasör** (Çince/Korece görünen anlamsız adlar: `耀갯`, `쀀⨧`, `琀ⷧ`, `灡⹥湰g` vb.). Hepsinin içinde yalnız `.thumbnails/fail/blender` ve `.thumbnails/large` var; Blender'ın yanlış yola yazdığı küçük resim önbelleği. Kural: kökte adı ASCII olmayan ve içinde yalnız `.thumbnails` bulunan her klasörü sil. Silmeden önce listeyi yazdır, içinde başka dosya olanı silme.
- `Tools/__pycache__/`, `Tools/Blender/__pycache__/`
- `Config/products.json.bak`, `Config/planograms.json.bak`
- `Claude outputs/` (Eylül'den kalma geçici çıktılar, git dışı)
- `Docs/Devam_G074_Menu/` (29.09 menü işinin arşivi; içerik çoktan `Source/`'ta, derlendi)
- `Docs/Surec/bekleyen/C16/` (D9a ile koda taşındı ve 04.10'da doğrulandı)
- `Tools/Arsiv/katalog_olustur.py` (AGENTS.md "kullanma, stüdyo verisini siler" diyor)
- `Saved/` kökündeki eski G-088 geçici betikleri: `stage_g088.py`, `stage_g088_r2.py`, `finalize_g088.py`, `G088_R2_continuation.md`, `G088_R2_entry.md`, `G088Selected.patch`, `stage_store_editor*.py` (7 dosya), `store_contact_fix.py`, `store_map_revision.py`, `store_review_2d.py`, `store_review_r2.py`, `store_ui_turkish.py`, `store_visual_controls.py`, `update_g088_r2_docs.py`, `update_store_ui.py`, `check_store_routes.py` (Tools'taki asıl sürümü kalır), `SM_Handmade*.tmp` (4 dosya)
- `Saved/Claude/is_git_onceki.cmd`, `Saved/Claude/son_git_onceki.log`, `Saved/Claude/m69_git.py` (eski git'li çalıştırıcı; artık kullanılmıyor). **`m69_ic_ad.py`, `m69c_mesaj.txt`, `is.cmd`, `son.log` kalsın** — iç ad değişikliği henüz yapılmadı.

---

## Bölüm 3 — Kontrol edip sil (önce referans bak, emin değilsen dokunma, raporla)

- **Unreal şablon içerikleri:** `Content/ThirdPerson/`, `Content/LevelPrototyping/`, `Content/__ExternalActors__/ThirdPerson/`, `Content/__ExternalObjects__/ThirdPerson/`, `Content/Developers/`. Kod bunlara başvurmuyor; oyun `/Engine/Maps/Entry` açıyor. Unreal Editor'da "Reference Viewer" ile bak, referans yoksa editörden sil (dosya sisteminden değil). `Content/Input/` de şablondan; `DefaultInput.ini` ve kodla kullanılıp kullanılmadığını kontrol et.
- **`Content/Characters/Mannequins/` SİLİNMEZ:** `MarketPeople.cpp` ve `Tools/MetaHuman/hazirla_animasyon.py` yürüyüş/bekleme animasyonlarını buradan alıyor.
- `Docs/Surec/bekleyen/M28_M29_sirket_finansi.patch` ve `M28_M30_sirket_finansi.patch` (M28–M31 G-094'te uygulandı). Uygulandığını doğrula, sonra sil.

---

## Bölüm 4 — Codex silmesin (Mustafa/Claude karar verecek)

Bunlar yer kaplıyor ya da eskimiş olabilir ama karar gerekiyor; bu turda dokunma:

- `Content/MetaHumans/NewMetaHumanCharacter.uasset` (147 MB) — MetaHuman kaynağı olabilir; silinirse `MH_Teyze` düzenlenemez.
- `Docs/Surec/akislar/` deney verileri (C3–C11 CSV'leri, ~30 MB) ve `Tools/c8_*`, `c9_*`, `c10_*`, `c11_*` rapor betikleri — C9 hedefleri geçersiz kaldı, arşivlenebilir.
- `Docs/Surec/promptlar/` eski iş emirleri.
- Eski hikâyeyi (2011, Lüleburgaz, Bereket, "Miras Market") anlatan tasarım belgeleri: `Docs/OYUN_TASARIMI.md`, `Docs/GELISTIRME_PLANI.md`, `Docs/DOGRULAMA.md`, `Docs/Planlama/`, `Docs/Kurgu/02_DERIN_INCELEME.md`, `Docs/Kurgu/05_YOL_HARITASI.md`. Kurgu Claude'un alanı; Claude arşive taşıyacak.
- `Saved/StoreBackups/`, `Saved/AutoPlay/` eski koşular, `Docs/Images/StoreEdges/*.gif` (23 MB) — disk temizliği, isteğe bağlı.

---

## Teslim

- Silinen her şeyin listesi ve kazanılan yer GUNLUK'e.
- GOREVLER'e: "G-117 Proje temizliği (Image-blaster kaldırıldı)" satırı.
- DERLE/TEST/Smoke sonucu.
