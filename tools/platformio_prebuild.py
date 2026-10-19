"""PlatformIO pre-build hook: generate UI assets from edited PNG automatically."""
Import('env')
import subprocess
from pathlib import Path
root=Path(env.subst('$PROJECT_DIR'))
subprocess.check_call([env.subst('$PYTHONEXE'),str(root/'tools/generate_ui_assets.py')])
