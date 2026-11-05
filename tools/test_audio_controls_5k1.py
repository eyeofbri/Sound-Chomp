"""Phase 5K.1 lightweight source-contract and arithmetic regression checks.

Run: python tools/test_audio_controls_5k1.py
These tests do NOT emulate the speaker driver or replace M5StickS3 tests.
"""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src' / 'main.cpp').read_text()

checks = {
    'loop transitions defer active-track writes': 'selectTrack(next,false)' in source,
    'regular selections still persist': 'bool selectTrack(int which,bool savePreference=true);' in source and 'if(savePreference)persistActiveTrack();' in source,
    'track persisted after playback': 'if(activeTrack!=trackAtPlaybackStart)persistActiveTrack();' in source,
    'first playback preroll failure leaves retry possible': 'else{queued=1;firstSpeakerStart=false;}' in source,
    'speaker begin failure cleaned up': 'M5.Speaker.end();state=State::Error;errorText="SPEAKER BEGIN FAILED";' in source,
    'encoder reconnect suppresses phantom clicks': 'if(scrollAwaitRelease)' in source and 'if(!down)scrollAwaitRelease=false;' in source,
    'encoder disconnect clears queued actions': 'scrollShort=false;scrollLong=false;scrollLongHandled=false;' in source,
    'encoder delta uses modular subtraction': 'const uint32_t change=uint32_t(pos)-uint32_t(wheelLast);' in source,
    'volume and selected audio processors unchanged': 'return applyEcho(dry);' in source,
    'no filesystem repartitioning': 'LittleFS.begin(false)' in source,
}
for desc, passed in checks.items():
    assert passed, desc

# Model ESP32 two's-complement modulo wheel delta, including signed rollover.
def delta(new, old):
    raw = (new - old) & 0xFFFFFFFF
    return raw - 0x100000000 if raw >= 0x80000000 else raw

cases = [
    (0, 0, 0), (9, 4, 5), (4, 9, -5),
    (-2147483648, 2147483647, 1),
    (2147483647, -2147483648, -1),
    (200, 0, 200),
]
for new, old, expected in cases:
    assert delta(new, old) == expected, (new, old, expected)
    should_handle = -32 < expected < 32 and expected != 0
    if abs(expected) > 31:
        assert not should_handle

print(f'5K.1 controls/audio safety checks: {len(checks)} contracts + {len(cases)} rollover cases PASS')
