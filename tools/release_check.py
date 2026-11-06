"""Local/CI release preflight. Hardware and toolchain builds are separate tests."""
from pathlib import Path
from urllib.parse import unquote
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
version = (ROOT / 'tools/release_version.txt').read_text().strip()
firmware_src = (ROOT / 'src/main.cpp').read_text()
assert re.search(r'kFirmwareVersion\s*=\s*"' + re.escape(version) + '"', firmware_src), 'About firmware version mismatch'
assert version in (ROOT / 'README.md').read_text(), 'README version mismatch'
assert version in (ROOT / 'CHANGELOG.md').read_text(), 'changelog version mismatch'
assert version in (ROOT / 'docs/releasing.md').read_text(), 'release guide version mismatch'
for required in ('LICENSE', 'platformio.ini', 'README.md', 'docs/getting-started.md', 'docs/hardware.md', 'Phase Notes/PHASE_5K3_NOTES.md', 'Phase Notes/PHASE_5K4_NOTES.md', '.github/workflows/build.yml'):
    assert (ROOT / required).is_file(), f'Missing {required}'
config = (ROOT / 'platformio.ini').read_text()
assert 'upload_port = COM' not in config and 'monitor_port = COM' not in config, 'Machine-specific COM port detected'
assert 'default_8MB.csv' in config, 'Partition scheme changed from validated baseline'
for md in (ROOT / 'README.md', *sorted((ROOT / 'docs').glob('*.md'))):
    content = md.read_text()
    for target in re.findall(r'(?<!!)\[[^]]+\]\(([^)]+)\)', content):
        if target.startswith(('http:', 'https:', '#', 'mailto:')):
            continue
        link = (md.parent / unquote(target.split('#')[0])).resolve()
        assert link.exists(), f'Broken Markdown link in {md.relative_to(ROOT)}: {target}'
print(f'Release preflight {version}: version, files, partition setting, and local doc links PASS')
for name in ('test_palette_remap.py', 'test_fx_slots.py', 'test_audio_controls_5k1.py', 'test_recording_safety_5k2.py', 'test_splash_orientation_5k3.py', 'validate_builtin_skins.py'):
    print(f'Running {name}...', flush=True)
    subprocess.run([sys.executable, str(ROOT/'tools'/name)], cwd=ROOT, check=True)
print('Release preflight: all source-level and asset checks PASS (not a firmware compile or device test)')
