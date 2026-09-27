> **Not (27.09.2026):** Kullanıcının dış ajana vereceği güncel, kısa promptlar `Docs/Uretim/` klasöründedir. Bu dosya planlama arşividir.

# Dış araçlar için prompt kütüphanesi

Durum: kullanım için hazırlanmış metinler; bu promptlarla görsel/model üretilmedi. Prompt, aracın desteklemediği dosya biçimini, doğru UV'yi veya kusursuz marka yazısını garanti etmez. Çıktılar 03 ve 08'deki kabul sürecinden geçer.

## Kullanım

Her görev için önce kısa **Ortak teslim notunu**, sonra ilgili promptu gönder. Köşeli parantezli alanları doldur; gerekli görsel/model/UV şablonunu ayrıca ekle. Bütün tasarım paketini her seferinde yapıştırma. Tek istekte yüzlerce ürün isteme; kabul edilmiş örneği ve yalnızca değişen brief'i kullan.

Görsel üretim araçları için promptlar İngilizce; açıklamalar Türkçe tutuldu. Türkçe metin içeren etiketlerde son yazı kontrolü insan tarafından yapılır. Görsel üreticiden bir 3B dosya; 3B üreticiden desteklemediği hassas CAD/UV garantisi istenmez.

**Ortak teslim notu**

```text
Project: Miras Market, a PC supermarket and retail-company simulation.
Task type: [2D artwork / 3D asset / animation / UX specification / review].
Asset ID: [ID]. Brief version: [VERSION].
Reference files attached: [LIST].
Required dimensions and output format: [EXACT REQUIREMENTS].
Separate facts taken from references, assumptions, and unsupported requirements.
Do not invent historical accuracy, hidden surfaces, exact brand text, measurements,
or successful export/testing that you cannot verify. List missing inputs.
Return only the requested asset or deliverable, with a short limitation report.
Do not add unrelated objects, baked prices, random dates, watermarks, or scene props.
```

Üretici yalnızca tek satırlık prompt kabul ediyorsa ilgili promptun geometri/görsel bölümünü kullan; dosya ve kalite şartlarını teslim sonrası kontrol listesine taşı. Her araca uzun talimatın aynı biçimde işleyeceği varsayılmaz.

## P01 — Ürün araştırma/teknik brief'i

Araç: araştırma yapabilen metin asistanı. Girdi: marka/model, ülke, hedef yıl ve referanslar. Çıktı görsel değildir.

```text
Prepare a production brief for [PRODUCT], sold in [COUNTRY] around [YEAR].
Use primary manufacturer/catalogue references where available and cite exact pages.
Separate verified information from unknowns. Do not infer dimensions from volume
alone. Record package type, measured width/depth/height if sourced, sale unit,
package size, reference views, label-language variant and packaging-era evidence.
Do not invent GTINs, ingredients, safety claims or an old packaging design.
Return: verified facts table, source dates, missing reference views, provisional
asset ID, and the simplest reusable geometry family. If historical packaging
cannot be verified, mark it as a deliberate fictional/modern placeholder.
```

## P02 — Tek ambalaj yüzü

Araç: referans görsel düzenleyebilen görüntü üreticisi. Girdi: ilgili yüzün doğrulanmış resmi + yüz şablonu. Ön/arka/yan yüz için ayrı çalıştır.

```text
Create a flat, front-facing print artwork for the [FACE] face of [PRODUCT].
Use the attached face template and supplied packaging reference only.
Canvas: [WIDTH_PX] x [HEIGHT_PX]. Physical face: [WIDTH_MM] x [HEIGHT_MM] mm.
Preserve the supplied brand mark and approved wording as accurately as the tool
allows. Keep the exact face aspect ratio. Flat print artwork only: no perspective,
box mockup, folds, cast shadows, reflections, studio background, price stickers,
or invented certification marks. Keep critical text inside the supplied safe area.
Do not fabricate illegible or missing text. If exact typography cannot be preserved,
provide the artwork with a clean reserved text/logo area and identify what must be
placed manually as a separate layer. Export PNG, and layers if supported.
```

Örnek P02 değişkeni: düz demo kutunun ön yüzü için 560×1600 px ve 70×200 mm. Gerçek ürüne bu örnek ölçüler otomatik uygulanmaz.

## P03 — Fotoğraftan düz etiket çıkarma

Araç: görüntü düzenleyici. Fotoğrafta görünmeyen arka/yan yüzleri üretmek için kullanılmaz.

