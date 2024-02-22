# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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

    return result


def IDHitAnalysis(flags): 

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from HitAnalysis.HitAnalysisConfig import PixelHitAnalysisCfg, SCTHitAnalysisCfg
    result = ComponentAccumulator()
    result.merge(PixelHitAnalysisCfg(flags))
    result.merge(SCTHitAnalysisCfg(flags))
 
    result.getService("THistSvc").Output = ["SiHitAnalysis DATAFILE='SiHitValid.root' OPT='RECREATE'"]
 
    return result
