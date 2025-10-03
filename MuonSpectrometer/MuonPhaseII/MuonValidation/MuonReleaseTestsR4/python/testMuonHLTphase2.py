#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
CA module to configure a test for the HLT running with phase2 geometry 
building style for athena. A default RUN3 input file is provided, as well as
a default geometry file. The --defaultGeoFile argument allows to easily
switch between RUN3 and RUN4 layout. If desired, it is possible to 
provide a customized input file using the --filesInput argument. A default
RUN4 input file will be provided in future.

Usage:
  athena [options] MuonReleaseTestsR4/test_muonHLTphase2.py [flags]
  python -m MuonReleaseTestsR4.test_muonHLTphase2  # not recommended (due to missing LD_PRELOADs)

"""

if __name__ == "__main__":
   
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    parser = flags.getArgumentParser()
    parser.add_argument("--defaultGeoFile", help="Use the  predefined GeoModel files on cvmfs", choices=["RUN3", "RUN4"], required=True)

    args = flags.fillFromArgs(parser=parser)

    # Use  RUN3 ttbar MC dataset having muons if the input file is not provided. A RUN4 default dataset will be provided in future.
    if vars(args).get('filesInput', None) is None:
        flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TriggerTest/valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4159_s4114_r14799_tid34171421_00/RDO.34171421._000011.pool.root.1",
                             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TriggerTest/valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4159_s4114_r14799_tid34171421_00/RDO.34171421._000016.pool.root.1"]

    # Set phase2 geometry 
    from MuonGeoModelTestR4.testGeoModel import geoModelFileDefault, configureDefaultTagsCfg
    flags.GeoModel.SQLiteDB = True
    flags.GeoModel.SQLiteDBFullPath = geoModelFileDefault(useR4Layout = (args.defaultGeoFile == "RUN4"))

    # Set default geometry tag and default condition tag
    configureDefaultTagsCfg(flags)
        
    # Configure monitoring
    if vars(args).get('perfmon', None) is None:
        flags.PerfMon.doFastMonMT = True   # also doFullMonMT is available
    
    # Set only the muon slice of HLT
    flags.Trigger.triggerMenuSetup = "Physics_pp_run3_v1"
    flags.Trigger.enabledSignatures = ["Muon"]
    flags.Trigger.selectChains = ["HLT_mu26_ivarmedium_L1MU14FCH", "HLT_mu22_mu8noL1_L1MU14FCH", "HLT_2mu14_L12MU8F"]

    import sys
    from TriggerJobOpts.runHLT import athenaCfg  
    
    sys.exit(athenaCfg(flags, parser = parser).run().isFailure())
