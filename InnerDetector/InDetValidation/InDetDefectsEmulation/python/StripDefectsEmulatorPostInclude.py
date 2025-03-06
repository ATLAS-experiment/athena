# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# can be added as a post include to a Reco_tf
#   --postInclude "StripDefectsEmulatorPostInclude.emulateITkStripDefects"
# or
#  --postExec "from InDetDefectsEmulation.StripDefectsEmulatorPostInclude import emulateITkStripDefects;
#              emulateITkStripDefects(flags,cfg,HistogramFileName="itk_strip_defects.root",StripDefectProb=1e-2,ModuleDefectProb=1e-2,NoiseProb=1e-3,PropagateDefectsToStatus=True);"

def emulateITkStripDefects(flags,
                           cfg,
                           StripDefectProb: float=1e-2,
                           ModuleDefectProb: float=1e-2,
                           NoiseProb: float=0.,
                           MaxRandomPositionAttempts: int=10,
                           FillHistogramsPerPattern: bool=True,
                           FillEtaPhiHistogramsPerPattern: bool=True,
                           HistogramGroupName: str="ITkStripDefects",
                           HistogramFileName: str=None,
                           PropagateDefectsToStatus=True) :
    """
    Schedule algorithms to create ITk strip defect conditions data and to emulate
    defects by dropping ITk strip RDOs overlapping with such defects. ModuleDefectProb controls the number of
    defect strip modules and DefectProb the number of defect strips. If a strip group is overlapping with
    a single strip defect, the strip group will be split.
    """

    from InDetDefectsEmulation.StripDefectsEmulatorConfig import (
        ITkStripDefectsEmulatorCondAlgCfg,
        ITkStripDefectsEmulatorAlgCfg,
        ITkStripDefectsEmulatorToDetectorElementStatusCondAlgCfg,
        DefectsHistSvcCfg,
        moduleDefect,
        combineModuleDefects
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    # dummy "fractions" for strips, module pattern are just examples, and currently all
    # modules get the same defect probabilities irrespectively of e.g. strip length.
    module_pattern, module_defect_prob, fractions, ignore_NoiseProbability,ignore_NoiseShape = combineModuleDefects([
                                         moduleDefect(bec=[-2,-2],layer=[0,99], phi_range=[-99,99],eta_range=[-99,99], # select all endcap A modules
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb   # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[-1,1],layer=[0,1], phi_range=[-99,99],eta_range=[-99,99], # select all barrel, layer 0,1 modules
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb   # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[-1,1],layer=[2,99], phi_range=[-99,99],eta_range=[-99,99], # select all other barrel layers
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb   # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[2,2],layer=[0,99], phi_range=[-99,99],eta_range=[-99,99], # select all endcap C modules
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb   # probability of a strip to be defect
                                                                   ])])

    if NoiseProb > 0. :
        # strip noise
        import math
        def norm(dist) :
            scale = sum(dist)
            return [elm/scale if scale > 0. else 0. for elm in dist]
        def gaussDist(mean, width, n) :
            scale = -1./(2*width)
            return norm([ math.exp( scale *math.pow(i - mean,2.) ) for i in range(0,n) ])
        # Gaussian distribution between 1 and 2
        time_bin_dist = gaussDist(1.51,0.03,3)

        # different noise per module of different strip length
        defects=[]
        for length in [15093,18093,18981,23981,24092,24180,26223,27093,28980,30807,30808,31981,32223,40161,48360,54580,54581,60162] :
            # assume noise scales with strip length but there is a constant noise
            defects += [   moduleDefect(column_or_strip_length=[length-1,length+1],
                                        probability=[],
                                        noiseProbability=NoiseProb * 0.5*( 1 + length / 37627.5 ),
                                        noiseShape=time_bin_dist)]
            module_pattern_noise, ignore_module_defect_prob, ignore_fractions, NoiseProbability,NoiseShape = combineModuleDefects(defects)
    else :
        module_pattern_noise=[]
        NoiseProbability=[]
        NoiseShape=[]

    # schedule custom defects generating conditions alg
    cfg.merge( ITkStripDefectsEmulatorCondAlgCfg(flags,
                                                 # to enable histogramming:
                                                 HistogramGroupName=f"/{HistogramGroupName}/StripEmulatedDefects/" if HistogramGroupName is not None else "",
                                                 MaxRandomPositionAttempts=MaxRandomPositionAttempts,
                                                 ModulePatterns=module_pattern,
                                                 DefectProbabilities=module_defect_prob,
                                                 FillHistogramsPerPattern=FillHistogramsPerPattern,
                                                 FillEtaPhiHistogramsPerPattern=FillEtaPhiHistogramsPerPattern,
                                                 WriteKey="ITkStripEmulatedDefects", # the default should match the key below
                                                 ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( ITkStripDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="ITkStripEmulatedDefects",
                                             ModulePatterns=module_pattern_noise,
                                             NoiseProbability = NoiseProbability,
                                             NoiseShape = NoiseShape,
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/StripRejectedRDOs/" if HistogramGroupName is not None else "",
                                             OutputLevel=INFO))

    if PropagateDefectsToStatus :
        # propagate defects to detector element status
        cfg.merge(ITkStripDefectsEmulatorToDetectorElementStatusCondAlgCfg(flags,
                                                                           name="ITkStripDefectsEmulatorToDetectorElementStatusCondAlg",
                                                                           EmulatedDefectsKey="ITkStripEmulatedDefects",
                                                                           WriteKey="ITkStripDetectorElementStatusFromEmulatedDefects"))
        strip_det_el_status_cond_alg=cfg.getCondAlgo("ITkStripDetectorElementStatusCondAlgNoByteStreamErrors")
        strip_det_el_status_cond_alg.ConditionsSummaryTool.SCTDetElStatusCondDataBaseKey="ITkStripDetectorElementStatusFromEmulatedDefects"
