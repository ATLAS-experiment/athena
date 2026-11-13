# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from textwrap import dedent

from AthenaCommon.SystemOfUnits import GeV
from EvgenJobTransforms.EvgenCAConfig import EvgenConfig
from GeneratorFilters.GeneratorFiltersConfig import xAODMultiMuonFilterCommonCfg
from MadGraphControl.MadGraphConfig import MadGraphCfg
from MadGraphControl.MadGraphPDFSettings import MadGraphPDFSets
from Pythia8_i.Pythia8Config import (
    Pythia8_A14_NNPDF23LO_Common_Cfg,
    Pythia8_MadGraph_Cfg,
    Pythia8EvtGenBaseCfg,
)


# The MG Control config
# This is the equivalent of the old-style MadGraph control files.
def TetrabCfg(flags, *, 
              m4b,
              safety):
    
    # Process definition
    process = dedent("""
                     import model HEL_4b_UFO
                     generate g g > tetrab, (tetrab > upsilon mu+ mu-, upsilon > mu+ mu-)
                     output -f
                     """)

    sampleConfig = MadGraphCfg(
        flags,
        process_definition=process,
        safety=safety,
        pdf_setting=MadGraphPDFSets.NNPDF30NLOnf4,
        run_card_settings={
            "ptl": 0.0,
            "etal": -1.0,
            "drll": 0.0,
        },
        param_card_settings={
            "mass": {"100550551": m4b},
            "newcoup": {
                "cVV4": 0.0,
                "cGG6": 0.0,
                "tcVV6": 0.001,
                "tcGG6": 0.001,
            },
        },
        lhe_version=3,
        prepare_lhe_for_shower=True
    )

    # Generate EvtGen user decay file on the fly Upsilon -> mumu
    decay_file = "MY_UPSILON_USER_FORCED.DEC"
    with open(decay_file, "w") as f:
        f.write(dedent("""\
            Decay Upsilon
            1.0000    mu+  mu-             PHOTOS  VLL;
            Enddecay
            End
            """))

    # Add Pythia with A14 tune - don't use the Py8+EG config via
    # ShowerCfg=Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg, since we 
    # want to use a custom decay file.
    sampleConfig.merge(Pythia8_MadGraph_Cfg(
        flags,
        ShowerCfg=Pythia8_A14_NNPDF23LO_Common_Cfg,
        Commands=["553:onMode = off"],
    ))

    # Add EvtGen with custom decay file
    sampleConfig.merge(Pythia8EvtGenBaseCfg(
        flags,
        userDecayFile=decay_file
    ))

    # Add filter
    sampleConfig.merge(xAODMultiMuonFilterCommonCfg(
        flags,
        Ptcut=2 * GeV,
        Etacut=3.0,
        NMuons=3,
    ))

    return sampleConfig


# The Sample class that is used by the EvgenJobTransforms to configure the sample
# This will be derived by the jO that will set custom properties, e.g. safety factor.
class TetrabSample(EvgenConfig):
    
    # One way to set a generation parameter using the get_physics_short() method.
    # The property decorator allows us to define a method that can be accessed like
    # an attribute, so we can later call self.m4b and not self.m4b().
    # Then self.m4b stays invariant whether we extract it here like this (from physics short)
    # or if we set it in the jO, e.g. Sample.m4b = 17.0
    @ property
    def m4b(self):
        from MCJobOptionUtils.JOsupport import get_physics_short
        return get_physics_short().split("_")[-2].replace("m", "")

    def setupFlags(self, flags):
        self.description = f"4b Tetraquark Meson, https://arxiv.org/pdf/1709.09605.pdf m4b={self.m4b} GeV"
        self.keywords = ["heavyFlavour", "Upsilon", "4muon"]
        self.contact = ["Spyros Argyropoulos <spyros.argyropoulos@cern.ch>"]
        self.process = "g g > tetrab, (tetrab > upsilon mu+ mu-, upsilon > mu+ mu-)"

    def setupProcess(self, flags):
        # Return the config from the control file that has the CA object
        return TetrabCfg(
            flags,
            m4b=self.m4b, # This can be set in the jO or derived from the physics short as above
            safety=self.safety, # If this is not set in the jO it will raise AttributeError
        )
