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

    result.getService("THistSvc").Output = [
        "SiHitAnalysis DATAFILE='SiHitValid.root' OPT='RECREATE'"]

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

    result.getService("THistSvc").Output = []
    if flags.Detector.EnablePixel or flags.Detector.EnableSCT:
        result.getService("THistSvc").Output += [
            "SiHitAnalysis DATAFILE='SiHitValid.root' OPT='RECREATE'"]
    if flags.Detector.EnableTRT:
        result.getService("THistSvc").Output += [
            "TRTHitAnalysis DATAFILE='TRTHitValid.root' OPT='RECREATE'"]
 
    return result
