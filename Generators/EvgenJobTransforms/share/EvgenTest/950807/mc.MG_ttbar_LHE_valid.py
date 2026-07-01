# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig


class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "MadGraph LO ttbar"
        self.keywords = ["ttbar"]
        self.contact = ["spyros.argyropoulos@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):
        # MadGraph process to generate
        process_def = """
                      import model sm
                      define p = g u c d s u~ c~ d~ s~
                      define j = g u c d s u~ c~ d~ s~
                      generate p p > t t~
                      output -f
                      """

        # Custom settings to pass to MadGraphControl.
        # These are native MadGraph settings, not MadGraphControl settings.
        settings = {
            "cut_decays": "F",
        }

        # Use 4F NNPDF set 
        from MadGraphControl.MadGraphPDFSettings import MadGraphPDFSets
        pdf_set = MadGraphPDFSets.NNPDF30NLOnf4

        # This sample is LHE-only by construction: no showering stage.
        from MadGraphControl.MadGraphConfig import MadGraphCfg
        return MadGraphCfg(
            flags,
            process_definition=process_def,
            settings=settings,
            pdf_setting=pdf_set
        )

        # Non-default settings can be passed via the kwargs, for example:
        # return MadGraphCfg(
        #     flags,
        #     process_definition=process_def,
        #     settings=settings,
        #     lhe_version=2,
        #     saveProcDir=True,
        #     safety=1.2, # increase the safety factor for number of events to be generated
        # )
