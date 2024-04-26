# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createPerfMonConfigFlags():
    pcf = AthConfigFlags()

    # Two basic flags for PerfMonMT
    pcf.addFlag('PerfMon.doFastMonMT', False)
    pcf.addFlag('PerfMon.doFullMonMT', False)
    pcf.addFlag('PerfMon.OutputJSON', 'perfmonmt.json')
    pcf.addFlag('PerfMon.doGPerfProf', False)
    # List of algorithms to profile e.g from
    # callgrind/valkyrie or Vtune
    pcf.addFlag('PerfMon.VTune.ProfiledAlgs', [])
    pcf.addFlag('PerfMon.Valgrind.ProfiledAlgs', [])
    
    return pcf
