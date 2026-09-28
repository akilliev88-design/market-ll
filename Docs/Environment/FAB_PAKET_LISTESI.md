# İndirilecek varlıklar (güncel)

Güncelleme 28.09.2026 — Mustafa kararı: ücretli market paketleri **alınmıyor**; kendi Blender modellerimiz kalıyor. Yalnızca ücretsiz dokular ve MetaHuman müşteriler indirilecek. Claude oyuna bağlar.

## 1. Dokular (ücretsiz, CC0 — ticari kullanım serbest)

Her doku için: **2K**, **PNG**. Gereken haritalar: renk (Color / Diffuse), **Normal DirectX** (ambientCG: `NormalDX`, Poly Haven: `nor_dx`), pürüzlülük (Roughness / `rough`). AO ve Displacement varsa zararı yok.

| Nerede kullanılacak | Doku | Site | Klasör |
|---|---|---|---|
| Zemin | **Terrazzo004** (beyaz terrazzo) | ambientCG | `AssetInbox\Textures\Harici\Terrazzo004\` |
| Duvar | **beige_wall_001** (düz bej boyalı sıva) | Poly Haven | `AssetInbox\Textures\Harici\beige_wall_001\` |
| Dökme reyon, kasa, masa (koyu ahşap) | **american_walnut_veneer** | Poly Haven | `AssetInbox\Textures\Harici\american_walnut_veneer\` |
| Raf altı ve başlıkları (açık ahşap) | **ash_veneer** | Poly Haven | `AssetInbox\Textures\Harici\ash_veneer\` |
| Depo kolileri | **Cardboard004** | ambientCG | `AssetInbox\Textures\Harici\Cardboard004\` |

- ambientCG: sayfada **2K-PNG** zip'ini indir, klasöre aç.
- Poly Haven: sayfada çözünürlük **2K**, biçim **PNG** seç; Diffuse, nor_dx ve Rough dosyalarını (ya da zip'i) indir, klasöre koy.
- Dosya adlarını değiştirme. Raf metali, tavan ve tabelalar için doku gerekmiyor; bizim renk + eskime katmanımız kalıyor.

## 2. Müşteriler: MetaHuman (Unreal 5.8 içinde, ücretsiz)

1. Epic Games Launcher → Unreal Engine 5.8 → **Seçenekler** → **MetaHuman Creator Core Data** işaretle, kur.
2. Projeyi aç (`STUDYO.cmd`) → **Edit > Plugins** → **MetaHuman Creator** eklentisini aç → editörü yeniden başlat.
3. Content Browser'da **`Content/MetaHumans/`** klasörü oluştur; içinde sağ tık → MetaHuman → **MetaHuman Character** oluştur. Epic hesabıyla giriş ister.
4. Karakteri düzenle, sonra **Assemble** ile derle. Oyun için optimize / düşük ayrıntı seçeneği varsa onu seç (çok müşteri aynı anda yürüyecek).
5. Önerilen 4 karakter (2011 Lüleburgaz mahallesi), adları aynen böyle olsun:
   - `MH_Teyze` — 60'larında kadın, hırka, başörtüsü
   - `MH_Amca` — 60'larında erkek, kasket, yelek
   - `MH_Anne` — 35–40 yaş kadın, günlük mont
   - `MH_Genc` — 20'lerinde erkek, eşofman üstü / kot
6. Bittiğinde editörü kapat ve Claude'a haber ver. Yürüme/bekleme animasyonları ve oyuna bağlama Claude'da.

Hepsi `C:\Users\mtass\Desktop\market-ll\` proje klasörünün içinde kalır; başka yere koyma.

## Kaynaklar

- ambientCG Terrazzo004: https://ambientcg.com/view?id=Terrazzo004
- ambientCG Cardboard004: https://ambientcg.com/view?id=Cardboard004
- Poly Haven dokuları: https://polyhaven.com/textures (lisans: https://polyhaven.com/license)
- MetaHuman Creator (Unreal içinde): https://dev.epicgames.com/documentation/metahuman/metahuman-creator-in-unreal-engine?lang=en-US
- Megascans fiyat değişikliği: https://www.cgchannel.com/2024/10/epic-games-has-made-megascans-free-to-all-but-only-until-the-end-of-2024/
