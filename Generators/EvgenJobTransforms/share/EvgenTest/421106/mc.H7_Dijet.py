# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Herwig7 dijet sample with MMHT2014 PDF and corresponding tune"
        self.keywords = ["SM", "QCD", "dijet"]
        self.contact = ["spyros.argyropoulos@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):
        from Herwig7_i.Herwig7CAConfig import Herwig7BuiltinMECfg
        commands = """
                   ## ------------------
                   ## Hard process setup
                   ## ------------------
                   insert /Herwig/MatrixElements/SubProcess:MatrixElements[0] /Herwig/MatrixElements/MEQCD2to2
                   set /Herwig/UnderlyingEvent/MPIHandler:IdenticalToUE 0
                   set /Herwig/Cuts/JetKtCut:MinKT 15*GeV
                   set /Herwig/Cuts/LeptonKtCut:MinKT 0.0*GeV
                   """

        return Herwig7BuiltinMECfg(
            flags,
            run_name="HerwigBuiltinME",
            commands=commands,
            me_pdf_order="NLO",
            me_pdf_name="MMHT2014nlo68cl",
            shower_pdf_order="NLO",
            shower_pdf_name="MMHT2014nlo68cl"
        )
