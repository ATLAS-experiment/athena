#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
log = logging.getLogger('decodeBS_TLA_AOD.py')


# Configure AOD output
def DAOD_TLA_OutputCfg(flags,additional_items=[]):
    """ Configure AOD output """
    acc = ComponentAccumulator()

    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    edmList = getTriggerEDMList(flags, key=flags.Trigger.AODEDMSet)

    ItemList = []
    for edmType, edmKeys in edmList.items():
        for key in edmKeys:
            ItemList.append(edmType+'#'+key)
    ItemList += [ "xAOD::EventInfo#EventInfo", "xAOD::EventAuxInfo#EventInfoAux." ]
    ItemList += [ 'xAOD::TrigCompositeContainer#*' ]
    ItemList += [ 'xAOD::TrigCompositeAuxContainer#*' ]
    ItemList += [ 'xAOD::TrigDecision#*' ]
    ItemList += [ 'xAOD::TrigDecisionAuxInfo#*' ]
    ItemList += [ 'xAOD::TrigConfKeys#*' ]
    ItemList += [ 'xAOD::BunchConfKey#*' ]
    ItemList += additional_items

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, "AOD", ItemList=ItemList))
    

    return acc
