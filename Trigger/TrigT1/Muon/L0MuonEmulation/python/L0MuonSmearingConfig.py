#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def L0MuonSmearingCfg(flags, name = "L0MuonSmearingAlg", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.L0MuonSmearingAlg(name = name,
                                               **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('track_input_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{truth};Muons', xbins=50, xmin=-3, xmax=3)
    monTool.defineHistogram('track_input_phi', path='EXPERT', type='TH1F', title=';#phi_{#mu}^{truth};Muons', xbins=50, xmin=-4, xmax=4)
    monTool.defineHistogram('track_input_pt',  path='EXPERT', type='TH1F', title=';p_{T,#mu}^{truth} (GeV);Muons', xbins=50, xmin=0, xmax=250)
    monTool.defineHistogram('track_input_curv',path='EXPERT', type='TH1F', title=';q/p_{T,#mu}^{truth} (1/GeV);Muons', xbins=50, xmin=-0.4, xmax=0.4)
    monTool.defineHistogram('track_output_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{L0};Muons', xbins=50, xmin=-3, xmax=3)
    monTool.defineHistogram('track_output_phi', path='EXPERT', type='TH1F', title=';#phi_{#mu}^{L0};Muons', xbins=50, xmin=-4, xmax=4)
    monTool.defineHistogram('track_output_pt',  path='EXPERT', type='TH1F', title=';p_{T,#mu}^{L0} (GeV);Muons', xbins=50, xmin=0, xmax=250)
    monTool.defineHistogram('track_output_curv',path='EXPERT', type='TH1F', title=';q/p_{T,#mu}^{L0} (1/GeV);Muons', xbins=50, xmin=-0.4, xmax=0.4)
    monTool.defineHistogram('roi_output_eta', path='EXPERT', type='TH1F', title=';#eta_{RoI}^{L0};RoIs', xbins=50, xmin=-3, xmax=3)
    monTool.defineHistogram('roi_output_phi', path='EXPERT', type='TH1F', title=';#phi_{RoI}^{L0};RoIs', xbins=50, xmin=-4, xmax=4)
    monTool.defineHistogram('roi_output_pt',  path='EXPERT', type='TH1F', title=';p_{T,RoI}^{L0} (GeV);RoIs', xbins=64, xmin=0, xmax=128.0)
    monTool.defineHistogram('roi_output_curv',path='EXPERT', type='TH1F', title=';q/p_{T,RoI}^{L0} (1/GeV);RoIs', xbins=50, xmin=-0.4, xmax=0.4)
    monTool.defineHistogram('delta_eta', path='EXPERT', type='TH1F', title=';#eta_{RoI}^{L0}-#eta_{#mu}^{truth};', xbins=50, xmin=-0.005,xmax=0.005)
    monTool.defineHistogram('delta_phi', path='EXPERT', type='TH1F', title=';#phi_{RoI}^{L0}-#phi_{#mu}^{truth};', xbins=50, xmin=-0.1,xmax=0.1)
    monTool.defineHistogram('delta_pt',  path='EXPERT', type='TH1F', title=';(p_{T}^{L0}-p_{T}^{truth})/p_{T}^{truth};', xbins=50, xmin=-1,xmax=1)
    monTool.defineHistogram('delta_curv',path='EXPERT', type='TH1F', title=';((q/p_{T})^{L0} - (q/p_{T})^{truth}) / (q/p_{T})^{truth}',xbins=50,xmin=-1,xmax=1)

    alg.MonTool = monTool

    from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig
    result.merge(TriggerHistSvcConfig(flags))

    result.addEventAlgo(alg)
    return result
  

if __name__ == "__main__":
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Constants import DEBUG
    flags = initConfigFlags()    
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Input.isMC=True
    flags.Exec.MaxEvents = 20
    flags.Common.MsgSuppression = False
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)        
 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # example to smear the truth particles 
    smearerTruth = L0MuonSmearingCfg(flags,
                                     name = "L0MuonTruthSmearing",
                                     InputTruthParticle = "TruthParticles",
                                     OutputLevel = DEBUG)
    acc.merge(smearerTruth)

    # below is validation
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())