```text
Rectify the visible [FACE/LABEL] from the attached package photograph into flat
print artwork. Correct perspective and crop to the marked four-corner boundary.
Preserve the observed graphic layout and supplied brand lettering. Reduce lighting
gradients and photographic reflections without inventing covered details.
Output [W] x [H] PNG with the supplied physical aspect ratio. Do not create a 3D
render. Flag occluded, blurred or unreadable areas in a separate note; do not
silently hallucinate them. Provide an unmodified reference crop alongside the
cleaned result so the changes can be checked.
```

## P04 — Silindirik sargı

Araç: görüntü düzenleyici. Ölçü ve seam, model şablonundan gelir; fotoğraftan otomatik çevre ölçümü beklenmez.

```text
Prepare a flat wrap-label artwork for the supplied cylindrical label template.
Use the provided unwrapped dimensions [W_MM] x [H_MM], canvas [W_PX] x [H_PX],
and the marked front-centre and back-seam positions. Preserve the attached artwork
and brand references. Keep the seam-safe zones free of critical text.
Do not render a bottle, add cylindrical shading, or distort the logo to simulate
curvature. Do not infer an unseen rear label from a single front photograph.
If the template describes a tapered or curved shoulder rather than a true cylinder,
request that model's UV template instead of treating it as a rectangle.
```

## P05 — Özel UV şablonuna baskı yerleştirme

Araç: katmanlı görüntü/DCC asistanı. Girdi: aynı modelden alınmış UV, yüz adları ve referanslar.

```text
Apply the supplied packaging artwork to the attached UV template for model
[MODEL_ID], UV layout version [VERSION]. Keep the canvas dimensions, island
positions, island scales, orientation and padding unchanged. Place each approved
face artwork only in its labelled UV region, following the supplied rotation and
mirror table. Do not move UV islands or repaint logos as approximate text.
Extend edge colours into the specified gutters, keep critical artwork away from
seams, and export a clean base-colour atlas without the UV wireframe.
Also return a separate preview with the UV overlay. If no 3D preview is possible,
state that seam alignment on the model remains unverified.
```

## P06 — Basit kutu için Blender üretimi

Araç: Blender kullanabilen teknik asistan/3B sanatçı. Normal görüntü üreticisine verilmez. Sıfırdan her marka için kutu üretmek yerine şablon oluşturur.

```text
Create a reusable rectangular retail carton template in Blender for this project.
Final physical dimensions: width 70 mm, depth 50 mm, height 200 mm.
The normalized target convention is +X front, +Y width, +Z up; pivot at the
bottom centre. Use clean low-complexity geometry with a small realistic bevel,
no gable top, no baked logo, no prices, and no scene/background objects.
Provide six named printable regions and use the supplied box_70_50_200_v1
UV rectangle table exactly. Define each face's rotation so FRONT/BACK/LEFT/RIGHT
test text is upright and not mirrored when viewed from outside.
Provide an editable .blend, an FBX export candidate, and a dimension/UV report.
Do not claim Unreal compatibility until the 100 mm scale and facing tests are run.
If a Blender script is used, it must modify only the newly created asset collection.
```

P06'ya 03 belgesindeki altı satırlık koordinat tablosunu ekle. Bu prompt, gerçek bir marka ambalajını 70×50×200 mm kabul etmez.

## P07 — Meshy için şişe gövdesi

Araç: Meshy veya benzer 3B üretici. Ölçü, malzeme slotu ve UV sonradan Blender'da kontrol edilir.

```text
A single realistic retail [PET/glass] bottle based on the attached silhouette
references. Straight upright pose, separate readable bottle body and cap shapes,
symmetrical cross-section where the reference is symmetrical, clean label band.
Unbranded neutral material; no embossed random text, no logo, no price tag,
no liquid splash, no hands, no table or background geometry. Preserve the main
silhouette and avoid unnecessary microscopic geometry. Intended dimensions:
[WIDTH] x [DEPTH] x [HEIGHT] mm, to be normalized in Blender after generation.
The label will be applied separately in our product tool. Prioritize clean geometry
and a usable continuous label surface over decorative detail.
```

Meshy arayüzünde destekleniyorsa geometriyi sadeleştir ve FBX/GLB ile doku setini indir. Promptun kendisi doğru fiziksel ölçü, ayrılmış kapak veya istenen üçgen sayısını garanti etmez.

## P08 — Yumuşak paket/poşet

