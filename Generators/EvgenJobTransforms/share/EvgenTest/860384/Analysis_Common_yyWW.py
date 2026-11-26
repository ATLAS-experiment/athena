# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


def GetSuperChicConfig():
    """
    Returns a dictionary with common SuperChic settings for the
    yy->WW analysis.
    """

    Config = {
        "isurv": 4,
        "PDFname": 'MSHT20qed_nnlo',
        "PDFmember": 0,
        "beam": 'prot',
        "genunw": True,
        "ymin": -5.0,
        "ymax": 5.0,
        "gencuts": True,
        "scorr": True,
        "fwidth": True,
        "ptxmax": 300,
        "ptamin": 3.0,
        "ptbmin": 3.0,
        "etaamin": -3.0,
        "etaamax": 3.0,
        "etabmin": -3.0,
        "etabmax": 3.0,
        "acoabmax": 100,
        "ncall": 100000,
        "ncall1": 100000
    }

    return Config


def GetPythia8DissociationCommands(scConfig):
    """
    Returns Pythia8 dissociation tune list for the yy->WW analysis.
    """

    Commands = ["TimeShower:recoilStrategyRF = 1"]

    match scConfig['diff']:

        case 'el':
            Commands += [
                "PartonLevel:MPI = off",
                "SpaceShower:pTmaxMatch = 2",
                "SpaceShower:pTdampMatch = 1",
                "LesHouches:matchInOut = off",
                "SpaceShower:dipoleRecoil = on",
                "PDF:BeamA2gamma = on",
                "PDF:BeamB2gamma = on",
                "Photon:ProcessType = 4",
                "PDF:Proton2gammaSet = 1",
                "Photon:sampleQ2 = off"
            ]

        case 'sda':
            Commands += [
                "PartonLevel:MPI = off",
                "SpaceShower:pTmaxMatch = 2",
                "SpaceShower:pTdampMatch = 1",
                "SpaceShower:dipoleRecoil = on",
                "PDF:BeamA2gamma = off",
                "PDF:BeamB2gamma = on",
                "Photon:ProcessType = 2",
                "PDF:Proton2gammaSet = 1",
                "Photon:sampleQ2 = off",
                "BeamRemnants:unresolvedHadron = 2"
            ]

        case 'sdb':
            Commands += [
                "PartonLevel:MPI = off",
                "SpaceShower:pTmaxMatch = 2",
                "SpaceShower:pTdampMatch = 1",
                "SpaceShower:dipoleRecoil = on",
                "PDF:BeamA2gamma = on",
                "PDF:BeamB2gamma = off",
                "Photon:ProcessType = 3",
                "PDF:Proton2gammaSet = 1",
                "Photon:sampleQ2 = off",
                "BeamRemnants:unresolvedHadron = 1"
            ]

        case 'dd':
            Commands += [
                "PartonLevel:MPI = off",
                "SpaceShower:pTmaxMatch = 2",
                "SpaceShower:pTdampMatch = 1",
                "SpaceShower:dipoleRecoil = on",
                "PDF:BeamA2gamma = off",
                "PDF:BeamB2gamma = off",
                "Photon:ProcessType = 1",
                "PDF:Proton2gammaSet = 1",
                "Photon:sampleQ2 = off",
                "BeamRemnants:unresolvedHadron = 0"
            ]
        case _:
            raise Exception("Unknown SuperChic 'diff' parameter used to get Pythia8 dissociation tunes.")

    return Commands

def SetUpFilters(flags, sampleConfig, MLFil, NchFil):
    """
    Merge lepton filter or charged track filter to the main
    ComponentAccumulator object depending on MLFil, NchFil
    values.
    """

    if(MLFil):
        # Add multi lepton filter
        from GeneratorFilters.GeneratorFiltersConfig import xAODMultiLeptonFilterCommonCfg
        sampleConfig.merge(xAODMultiLeptonFilterCommonCfg(
            flags,
            PtCut=3500.,
            EtaCut=2.6,
            NLeptons=2
        ))

    if(NchFil):
        # Add charged track filter
        from GeneratorFilters.GeneratorFiltersConfig import xAODChargedTracksFilterCommonCfg
        sampleConfig.merge(xAODChargedTracksFilterCommonCfg(
            flags,
            NTracks=1,
            NTracksMax=30,
            Ptcut=500.,
            Etacut=2.5
        ))