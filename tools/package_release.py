"""Create reproducible source or compiled-binary release archives.

Usage:
    python tools/package_release.py --source
    python tools/package_release.py --firmware   # requires a successful PlatformIO build
"""
from __future__ import annotations
import argparse
import hashlib
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
VERSION = (ROOT / 'tools/release_version.txt').read_text().strip()
ENV = 'm5stack-sticks3'
EXCLUDED = {'.pio', '.git', 'dist', '__pycache__', '.pytest_cache', '.DS_Store'}


def archive(zippath: Path, members: list[tuple[Path, str]]) -> None:
    zippath.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(zippath, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for local, name in sorted(members, key=lambda pair: pair[1]):
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = (0o644 & 0xFFFF) << 16
            z.writestr(info, local.read_bytes(), compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    with zipfile.ZipFile(zippath) as z:
        assert z.testzip() is None
    print(f'Created {zippath} ({zippath.stat().st_size:,} bytes)')


def make_source(out: Path) -> None:
    files = [p for p in ROOT.rglob('*') if p.is_file()
             and not any(segment in EXCLUDED for segment in p.relative_to(ROOT).parts)
             and p.suffix not in {'.pyc', '.pyo'}]
    archive(out / f'Sound-Chomp-{VERSION}-Source.zip',
            [(p, f'Sound-Chomp/{p.relative_to(ROOT).as_posix()}') for p in files])


def make_firmware(out: Path) -> None:
    build = ROOT / '.pio' / 'build' / ENV
    firmware = build / 'firmware.bin'
    if not firmware.is_file():
        raise SystemExit(f'No compiled firmware at {firmware}. Run: pio run -e {ENV}')
    binaries = [build / name for name in ('firmware.bin', 'bootloader.bin', 'partitions.bin', 'boot_app0.bin')]
    binaries = [p for p in binaries if p.exists()]
    notes = [f'Sound-Chomp {VERSION} binary artifacts for {ENV}.',
             'These are build outputs, not a ready-to-use one-click upgrade image.',
             'Use PlatformIO Upload for a normal update that preserves existing recordings.',
             'Never erase flash or overwrite LittleFS to apply an ordinary firmware upgrade.',
             'Inspect the exact build flash offsets and partition layout before manual flashing.', '']
    notes.extend(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}' for p in binaries)
    archive_members = [(p, f'Sound-Chomp-Firmware/{p.name}') for p in binaries]
    # Keep manifest in ZIP without generating extra files in the user's source tree.
    zippath = out / f'Sound-Chomp-{VERSION}-Firmware.zip'
    out.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(zippath, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for local, name in sorted(archive_members, key=lambda x: x[1]):
            z.write(local, name)
        z.writestr('Sound-Chomp-Firmware/MANIFEST.txt', '\n'.join(notes) + '\n')
    with zipfile.ZipFile(zippath) as z:
        assert z.testzip() is None
    print(f'Created {zippath} ({zippath.stat().st_size:,} bytes)')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--source', action='store_true')
    group.add_argument('--firmware', action='store_true')
    parser.add_argument('--out', type=Path, default=ROOT / 'dist')
    args = parser.parse_args()
    (make_source if args.source else make_firmware)(args.out)
