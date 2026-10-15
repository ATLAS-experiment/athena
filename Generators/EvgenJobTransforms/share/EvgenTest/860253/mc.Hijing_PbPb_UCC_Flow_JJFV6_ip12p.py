# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Hijing"
        self.keywords = ["minBias"]
        self.contact = ["pawel.rybczynski@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):

        Initialize = [
            "bmin 12", # impact parameter bigger than 12 fm
            "ihpr2 4 0", # turn OFF jet quenching
            "ihpr2 12 2", # Jan24,06 turns ON decays charm and  bottom but not pi0, lambda, ... 
            "ihpr2 21 1" # turns ON retaining of particle history - truth information
        ]

        from Hijing_i.HijingConfig import HijingBaseCfg
        sampleConfig = HijingBaseCfg(
            flags,
            Initialize=Initialize,
            McEventKey="HIJING_EVENT"
        )

        from FlowAfterburner.FlowAfterburnerConfig import AddFlowByShiftingCfg
        sampleConfig.merge(AddFlowByShiftingCfg(
            flags,
            McTruthKey="HIJING_EVENT",
            McFlowKey="GEN_EVENT",
            FlowFunctionName="jjia_minbias_new",
            FlowImplementation="exact",
            RandomizePhi=0
        ))

        return sampleConfig
