# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

from AthenaCommon.Logging import logging

def CaloComputeNoiseCfg(flagsIn,mu=60,nsamp=4,dt=25,output='cellnoise_data.root'):

    if (dt!=25):
        raise RuntimeError("At this point (early run 3), only a dt of 25ns is supported")
    
    #Clone flags-container and modify it, since this is not a standard reco job
    flags=flagsIn.clone()
    flags.Calo.Noise.fixedLumiForNoise=mu*0.17241*50/25
    flags.LAr.doHVCorr = False #Avoid double-rescaling
    flags.LAr.ROD.NumberOfCollisions = mu # for OFC computation
    flags.LAr.ROD.nSamples = nsamp # number of samples to use
    flags.lock()

    msg = logging.getLogger("CaloComputeNoiseCfg")
    #pick noise-tag depending on mu and dt

    #FIXME: make this configurable
    if flags.LArCalib.isSC:
       minbiastag="LARElecCalibMCSCMinBias-mc16-Epos-A3-s3687"
       fsampltag="LARElecCalibMCSCfSampl-000"
       shapetag="LARElecCalibMCSCShape-000"
       #hvcorrtag="LARElecCalibMCSC-" not existing yet
    else:
       minbiastag="LARElecCalibMCMinBias-mc16-Epos-A3-s3687"
       fsampltag="LARElecCalibMCfSampl-G4101-20371-FTFP_BERT_BIRK_v2"
       shapetag="LARElecCalibMCShapeLArPileupShape-RUN2-2018"
       hvcorrtag="LARElecCalibMCHVScaleCorr-IOVDEP-02"

    msg.info("Noise computing for mu=%i and dt=%i" , mu,dt)

    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    if flags.LArCalib.isSC:
       from LArRecUtils.LArADC2MeVSCCondAlgConfig import LArADC2MeVSCCondAlgCfg
       result.merge(LArADC2MeVSCCondAlgCfg(flags))
       from LArRecUtils.LArRecUtilsConfig import LArOFCSCCondAlgCfg
       result.merge(LArOFCSCCondAlgCfg(flags))
       #result.addCondAlgo(CompFactory.LArHVCondAlg(doHV=False, doAffectedHV=False))

    else:
       from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
       from LArRecUtils.LArRecUtilsConfig import LArOFCCondAlgCfg
       from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
       result.merge(LArADC2MeVCondAlgCfg(flags))
       result.merge(LArOFCCondAlgCfg(flags))
       result.merge(LArHVScaleCfg(flags))

    #make proper overrides
    from IOVDbSvc.IOVDbSvcConfig import addOverride
    if flags.LArCalib.isSC:
       #result.merge(addOverride(flags,"/LAR/NoiseOflSCi/CellNoise", "LARNoiseOflSCCellNoise-empty"))
       result.getService("IOVDbSvc").Folders.append("/LAR/ElecCalibMCSC/HVScaleCorr")
       result.getCondAlgo("CondInputLoader").Load.add(("CondAttrListCollection","/LAR/ElecCalibMCSC/HVScaleCorr"))
       result.addCondAlgo(CompFactory.getComp("LArFlatConditionsAlg<LArHVScaleCorrSC>")(ReadKey="/LAR/ElecCalibMCSC/HVScaleCorr", WriteKey="LArHVScaleCorrSC"))
       pass
    else:
       result.merge(addOverride(flags,"/LAR/ElecCalibMC/MinBias", minbiastag))
       result.merge(addOverride(flags,"/LAR/ElecCalibMC/fSampl", fsampltag))
       result.merge(addOverride(flags,"/LAR/ElecCalibMC/Shape", shapetag))
       result.merge(addOverride(flags,"/LAR/ElecCalibMC/HVScaleCorr", hvcorrtag))
       result.merge(addOverride(flags,"/LAR/NoiseOfl/CellNoise", "LARNoiseOflCellNoise-empty"))
    result.merge(addOverride(flags,"/TILE/OFL02/NOISE/CELL", "TileOfl02NoiseCell-OF2-05-25ns-R2-single-IOV"))
    result.merge(addOverride(flags,"/CALO/Ofl/Noise/CellNoise", "CaloOflNoiseCellnoise-empty"))

    #if not flags.LArCalib.isSC:
    if True:
       #we need a pedestal folder from data
       #first remove the MC folder
       iovdbsvc=result.getService("IOVDbSvc")
       # check existing list of folders and remove it if found
       folder="/LAR/ElecCalibMCSC/Pedestal" if flags.LArCalib.isSC else "/LAR/ElecCalibMC/Pedestal"
       for i in range(0,len(iovdbsvc.Folders)):
           if (iovdbsvc.Folders[i].find(folder)>=0):
               del iovdbsvc.Folders[i]
               break
       condInputLoader=result.getCondAlgo("CondInputLoader")
       for cil_Loadval in condInputLoader.Load:
           if folder in cil_Loadval:        
               condInputLoader.Load.remove(cil_Loadval)
               break
 
       #and now configure data one       
       dfolder="/LAR/ElecCalibFlatSC/Pedestal" if flags.LArCalib.isSC else "/LAR/ElecCalibFlat/Pedestal"        
       iovdbsvc.Folders.append(dfolder+"<db>COOLONL_LAR/CONDBR2</db>")
       condInputLoader.Load.add(("CondAttrListCollection",dfolder))
       if flags.LArCalib.isSC:
          #result.addCondAlgo(CompFactory.getComp("LArFlatConditionsAlg<LArPedestalSC>")(ReadKey=dfolder,WriteKey="LArPedestalSC"))
          result.getCondAlgo("LArFlatConditionsAlg<LArPedestalSC>").ReadKey=dfolder
       else:   
          result.addCondAlgo(CompFactory.getComp("LArFlatConditionsAlg<LArPedestalFlat>")(ReadKey=dfolder,WriteKey="LArPedestal"))

    if flags.LArCalib.isSC:
       result.addEventAlgo(CompFactory.CaloNoiseCompCondAlg(NMinBias=flags.Calo.Noise.fixedLumiForNoise,
                                                            ADC2MeVKey="LArADC2MeVSC",PedestalKey="LArPedestalSC",            
                                                            AutocorrKey="LArAutoCorrSC",OFKey="LArOFCSC",
                                                            ShapeKey="LArShapeSC",fSamplKey="LArfSamplSC",
                                                            LArMinBiasKey="LArMinBiasSC", SuperCell=True, DiagnosticHG=True
                                                          ))
    else:   
       result.addEventAlgo(CompFactory.CaloNoiseCompCondAlg(NMinBias=flags.Calo.Noise.fixedLumiForNoise))
    
    result.addEventAlgo(CompFactory.CaloRescaleNoise(absScaling=True, SuperCell=flags.LArCalib.isSC,
                                         OnlineLArHVScaleCorr="LArHVScaleCorrSC" if flags.LArCalib.isSC else "LArHVScaleCorr",
                                         CablingKey= "LArOnOffIdMapSC" if flags.LArCalib.isSC else "LArOnOffIdMap",
                                         ElecNoiseKey="elecNoise",PileupNoiseKey="pileupNoise"))

    result.getCondAlgo("LArADC2MeVCondAlg").LArHVScaleCorrKey = "" #Avoid double-rescaling

    import os
    if os.path.exists(output):
        os.remove(output)
    result.addService(CompFactory.THistSvc(Output = ["file1 DATAFILE='"+output+"' OPT='RECREATE'"]))
    result.setAppProperty("HistogramPersistency","ROOT")

    return result


