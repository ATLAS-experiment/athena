#!/usr/bin/env python3

import os
from pathlib import Path
os.environ["EPO"] = os.environ["EPO"] + "/"
BASE = os.environ["EPO"]
os.environ["SRC"]    = f"{BASE}/src"
os.environ["SRCEXT"] = f"{BASE}/srcext"
os.environ["CONF"]   = f"{BASE}/config"
os.environ["DAT"]    = f"{BASE}/"
os.environ["OPT"]    = str(Path.cwd()) + "/"
os.environ["HTO"]    = str(Path.cwd()) + "/"
os.environ["CHK"]    = str(Path.cwd()) + "/"
OPT = os.environ["OPT"]
os.environ["OPX"] = str(Path.cwd()) + "/" if OPT == "./" else OPT

def build_config_content(energy, number_of_events, laproj=1, maproj=1, latarg=1, matarg=1, hydro=True,
                         list_of_particle_ids ="110 20 2130 -2130 2230 -2230 1130 -1130 1330 -1330 2330 -2330 3331 -3331",
                         centralityClass=0):
    
    # Type validation
    errors = []

    if not isinstance(energy, (int, float)):
        errors.append("energy must be a number (int or float).")

    if not isinstance(number_of_events, int):
        errors.append("number_of_events must be an integer.")
        
    if not isinstance(centralityClass, int):
        errors.append("centralityClass must be an integer (To check the class numbers, see the relevant file in: src/KWt/)")
        
    for name, value in [
        ("laproj", laproj),
        ("maproj", maproj),
        ("latarg", latarg),
        ("matarg", matarg),
    ]:
        if not isinstance(value, int):
            errors.append(f"{name} must be an integer.")

    if not isinstance(hydro, bool):
        errors.append("hydro must be a boolean (True/False).")

    if isinstance(list_of_particle_ids, (list, tuple)):
        try:
            list_of_particle_ids = " ".join(str(int(x)) for x in list_of_particle_ids)
        except ValueError:
            errors.append("list_of_particle_ids contains elements that cannot be converted to integers.")
    elif not isinstance(list_of_particle_ids, str):
        errors.append("list_of_particle_ids must be a string or a list of numbers.")

    if errors:
        print("INPUT PARAMETER ERRORS:")
        for e in errors:
            print(" -", e)
        return None
    
    
    content1 = f"""!EPOS4 configuration file
!to define the reaction and simulation options
!to define the output
!to add analysis plugins (optional)
!--------------------------------------------------------------------
! proton-proton collision no hydro no hadronic cascade
!--------------------------------------------------------------------
application hadron !hadron-hadron, hadron-nucleus, or nucleus-nucleus
set laproj {laproj} !projectile atomic number
set maproj {maproj} !projectile mass number
set latarg {latarg} !target atomic number
set matarg {matarg} !target mass number
set ecms {energy} !sqrt(s)_pp
set istmax 25 !max status considered for storage
set iranphi 1 !for internal use.
!if iranphi=1 event will be rotated such that the impact parameter angle
!and the (n=2) event plane angle (based on string segments) coincide.
!Particles rotated back at the end.
ftime on !string formation time non-zero
!suppressed decays (using EPOS id codes, see src/KWt/idt.dt)
nodecays {list_of_particle_ids} end
"""
    if hydro:
        content2=f"""core full !core/corona activated
    hydro hlle !hydro activated
    eos x3ff !eos activated
    hacas full !hadronic cascade activated
    """ 
    else:
        content2=f"""core off !core/corona not activated
    hydro off !hydro not activated
    eos off !eos not activated
    hacas off !hadronic cascade not activated
    """ 
    content3 =f"""set ninicon 1 !number of initial conditions used for hydro evolution
set nfull {number_of_events} !number of events
set nfreeze 1 !number of freeze out events per hydro event
set modsho 1 !printout every modsho events
set centrality {centralityClass} !0=min bias
"""

    return content1 + content2 + content3

