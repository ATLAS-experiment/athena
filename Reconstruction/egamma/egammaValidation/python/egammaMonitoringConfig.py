# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def egammaMonitoringCfg(flags, particleType = 'electron',
                            outputFileName = 'Nightly-monitoring.hist',
                            addFwd = False):

    acc = ComponentAccumulator()

    # Preparing selection tools (ID and iso)
    from ROOT import LikeEnum
    from ElectronPhotonSelectorTools.EgammaPIDdefs import egammaPID

    eidWP = { 'Loose' : LikeEnum.Loose, 'Medium' : LikeEnum.Medium, 'Tight' : LikeEnum.Tight }
    gidWP = { 'Loose' : egammaPID.PhotonIDLoose, 'Tight' : egammaPID.PhotonIDTight }
    gisoWP = [ 'FixedCutTight', 'FixedCutLoose', 'TightCaloOnly' ]

    kwarg = {}

    if particleType == 'electron' or particleType == 'dataZ':
        kwarg['ElectronsKey'] = 'Electrons'
        kwarg['GSFTrackParticlesKey'] = 'GSFTrackParticles'

    if particleType == 'electron':
        from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import (
            AsgElectronLikelihoodToolCfg)
        for k,w in eidWP.items():
            t = AsgElectronLikelihoodToolCfg(flags, k+'LHSelector', w)
            kwarg[k+'_Ele'] = t.popPrivateTools()
            acc.merge(t)
        if addFwd:
            kwarg['FwdElectronsKey'] = 'ForwardElectrons'
            from ElectronPhotonSelectorTools.AsgForwardElectronLikelihoodToolConfig import (
                AsgForwardElectronLikelihoodToolCfg)
            for k in eidWP.keys():
                # do not use the WP name as mapping points to a corrupted ID (in main, 24/05/24)
                # anyway, not appropriate for ITk but least corrupted within rel21.2/Run2
                configFile = f'dev/ElectronPhotonSelectorTools/offline/mc16_20190729/FwdLH{k}Conf.conf'
                t = AsgForwardElectronLikelihoodToolCfg(flags,k+'FwdSelector',
                                                        ConfigFile = configFile)
                kwarg[k+'_FwdEle'] = t.popPrivateTools()
                acc.merge(t)

    if particleType == 'gamma':
        from ElectronPhotonSelectorTools.AsgPhotonIsEMSelectorsConfig import (
            AsgPhotonIsEMSelectorCfg)
        for k,w in gidWP.items():
            t = AsgPhotonIsEMSelectorCfg(flags, k+'_Photon', w)
            kwarg[k+'_Photon'] = t.popPrivateTools()
            acc.merge(t)

        from IsolationSelection.IsolationSelectionConfig import IsolationSelectionToolCfg
        for k in gisoWP:
            t = IsolationSelectionToolCfg(flags, 'PhIso'+k, PhotonWP=k)
            name = f'Iso{k}'
            kwarg[name] = t.popPrivateTools()
            acc.merge(t)
        kwarg['PhotonsKey'] = 'Photons'

    # a Truth classifier
    if particleType != 'dataZ':
        from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
        MCClassifier = acc.popToolsAndMerge(MCTruthClassifierCfg(flags))
        kwarg['egammaTruthParticlesKey'] = 'egammaTruthParticles'
        kwarg['truthParticlesKey'] = 'TruthParticles'
    else:
        MCClassifier = None

    # The monitoring alg
    egMon = CompFactory.EgammaMonitoring(
        name = 'egammaMonitoringAlg',
        sampleType = particleType,
        MCTruthClassifier = MCClassifier,
        **kwarg)
    acc.addEventAlgo(egMon)

    # Add the histogram service
    svc = CompFactory.THistSvc(name="THistSvc")
    svc.Output = [f"MONITORING DATAFILE='{outputFileName}.root' OPT='RECREATE'"]
    acc.addService(svc)

    return acc
