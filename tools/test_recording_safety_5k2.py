"""Guardrails for transactional recordClip; static checks, not hardware tests."""
from pathlib import Path
s=(Path(__file__).resolve().parents[1]/'src/main.cpp').read_text()
a=s.index('void recordClip(){')
b=s.index('// Generate output from the original PCM',a)
record=s[a:b]
assert record.index('heap_caps_malloc(kMaxSamples*sizeof(int16_t)') < record.index('M5.Mic.begin()')
assert 'M5.Mic.record(staged+newCount,count,kSampleRate)' in record
assert 'tracks[activeTrack].pcm=staged;' in record
assert record.index('saved=saveTrack(activeTrack)') < record.index('free(const_cast<int16_t*>(previousSamples))')
assert 'tracks[activeTrack].pcm=const_cast<int16_t*>(previousSamples);' in record
assert 'tracks[activeTrack].length=previousLength;' in record
assert 'if(!captureError&&newCount>0)' in record
assert 'storageSaveFailed=true; // No new recording was committed' in record
assert 'if(autoPlay&&state==State::Idle&&!storageSaveFailed&&captured>0)playClip();' in s
assert 'LittleFS.begin(false)' in s
assert 'LittleFS.begin(true)' not in s
assert 'upload_port = COM9' not in (Path(__file__).resolve().parents[1]/'platformio.ini').read_text()
print('5K.2 recording transaction source contracts: PASS (hardware runtime unverified)')
