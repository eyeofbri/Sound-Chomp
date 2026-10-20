#!/usr/bin/env python3
"""Regenerate assets and verify generated references for built-in skins."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[1]
for tool in ('generate_ui_assets.py','generate_builtin_skins.py'):
 subprocess.run([sys.executable,str(root/'tools'/tool)],check=True)
h=(root/'include/generated/builtin_skins.h').read_text()
for name in ('minimal','arcade'):
 for orientation in ('landscape','portrait'):
  for theme in ('Green','Red','Blue'):
   assert f'{name}_bg_{orientation}_{theme}' in h
print('All 12 alternate background variants and sprite mappings generated.')
