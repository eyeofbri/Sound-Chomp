"""Native C++ extraction smoke test for slot arbitration (requires g++)."""
import subprocess,tempfile,pathlib
source=pathlib.Path(__file__).resolve().parents[1]/'src/main.cpp'
s=source.read_text()
start=s.index('enum EffectId:uint8_t')
end=s.index('const char* slotLabel',start)
segment=s[start:end]
program='''#include <cstdint>
#include <cassert>
#include <cstdio>
'''+segment+'''
int main(){
  assert(effectForSlot(1)==FxEcho);
  assert(effectForSlot(2)==FxStutter);
  assert(effectForSlot(3)==FxRobot);
  setEffectSlot(FxWarble,2);
  assert(effectForSlot(2)==FxWarble);
  assert(effectSlot[FxStutter]==0);
  effectOn[FxWarble]=true;
  setEffectSlot(FxWarble,0);
  assert(effectForSlot(2)<0);
  assert(!effectOn[FxWarble]);
  setEffectSlot(FxTelephone,3);
  assert(effectSlot[FxRobot]==0);
  assert(effectForSlot(3)==FxTelephone);
  setEffectSlot(FxBitcrush,1);
  assert(effectForSlot(1)==FxBitcrush);
  assert(effectSlot[FxEcho]==0);
  puts("FX slot assignment and displacement: PASS");
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp=pathlib.Path(directory)/'test.cpp'; exe=pathlib.Path(directory)/'test'
    cpp.write_text(program)
    subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
