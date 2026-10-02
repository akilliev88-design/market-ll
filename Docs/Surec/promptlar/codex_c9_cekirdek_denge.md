Miras Market, **C9 (Codex): çekirdek denge — C8 bulgularının bot tarafı, Claude'un oyun tarafı düzeltmeleri, hedef eğri ölçümü.** Ana klasör: `C:\Users\mtass\Desktop\market-ll`.

Önce oku: `Docs/Kurgu/06_GIDIS_YOLU.md` (sürüm 2; §4 C9, **§5 hedef eğriler**), `Docs/Surec/akislar/C.md` (C9 satırı), kendi `C8_aile_dukkani_rapor.md`.

Claude'un C9 oyun tarafı (derlenmedi): toptancının **babanın hatırına** acil malı (nakit krizinde haftada bir, ~3 günlük mal, 3 gün vadeli: `MarketSuppliers::LifelineAllowance`, `OrderAllowance` bunu da verir), **mal parası erken uyarısı** (kasa bir haftalık alımdan azsa ayda bir haber), **İK müdürü** 8 çalışan ya da 2 açık şubeden sonra (3 değil), haftalık grafikte **işaretli eksen**. Yeni test `Suppliers.Lifeline`; alt sınır **152**.

1. **Al:** `git status`; Claude'un dosyaları tek commit `C9: çekirdek denge oyun tarafı (Claude, derlenmedi)`. Derle, test et (152), Smoke. En küçük düzeltme, `A.md`'ye dosya:satır. Sonra `akis-a`.
2. **Bot (A, C8 raporundaki üç neden):**
   - Fiyat: büyüme indirimi kapısı `State.Branches.IsEmpty()` yerine **açık şube sayısı** (`MarketBranches::OpenCount`); ilk şubeyle birlikte ilk dükkânın fiyatını bir anda %18 yükseltme. Fiyat hedefini rakip fiyatına ve yerel paya göre koru.
   - İşe alma: İK müdürü, aile müdürü ve ek çalışan ancak **ek brüt kâr aylık toplam maliyeti karşılıyorsa** (ya da hizmet kaybı ölçülüyorsa). Nakit eşiği tek başına yetmez.
   - Mal parası: siparişe **korunmuş işletme sermayesi** (en az iki haftalık alım); maaş artışı, kâr payı, işe alma ve şube açılışı bu tutarın üstündeki paradan. Temkinli bot da `OrderAllowance`'ı (vade + toptancı acil malı) kullansın.
3. **Koşular:** 10 yıl × 3 tarz × tohum 21/22/23 ve 30 yıl × 3 tarz × 21. Rapor önce **§5 hedef eğri tablosunu** doldursun (tarz başına: ilk şube günü, 3. ve 10. yıl mağaza, 10./20. yıl ulusal sıra, 30 yılda kurtarma, ilk dükkânın yıllık faaliyet kârı eğrisi, sıkıcı dönem). Hedefin dışında kalan her satıra: hangi kalem, hangi kaynak satırı, önerilen sabit (uygulamadan).
   - Ayrıca: toptancı acil malı kaç kez kullanıldı, boş raf ayları, mal parası uyarısı sayısı; patron maaşı/kâr payı/servet.
   - `AutoPlay.LateCarefulGrowth`: yeni botla geçiyor mu? Geçmiyorsa eşiği değiştirme; gün ve neden.
4. **Menü:** C8'in üç kalanı için yalnız görüntü: haftalık grafik (işaretli eksen), kampanya formu, ilk gün yönetim/reklam.
5. **Teslim:** commit + push; `A.md`'ye "C9".

Kurallar: oyun sabitlerini değiştirme (öneri yaz), eski kayıt dalı yazma (M27), DERLE + TEST + Smoke geçmeden bitti deme.

Bitince kısa Türkçe özet: düzeltmeler, testler, §5 tablosu (tarz başına, hedefe göre ✓/✗), en önemli 5 denge önerisi (kaynak satırıyla), LateCarefulGrowth.
