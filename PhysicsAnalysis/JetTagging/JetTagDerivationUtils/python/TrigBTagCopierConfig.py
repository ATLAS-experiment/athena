# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TrigBTagCopierAlgCfg(flags):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.ftag.TrigBTagCopierAlg(
            'TrigBTagCopierAlg',
            TrigEDMVersion = flags.Trigger.EDMVersion,
            # OutputLevel=2,
        )
    )
    return acc