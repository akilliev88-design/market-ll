import { readFile, writeFile, copyFile, mkdir, readdir, rename } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const repo = path.join(root, 'Saved/ImageBlaster/image-blaster');
const inbox = path.join(root, 'AssetInbox/ImageBlaster/MahalleMarket');
const slug = 'mahalle-market';
const output = path.join(repo, 'worlds', slug, 'output/world');
const mode = process.argv[2] || 'check';
const prompt = 'An empty contemporary neighborhood grocery interior, approximately 8 by 10 meters. Preserve the source architecture, terrazzo floor, off-white plaster, simple ceiling and fixed columns. The street-facing facade is floor-to-ceiling glass with an open wide entrance onto the sidewalk; a small adjacent neighborhood street remains visible. Natural daylight. The rest of the building is enclosed normally. Clear walkable floor. No shelves, checkout, refrigerators, groceries, people or loose decoration. Preserve coherent straight walls and the open entrance; do not seal it with an opaque wall.';

async function run() {
  process.chdir(repo);
  const helpers = await import(pathToFileURL(path.join(repo, '.claude/scripts/asset-pipeline/fal-queue.mjs')));
  await helpers.loadDotEnv();
  const key = process.env.WORLD_LABS_API_KEY;
  const ready = Boolean(key && key !== 'BURAYA_ANAHTARINI_YAZ');
  if (mode === 'check') {
    await readFile(path.join(inbox, 'mahalle-market-bos.png'));
    await import(pathToFileURL(path.join(repo, '.claude/scripts/world/generate-world.mjs')));
    console.log('Kaynak gorsel ve uretim modulu hazir.');
    console.log(`World Labs anahtari: ${ready ? 'hazir' : 'eksik'}`);
    return;
  }
  if (!ready) throw new Error('Once Saved/ImageBlaster/image-blaster/.env dosyasina World Labs API anahtarini ekle.');
  if (mode === 'generate') {
    const { ensureProjectState } = await import(pathToFileURL(path.join(repo, '.claude/scripts/project/project-state.mjs')));
    await ensureProjectState({ slug, displayName: 'Mahalle marketi' });
    const source = path.join(repo, 'worlds', slug, 'source/1-market.png');
    await copyFile(path.join(inbox, 'mahalle-market-bos.png'), source);
    const { generateWorld } = await import(pathToFileURL(path.join(repo, '.claude/scripts/world/generate-world.mjs')));
    console.log('Marble 1.1 uretimi basliyor (yaklasik 1580 kredi). Mevcut is varsa devam edilir.');
    const result = await generateWorld({ world: slug, image: source, prompt });
    await mkdir(path.join(root, 'Saved/ImageBlaster'), { recursive: true });
    await writeFile(path.join(root, 'Saved/ImageBlaster/generation-result.json'), JSON.stringify(result, null, 2));
    console.log(`Ortam dosyalari: ${output}`);
    return;
  }
  if (mode !== 'export') throw new Error('Kullanim: market-world.mjs check|generate|export');
  const files = (await readdir(output)).filter(n => /^\d+-world\.json$/.test(n)).sort((a, b) => parseInt(b) - parseInt(a));
  if (!files.length) throw new Error('Once MARKET_URET.cmd ile ortami uret.');
  const world = JSON.parse(await readFile(path.join(output, files[0]), 'utf8'));
  const id = world.world_id || world.id;
  if (!id) throw new Error('Dunya kimligi bulunamadi; world JSON dosyasini incelemek gerekiyor.');
  const dest = path.join(root, 'Saved/ImageBlaster/UnrealImport');
  await mkdir(dest, { recursive: true });
  const target = path.join(dest, 'mahalle-market-textured.glb');
  try { await readFile(target); console.log(`Model zaten hazir: ${target}`); return; } catch (e) { if (e.code !== 'ENOENT') throw e; }
  const request = async (url, body) => {
    const response = await fetch(url, {
      method: body ? 'POST' : 'GET',
      headers: { 'WLT-Api-Key': key, ...(body ? { 'Content-Type': 'application/json' } : {}) },
      ...(body ? { body: JSON.stringify(body) } : {}),
    });
    if (!response.ok) throw new Error(`World Labs HTTP ${response.status}; hesap/kredi durumunu kontrol et.`);
    return response.json();
  };
  const base = 'https://api.worldlabs.ai/marble/v1';
  const meta = path.join(dest, 'mesh-export-operation.json');
  let previous;
  try { previous = JSON.parse(await readFile(meta, 'utf8')); } catch (e) { if (e.code !== 'ENOENT') throw e; }
  console.log('Dokulu HQ model aktarimi (yeni aktarimsa 3500 kredi; mevcut is tekrar kullanilir).');
  let operation = previous?.world_id === id && previous?.operation_id
    ? await request(`${base}/operations/${encodeURIComponent(previous.operation_id)}`)
    : await request(`${base}/worlds/${encodeURIComponent(id)}:export`, { asset_type: 'mesh', format: 'glb', mesh_variant: 'textured' });
  const saveOperation = () => writeFile(meta, JSON.stringify({ ...operation, world_id: id }, null, 2));
  await saveOperation();
  let polls = 0;
  while (!operation.done) {
    if (!operation.operation_id) throw new Error('Aktarim islem kimligi eksik.');
    await new Promise(resolve => setTimeout(resolve, 15000));
    operation = await request(`${base}/operations/${encodeURIComponent(operation.operation_id)}`);
    await saveOperation();
    if (++polls % 4 === 0) console.log('World Labs modeli hazirliyor; pencere kapanirsa is sunucuda devam eder.');
  }
  if (operation.error) throw new Error('HQ aktarimi basarisiz; mesh-export-operation.json dosyasini incele.');
  const url = operation.response?.url;
  if (!url) throw new Error('Dokulu model indirme adresi gelmedi; aktarim kaydini incele.');
  const response = await fetch(url);
  if (!response.ok) throw new Error(`Model indirme HTTP ${response.status}`);
  const bytes = Buffer.from(await response.arrayBuffer());
  if (bytes.length < 12 || bytes.toString('ascii', 0, 4) !== 'glTF') throw new Error('Indirilen dosya GLB degil.');
  await writeFile(target + '.partial', bytes);
  await rename(target + '.partial', target);
  console.log(`Unreal icin model hazir: ${target}`);
}

run().catch(error => { console.error(error.message); process.exitCode = 1; });
