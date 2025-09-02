# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/IDAlignConditionConfig.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

from IOVDbSvc.IOVDbSvcConfig import addOverride

def UpdateTagsCfg(flags, InputLocalDatabase = ""):
    cfg = ComponentAccumulator()
    
    if flags.InDet.Align.IBLDistTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.IBLDistTag:
            cfg.merge(addOverride(flags, '/Indet/IBLDist', tag = flags.InDet.Align.IBLDistTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
        
        else:
            cfg.merge(addOverride(flags, '/Indet/IBLDist', flags.InDet.Align.IBLDistTag))
    
    if flags.InDet.Align.L1IDTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L1IDTag:
            cfg.merge(addOverride(flags, '/Indet/AlignL1/ID', tag = flags.InDet.Align.L1IDTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
        
        else:
            cfg.merge(addOverride(flags, '/Indet/AlignL1/ID', flags.InDet.Align.L1IDTag))
                    
    if flags.InDet.Align.L2PIXTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L2PIXTag:
            cfg.merge(addOverride(flags, '/Indet/AlignL2/PIX', tag = flags.InDet.Align.L2PIXTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
        
        else:
            cfg.merge(addOverride(flags, '/Indet/AlignL2/PIX', flags.InDet.Align.L2PIXTag))

    if flags.InDet.Align.L2SCTTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L2SCTTag:
            cfg.merge(addOverride(flags, '/Indet/AlignL2/SCT', tag = flags.InDet.Align.L2SCTTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
        
        else:
            cfg.merge(addOverride(flags, '/Indet/AlignL2/SCT', flags.InDet.Align.L2SCTTag))

    if flags.InDet.Align.L3SiTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L3SiTag:
            cfg.merge(addOverride(flags, '/Indet/AlignL3', tag = flags.InDet.Align.L3SiTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
    
        else:
            cfg.merge(addOverride(flags, '/Indet/AlignL3', flags.InDet.Align.L3SiTag))

    if flags.InDet.Align.L1TRTTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L1TRTTag:
            cfg.merge(addOverride(flags, '/TRT/AlignL1/TRT', tag = flags.InDet.Align.L1TRTTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
    
        else:
            cfg.merge(addOverride(flags, '/TRT/AlignL1/TRT', flags.InDet.Align.L1TRTTag))

    if flags.InDet.Align.L2TRTTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L2TRTTag:
            cfg.merge(addOverride(flags, '/TRT/AlignL2', tag = flags.InDet.Align.L2TRTTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
    
        else:
            cfg.merge(addOverride(flags, '/TRT/AlignL2', flags.InDet.Align.L2TRTTag))
            
    if flags.InDet.Align.L3TRTTag:
        if InputLocalDatabase and "T0" in flags.InDet.Align.L3TRTTag:
            cfg.merge(addOverride(flags, '/TRT/Calib/DX', tag = flags.InDet.Align.L3TRTTag, db = f"sqlite://;schema={InputLocalDatabase};dbname=CONDBR2"))
    
        else:
            cfg.merge(addOverride(flags, '/TRT/Calib/DX', flags.InDet.Align.L3TRTTag))
                        
    if flags.InDet.Align.TRTCalibT0TagCos:
        cfg.merge(addOverride(flags, '/TRT/Calib/T0', flags.InDet.Align.TRTCalibT0TagCos))
            
    if flags.InDet.Align.TRTCalibRtTagCos:
        cfg.merge(addOverride(flags, '/TRT/Calib/RT', flags.InDet.Align.TRTCalibRtTagCos))
            
    if flags.InDet.Align.MDNTag:
        cfg.merge(addOverride(flags, "/PIXEL/PixelClustering/PixelNNCalibJSON", flags.InDet.Align.MDNTag))
            
    if flags.InDet.Align.pixelDistortionTag:
        cfg.merge(addOverride(flags, '/Indet/PixelDist', flags.InDet.Align.pixelDistortionTag))
            
    if flags.Beam.Type is not BeamType.Cosmics:
        if flags.InDet.Align.beamSpotTag:
            cfg.merge(addOverride(flags, '/Indet/Beampos', flags.InDet.Align.beamSpotTag))
                
        if flags.InDet.Align.lorentzAngleTag:
            cfg.merge(addOverride(flags, '/PIXEL/LorentzAngleScale', flags.InDet.Align.lorentzAngleTag))
        
    return cfg
