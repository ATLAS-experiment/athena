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
