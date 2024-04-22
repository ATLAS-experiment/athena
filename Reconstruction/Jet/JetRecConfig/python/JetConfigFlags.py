# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode
from JetRecConfig.StandardJetContext import createJetContextFlags

def createJetConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag("Jet.doUpstreamDependencies", False)
    flags.addFlag("Jet.WriteToAOD", 
                  lambda prevFlags: prevFlags.Reco.HIMode in [HIMode.HI,HIMode.UPC,HIMode.HIP] )
    flags.addFlag("Jet.useCalibJetThreshold", True)

    flags.addFlagsCategory("Jet.Context",createJetContextFlags)
    return flags


# This jet internal flag is to signal some jet components we're in a RAW/RDO -> ESD job.
# As long as we need compatibility with runII reco steering, it seems we can't rely on other AthConfigFlags
# And we need to be able to switch such flag from JetRec_jobOptions.py or JetRecoSteeringCfg.py
# thus we have this internal flags 
jetInternalFlags = AthConfigFlags()
jetInternalFlags.addFlag("isRecoJob", False)
