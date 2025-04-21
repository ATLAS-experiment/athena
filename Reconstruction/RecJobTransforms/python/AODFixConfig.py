# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
from AthenaCommon.Logging import logging


_doneAODFixes=set()


def AODFixCfg(flags):

    nAODFixes=0
    msg=logging.getLogger("AODFixCfg")
    aodFixesDone=flags.Input.AODFixesDone

    if isinstance(aodFixesDone,str):
        aodFixesDone=aodFixesDone.split()

    for doneFix in aodFixesDone:
        _doneAODFixes.add(doneFix)

    result=ComponentAccumulator()

    # #Add list of known AOD Fixes here:
    from RecJobTransforms.AODFixEGAmbiguityLinksConfig import AODFixEGAmbiguityLinksCfg
    listOfFixes=[AODFixEGAmbiguityLinksCfg,]

    for aodFix in listOfFixes:
        aodFixName=aodFix.__name__ 
        if aodFixName in _doneAODFixes:
            msg.warning("AODFix %s already applied, not applying again",aodFixName)
        
        ca=aodFix(flags)
        #The method is supposed to verify if the AOD-fix must be applied for the input data,
        #typically based on flags.Input.Release. If yes, returns a ComponentAccumulator, otherwise None
        if ca is not None:
            msg.info("Applying AOD fix %s",aodFixName)
            result.merge(ca)
            _doneAODFixes.add(aodFixName)
            nAODFixes+=1
        else:
            msg.info("AODFix \"%s\" not applicable for this input AOD",aodFixName)


    if nAODFixes>0:
        result.merge(TagInfoMgrCfg(flags,{"AODFixVersion":" ".join(_doneAODFixes)}))
    else:
        msg.info("No AOD fix scheduled")
    return result