if __name__=="__main__":
    import  argparse
    parser= argparse.ArgumentParser(description="(Compute noise")

    parser.add_argument('-r', '--run', type=int, default=358013, help="Which run number to use ")
    parser.add_argument('-t', '--globaltag', type=str, default="OFLCOND-MC21-SDR-RUN3-12",help="Global conditions tag ")
    parser.add_argument('-o', '--output',type=str,default="cellnoise_data.root",help="name stub for root and sqlite output files")
    parser.add_argument('-m', '--mu', type=int, default=60, help="Which mu to use ")
    parser.add_argument('-n', '--nsamples', type=int, default=4, help="Number of samples for OFC/Autocorr ")
    parser.add_argument('-s', '--isSC', default=False, action="store_true", help="Running for SC? ")
    parser.add_argument('--olevel', type=int, default=3, help="Output level to use ")

    args = parser.parse_args()
    print(args)


    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    addLArCalibFlags(flags)

    flags.Input.RunNumbers=[args.run]
    print("set the runnumber: ",flags.Input.RunNumbers)
    flags.Input.Files=[]
    flags.IOVDb.DatabaseInstance="OFLP200"
    flags.Input.isMC=True
    from Campaigns.Utils import Campaign
    flags.Input.MCCampaign=Campaign.Unknown
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.LAr.ROD.UseHighestGainAutoCorr = True
    flags.LArCalib.isSC=args.isSC

    #FIXME need to upload to COOL
    flags.IOVDb.SqliteFolders=("/LAR/ElecCalibMCSC/HVScaleCorr",)
    flags.IOVDb.SqliteInput="SCHVcorr_mc.db"
  
    if args.globaltag:
        flags.IOVDb.GlobalTag=args.globaltag

    flags.lock()
    cfg=MainEvgenServicesCfg(flags)
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
                                 FirstLB           = 0,
                                 EventsPerRun      = 1,
                                 FirstEvent        = 1,
                                 InitialTimeStamp  = 0,
                                 TimeStampInterval = 1))

    cfg.merge(CaloComputeNoiseCfg(flags,output=args.output,mu=args.mu,nsamp=args.nsamples))

    # could not put into algo config
    if flags.LArCalib.isSC:
       cfg.getCondAlgo("LArADC2MeVSCCondAlg").LArHVScaleCorrKey=""
    else:
       cfg.getCondAlgo("LArADC2MeVCondAlg").LArHVScaleCorrKey=""

    cfg.getService("DetectorStore").Dump=True
    cfg.getService("ConditionStore").Dump=True

    cfg.getService("MessageSvc").OutputLevel=args.olevel
    if args.olevel < 3:
       if flags.LArCalib.isSC:
          cfg.getCondAlgo("LArAutoCorrTotalSCCondAlg").OutputLevel=3
          cfg.getCondAlgo("LArOFCSCCondAlg").OutputLevel=3
       else:
          cfg.getCondAlgo("LArAutoCorrTotalCondAlg").OutputLevel=3
          cfg.getCondAlgo("LArOFCCondAlg").OutputLevel=3
    cfg.getService("MessageSvc").defaultLimit=999999999

    cfg.printConfig(withDetails=True)

    print("Start running...")
    cfg.run(1)