```text
Generate one sealed retail [pasta/snack/frozen food] pouch from the supplied
references. Upright, lightly filled, with restrained folds and clearly readable
front and back print regions. No tears, open contents, display stand or extra bags.
Neutral unbranded base material: the packaging artwork will be mapped separately.
Keep the silhouette suitable for repeated shelf placement. Target physical size
[W] x [D] x [H] mm is a production constraint to verify after generation.
Avoid deep folds crossing the main logo area. Return only the pouch model.
```

Bu ailede dikdörtgen kutunun altı yüz eşlemesi kullanılmaz; kendi UV ve destek/istif profili gerekir.

## P09 — Elektronik ürün: demo ve satış kutusu

```text
Prepare a 3D production brief and, if supported, separate models for [TELEVISION /
HAIR DRYER / SMALL APPLIANCE] based on supplied references. Keep the demo device
and its closed retail box as separate assets. Do not fuse accessories, power
cables or a display shelf into the device. Use the provided measurements; mark
missing measurements rather than guessing exact values. The retail box follows
our box template workflow; its branding is a separate texture package.
Specify support points, optional security-cable point, interaction area, and whether
the customer carries the item or receives a stock-pickup/delivery ticket.
Do not claim articulated or electrical functionality from a static mesh.
```

## P10 — Düz raf modülü

Araç: Blender/parametrik modelleme önerilir. Ölçülü modüler raf, generatif şekil denemesinden daha fazla hassasiyet ister.

```text
Create one modular supermarket shelving bay, 1000 mm wide, 450 mm deep and
1800 mm high, using the attached technical sketch. Single customer-facing side.
Neutral powder-coated metal, configurable shelf levels, clean repeated structure.
No products, logos, floor plane, people or baked store lighting. Keep uprights,
shelf decks, back panel and price-strip profiles identifiable for editing.
Use the project's normalized +X front / +Y width / +Z up convention and bottom-
centre pivot. Provide clear usable shelf volumes separately from decorative mesh.
Preserve side connection positions so multiple bays align without gaps.
Return source .blend, FBX candidate, material list, measured bounds and a diagram
of shelf zones and customer/service access points. Do not invent a certified load
rating; use [GAME_DESIGN_LOAD_LIMIT] as labelled simulation metadata only.
```

## P11 — Soğutmalı dolap

```text
Create a modular [open refrigerated wall / glass-door fridge / chest freezer]
for a realistic supermarket game, matching the supplied dimensions and sketch.
Keep doors, handles, shelving and cabinet body separable where they must move.
Provide shelf-support areas, door swing/slide clearance, front customer approach,
rear/side service access and power-connection marker. No products, logos, people
or store background. Use restrained physically plausible material appearance.
Do not bake reflections or lighting into base colour. Temperature, energy use and
capacity are game metadata supplied separately, not facts inferred from the model.
Return the editable source, export candidate and clearance/measurement report.
```

## P12 — Pastane ve lokum teşhiri

```text
Design one [refrigerated pastry case / ambient Turkish delight display] using
the provided technical dimensions. Separate the customer-facing display, rear
staff access, removable trays/dividers and price-label rail. No baked food or text.
The model must support interchangeable product trays in defined placement zones.
Avoid inaccessible compartments or glass intersecting the serving opening.
Return an editable model with neutral materials, a tray-zone diagram, customer
and staff interaction markers, and any unresolved geometry assumptions.
This is a retail display fixture, not a complete bakery/restaurant simulation.
```

## P13 — Kasap veya restoran iş istasyonu

```text
Create a modular equipment concept and technical asset breakdown for [BUTCHER
SERVICE COUNTER / HOT-FOOD SERVICE / RESTAURANT ORDER AND PICKUP COUNTER].
Use the supplied floor footprint, utility points and workflow sketch.
Separate display, preparation, handover, storage and queue approach where relevant.
Show staff-only versus customer-facing sides. Do not invent legal compliance;
identify clearances or sanitation assumptions requiring project validation.
Output a parts list, top/front/side technical views and model boundaries. Keep food,
workers and decorative scene elements separate from the equipment mesh.
```

## P14 — Yeni raf türü brief'i

```text
Specify a game-ready fixture family for [MAGAZINES / HAIR-DRYER BOXES / TV DISPLAY /
PEGBOARD ACCESSORIES / PRODUCE / FLOWERS / PALLET GOODS].
Inputs: product dimensions, sale method, store format, budget and reference images.
Return: equipment dimensions, supported orientations, placement/support mode,
usable zones, access points, capacity calculation method, price-label association,
collision/clearance needs and staff/customer actions. Distinguish measured facts
from proposed game rules. Explain what can reuse existing fixture families and
what genuinely requires a new interaction. Do not assume every product is a cube.
```

