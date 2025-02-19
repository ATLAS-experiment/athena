# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def NswOccupancyAlgCfg(flags, binWidth = 100):
    result = ComponentAccumulator()

    histSvc = CompFactory.THistSvc(Output=["NSWSTORIES DATAFILE='NswFairyTales.root' OPT='RECREATE'"])
    result.addService(histSvc) 
    the_alg = CompFactory.NswOccupancyAlg("NswOccupancyAlgBin{width}".format(width = binWidth), BinWidth = binWidth)
    result.addEventAlgo(the_alg, primary = True)
    return result

def AddMetaAlgCfg(flags, alg_name="MuonTPMetaAlg", OutStream="NSWPRDValAlg", **kwargs):
    result = ComponentAccumulator()
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
    result.merge(CutFlowSvcCfg(flags))
    result.merge(MetaDataSvcCfg(flags))
    if len(OutStream):
        kwargs.setdefault("OutStream", OutStream)
        alg_name += "_" + OutStream
    kwargs.setdefault("isData", not flags.Input.isMC)    
    kwargs.setdefault("ExtraOutputs", [('xAOD::EventInfo', 'StoreGateSvc+EventInfo.MetaData' + OutStream)])
    the_alg = CompFactory.MuonVal.MetaDataAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)  # top sequence
    return result
