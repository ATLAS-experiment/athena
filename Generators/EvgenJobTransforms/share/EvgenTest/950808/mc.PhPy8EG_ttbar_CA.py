# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig


class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Powheg+Pythia8+EvtGen ttbar CA example"
        self.keywords = ["SM", "top", "ttbar"]
        self.contact = ["atlas-generators-powheg@cern.ch"]
        self.nEventsPerJob = 10000

        from PowhegControl.PowhegConfig import setupPowhegFlags
        setupPowhegFlags(flags)

    def setupProcess(self, flags):
        from PowhegControl.PowhegConfig import PowhegCfg, PowhegOutputFile

        lhe_file = PowhegOutputFile(flags)
        ca = PowhegCfg(
            flags,
            process="tt",
            output_lhe=lhe_file,
            settings={
                "decay_mode": "t t~ > all",
                "hdamp": 258.75,
            },
        )

        from Pythia8_i.Pythia8Config import (
            Pythia8EvtGenBaseCfg,
            Pythia8_Powheg_Main31_Cfg,
            Pythia8_A14_NNPDF23LO_Common_Cfg,
        )
        ca.merge(Pythia8_Powheg_Main31_Cfg(
            flags,
            ShowerCfg=Pythia8_A14_NNPDF23LO_Common_Cfg,
            LHEFile=lhe_file,
            Commands=[
                "Powheg:NFinal = 2",
            ],
        ))
        ca.merge(Pythia8EvtGenBaseCfg(flags))
        return ca
