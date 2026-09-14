# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg

_doneAODFixes = set()


def AODFixCfg(flags):
    msg = logging.getLogger("AODFixCfg")

    if not isinstance(flags.Input.AODFixesDone, set):
        raise TypeError("flags.Input.AODFixesDone must be a set, but is %s" % type(flags.Input.AODFixesDone))

    if flags.Input.AODFixesDone:
        msg.info("Already done AOD fixes = %s", flags.Input.AODFixesDone)

    for doneFix in flags.Input.AODFixesDone:
        _doneAODFixes.add(doneFix)

    result = ComponentAccumulator()

    # #Add list of known AOD Fixes here:
    from egammaAlgs.egammaAODFixesConfig import egammaAODFixesCfg
    from LArCellRec.EventInfoClearAlgConfig import EventVetoCearAlgCfg
    listOfFixes = [
        egammaAODFixesCfg,
        EventVetoCearAlgCfg,
    ]

    fixApplied = False
    for aodFix in listOfFixes:
        # Basic string check to avoid applying the same AODFix twice, even if it is called from different places
        if aodFix.__name__ in _doneAODFixes:
            msg.warning("AODFix %s already applied, not applying again", aodFix.__name__)
            continue

        # The method is supposed to verify if the AOD-fix must be applied for the input data,
        # typically based on flags.Input.Release. If yes, returns a ComponentAccumulator, otherwise None
        ca = aodFix(flags)
        if ca is not None:
            msg.info("Applying AOD fix %s", aodFix.__name__)
            result.merge(ca)
            if " " in aodFix.__name__:
                for fix in aodFix.__name__.split(" "):
                    _doneAODFixes.add(fix)
            else:
                _doneAODFixes.add(aodFix.__name__)
            fixApplied = True
        else:
            msg.info("AODFix \"%s\" not applicable for this input AOD", aodFix.__name__)

    if fixApplied:
        result.merge(TagInfoMgrCfg(flags,{"AODFixVersion":" ".join(_doneAODFixes)}))
    else:
        msg.info("No AOD fix scheduled")
    return result