## P15 — Marka kimliği

```text
Develop three distinct retail-brand identity directions for [CHAIN NAME], a family
market founded in early-2010s Lüleburgaz that can expand internationally.
For each direction provide colours, typography categories, material palette,
signage hierarchy, department markers, staff-uniform concept and a low-cost and
premium implementation. Show how the same identity adapts to a corner store,
supermarket and hypermarket without forcing the same floor plan.
Use original brand presentation. Do not copy Tesla's logo, proprietary iconography
or a real retailer's entire identity. Separate visual references from final assets.
Return a concise style specification and labelled concept boards if supported.
```

## P16 — Tesla esintili ana yönetim ekranı

Araç: UI tasarım asistanı veya görsel konsept üreticisi. Statik görsel, tıklanabilir/çalışır ekran değildir.

```text
Design a desktop management UI concept for Miras Market, inspired by the calm,
product-centred clarity of the Tesla mobile app, adapted to a retail simulation.
Reference viewport: 1920x1080. Dark graphite surfaces, generous spacing, restrained
accent colour, readable Turkish typography, a store overview and clear actions.
Use a left navigation rail, a company/region/store selector, time/pause status,
cash and operating-result summaries, and the three most important exceptions.
Include a practical dense table view for large catalogues, not only oversized cards.
Do not reproduce Tesla logos or car controls. Keep current cash distinct from profit.
Deliver the overview, an expanded detail panel, a light-theme variant and UI tokens.
Label invented values as sample game data. Do not generate game code.
```

## P17 — Sipariş ekranları ve tıklanabilir akış

```text
Design the menu-driven ordering flow for the attached Miras Market UI brief.
Screens: store selection, catalogue/filter, supplier comparison, cart, delivery,
confirmation and order tracking. Show "3 cases x 12 units = 36 units", available,
reserved and inbound stock separately, landed cost, payment now/later and capacity.
Include empty, loading, invalid quantity, insufficient funds, changed quote,
partial delivery and cancellation states. Keep the selected store visible.
Specify keyboard focus and duplicate-submit prevention. If using a prototyping
tool, connect the screens and component states; otherwise return an interaction
specification and clearly state that no functional prototype was created.
```

## P18 — Ürün Stüdyosu UX

```text
Design an editor-side Product Studio workflow, not an in-game runtime FBX importer.
Steps: identity, template/custom mesh, scale/orientation, face/UV artwork, materials,
shelf placement, commercial metadata, validation, preview and content publishing.
Include front/back/left/right/top/bottom image assignment for a compatible box
template and a distinct custom-UV path. Show seam previews and aspect-ratio errors.
Do not promise that one photograph contains all hidden surfaces. Keep artist-facing
technical controls out of ordinary player menus. Return screen wireframes, field
groups, validation messages and draft/accepted/versioned state transitions.
```

## P19 — Mağaza yerleşim alternatifi

Araç: planlama/optimizasyon asistanı. Girdi olmadan kuşbakışı mağaza resmi üretmek yerine eksikleri belirler.

```text
Propose three retail layouts for the supplied dimensioned floor polygon, columns,
entrances/exits, loading access, utilities, fixture catalogue and brand profile.
Respect the provided hard constraints and locked regions. Compare an economical,
balanced and service-focused option. Return department polygons, fixture IDs with
positions/orientations, customer/staff circulation, queue areas, cost breakdown,
assumptions and unverified constraints. If you cannot perform geometric validation,
label the outputs as conceptual proposals, not validated buildable layouts.
Do not obstruct exits or claim an optimum without solving and reporting the model.
```

## P20 — Lüleburgaz dönem/çevre araştırması

```text
Build a visual research brief for a fictional local market inspired by Lüleburgaz,
Turkey, around 2011. Use dated primary or archival references where available.
Separate supported period details from illustrative design suggestions. Cover
shopfront scale, signage, pavement, delivery vehicles, tills, lighting, shelving,
printed promotions and typical interior wear. Do not describe a modern photograph
as a verified 2011 scene. Do not invent a real shop's history. Return a source-linked
reference list, a proposed modular environment asset list and unresolved questions.
```

## P21 — Müşteri/personel karakteri

