import { extractApks, LIMITS } from './extractor-core.mjs';

const $ = id => document.getElementById(id);
const fileInput=$('apk-input'), drop=$('drop-zone'), selected=$('selected-list'), extractBtn=$('extract-btn'), resetBtn=$('reset-btn');
const status=$('status-text'), detail=$('status-detail'), progress=$('progress-fill'), progressLabel=$('progress-label');
const result=$('result'), downloadBtn=$('download-btn'), shareBtn=$('share-btn'), resultInfo=$('result-info'), warning=$('warning-box');
let files=[], outputUrl=null, outputFile=null;

function human(bytes){if(bytes<1024)return `${bytes} B`;const u=['KiB','MiB','GiB'];let n=bytes/1024,i=0;while(n>=1024&&i<u.length-1){n/=1024;i++;}return `${n.toFixed(n>=100?0:n>=10?1:2)} ${u[i]}`;}
function setStatus(title,sub='',pct=0){status.textContent=title;detail.textContent=sub;progress.style.width=`${Math.max(0,Math.min(100,pct))}%`;progressLabel.textContent=`${Math.round(pct)}%`;}
function safeFileList(list){return [...list].filter(f=>/\.apk$/i.test(f.name));}
function renderFiles(){
  selected.innerHTML='';
  if(!files.length){selected.innerHTML='<div class="empty-row">NO APK SELECTED</div>';extractBtn.disabled=true;return;}
  for(const f of files){const row=document.createElement('div');row.className='file-row';row.innerHTML=`<img src="selector/select0_ball_1.png" alt=""><div><strong></strong><span></span></div>`;row.querySelector('strong').textContent=f.name;row.querySelector('span').textContent=human(f.size);selected.append(row);}
  extractBtn.disabled=false;
}
function choose(list){
  const next=safeFileList(list); warning.hidden=true;
  if(!next.length){warning.hidden=false;warning.textContent='Choose a Dragon Ball Tap Battle .apk file.';return;}
  const tooLarge=next.find(f=>f.size>LIMITS.MAX_APK_SIZE);if(tooLarge){warning.hidden=false;warning.textContent=`${tooLarge.name} is larger than the 1 GiB safety limit.`;return;}
  files=next;renderFiles();result.hidden=true;setStatus('APK READY','Press EXTRACT DATA FOR PS VITA',0);
}
fileInput.addEventListener('change',()=>choose(fileInput.files));
drop.addEventListener('click',()=>fileInput.click());
drop.addEventListener('keydown',e=>{if(e.key==='Enter'||e.key===' '){e.preventDefault();fileInput.click();}});
for(const ev of ['dragenter','dragover'])drop.addEventListener(ev,e=>{e.preventDefault();drop.classList.add('dragging');});
for(const ev of ['dragleave','drop'])drop.addEventListener(ev,e=>{e.preventDefault();drop.classList.remove('dragging');});
drop.addEventListener('drop',e=>choose(e.dataTransfer.files));

resetBtn.addEventListener('click',()=>{files=[];fileInput.value='';if(outputUrl){URL.revokeObjectURL(outputUrl);outputUrl=null;}outputFile=null;result.hidden=true;warning.hidden=true;renderFiles();setStatus('SELECT APK','Everything runs locally on this device.',0);});

extractBtn.addEventListener('click',async()=>{
  if(!files.length)return;extractBtn.disabled=true;resetBtn.disabled=true;result.hidden=true;warning.hidden=true;
  try{
    const out=await extractApks(files,{onProgress:p=>setStatus((p.message||'WORKING').toUpperCase(),p.apk?`${p.apk}${p.file?` • ${p.file}`:''}`:'',p.percent||0)});
    if(outputUrl)URL.revokeObjectURL(outputUrl);outputFile=new File([out.blob],out.filename,{type:'application/zip'});outputUrl=URL.createObjectURL(outputFile);
    downloadBtn.href=outputUrl;downloadBtn.download=out.filename;
    const chars=out.reports.map(r=>r.layout==='raw'?`${r.apk}: original raw layout`:`${r.apk}: ${r.character_count} characters${r.character_runtime_compatible?'':' ⚠'}`).join(' • ');
    resultInfo.textContent=`${out.filename} • ${human(out.packageSize)} • ${out.files} files • ${chars}`;
    shareBtn.hidden=!(navigator.share&&navigator.canShare&&navigator.canShare({files:[outputFile]}));result.hidden=false;setStatus('DATA READY FOR PS VITA','Download the ZIP, extract it, then copy its data folder to ux0:.',100);
    result.scrollIntoView({behavior:'smooth',block:'nearest'});
  }catch(err){console.error(err);warning.hidden=false;warning.textContent=err?.message||String(err);setStatus('EXTRACTION FAILED','No partial package was published. Your APK is unchanged.',0);}
  finally{extractBtn.disabled=!files.length;resetBtn.disabled=false;}
});
shareBtn.addEventListener('click',async()=>{if(!outputFile)return;try{await navigator.share({files:[outputFile],title:'Dragon Ball Tap Battle Vita data',text:'Vita-ready data package generated locally in the browser.'});}catch(e){if(e?.name!=='AbortError'){warning.hidden=false;warning.textContent='Could not open the system share sheet. Use DOWNLOAD ZIP instead.';}}});

renderFiles();setStatus('SELECT APK','Everything runs locally on this device.',0);
