# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import GeneratorSettingsPrecedence

def Pythia8ShowerWeightsCfg(flags, name="Pythia8_i", include_pdf_variations=False):
    """
    Configure Pythia8 shower-weight variations.

    This is the CA equivalent of share/common/Pythia8_ShowerWeights.py,
    including optional PDF-variation weights used with NNPDF-based tunes.
    """
    from Pythia8_i.Pythia8Config import Pythia8CommandsCfg

    weight_entries = [
        "Var3cUp isr:muRfac=0.549241",
        "Var3cDown isr:muRfac=1.960832",
        "isr:muRfac=2.0_fsr:muRfac=2.0 isr:muRfac=2.0 fsr:muRfac=2.0",
        "isr:muRfac=2.0_fsr:muRfac=1.0 isr:muRfac=2.0 fsr:muRfac=1.0",
        "isr:muRfac=2.0_fsr:muRfac=0.5 isr:muRfac=2.0 fsr:muRfac=0.5",
        "isr:muRfac=1.0_fsr:muRfac=2.0 isr:muRfac=1.0 fsr:muRfac=2.0",
        "isr:muRfac=1.0_fsr:muRfac=0.5 isr:muRfac=1.0 fsr:muRfac=0.5",
        "isr:muRfac=0.5_fsr:muRfac=2.0 isr:muRfac=0.5 fsr:muRfac=2.0",
        "isr:muRfac=0.5_fsr:muRfac=1.0 isr:muRfac=0.5 fsr:muRfac=1.0",
        "isr:muRfac=0.5_fsr:muRfac=0.5 isr:muRfac=0.5 fsr:muRfac=0.5",
        "isr:muRfac=1.75_fsr:muRfac=1.0 isr:muRfac=1.75 fsr:muRfac=1.0",
        "isr:muRfac=1.5_fsr:muRfac=1.0 isr:muRfac=1.5 fsr:muRfac=1.0",
        "isr:muRfac=1.25_fsr:muRfac=1.0 isr:muRfac=1.25 fsr:muRfac=1.0",
        "isr:muRfac=0.625_fsr:muRfac=1.0 isr:muRfac=0.625 fsr:muRfac=1.0",
        "isr:muRfac=0.75_fsr:muRfac=1.0 isr:muRfac=0.75 fsr:muRfac=1.0",
        "isr:muRfac=0.875_fsr:muRfac=1.0 isr:muRfac=0.875 fsr:muRfac=1.0",
        "isr:muRfac=1.0_fsr:muRfac=1.75 isr:muRfac=1.0 fsr:muRfac=1.75",
        "isr:muRfac=1.0_fsr:muRfac=1.5 isr:muRfac=1.0 fsr:muRfac=1.5",
        "isr:muRfac=1.0_fsr:muRfac=1.25 isr:muRfac=1.0 fsr:muRfac=1.25",
        "isr:muRfac=1.0_fsr:muRfac=0.625 isr:muRfac=1.0 fsr:muRfac=0.625",
        "isr:muRfac=1.0_fsr:muRfac=0.75 isr:muRfac=1.0 fsr:muRfac=0.75",
        "isr:muRfac=1.0_fsr:muRfac=0.875 isr:muRfac=1.0 fsr:muRfac=0.875",
        "hardHi fsr:cNS=2.0 isr:cNS=2.0",
        "hardLo fsr:cNS=-2.0 isr:cNS=-2.0",
    ]
    shower_weight_names = [
        "Var3cUp",
        "Var3cDown",
        "isr:muRfac=2.0_fsr:muRfac=2.0",
        "isr:muRfac=2.0_fsr:muRfac=1.0",
        "isr:muRfac=2.0_fsr:muRfac=0.5",
        "isr:muRfac=1.0_fsr:muRfac=2.0",
        "isr:muRfac=1.0_fsr:muRfac=0.5",
        "isr:muRfac=0.5_fsr:muRfac=2.0",
        "isr:muRfac=0.5_fsr:muRfac=1.0",
        "isr:muRfac=0.5_fsr:muRfac=0.5",
        "isr:muRfac=1.75_fsr:muRfac=1.0",
        "isr:muRfac=1.5_fsr:muRfac=1.0",
        "isr:muRfac=1.25_fsr:muRfac=1.0",
        "isr:muRfac=0.625_fsr:muRfac=1.0",
        "isr:muRfac=0.75_fsr:muRfac=1.0",
        "isr:muRfac=0.875_fsr:muRfac=1.0",
        "isr:muRfac=1.0_fsr:muRfac=1.75",
        "isr:muRfac=1.0_fsr:muRfac=1.5",
        "isr:muRfac=1.0_fsr:muRfac=1.25",
        "isr:muRfac=1.0_fsr:muRfac=0.625",
        "isr:muRfac=1.0_fsr:muRfac=0.75",
        "isr:muRfac=1.0_fsr:muRfac=0.875",
        "hardHi",
        "hardLo",
    ]

    if include_pdf_variations:
        weight_entries.extend([
            "isr:PDF:plus isr:PDF:plus=1",
            "isr:PDF:minus isr:PDF:minus=2",
        ])
        shower_weight_names.extend([
            "isr:PDF:plus",
            "isr:PDF:minus",
        ])

    ca = Pythia8CommandsCfg(
        flags,
        source="pythia8_shower_weights",
        commands=[
            "UncertaintyBands:doVariations = on",
            f"UncertaintyBands:List = {{{', '.join(weight_entries)}}}",
        ],
        precedence=GeneratorSettingsPrecedence.WEIGHTS,
        name=name,
    )
    ca.addEventAlgo(CompFactory.Pythia8_i(name, ShowerWeightNames=tuple(shower_weight_names)))
    return ca
