# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMuonAlignCondAlgCfg(flags, name="ActsMuonAlignCondAlg", **kwargs):
    result = ComponentAccumulator()
    ### Do not setup the Acts alignment cond alg if no alignment or passivation is requested
    if not flags.Muon.usePhaseIIGeoSetup or ( not flags.Muon.enableAlignment and \
                                            not flags.Muon.applyMMPassivation):
        return result
    
    from MuonConfig.MuonGeometryConfig import MuonAlignmentCondAlgCfg
    kwargs.setdefault("applyMmPassivation", flags.Muon.applyMMPassivation)
    kwargs.setdefault("FillAlignCache", False)
    kwargs.setdefault("FillGeoAlignStore", False)
    

    if kwargs["applyMmPassivation"]:
        from MuonConfig.MuonCondAlgConfig import NswPassivationDbAlgCfg
        result.merge(NswPassivationDbAlgCfg(flags))
    if flags.Muon.enableAlignment:
        result.merge(MuonAlignmentCondAlgCfg(flags))
    kwargs.setdefault("applyALines", flags.Muon.Align.UseALines)
    kwargs.setdefault("applyBLines", flags.Muon.Align.UseBLines)
    kwargs.setdefault("applyNswAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "NswAsBuiltCondAlg"])>0)
    kwargs.setdefault("applyMdtAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "MdtAsBuiltCondAlg"])>0)

    the_alg = CompFactory.ActsMuonAlignCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg)
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
    
