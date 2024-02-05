# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


def TrigGenericHypoToolFromDict(chainDict):
    passString = ''
    if 'mistimemonj400' in chainDict['chainParts'][0]['monType']:
        passString = "HLT_TrigCompositeMistimeJ400.pass"

    tool = CompFactory.TrigGenericHypoTool(name=chainDict['chainName'],
                                           PassString=passString)
    return tool
