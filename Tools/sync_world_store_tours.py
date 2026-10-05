"""Install the four authored layouts, backing up any local edits first."""
import json, shutil
from datetime import datetime
from validate_stores import ROOT
folder=ROOT/'Saved/StoreTours';folder.mkdir(parents=True,exist_ok=True)
backup=ROOT/'Saved/StoreBackups'/('BeforeWorldLayouts_'+datetime.now().strftime('%Y%m%d_%H%M%S'))
for s in json.loads((ROOT/'Config/magazalar.json').read_text(encoding='utf-8'))['stores']:
    path=folder/(s['id']+'.json');data=dict(schemaVersion=1,stores=[s])
    if path.exists() and json.loads(path.read_text(encoding='utf-8-sig'))==data:continue
    if path.exists():backup.mkdir(parents=True,exist_ok=True);shutil.copy2(path,backup/path.name)
    path.write_text(json.dumps(data,ensure_ascii=False,indent='\t')+'\n',encoding='utf-8')
    print('LOCAL_STORE_TOUR_UPDATED='+s['id'])
print('LOCAL_TOUR_BACKUP='+str(backup))
