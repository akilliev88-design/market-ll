"""Install the requested larger layouts in local tours, keeping edited backups."""
import json,shutil
from pathlib import Path
from datetime import datetime
root=Path(__file__).resolve().parents[1]
folder=root/'Saved/StoreTours';folder.mkdir(parents=True,exist_ok=True)
backup=root/'Saved/StoreBackups'/('BeforeHandmadeLarge_'+datetime.now().strftime('%Y%m%d_%H%M%S'))
for s in json.loads((root/'Config/magazalar.json').read_text(encoding='utf-8'))['stores']:
    if s['id'] not in ('kucuk_01','buyuk_01','hiper_01'):continue
    path=folder/(s['id']+'.json');data=dict(schemaVersion=1,stores=[s])
    if path.exists() and json.loads(path.read_text(encoding='utf-8-sig'))==data:continue
    if path.exists():backup.mkdir(parents=True,exist_ok=True);shutil.copy2(path,backup/path.name)
    path.write_text(json.dumps(data,ensure_ascii=False,indent='\t')+'\n',encoding='utf-8')
    print('LOCAL_STORE_TOUR_UPDATED='+s['id'])
print('LOCAL_TOUR_BACKUP='+str(backup))
