# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Function to recreate CSSKGParticleFlowObjects and get the links to the PFlows associated to the UFOs.
def CSSKMergingCfg(flags):
    from JetRecConfig.JetRecConfig import getInputAlgs
    from JetRecConfig.StandardJetConstits import stdConstitDic
    algs = getInputAlgs(stdConstitDic.GPFlowCSSK, flags)
    acc = ComponentAccumulator()
    # instantiate algorithm configuration object setting its properties
    del algs[2] # remove a pflowselalgs starting from original pflow not existing in input FTAG1
    for a in algs:
        acc.addEventAlgo(a)
    return acc
