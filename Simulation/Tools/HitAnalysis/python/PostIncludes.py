# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Adding SiHitValidation for whichever parts of ITk are running
def ITkHitAnalysis(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from HitAnalysis.HitAnalysisConfig import ITkPixelHitAnalysisCfg, ITkStripHitAnalysisCfg, PLR_HitAnalysisCfg

    result = ComponentAccumulator()

    if flags.Detector.EnableITkPixel:
        result.merge(ITkPixelHitAnalysisCfg(flags))

    if flags.Detector.EnableITkStrip:
        result.merge(ITkStripHitAnalysisCfg(flags))

    if flags.Detector.EnablePLR:
        result.merge(PLR_HitAnalysisCfg(flags))

    return result

def HGTDHitAnalysis(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from HitAnalysis.HitAnalysisConfig import HGTD_HitAnalysisCfg

    result = ComponentAccumulator()

    if flags.Detector.EnableHGTD:
        result.merge(HGTD_HitAnalysisCfg(flags))

    result.getService("THistSvc").Output = [
        "HGTDHitAnalysis DATAFILE='HGTDHitValid.root' OPT='RECREATE'"]

    return result


def IDHitAnalysis(flags): 

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from HitAnalysis.HitAnalysisConfig import PixelHitAnalysisCfg, SCTHitAnalysisCfg, TRTHitAnalysisCfg
    result = ComponentAccumulator()
    if flags.Detector.EnablePixel:
        result.merge(PixelHitAnalysisCfg(flags))
    if flags.Detector.EnableSCT:
        result.merge(SCTHitAnalysisCfg(flags))
    if flags.Detector.EnableTRT:
        result.merge(TRTHitAnalysisCfg(flags))
    return result

def MuonHitAnalysis(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    if flags.Muon.usePhaseIIGeoSetup:
        from HitAnalysis.HitAnalysisConfig import xMuonHitAnalysisCfg
        from ROOT.Muon.MuonStationIndex import TechnologyIndex
        if flags.Detector.EnableMDT:
            result.merge(xMuonHitAnalysisCfg(flags,
                                             name="MdtSimHitTester",
                                             InputKey="xMdtSimHits",
                                             HistPath="xMuonSimHit/histos/MDT/Hits",
                                             techIndex=TechnologyIndex.MDT))
        if flags.Detector.EnableRPC:
            result.merge(xMuonHitAnalysisCfg(flags,
                                             name="RpcSimHitTester",
                                             InputKey="xRpcSimHits",
                                             HistPath="xMuonSimHit/histos/RPC/Hits",
                                             techIndex=TechnologyIndex.RPC))
        if flags.Detector.EnableTGC:
            result.merge(xMuonHitAnalysisCfg(flags,
                                             name="TgcSimHitTester",
                                             InputKey="xTgcSimHits",
                                             HistPath="xMuonSimHit/histos/TGC/Hits",
                                             techIndex=TechnologyIndex.TGC))
        if flags.Detector.EnablesTGC:
            result.merge(xMuonHitAnalysisCfg(flags,
                                             name="sTgcSimHitTester",
                                             InputKey="xStgcSimHits",
                                             HistPath="xMuonSimHit/histos/sTGC/Hits",
                                             techIndex=TechnologyIndex.STGC))
        if flags.Detector.EnableMM:
            result.merge(xMuonHitAnalysisCfg(flags,
                                             name="MmSimHitTester",
                                             InputKey="xMmSimHits",
                                             HistPath="xMuonSimHit/histos/MM/Hits",
                                             techIndex=TechnologyIndex.MM))
        from MuonPRDTestR4.MuonHitTestConfig import MuonHitTesterCfg
        result.merge(MuonHitTesterCfg(flags, outFile=flags.Output.HISTFileName))
    else:
        if flags.Detector.EnableMDT:
            from HitAnalysis.HitAnalysisConfig import MDTHitAnalysisCfg
            result.merge(MDTHitAnalysisCfg(flags))
        if flags.Detector.EnableRPC:
            from HitAnalysis.HitAnalysisConfig import RPCHitAnalysisCfg
            result.merge(RPCHitAnalysisCfg(flags))
        if flags.Detector.EnableTGC:
            from HitAnalysis.HitAnalysisConfig import TGCHitAnalysisCfg
            result.merge(TGCHitAnalysisCfg(flags))
        if flags.Detector.EnablesTGC:
            from HitAnalysis.HitAnalysisConfig import sTGCHitAnalysisCfg
            result.merge(sTGCHitAnalysisCfg(flags))
        if flags.Detector.EnableMM:
            from HitAnalysis.HitAnalysisConfig import MMHitAnalysisCfg
            result.merge(MMHitAnalysisCfg(flags)) 
        if flags.Detector.EnableCSC:
            from HitAnalysis.HitAnalysisConfig import CSCHitAnalysisCfg
            result.merge(CSCHitAnalysisCfg(flags))
        if flags.Detector.EnableMuon:
            from MuonPRDTest.HitValAlgSim import HitValAlgSimCfg
            result.merge(HitValAlgSimCfg(flags, outFile=flags.Output.HISTFileName))
 
 
    return result


def SimHitAnalysis(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    result.merge(ITkHitAnalysis(flags))
    result.merge(IDHitAnalysis(flags))
    result.merge(MuonHitAnalysis(flags))
    from HitAnalysis.HitAnalysisConfig import CaloHitAnalysisCfg, TrackRecordAnalysisCfg, TruthHitAnalysisCfg
    result.merge(CaloHitAnalysisCfg(flags))
    if flags.Detector.EnableMuon:
        result.merge(TrackRecordAnalysisCfg(flags))
    result.merge(TruthHitAnalysisCfg(flags))
    return result