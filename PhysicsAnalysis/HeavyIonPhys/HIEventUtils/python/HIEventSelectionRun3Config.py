#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_HILoose_Cfg

def HIEventSelectionRun3MonToolCfg(flags):
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, "MonTool")
    monTool.defineHistogram( 'fcalEt,zdcE;fcalEt_vs_zdcE_all', path='EXPERT', type='TH2F', title=';FCal Et;ZDC E',
                             xbins=160, xmin=0, xmax=1000, ybins=120, ymin=0, ymax=60)

    monTool.defineHistogram( 'fcalEt,zdcE;fcalEt_vs_zdcE_passed', cutmask='PUFCalVsZDCAny_passed', path='EXPERT', type='TH2F', title=';FCal Et;ZDC E',
                             xbins=160, xmin=0, xmax=1000, ybins=120, ymin=0, ymax=60)

    monTool.defineHistogram( 'fcalEt,zdcE;fcalEt_vs_zdcE_failed', cutmask='PUFCalVsZDCAny_failed', path='EXPERT', type='TH2F', title=';FCal Et;ZDC E',
                             xbins=160, xmin=0, xmax=1000, ybins=120, ymin=0, ymax=60)

    monTool.defineHistogram( 'fcalEt,nTrk;fcalEt_vs_nTrk_all', path='EXPERT', type='TH2F', title=';FCal Et;nTrk',
                             xbins=160, xmin=0, xmax=1000, ybins=120, ymin=0, ymax=600)

    monTool.defineHistogram( 'fcalEt,nTrk;fcalEt_vs_nTrk_passed', cutmask='PUFCalVsNTrackAny_passed', path='EXPERT', type='TH2F', title=';FCal Et;nTrk',
                             xbins=160, xmin=0, xmax=1000, ybins=120, ymin=0, ymax=600)

    monTool.defineHistogram( 'fcalEt,nTrk;fcalEt_vs_nTrk_failed', cutmask='PUFCalVsNTrackAny_failed', path='EXPERT', type='TH2F', title=';FCal Et;nTrk',
                             xbins=160, xmin=0, xmax=1000, ybins=120, ymin=0, ymax=600)
    


    monTool.defineHistogram( 'zdcPreSampleA;zdcPreSampleA_all', path='EXPERT', type='TH1F', title='all;ZDCPreampleAmp side A',
                             xbins=100, xmin=-400, xmax=1000)

    monTool.defineHistogram( 'zdcPreSampleC;zdcPreSampleC_all', path='EXPERT', type='TH1F', title='all;ZDCPreampleAmp side C',
                             xbins=100, xmin=-400, xmax=1000)


    monTool.defineHistogram( 'zdcPreSampleA,zdcPreSampleC', path='EXPERT', type='TH2F', title='correlation;ZDCPreampleAmp side A;ZDCPreampleAmp side C',
                             xbins=50, xmin=-400, xmax=1000, ybins=50, ymin=-400, ymax=1000)

    monTool.defineHistogram( 'zdcPreSampleA;zdcPreSampleA_failed', cutmask="NoPUZDCPresampler_failed", path='EXPERT', type='TH1F', title='failed;ZDCPreampleAmp side A',
                             xbins=100, xmin=-400, xmax=1000)

    monTool.defineHistogram( 'zdcPreSampleC;zdcPreSampleC_failed', cutmask="NoPUZDCPresampler_failed", path='EXPERT', type='TH1F', title='failed;ZDCPreampleAmp side C',
                             xbins=100, xmin=-400, xmax=1000)
    
    monTool.defineHistogram( 'fcalEt;OO_all', path='EXPERT', type='TH1F', title='all;FCal Et sum',
                             xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram( 'fcalEt;OO_1_FCalNtrk', cutmask='OO_1_passed',  path='EXPERT', type='TH1F', title='FCalNtrk;FCal Et sum',
                             xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram( 'fcalEt;OO_2_Vtx', cutmask='OO_2_passed',  path='EXPERT', type='TH1F', title='FCalNtrk and Vtx;FCal Et sum',
                             xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram( 'fcalEt;OO_3_FCalZdc', cutmask='OO_3_passed',  path='EXPERT', type='TH1F', title='FCalNtrk and Vtx and FCal ZDC;FCal Et sum',
                             xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram( 'fcalEt;OO_4_TCFCal', cutmask='OO_4_passed',  path='EXPERT', type='TH1F', title='FCalNtrk and Vtx and FCal ZDC and topoClusterInFCal;FCal Et sum',
                             xbins=100, xmin=0, xmax=1000)

    
    prefix=flags.Input.Files[0].split("/")[-1]
    histsvc = CompFactory.THistSvc(Output=[f"EXPERT DATAFILE='{prefix}HIEventSelectionRun3Validation.root' OPT='RECREATE'"])
    acc.addService(histsvc)        
    acc.setPrivateTools(monTool)
    return acc
    
    

def HIEventSelectionRun3Cfg(flags, enableValidation=False):
    acc = ComponentAccumulator()
    # in future decide cut level for tracks depending on input dataset
    # for now default to HILoose cuts set with 0.5 GeV
    trackSelectionTool = acc.popToolsAndMerge(InDetTrackSelectionTool_HILoose_Cfg(flags,
                                                                                  minPt=0.5*Units.GeV))
    filterTool = CompFactory.HI.HIEventSelectionToolRun3(TrackSelectionTool=trackSelectionTool)

    # ZDC modules collection change its name
    zdcKey=None
    zdcNeeded=True  # in the future add check if ZDC info is required in fact
    if zdcNeeded and "ZdcModules" in flags.Input.Collections:
        zdcKey="ZdcModules"
    elif zdcNeeded and "ZDCModules" in flags.Input.Collections:
        zdcKey="ZDCModules"
    elif zdcNeeded and "ZdcSums" in flags.Input.Collections:
        zdcKey="ZdcSums"

    if zdcNeeded and not zdcKey:
        raise ConfigurationError("The input file does not have any ZDCModules (any capitalisation) container and ZDC info is needed for selection")

    monTool = acc.popToolsAndMerge(HIEventSelectionRun3MonToolCfg(flags)) if enableValidation else None

    filterAlg = CompFactory.HI.HIEventFilterAlgRun3(name="HIEventFilterAlgRun3",
                                                    SelectionTool=filterTool,
                                                    ZDC=zdcKey, 
                                                    MonTool=monTool )
    acc.addEventAlgo(filterAlg)
    return acc

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    data_hi="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/"
    test_files = {"23PbPb":"data23_hi.00463364.physics_HardProbes.AOD.r16069_p6447_skim",
                  "24PbPb":"data24_hi.00490145.physics_HardProbes.AOD.f1550_m2267_skim",
                  "25OO": "data25_hi.00501859.physics_MinBias.AOD.f1606_m2272_skim", 
                  "25NeNe": "data25_hi.00502008.physics_MinBias.AOD.f1606_m2272_skim",
                  "25pO": "data25_hip.00501607.physics_MinBias.AOD.f1604_m2272_skim" }
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    flags.addFlag("HIPeriodToTest", "23")
    flags.Exec.MaxEvents=10
    flags.Exec.MaxEvents=-1
    flags.Input.Files=lambda fl: [data_hi+test_files[fl.HIPeriodToTest]]
    flags.fillFromArgs()
    flags.lock()

    acc=MainServicesCfg(flags)
    # if need to read POOL file
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    from AthenaCommon.Constants import DEBUG

    acc.merge(HIEventSelectionRun3Cfg(flags, enableValidation=True))
    acc.foreach_component("**/AthAlgSeq/*Run3*").OutputLevel=DEBUG

    filterAlg = acc.getEventAlgo("HIEventFilterAlgRun3")
    # test if we can set custom selection (required python access to enums defined in C++)
    # import ROOT
    # filterAlg.SelectionMask=ROOT.HI.SelectionMask.NoEventError & ROOT.HI.SelectionMask.PUFCalVsZDCAny
    filterAlg.UseIonDataTypeDefaultMask=True
    acc.printConfig(withDetails=True)
    # either
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
