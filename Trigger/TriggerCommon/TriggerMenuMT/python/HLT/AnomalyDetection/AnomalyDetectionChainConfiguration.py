# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TrigAnomalyDetectionHypo.TrigADHypoConfig import TrigADComboHypoToolFromDict

def TrigADComboHypoToolCfg(flags, chainDict):
    tool = TrigADComboHypoToolFromDict(chainDict)
    return tool