```text
Create a retail-game character based on the attached style and clothing brief.
Role: [CUSTOMER / CASHIER / STOCK CLERK]. Period/region: [REFERENCE BRIEF].
Neutral A-pose, consistent proportions, separated clothing where needed, no shopping
bags or props fused into the hands. Avoid baked pose shadows and extreme detail.
Target the supplied skeleton/rig specification if the tool supports it; otherwise
deliver an unrigged model and state that retopology and rigging remain necessary.
Provide front/side/back previews and editable/export files supported by the tool.
Vary individuals naturally; do not encode personality or trustworthiness in appearance.
```

## P22 — Animasyon paketi

```text
Prepare animation clips for the supplied retail character skeleton and grip points:
walk, turn, idle, read label, reach at low/mid/high shelf, pick small item, place in
basket, push cart, scan item and carry carton. Keep each clip separate and named.
Use the specified frame rate and in-place/root-motion policy [SUPPLIED POLICY].
Do not bake product meshes into the rig. Identify hand/foot contact intervals and
looping clips. Test scale, skeleton mapping and transitions on the supplied model.
If you cannot export or validate animated FBX, report that limit rather than claiming
the clips are ready for Unreal. Provide no unrelated acting or camera animation.
```

## P23 — Ses üretimi

```text
Create isolated retail sound effects for [BARCODE BEEP / PAPER RECEIPT / CARTON
HANDLING / FRIDGE HUM / SHOP DOOR BELL], following the supplied duration range.
No music, voices, brand jingle, room soundtrack or unrelated background sound.
For one-shots provide several subtle variations without abrupt clipping.
For ambience provide a seamless loop with consistent noise floor.
Deliver WAV at the requested project sample rate/bit depth if supported and state
the actual format. Keep source/master separate from game-compressed versions.
Do not claim measured loudness or seamless looping unless checked.
```

## P24 — Varlık kabul incelemesi

Araç: gerçekten dosyayı açabilen Blender/Unreal teknik asistanı. Yalnızca ekran görüntüsü gören asistan tüm kontrolleri yapmış sayılmaz.

```text
Audit the attached asset against the supplied Miras Market delivery specification.
Do not modify the original. Inspect dimensions/units, front/up axes, pivot, geometry,
normals, UV overlaps in unique print regions, material slots, texture paths/colour
spaces, collisions, support/grip points and shelf-fit metadata. Preview the front,
back, seam, low mip/LOD appearance and a repeated shelf arrangement if possible.
Return PASS / FIX / UNVERIFIED for each item, measured values, evidence and the
smallest corrective action. Do not infer successful import from an attractive render.
Do not mark unavailable measurements or engine tests as passed.
```

## P25 — Davranış/öğrenme deneyi tasarımı

```text
Review this bounded simulation problem: [PROBLEM]. Compare a rule-based baseline,
constraint/optimization approach, and a learned policy only if justified.
Specify observable state, allowed actions, hard invariants, evaluation scenarios,
failure recovery and data requirements. For learning, define training/validation/test
separation by whole store layout, reward-hacking checks, reproducibility information,
deployment cost and fallback behaviour. Do not promise a model will learn realistic
human behaviour from a small unspecified dataset. Return a decision memo and pilot
acceptance criteria, not a claim of trained AI or working game code.
```

## İlk üç dış görev için hazır sipariş kartları

| Görev | Prompt | Eklenecekler | Başarı |
|---|---|---|---|
| Düz demo kutu | P06 | 03'teki ölçü/atlas tablosu | Ölçü, yön, alt merkez pivot ve altı yüz testi |
| Basit şişe | P07 | Ön/yan/arka siluet ve ölçü | Temiz label bandı; Blender'da ölçü/UV kontrolü |
| 1 m raf modülü | P10 | Teknik taslak ve oyun kapasite notu | Yan yana bağlantı, ürün sığması ve erişim |

Bu üçlü kabul edilince bir gerçek ürün yüzü P02/P03 ile eklenir. Ardından bir soğuk dolap ve bir servis teşhiri denenir. İlk doğru şablonlar çıkmadan toplu üretim yapılmaz.

## Teslimde daima sorulacak üç şey

Dosyanın kendisi var mı, yoksa sadece örnek görsel mi? Ölçüler ve baskı gerçekten kontrol edildi mi? Hangi gereksinim henüz doğrulanmadı? Bu yanıtlar prompt metninin ne kadar iddialı olduğundan daha önemlidir.
