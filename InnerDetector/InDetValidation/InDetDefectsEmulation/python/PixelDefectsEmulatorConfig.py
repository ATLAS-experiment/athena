#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
        histSvc = CompFactory.THistSvc(Output = [f"{HistogramGroup} DATAFILE='{FileName}', OPT='RECREATE'"] )
        acc.addService(histSvc)
    return acc

def ITkDefectsHistSvcCfg(flags, HistogramGroup="ITkPixelDefects") -> ComponentAccumulator:
    return DefectsHistSvcCfg(flags,HistogramGroup)


def PixelDefectsEmulatorCondAlgCfg(flags,
                                   name: str = "PixelDefectsEmulatorCondAlg",
                                   **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ModulePatterns", [[-2,2,0,99,-99,99,-99,99,0,9999,0,0,0]]) # ranges: barrel/ec, all layers, all eta, all phi, all column counts,
                                                                                  # all sides, don't auto-match connected rows
    kwargs.setdefault("DefectProbabilities", [[0.,1e-3,0.,0.]])                   # probabilities: module, pixel, core-column, circuit
    kwargs.setdefault("NDefectFractionsPerPattern",[[1.,-1, 1.]])                 # fractions for exactly 1..N core-column, circuit defects
    kwargs.setdefault("DetEleCollKey", "PixelDetectorElementCollection")
    kwargs.setdefault("WriteKey", "PixelEmulatedDefects")
    kwargs.setdefault("HistogramGroupName","") # disable histogramming; enable: e.g. /PixelDefects/EmulatedDefects/

    kwargs.setdefault("RngPerDefectType",False) # If True use one RNG per defect type (module, chip-defects, core-column, pixel-defects, corner-defects)
    kwargs.setdefault("DefectsInputFiles",[])   # If not empty read defects from input files and merge defects (root RNTuple/json; extension: .root .json")
    kwargs.setdefault("DefectsOutputFile","")   # If not empty write defects to a  output file (root RNTuple/json; (extension: .root .json")
    if "DefectsOutputFile" in kwargs and kwargs["DefectsOutputFile"] is None :
        kwargs["DefectsOutputFile"]=""

    acc.addCondAlgo(CompFactory.InDet.PixelDefectsEmulatorCondAlg(name,**kwargs))
    return acc

def ITkPixelDefectsEmulatorCondAlgCfg(flags,
                                      name: str = "ITkPixelDefectsEmulatorCondAlg",
                                      **kwargs: dict) -> ComponentAccumulator:
    kwargs.setdefault("ModulePatterns", [[-2,2,0,99,-99,99,-99,99,0,9999,0,0,0]]) # range-pairs+flag: barrel/ec, all layers, all eta, all phi, all column counts,
                                                                                  # all sides, don't auto-match connected rows
    kwargs.setdefault("DefectProbabilities", [[0.,1e-2, 1e-1,0.]])                # probabilities: module, pixel, core-column, circuit
    kwargs.setdefault("NDefectFractionsPerPattern",[[1.,-1, 1.]])                 # fractions for exactly 1..N core-column, circuit defects

    kwargs.setdefault("DetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("WriteKey", "ITkPixelEmulatedDefects")

    kwargs.setdefault("RngPerDefectType",False) # If True use one RNG per defect type (module, chip-defects, core-column, pixel-defects, corner-defects)
    kwargs.setdefault("DefectsInputFiles",[])   # If not empty read defects from input files and merge defects (root RNTuple/json; extension: .root .json")
    kwargs.setdefault("DefectsOutputFile","")   # If not empty write defects to a  output file (root RNTuple/json; (extension: .root .json")
    if "DefectsOutputFile" in kwargs and kwargs["DefectsOutputFile"] is None :
        kwargs["DefectsOutputFile"]=""

    return PixelDefectsEmulatorCondAlgCfg(flags,name,**kwargs)


def PixelDefectsEmulatorAlgCfg(flags,
                                  name: str = "PixelDefectsEmulatorAlg",
                                  **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if "InputKey" not in kwargs :
        # rename original RDO collection
        acc.merge(PixelRDORemappingCfg(flags))
        kwargs.setdefault("InputKey","PixelRDOs_ORIG")

    if "EmulatedDefectsKey" not in kwargs :
        # create defects conditions data
        acc.merge( PixelDefectsEmulatorCondAlgCfg(flags))
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

def ITkPixelDefectsEmulatorToDetectorElementStatusCondAlgCfg(flags,
                                                             name: str = "ITkPixelDefectsEmulatorToDetectorElementStatusCondAlgCfg",
                                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("EmulatedDefectsKey","ITkPixelEmulatedDefects")
    kwargs.setdefault("WriteKey","ITkPixelDetectorElementStatusFromEmulatedDefects")
    acc.addCondAlgo(CompFactory.InDet.PixelEmulatedDefectsToDetectorElementStatusCondAlg(name,**kwargs))
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

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelDetectorElementStatusCondAlgNoByteStreamErrorsCfg
    acc.merge(ITkPixelDetectorElementStatusCondAlgNoByteStreamErrorsCfg(flags))

    from PixelDefectsEmulatorPostInclude import emulateITkPixelDefectsPoisson
    emulateITkPixelDefectsPoisson(flags,acc)

    acc.printConfig(withDetails=True, summariseProps=True,printDefaults=True)
    sc = acc.run()

    if sc.isFailure():
        import sys
        sys.exit(1) 
