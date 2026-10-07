import assert from 'node:assert/strict';
import {sanitizeProfileName, canonicalName, detectCommunityProfile, characterInventory, crc32Bytes} from '../web/extractor-core.mjs';

assert.equal(sanitizeProfileName('tap battle android 14'),'tap_battle_android_14');
assert.equal(sanitizeProfileName('CON'),'Mod_CON');
assert.equal(sanitizeProfileName('***'),'Mod');
assert.equal(canonicalName('9036.pac','community14-invasion-05aa0c5e'),'common.pac');
assert.equal(canonicalName('095315.pac','community14-invasion-05aa0c5e'),'char15.pac');
assert.equal(canonicalName('normal.pac','community14-invasion-05aa0c5e'),'normal.pac');
assert.equal(detectCommunityProfile(['9036.pac','7E8F.pac','095300.pac','364E00.pac','91F90000.pac']),'community14-invasion-05aa0c5e');
const names=new Set();
for(let i=0;i<13;i++){const a=String(i).padStart(2,'0'),b=String(i).padStart(4,'0');names.add(`char${a}.pac`);names.add(`chardemo${a}.pac`);names.add(`charf${b}.pac`);}
const roster=characterInventory(names);assert.equal(roster.count,13);assert.equal(roster.runtimeCompatible,true);assert.equal(roster.completeIndices,'00..12');
const crc=(crc32Bytes(new TextEncoder().encode('123456789'))^0xffffffff)>>>0;assert.equal(crc,0xcbf43926);
console.log('Web extractor core tests: PASS');
