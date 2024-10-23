# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/CondConfig.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

from IOVDbSvc.IOVDbSvcConfig import addFolders, addFoldersSplitOnline, addOverride
from EventSelectorAthenaPool.CondProxyProviderConfig import CondProxyProviderCfg

def CondCfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    ## see https://gitlab.cern.ch/atlas/athena/-/blob/main/Database/IOVDbSvc/python/IOVDbSvcConfig.py
    if flags.InDet.Align.siPoolFile:
        cfg.merge(CondProxyProviderCfg(flags, flags.InDet.Align.siPoolFile))
    
    if flags.InDet.Align.inputAlignmentConstants:
        cfg.merge(CondProxyProviderCfg(flags,
                                       flags.InDet.Align.inputAlignmentConstants))
        
    if flags.InDet.Align.ErrorScalingTag:
        if ".db" in flags.InDet.Align.ErrorScalingTag:
            cfg.merge(addFolders(
                flags, 
                f'<dbConnection>sqlite://X;schema={flags.InDetAlign.ErrorScalingTag};dbname=CONDBR2</dbConnection>/Indet/TrkErrorScaling',
                tag = 'IndetTrkErrorScaling_nominal'))

        else:
            cfg.merge(addOverride(flags, '/Indet/TrkErrorScaling',
                                  flags.InDet.Align.ErrorScalingTag))
        
    if flags.InDet.Align.BeamSpotTag and flags.Beam.Type is not BeamType.Cosmics:
        cfg.merge(addOverride(flags, '/Indet/Beampos', flags.InDet.Align.BeamSpotTag))
        
    if flags.InDet.Align.LorentzAngleTag and flags.Beam.Type is not BeamType.Cosmics:
        cfg.merge(addOverride(flags, '/PIXEL/LorentzAngleScale',
                              flags.InDet.Align.LorentzAngleTag))
        
    if flags.InDet.Align.TRTCalibT0TagCos:
        cfg.merge(addOverride(flags, '/TRT/Calib/T0',
                              flags.InDet.Align.TRTCalibT0TagCos))
        
    if flags.InDet.Align.TRTCalibRtTagCos:
        cfg.merge(addOverride(flags, '/TRT/Calib/RT',
                              flags.InDet.Align.TRTCalibT0TagCos))
        
    if flags.InDet.Align.MDNTag:
        cfg.merge(addOverride(flags, "/PIXEL/PixelClustering/PixelNNCalibJSON",
                              flags.InDet.Align.MDNTag))
        
    if flags.InDet.Align.inputBowingDatabase:
        if (".db" in flags.InDet.Align.inputBowingDatabase and
            not flags.InDet.Align.readL3Only):
            cfg.merge(addFolders(
                flags, 
                f'<dbConnection>sqlite://X;schema={flags.InDet.Align.inputBowingDatabase};dbname=CONDBR2</dbConnection>/Indet/IBLDist',
                tag = 'IndetIBLDist', 
                className = "CondAttrListCollection"))
            
        else:
            cfg.merge(addOverride(flags, '/Indet/IBLDist',
                                  flags.InDet.Align.inputBowingDatabase))
            
    if flags.InDet.Align.useDynamicAlignFolders:
        globalDB = flags.InDet.Align.inputDynamicGlobalDatabase
        if globalDB:
            if ".db" in globalDB and not flags.InDet.Align.readL3Only:
                cfg.merge(addFolders(
                    flags, 
                    f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/Indet/AlignL1/ID<tag>IndetL1Test</tag>',
                    className="CondAttrListCollection"))
                
                cfg.merge(addFolders(
                    flags,
                    f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/Indet/AlignL2/PIX<tag>IndetL2PIXTest</tag>',
                    className = "CondAttrListCollection"))
                    
                cfg.merge(addFolders(
                    flags,
                    f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/Indet/AlignL2/SCT<tag>IndetL2SCTTest</tag>',
                    className="CondAttrListCollection"))
                    
                cfg.merge(addFolders(
                    flags,
                    f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/TRT/AlignL1/TRT<tag>IndetL1TRTTest</tag>',
                    className = "CondAttrListCollection"))
                
                cfg.merge(addFolders(
                    flags,
                    f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/Indet/AlignL3<tag>IndetAlign_test</tag>',
                    className="AlignableTransformContainer"))
                    
                cfg.merge(addFolders(
                    flags,
                    f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/TRT/AlignL2<tag>TRTAlign_test</tag>',
                    className = "AlignableTransformContainer"))

        if globalDB == "" or flags.InDet.Align.readL3Only:
            if flags.InDet.Align.DynamicL1IDTag:
                cfg.merge(addOverride('/Indet/AlignL1/ID',
                                      flags.InDet.Align.DynamicL1IDTag))
                
            if flags.InDet.Align.DynamicL2PIXTag:
                cfg.merge(addOverride(flags, '/Indet/AlignL2/PIX',
                                      flags.InDet.Align.DynamicL2PIXTag))

            if flags.InDet.Align.DynamicL2SCTTag:
                cfg.merge(addOverride(flags, '/Indet/AlignL2/SCT',
                                      flags.InDet.Align.DynamicL2SCTTag))

            if flags.InDet.Align.DynamicL3SiTag:
                cfg.merge(addOverride(flags, '/Indet/AlignL3',
                                      flags.InDet.Align.DynamicL3SiTag))

            if flags.InDet.Align.DynamicL1TRTTag:
                cfg.merge(addOverride(flags, '/TRT/AlignL1/TRT',
                                      flags.InDet.Align.DynamicL1TRTTag))

            if flags.InDet.Align.DynamicL2TRTTag:
                cfg.merge(addOverride(flags, '/TRT/AlignL2',
                                      flags.InDet.Align.DynamicL2TRTTag))

        if ".db" in globalDB and flags.InDet.Align.readL3Only:
            cfg.merge(addFolders(
                flags,
                f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/Indet/AlignL3<tag>IndetAlign_test</tag>',
                className = "AlignableTransformContainer"))
                
            cfg.merge(addFolders(
                flags,
                f'<dbConnection>sqlite://X;schema={globalDB};dbname=CONDBR2</dbConnection>/TRT/AlignL2<tag>TRTAlign_test</tag>',
                className = "AlignableTransformContainer"))
                
    if flags.InDet.Align.PixelDistortionTag:
        cfg.merge(addOverride(flags, '/Indet/PixelDist', flags.InDet.Align.PixelDistortionTag))
        
    cfg.merge(addFoldersSplitOnline(
        flags, "TRT",
        "/TRT/Onl/Cond/StatusHT", "/TRT/Cond/StatusHT",
        className = 'TRTCond::StrawStatusMultChanContainer'))
        
    return cfg
