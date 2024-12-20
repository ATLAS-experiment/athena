#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
  Emulating pixel defects by dropping elements from the RDO input container
"""
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Constants import INFO


def PixelRDORemappingCfg(flags, InputKey="PixelRDOs") :
    acc = ComponentAccumulator()

    from SGComps.AddressRemappingConfig import AddressRemappingCfg
    renames = [ '%s#%s->%s' % ('PixelRDO_Container', InputKey, f"{InputKey}_ORIG") ]
    acc.merge(AddressRemappingCfg( renameMaps = renames ))
    return acc

def ITkPixelRDORemappingCfg(flags) :
    return PixelRDORemappingCfg(flags,"ITkPixelRDOs")

def DefectsHistSvcCfg(flags, HistogramGroup: str="PixelDefects", FileName: str='pixel_defects.root') -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if HistogramGroup is not None and len(HistogramGroup) > 0 and FileName is not None and len(FileName) > 0 :
        print("DEBUG DefectsHistSvcCfg",HistogramGroup,FileName,[f"{HistogramGroup} DATAFILE='{FileName}', OPT='RECREATE'"])
        histSvc = CompFactory.THistSvc(Output = [f"{HistogramGroup} DATAFILE='{FileName}', OPT='RECREATE'"] )
        acc.addService(histSvc)
    return acc

def ITkDefectsHistSvcCfg(flags, HistogramGroup="ITkPixelDefects") -> ComponentAccumulator:
    return DefectsHistSvcCfg(flags,HistogramGroup)


def PixelDefectsEmulatorCondAlgCfg(flags,
                                   name: str = "PixelDefectsEmulatorCondAlg",
                                   **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("DefectProbability", 1e-4)
    kwargs.setdefault("CoreColumnDefectProbability", 0.) # there are no core columns for run3 pixel modules
    kwargs.setdefault("PixelDetEleCollKey", "PixelDetectorElementCollection")
    kwargs.setdefault("WriteKey", "PixelEmulatedDefects")
    kwargs.setdefault("HistogramGroupName","") # disable histogramming; enable: e.g. /PixelDefects/EmulatedDefects/

    acc.addCondAlgo(CompFactory.InDet.PixelDefectsEmulatorCondAlg(name,**kwargs))
    return acc

def ITkPixelDefectsEmulatorCondAlgCfg(flags,
                                      name: str = "ITkPixelDefectsEmulatorCondAlg",
                                      **kwargs: dict) -> ComponentAccumulator:
    kwargs.setdefault("DefectProbability", 1e-4)

    # 1-(1-prob_col_def )**n_col_groups
    def probColGroupDefect(prob_col_group_defect_per_mod, n_col_groups) :
        # prob of at least one defects per module : prob of not no defect per module
        return 1. - pow( (1-prob_col_group_defect_per_mod), 1/(n_col_groups))
    kwargs.setdefault("CoreColumnDefectProbability", probColGroupDefect(.1, 400/8.) ) # ~20 % prob / chip  for 400/8. core groups / chip
    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("WriteKey", "ITkPixelEmulatedDefects")

    return PixelDefectsEmulatorCondAlgCfg(flags,name,**kwargs)


def PixelDefectsEmulatorAlgCfg(flags,
                                  name: str = "ITkPixelDefectsEmulatorAlg",
                                  **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if "InputKey" not in kwargs :
        # rename original RDO collection
        acc.merge(PixelRDORemappingCfg(flags))
        kwargs.setdefault("InputKey","PixelRDOs_ORIG")

    if "EmulatedDefectsKey" not in kwargs :
        # create defects conditions data
        acc.merge( ITkPixelDefectsEmulatorCondAlgCfg(flags))
        kwargs.setdefault("EmulatedDefectsKey", "PixelEmulatedDefects")
    kwargs.setdefault("OutputKey","PixelRDOs")
    kwargs.setdefault("HistogramGroupName","") # disable histogramming, enable e.g. /PixelDefects/RejectedRDOs/

    acc.addEventAlgo(CompFactory.InDet.PixelDefectsEmulatorAlg(name,**kwargs))
    return acc

def ITkPixelDefectsEmulatorAlgCfg(flags,
                                  name: str = "ITkPixelDefectsEmulatorAlg",
                                  **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if "InputKey" not in kwargs :
        # rename original RDO collection
        acc.merge(ITkPixelRDORemappingCfg(flags))
        kwargs.setdefault("InputKey","ITkPixelRDOs_ORIG")

    if "EmulatedDefectsKey" not in kwargs :
        # create defects conditions data
        acc.merge( ITkPixelDefectsEmulatorCondAlgCfg(flags))
        kwargs.setdefault("EmulatedDefectsKey", "ITkPixelEmulatedDefects")
    kwargs.setdefault("OutputKey","ITkPixelRDOs")

    kwargs.setdefault("HistogramGroupName","") # disable histogramming, enable e.g. /PixelDefects/RejectedRDOs/

    acc.addEventAlgo(CompFactory.InDet.PixelDefectsEmulatorAlg(name,**kwargs))
    return acc


if __name__ == "__main__":

    flags = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.GeoModel.Align.Dynamic = False
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1']

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True
    flags.Detector.GeometryBpipe = True
    flags.Detector.GeometryCalo = False

    flags.Concurrency.NumThreads = 8
    flags.Concurrency.NumConcurrentEvents = 8

    flags.Exec.MaxEvents = 10

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg( flags )
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Need geometry
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge( ActsTrackingGeometrySvcCfg(flags,
                                          OutputLevel=INFO,
                                          RunConsistencyChecks=False,
                                          ObjDebugOutput=False))

    acc.merge( ITkPixelDefectsEmulatorAlgCfg(flags,
                                             OutputLevel=INFO))

    acc.printConfig(withDetails=True, summariseProps=True,printDefaults=True)
    sc = acc.run()

    if sc.isFailure():
        import sys
        sys.exit(1)
