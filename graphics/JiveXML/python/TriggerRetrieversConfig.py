# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TriggerRetrieversCfg(flags):
    result = ComponentAccumulator()
    if flags.Reco.EnableTrigger:
        #--- LVL1 result from TrigDecision
        result.addPublicTool(
            CompFactory.JiveXML.LVL1ResultRetriever(
                name="LVL1ResultRetriever"
            )
        )

        #--- TriggerInfo (Etmiss, etc)
        result.addPublicTool(
            CompFactory.JiveXML.TriggerInfoRetriever(
                name="TriggerInfoRetriever"
            )
        )

        # new xAOD retrievers
        result.addPublicTool(
            CompFactory.JiveXML.xAODEmTauROIRetriever(
                name="xAODEmTauROIRetriever"
            )
        )

        result.addPublicTool(
            CompFactory.JiveXML.xAODJetROIRetriever(
                name="xAODJetROIRetriever"
            )
        )

        result.addPublicTool(
            CompFactory.JiveXML.xAODMuonROIRetriever(
                name="xAODMuonROIRetriever"
            )
        )

        result.addPublicTool(
            CompFactory.JiveXML.xAODTriggerTowerRetriever(
                name="xAODTriggerTowerRetriever"
            )
        )
    return result
