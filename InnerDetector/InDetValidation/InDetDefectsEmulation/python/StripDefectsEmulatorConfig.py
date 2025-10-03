#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
  Emulating strip defects by dropping elements from the RDO input container
"""
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Constants import INFO

def ordered_pairs(a_list) :
    """
    Return True if every two elements in a_list form a pair whose two elements are
    in ascending order. The element pairs themselves may have an arbitrary order.
    """
    for i in range(0,len(a_list),2) :
        if a_list[i]>a_list[i+1] :
            return False
    return True

def makeCornerDefectParam(probability=1e-1,
                          min_rx=0.,
                          max_rx=4e-3,
                          min_ry=0.,
                          max_ry=4e-3,
                          min_sagitta=0.,
                          max_sagitta=2e-3) :
    """
    Convenience method to create a parameter set for module corner defects.
    The corner defects have a circular shape and intersect the sensor edges at certain
    points in x and y direction. The line between these two intersection points forms
    a sagitta of the circle.
    probability: Probability of a module to have one or more corner defects.
    min_rx: minimum position from the corner in local x-direction of possible intersection points.
    max_rx: maximum position from the corner in local x-direction of possible intersection points.
    min_ry: minimum position from the corner in local y-direction of possible intersection points.
    max_ry: maximum position from the corner in local x-direction of possible intersection points.
    min_sagitta: minimum size of the sagitta (upper limit of circle radius)
    max_sagitta: maximum size of the sagitta (lower limit of circle radius)
    """
    assert  min_rx<max_rx
    assert  min_ry<max_ry
    assert  min_sagitta<max_sagitta
    return [probability,
            min_rx, max_rx-min_rx,
            min_ry, max_ry-min_ry,
            min_sagitta, max_sagitta-min_sagitta]

def moduleDefect(bec=[-2,2],
                 layer=[0,8],
                 eta_range=[-99,99],
                 phi_range=[-99,99],
                 columns_or_strips=[0,2000],
                 column_or_strip_length=[0,1000000],
                 side_range=[0,1],
                 all_rows=True,
                 probability=[1e-2],
                 fractionsOfNDefects=[],
                 noiseProbability=None,
                 noiseShape=[],
                 cornerDefectParam=None,
                 cornerDefectNCornerFractions=None,
                 ) :
    '''
    Convenience function to create parameters for strip or pixel defects emulation conditions data
    bec: range to select the bec identifier part (range is inclusive)
    layer: range to select the layer/disc identifier part (range is inclusive)
    eta_range: range to select the eta index identifier part (range is inclusive)
    phi_range: range to select the phi index identifier part (range is inclusive)
    columns_or_strips: match only modules with number of columns or strips in this range (range is inclusive)
    column_or_strip_length: match the strip lengths in microns (range is inclusive)
    side_range: to select both sides of a sensor [0,1], to select either side [0,0,1,1]
    all_rows: to select individal modules (0), or all modules associated to the same physical sensor (1)
    probabilities: module-defect, strip/pixel defect (for pixel additionally group defect probabilities:
                   at least one core-column defect, at least one defect circuit)
    fractions: per group defect, fractions to have exactly 1..n group defects under the condition that there is
               at least one such group defect.
    noiseProbability: None or probability of a strip (or pixel) to produce a spurious hit.
    noiseShape: empty list or binned pdf used to compute one random number per spurious hit (time bin for strips, tot for pixel).
    cornerDefectParam: None or parameters e.g. created with makeCornerDefectParam to create defects at module corners.
    cornerDefectNCornerFractions: if cornerDefectParam is not None the fractions of 1 to 4 corners to have a defect if the module
                                  has such defects.
    '''
    # test that the ranges have coorect number of elements and that ranges are ordered
    assert len(bec)%2==0 and len(bec) >= 2 and ordered_pairs(bec)
    assert len(layer)%2==0 and len(layer) >= 2 and ordered_pairs(layer)
    assert len(eta_range)%2==0 and len(eta_range) >= 2 and ordered_pairs(eta_range)
    assert len(phi_range)%2==0 and len(phi_range) >= 2 and ordered_pairs(phi_range)
    assert len(side_range)%2==0 and len(side_range) >= 2 and ordered_pairs(side_range)
    assert len(columns_or_strips)%2==0 and len(columns_or_strips) >= 2 and ordered_pairs(columns_or_strips)
    assert len(column_or_strip_length)%2==0 and len(column_or_strip_length) >= 2 and ordered_pairs(column_or_strip_length)
    assert len(fractionsOfNDefects)==0 or len(fractionsOfNDefects)+2==len(probability)

    assert cornerDefectParam is not None or cornerDefectNCornerFractions is None
    assert cornerDefectParam is None or cornerDefectNCornerFractions is not None

    length=[ l for l in set([len(bec),len(layer),len(eta_range),len(phi_range),len(side_range),len(columns_or_strips)]) ]
    # every range must contain a multiple of the number of elements of every other range
    for i in range(1,len(length)) :
        for j in range(0,i) :
            assert max(length[i],length[j])%min(length[i],length[j])==0

    module_pattern=[]
    prob=[]
    fractions=[]
    noiseProbList=[]
    noiseShapeList=[]
    cornerDefectList=[]
    cornerDefectCornerFractionList=[]
    if noiseProbability is not None and noiseProbability>0. :
        # ensure that the the noise shape integral is 1.
        shape_sum = sum(noiseShape)
        noiseShape = [ elm/shape_sum for elm in noiseShape ]
    else :
        noiseProbability=None

    for i in range(0,max(length),2) :
        module_pattern+=[ [bec[i%len(bec)], bec[(i+1)%len(bec)],
                           layer[i%len(layer)], layer[(i+1)%len(layer)],
                           eta_range[i%len(eta_range)], eta_range[(i+1)%len(eta_range)],
                           phi_range[i%len(phi_range)], phi_range[(i+1)%len(phi_range)],
                           columns_or_strips[i%len(columns_or_strips)], columns_or_strips[(i+1)%len(columns_or_strips)],
                           column_or_strip_length[i%len(column_or_strip_length)], column_or_strip_length[(i+1)%len(column_or_strip_length)],
                           side_range[i%len(side_range)], side_range[(i+1)%len(side_range)],
                           1 if all_rows else 0] ]
        prob += [ probability ]
        fractions += [ [fraction  for per_pattern in fractionsOfNDefects for fraction in per_pattern + [-1] ] ]
        if noiseProbability is not None:
            noiseProbList+=[noiseProbability]
            noiseShapeList+=[noiseShape]
        if cornerDefectParam is not None:
            cornerDefectList+=[ cornerDefectParam ]
            cornerDefectCornerFractionList+=[ cornerDefectNCornerFractions ]

    return module_pattern, prob, fractions, noiseProbList, noiseShapeList, cornerDefectList, cornerDefectCornerFractionList

def combineModuleDefects( defects ) :
    '''
    Convenience function to combine a list of outputs of calls of moduleDefect, to produce lists to
    set the ModulePatterns, DefectProbabilities, and NDefectFractionsPerPattern etc. properties of the
    pixel or strip DefectsEmulatorCondAlg.
    '''
    result=[]
    for i in range(0,len(defects[0])) :
        result.append( [ elm for sublist in defects for elm in sublist[i] ] )
    return result

def StripRDORemappingCfg(flags, InputKey="StripRDOs") :
    """
    Remapping service to rename the input strip RDO collection
    """
    acc = ComponentAccumulator()

    from SGComps.AddressRemappingConfig import AddressRemappingCfg
    renames = [ '%s#%s->%s' % ('SCT_RDO_Container', InputKey, f"{InputKey}_ORIG") ]
    acc.merge(AddressRemappingCfg( renameMaps = renames ))
    return acc

def ITkStripRDORemappingCfg(flags) :
    """
    Remapping service to rename the input ITk strip RDO collection
    """
    return StripRDORemappingCfg(flags,"ITkStripRDOs")

def DefectsHistSvcCfg(flags, HistogramGroup: str="StripDefects", FileName: str='strip_defects.root') -> ComponentAccumulator:
    """
    THistSvc to histogram some properties of emulated defects.
    """
    acc = ComponentAccumulator()
    if HistogramGroup is not None and len(HistogramGroup) > 0 and FileName is not None and len(FileName) > 0 :
        histSvc = CompFactory.THistSvc(Output = [f"{HistogramGroup} DATAFILE='{FileName}', OPT='RECREATE'"] )
        acc.addService(histSvc)
    return acc

def ITkDefectsHistSvcCfg(flags, HistogramGroup="ITkStripDefects") -> ComponentAccumulator:
    return DefectsHistSvcCfg(flags,HistogramGroup)


def StripDefectsEmulatorCondAlgCfg(flags,
                                   name: str = "StripDefectsEmulatorCondAlg",
                                   **kwargs: dict) -> ComponentAccumulator:
    """
    Schedule conditions algorithm to create emulated strip defect conditions data
    """
    acc = ComponentAccumulator()
    kwargs.setdefault("ModulePatterns", [[-2,2,0,99,-99,99,-99,99,0,9999,0,1,0]]) # ranges: barrel/ec, all layers, all eta, all phi, all column counts,
                                                                                  # all sides, don't auto-match connected rows
    kwargs.setdefault("DefectProbabilities", [[0.,1.e-4]]) # only strip defects
    kwargs.setdefault("DetEleCollKey", "StripDetectorElementCollection")
    kwargs.setdefault("WriteKey", "StripEmulatedDefects")
    kwargs.setdefault("IDName","SCT_ID")
    kwargs.setdefault("HistogramGroupName","") # disable histogramming; enable: e.g. /StripDefects/EmulatedDefects/

    kwargs.setdefault("RngPerDefectType",False) # If True use one RNG per defect type (module, chip-defects, core-column, pixel-defects, corner-defects)
    kwargs.setdefault("DefectsInputFiles",[])   # If not empty read defects from input files and merge defects (root RNTuple/json; extension: .root .json")
    kwargs.setdefault("DefectsOutputFile","")   # If not empty write defects to a  output file (root RNTuple/json; (extension: .root .json")
    if "DefectsOutputFile" in kwargs and kwargs["DefectsOutputFile"] is None :
        kwargs["DefectsOutputFile"]=""

    acc.addCondAlgo(CompFactory.InDet.StripDefectsEmulatorCondAlg(name,**kwargs))
    return acc

def ITkStripDefectsEmulatorCondAlgCfg(flags,
                                      name: str = "ITkStripDefectsEmulatorCondAlg",
                                      **kwargs: dict) -> ComponentAccumulator:
    """
    Schedule conditions algorithm to create emulated ITk strip defect conditions data
    """
    kwargs.setdefault("ModulePatterns", [[-2,2,0,99,-99,99,-99,99,0,9999,0,1,0]]) # ranges: barrel/ec, all layers, all eta, all phi,
                                                                                  # all sides, don't auto-match connected rows
    kwargs.setdefault("DefectProbabilities", [[0.,1.e-4]]) # only strip defects
    kwargs.setdefault("DetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("IDName","SCT_ID")
    kwargs.setdefault("WriteKey", "ITkStripEmulatedDefects")

    kwargs.setdefault("RngPerDefectType",False) # If True use one RNG per defect type (module, chip-defects, core-column, pixel-defects, corner-defects)
    kwargs.setdefault("DefectsInputFiles",[])   # If not empty read defects from input files and merge defects (root RNTuple/json; extension: .root .json")
    kwargs.setdefault("DefectsOutputFile","")   # If not empty write defects to a  output file (root RNTuple/json; (extension: .root .json")
    if "DefectsOutputFile" in kwargs and kwargs["DefectsOutputFile"] is None :
        kwargs["DefectsOutputFile"]=""

    return StripDefectsEmulatorCondAlgCfg(flags,name,**kwargs)


def StripDefectsEmulatorAlgCfg(flags,
                                  name: str = "ITkStripDefectsEmulatorAlg",
                                  **kwargs: dict) -> ComponentAccumulator:
    """
    Schedule algorithm to emulate strip defects by dropping RDOs overlapping with emulated
    defects conditions data.
    """
    acc = ComponentAccumulator()

    if "InputKey" not in kwargs :
        # rename original RDO collection
        acc.merge(StripRDORemappingCfg(flags))
        kwargs.setdefault("InputKey","StripRDOs_ORIG")

    if "EmulatedDefectsKey" not in kwargs :
        # create defects conditions data
        acc.merge( ITkStripDefectsEmulatorCondAlgCfg(flags))
        kwargs.setdefault("EmulatedDefectsKey", "StripEmulatedDefects")
    kwargs.setdefault("OutputKey","StripRDOs")
    kwargs.setdefault("HistogramGroupName","") # disable histogramming, enable e.g. /StripDefects/RejectedRDOs/

    acc.addEventAlgo(CompFactory.InDet.StripDefectsEmulatorAlg(name,**kwargs))
    return acc

def ITkStripDefectsEmulatorAlgCfg(flags,
                                  name: str = "ITkStripDefectsEmulatorAlg",
                                  **kwargs: dict) -> ComponentAccumulator:
    """
    Schedule algorithm to emulate ITk strip defects by dropping RDOs overlapping with emulated
    defects conditions data.
    """
    acc = ComponentAccumulator()
    if "InputKey" not in kwargs :
        # rename original RDO collection
        acc.merge(ITkStripRDORemappingCfg(flags))
        kwargs.setdefault("InputKey","ITkStripRDOs_ORIG")

    if "EmulatedDefectsKey" not in kwargs :
        # create defects conditions data
        acc.merge( ITkStripDefectsEmulatorCondAlgCfg(flags))
        kwargs.setdefault("EmulatedDefectsKey", "ITkStripEmulatedDefects")
    kwargs.setdefault("OutputKey","ITkStripRDOs")

    kwargs.setdefault("HistogramGroupName","") # disable histogramming, enable e.g. /StripDefects/RejectedRDOs/

    acc.addEventAlgo(CompFactory.InDet.StripDefectsEmulatorAlg(name,**kwargs))
    return acc

def ITkStripDefectsEmulatorToDetectorElementStatusCondAlgCfg(flags,
                                                             name: str = "ITkStripDefectsEmulatorToDetectorElementStatusCondAlgCfg",
                                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("EmulatedDefectsKey","ITkStripEmulatedDefects")
    kwargs.setdefault("WriteKey","ITkStripDetectorElementStatusFromEmulatedDefects")
    acc.addCondAlgo(CompFactory.InDet.StripEmulatedDefectsToDetectorElementStatusCondAlg(name,**kwargs))
    return acc


if __name__ == "__main__":
    #
    # Test ITk strip defect emulation
    #
    flags = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.GeoModel.Align.Dynamic = False
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1']

    flags.Detector.GeometryITkStrip = True
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

    from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripDetectorElementStatusCondAlgNoByteStreamErrorsCfg
    acc.merge(ITkStripDetectorElementStatusCondAlgNoByteStreamErrorsCfg(flags))

    # ITk strip defect configuration defined in post include
    from StripDefectsEmulatorPostInclude import emulateITkStripDefects
    emulateITkStripDefects(flags,acc)

    acc.printConfig(withDetails=True, summariseProps=True,printDefaults=True)
    sc = acc.run()

    if sc.isFailure():
        import sys
        sys.exit(1)
