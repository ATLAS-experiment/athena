# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def createPhysValConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("PhysVal.OutputFileName", "")

    icf.addFlag("PhysVal.doExample", True)
    icf.addFlag("PhysVal.doInDet", False)
    icf.addFlag("PhysVal.doInDetLargeD0", False)
    icf.addFlag("PhysVal.doBtag", False)
    icf.addFlag("PhysVal.doMET", False)
    icf.addFlag("PhysVal.doEgamma", False)
    icf.addFlag("PhysVal.doTau", False)
    icf.addFlag("PhysVal.doJet", False)
    icf.addFlag("PhysVal.doTopoCluster", False)
    icf.addFlag("PhysVal.doZee", False)
    icf.addFlag("PhysVal.doPFlow", False)
    icf.addFlag("PhysVal.doMuon", False)
    icf.addFlag("PhysVal.doLRTMuon", False)
    icf.addFlag("PhysVal.doLLPSecVtx", False)
    icf.addFlag("PhysVal.doLLPSecVtxLeptons", False)

    icf.addFlag("PhysVal.GRLs", ['GRL2015', 'GRL2016', 'GRL2017_Triggerno17e33prim', 'GRL2018_Triggerno17e33prim',
                'GRL2022', 'GRL2023', 'GRL2024'], help='List of GRL names to be used by PhysVal.')
    icf.addFlag("PhysVal.applyAllDataCleaning", False,
                help='Apply all data cleaning cuts, (applyGRL, applyEventStatusSelection, and Photon OQ).')
    icf.addFlag("PhysVal.applyGRL", False,
                help='Apply Good Run List selection. The GRL is hardcoded, it may be not updated.')
    icf.addFlag("PhysVal.applyEventStatusSelection", False,
                help='Apply Event Status selection via EventStatusSelectionAlg.')

    from InDetPhysValMonitoring.InDetPhysValFlags import createIDPVMConfigFlags
    icf.addFlagsCategory("PhysVal.IDPVM", createIDPVMConfigFlags, prefix=True)

    return icf
