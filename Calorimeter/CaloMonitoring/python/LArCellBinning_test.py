# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaPython.PyAthenaComps import Alg, StatusCode
import ROOT
import sys

def checkHistoBinning(filename,runnumber):
    nMultipleHits=0
    nNoHit=0
    f=ROOT.TFile.Open(filename)
    layers=["EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C",
            "HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C",
            "EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C",
            "FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"]


    for layer in layers:
        histpath="run_%i/CaloMonitoring/LArCellMon_NoTrigSel/2d_Occupancy/CellOccupancyVsEtaPhi_%s_noEth_rndm_CSCveto"%(runnumber,layer)
        hist=f.Get(histpath)
        print ("Checking Histogram",hist)
        print ("\tBinning x:",hist.GetNbinsX(),"y:",hist.GetNbinsY())
        for x in range (hist.GetNbinsX()):
            for y in range (hist.GetNbinsY()):
                n=hist.GetBinContent(x,y)
                if (n>1): 
                    print ("ERROR multiple hits in ",layer,x,y,n)
                    nMultipleHits+=1
                elif (n!=1): 
                    nNoHit+=1
                    print ("WARNING no hit in ", layer,x,y,n)
    print ("Summary")
    print ("\tNumber of bins not corresponding to any cell:",nNoHit)
    print ("\tNumber of bins corresponding to multiple cells:",nMultipleHits)
                    
    return nMultipleHits

class CreateDataAlg (Alg):
    def execute (self):
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find (ctx.eventID())
        ccc = ROOT.CaloCellContainer()
        for i in range (mgr.element_size()):
            elem = mgr.get_element (ROOT.IdentifierHash (i))
            cc=ROOT.CaloCell(elem,10000,0,0,0)
            ccc.push_back (cc)
            ROOT.SetOwnership (cc, False)
        ccc.order()
        ccc.updateCaloIterators()
        self.msg.info("Recorded CaloCellContainer %i",ccc.size())
        self.evtStore.record (ccc, 'AllCalo')
        
        return StatusCode.Success



def testCfg (flags):
    result = ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(LArGMCfg(flags))
    result.merge(TileGMCfg(flags))

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))

    result.addEventAlgo (CreateDataAlg ('CreateDataAlg'),sequenceName="AthAlgSeq")
    from CaloMonitoring.LArCellMonAlg import LArCellMonConfig
    result.merge( LArCellMonConfig(flags) )

    alg=result.getEventAlgo("LArCellMonAlg")
    alg.useReadyFilterTool=False
    alg.useBadLBTool=False
    alg.useLArCollisionFilterTool=False
    alg.useLArNoisyAlg=False
    alg.useBeamBackgroundRemoval=False
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles, defaultConditionsTags
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.Output.HISTFileName = 'LArCellMonOutput.root'
    flags.DQ.useTrigger = False
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA22
    flags.fillFromArgs()
    flags.lock()
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, HLTConfigSvcCfg, L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg

    acc.merge( L1TriggerByteStreamDecoderCfg(flags) )
    acc.merge( L1ConfigSvcCfg(flags) )
    acc.merge( HLTConfigSvcCfg(flags) )
    acc.merge( L1PrescaleCondAlgCfg(flags) )
    acc.merge( HLTPrescaleCondAlgCfg(flags) )

    from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
    acc.merge( BunchGroupCondAlgCfg( flags ) )
    acc.merge (testCfg (flags))

    sc=acc.run(1)
    if (sc.isFailure()): sys.exit(1)


    retval=checkHistoBinning(flags.Output.HISTFileName,431493)
    if retval!=0:
        sys.exit(1)
    else:
        sys.exit(0)
