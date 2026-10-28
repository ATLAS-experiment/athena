# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration
"""Headless regression tests using production method bodies and geometry doubles.

This isolates state/selection decisions; it does not test Qt, Coin or GeoModel ABI.
Run with --sanitize for AddressSanitizer and UndefinedBehaviorSanitizer checks.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--compiler", default="c++")
parser.add_argument("--sanitize", action="store_true")
args = parser.parse_args()
root = Path(__file__).resolve().parents[1] / "src"
def extract(source, signature):
    start=source.index(signature); brace=source.index('{',start); end=brace+1; depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}'); end+=1
    return source[start:end]
source=(root/'VP1GeometrySystem.cxx').read_text()
functions='\n'.join(extract(source,s) for s in ['bool itkNameIsOneOf(', 'bool itkNameStartsWithOneOf(', 'bool VP1GeometrySystem::Imp::selectITkComponent('])
state=extract((root/'VolumeHandle.cxx').read_text(),'void VolumeHandle::setState(')
prefix=r'''
#include <cassert>
#include <string>
#include <vector>
#include <initializer_list>
#include <iostream>
struct QString : std::string {
 using std::string::string;
 bool contains(const char* s) const {return find(s)!=npos;}
 bool startsWith(const char* s) const {return rfind(s,0)==0;}
 bool endsWith(const char* s) const {std::string t(s); return size()>=t.size() && compare(size()-t.size(),t.size(),t)==0;}
};
namespace VP1GeoFlags {enum VOLSTATE {CONTRACTED,EXPANDED,ZAPPED};}
struct VolumeHandle;
struct Common {void removeZappedVolumesFromGui(VolumeHandle*){} void addZappedVolumeToGui(VolumeHandle*){}};
struct Data {Common* commondata; bool isattached=false; void attach(VolumeHandle*){isattached=true;} void detach(){isattached=false;}};
struct VolumeHandle {
 using List=std::vector<VolumeHandle*>;
 enum {MUONCHAMBER,MUONCHAMBER_DIRTY}; int m_muonChamberState=0;
 Common common; Data data{&common}; Data* m_d=&data;
 VP1GeoFlags::VOLSTATE m_state=VP1GeoFlags::CONTRACTED;
 QString name,tag; bool ether; VolumeHandle* p=nullptr; List children;
 VolumeHandle(const char* n,const char* t="",bool e=false):name(n),tag(t),ether(e){}
 void add(VolumeHandle& c){children.push_back(&c);c.p=this;}
 QString getName(){return name;} QString getNameTag(){return tag;}
 bool isEther(){return ether;} bool isInMuonChamber(){return false;}
 VolumeHandle* topLevelParent(){return p?p->topLevelParent():this;}
 VolumeHandle* parent(){return p;}
 auto childrenBegin(){return children.begin();} auto childrenEnd(){return children.end();}
 unsigned nChildren(){return children.size();} bool childrenAreInitialised(){return true;}
 void initialiseChildren(){assert(nChildren());}
 bool haveParentsNotExpanded(){return p && (p->m_state!=VP1GeoFlags::EXPANDED || p->haveParentsNotExpanded());}
 void attachAllContractedChildren(){for(auto c:children){if(c->m_state==VP1GeoFlags::CONTRACTED)c->m_d->attach(c);else if(c->m_state==VP1GeoFlags::EXPANDED)c->attachAllContractedChildren();}}
 void detachAllContractedChildren(){for(auto c:children){if(c->m_state==VP1GeoFlags::CONTRACTED)c->m_d->detach();else if(c->m_state==VP1GeoFlags::EXPANDED)c->detachAllContractedChildren();}}
 void setState(const VP1GeoFlags::VOLSTATE&,bool=true);
};
struct VP1GeometrySystem {struct Imp {bool selectITkComponent(VolumeHandle*,bool,bool,bool,bool,bool,bool,bool,bool,unsigned=0);};};
'''
tests=r'''
int main(){
 VP1GeometrySystem::Imp selector;
 for(bool pixel:{false,true}) {
  VolumeHandle root("Detector"), support("AssemblyLV",pixel?"StaveSupport":"PetalSupport",true),
   sensors("AssemblyLV","Sensors",true), sensor(pixel?"Module_Sensor":"BRLSensor0"),
   electronics("AssemblyLV","B_HybridNear",true), chip(pixel?"Module_Chip":"ASIC"),
   services("AssemblyLV","SV_BarrelService",true), cable("Cable"), plate("Plate");
  root.add(support);support.add(sensors);sensors.add(sensor);support.add(electronics);electronics.add(chip);
  support.add(plate);root.add(services);services.add(cable);
  const unsigned limit=pixel?8:16;
  // Repeat in both directions so stale states from earlier selections are exercised.
  for(unsigned pass=0;pass<2;++pass)for(unsigned i=0;i<limit;++i){
   unsigned mask=pass?limit-1-i:i;
   selector.selectITkComponent(&root,pixel,mask&1,mask&2,mask&4,mask&1,mask&2,mask&4,mask&8);
   assert(sensor.data.isattached==bool(mask&1));
   assert(chip.data.isattached==bool(mask&(pixel?1:2)));
   assert(cable.data.isattached==bool(mask&(pixel?2:4)));
   assert(plate.data.isattached==bool(mask&(pixel?4:8)));
   assert(!support.data.isattached && !sensors.data.isattached && !services.data.isattached);
   assert((services.m_state==VP1GeoFlags::ZAPPED)==!(mask&(pixel?2:4)));
   if (!mask) assert(root.m_state==VP1GeoFlags::ZAPPED);
  }
 }
 VolumeHandle parent("Parent"), ether("AssemblyLV","SV",true), leaf("Cable");
 parent.add(ether);ether.add(leaf);
 // Visiting child lists must leave the parent representation contracted.
 parent.data.attach(&parent);
 parent.initialiseChildren();
 assert(parent.data.isattached && ether.m_state==VP1GeoFlags::CONTRACTED);
 assert(!ether.data.isattached && !leaf.data.isattached);
 // Explicit filter state changes beneath a hidden ancestor must not attach anything.
 leaf.setState(VP1GeoFlags::ZAPPED,false);leaf.setState(VP1GeoFlags::CONTRACTED,false);
 assert(!leaf.data.isattached);
 parent.setState(VP1GeoFlags::EXPANDED);
 assert(ether.m_state==VP1GeoFlags::EXPANDED && leaf.data.isattached);
 // A rejected ether branch must survive an explicit parent filter expansion.
 parent.setState(VP1GeoFlags::ZAPPED,false);
 ether.setState(VP1GeoFlags::ZAPPED,false);
 parent.setState(VP1GeoFlags::EXPANDED,false);
 assert(ether.m_state==VP1GeoFlags::ZAPPED && !leaf.data.isattached);
 // Empty ether leaves have no real geometry to select.
 VolumeHandle empty("AssemblyLV","SV_Empty",true);
 assert(!selector.selectITkComponent(&empty,false,false,false,false,false,false,true,false));
 assert(empty.m_state==VP1GeoFlags::ZAPPED && !empty.data.isattached);
 std::cout<<"PASS: 48 repeated checkbox combinations, hidden ancestors, and ordinary ether expansion\n";
}
'''
with tempfile.TemporaryDirectory(prefix="vp1-itk-test-") as directory:
    out = Path(directory) / "ITkSelection_test.cxx"
    out.write_text(prefix + state + "\n" + functions + "\n" + tests)
    command = [args.compiler, "-std=c++17", "-UNDEBUG", "-g"]
    if args.sanitize:
        command.append("-fsanitize=address,undefined")
    command += [str(out), "-o", str(out.with_suffix(""))]
    subprocess.run(command, check=True)
    subprocess.run([str(out.with_suffix(""))], check=True)
