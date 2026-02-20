# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonGeoAlignCondAlgCfg(flags, name="ActsMuonAlignCondAlg", **kwargs):
    result = ComponentAccumulator()
    ### Do not setup the Acts alignment cond alg if no alignment or passivation is requested
    if not flags.Muon.usePhaseIIGeoSetup or ( not flags.Muon.enableAlignment and \
                                            not flags.Muon.applyMMPassivation):
        return result
    
    from MuonConfig.MuonGeometryConfig import MuonAlignmentCondAlgCfg
    kwargs.setdefault("applyMmPassivation", flags.Muon.applyMMPassivation)
    

    if kwargs["applyMmPassivation"]:
        from MuonConfig.MuonCondAlgConfig import NswPassivationDbAlgCfg
        result.merge(NswPassivationDbAlgCfg(flags))
    if flags.Muon.enableAlignment:
        result.merge(MuonAlignmentCondAlgCfg(flags))
    kwargs.setdefault("applyALines", flags.Muon.Align.UseALines)
    kwargs.setdefault("applyBLines", flags.Muon.Align.UseBLines)
    kwargs.setdefault("applyNswAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "NswAsBuiltCondAlg"])>0)
    kwargs.setdefault("applyMdtAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "MdtAsBuiltCondAlg"])>0)
    from MuonConfig.MuonConfigFlags import GeoTrfCacheMode
    kwargs.setdefault("FillAlignCache", flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.FullCacheCond)
    kwargs.setdefault("FillGeoAlignStore", flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSplitALineCond or
                                           flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSlopyALineCond)
    the_alg = CompFactory.MuonR4.GeomAlignCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def MdtAnalyticRtCalibAlgCfg(flags, name="MdtAnalyticCalibDbAlg",
                                    diagnosticsFile="RtDiagnositcs.root", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutStream", "MDTANALYTICRTS")
    kwargs.setdefault("saveDiagnosticHist", True)
    if kwargs["saveDiagnosticHist"]:
        from MuonConfig.MuonConfigUtils import setupHistSvcCfg
        result.merge(setupHistSvcCfg(flags, outFile=diagnosticsFile, outStream=kwargs["OutStream"]))
    the_alg = CompFactory.MuonCalibR4.MdtAnalyticRtCalibAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
    
