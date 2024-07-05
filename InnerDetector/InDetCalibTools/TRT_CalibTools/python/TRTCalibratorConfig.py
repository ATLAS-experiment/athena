"""Define methods to TRT R-t calibration ntuple and obtain its constants 

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Tool to process R-t ntuple. Produces histograms and calibration text files.
def TRTCalibratorCfg(flags, name="TRTCalibratorTool", **kwargs) :
    
    acc = ComponentAccumulator()
    kwargs.setdefault("MinRt",10000)
    kwargs.setdefault("MinT0",500)
    kwargs.setdefault("Hittuple","merge.root")
    kwargs.setdefault("RtRel","basic")
    kwargs.setdefault("RtBinning","t")
    kwargs.setdefault("UseP0"  ,True)
    kwargs.setdefault("FloatP3",True)
    kwargs.setdefault("T0Offset",0.0)
    kwargs.setdefault("DoShortStrawCorrection",False)
    kwargs.setdefault("DoArXenonSep",True)
    
    kwargs.update(CaltagConfiguration("-1"))
    
    if "TRTStrawSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
        kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags)))
        
    if "NeighbourSvc" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawNeighbourSvcCfg
        kwargs.setdefault("NeighbourSvc", acc.getPrimaryAndMerge(TRT_StrawNeighbourSvcCfg(flags)))
        
    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))

    acc.setPrivateTools(CompFactory.TRTCalibrator(name, **kwargs))
    return acc
    



def CaltagConfiguration(caltag, **kwargs):
    # caltag dependent stuff
    if caltag == 'all':
        kwargs.setdefault("Selstring",'_*_-_-_-_-_-_-')
        kwargs.setdefault("NoHistograms",['TRT'])        
        kwargs.setdefault("CalibrateT0",['TRT'])
        kwargs.setdefault("CalibrateRt",['TRT'])
        kwargs.setdefault("PrintLog",['TRT'])
        kwargs.setdefault("PrintT0Out",['TRT'])
        kwargs.setdefault("PrintRtOut",['TRT'])
    elif caltag == 'barrel':
        kwargs.setdefault("Selstring",'_*_1_*_-_-_-_-')
        kwargs.setdefault("NoHistograms",['TRT'])        
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer'])
        kwargs.setdefault("PrintT0Out",['Layer'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-1':
        kwargs.setdefault("Selstring",'_*_-1_-_-_-_-_-')
        kwargs.setdefault("NoHistograms",['TRT'])        
        kwargs.setdefault("CalibrateT0",['TRT','Detector'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector'])
        kwargs.setdefault("PrintT0Out",['TRT','Detector'])
        kwargs.setdefault("PrintRtOut",['TRT','Detector'])
    elif caltag == '-1_0':
        kwargs.setdefault("Selstring",'_*_-1_0_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-1_1':
        kwargs.setdefault("Selstring",'_*_-1_1_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-1_2':
        kwargs.setdefault("Selstring",'_*_-1_2_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '1':
        kwargs.setdefault("Selstring",'_*_1_-_-_-_-_-')
        kwargs.setdefault("NoHistograms",['TRT'])        
        kwargs.setdefault("CalibrateT0",['TRT','Detector'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector'])
        kwargs.setdefault("PrintT0Out",['TRT','Detector'])
        kwargs.setdefault("PrintRtOut",['TRT','Detector'])
    elif caltag == '1_0':
        kwargs.setdefault("Selstring",'_*_1_0_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '1_1':
        kwargs.setdefault("Selstring",'_*_1_1_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '1_2':
        kwargs.setdefault("Selstring",'_*_1_2_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '2':
        kwargs.setdefault("Selstring",'_*_2_-_-_-_-_-')
        kwargs.setdefault("NoHistograms",['TRT'])        
        kwargs.setdefault("CalibrateT0",['TRT','Detector'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector'])
    elif caltag == '2_a':
        kwargs.setdefault("Selstring",'_*_2_0,1,2,3_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '2_b':
        kwargs.setdefault("Selstring",'_*_2_4,5,6,7_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '2_c':
        kwargs.setdefault("Selstring",'_*_2_8,9,10,11_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '2_d':
        kwargs.setdefault("Selstring",'_*_2_12,13_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-2':
        kwargs.setdefault("Selstring",'_*_-2_-_-_-_-_-')
        kwargs.setdefault("NoHistograms",['TRT'])        
        kwargs.setdefault("CalibrateT0",['TRT','Detector'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector'])
    elif caltag == '-2_a':
        kwargs.setdefault("Selstring",'_*_-2_0,1,2,3_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-2_b':
        kwargs.setdefault("Selstring",'_*_-2_4,5,6,7_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-2_c':
        kwargs.setdefault("Selstring",'_*_-2_8,9,10,11_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
    elif caltag == '-2_d':
        kwargs.setdefault("Selstring",'_*_-2_12,13_*_*_*_*')
        kwargs.setdefault("NoHistograms",['TRT','Detector','Chip','Straw'])
        kwargs.setdefault("CalibrateT0",['TRT','Detector','Layer','Module','Board','Chip'])
        kwargs.setdefault("CalibrateRt",['TRT','Detector'])
        kwargs.setdefault("PrintLog",['TRT','Detector','Layer','Module','Board','Chip'])                          
        kwargs.setdefault("PrintT0Out",['Layer','Module','Board','Chip','Straw'])
        kwargs.setdefault("PrintRtOut",['Layer'])
        
    return kwargs