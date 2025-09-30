# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# can be added as a post include to a Reco_tf
#   --postInclude "StripDefectsEmulatorPostInclude.emulateITkStripDefectsDefault"
# or (with noise 1e-3)
#  --postExec "from InDetDefectsEmulation.StripDefectsEmulatorPostInclude import emulateITkStripDefectsDefault;
#              emulateITkStripDefectsDefault(flags,cfg,HistogramFileName='itk_strip_defects.root',StripDefectProb=1e-2,ModuleDefectProb=1e-2,NoiseProb=1e-3,PropagateDefectsToStatus=True);"
#
# NOTES: - Will only work with the athena MT scheduler e.g. --threads=n with n>=1 otherwise the algorithms will not be scheduled in the
#          correct order by this post include
#        - accidentally using pixel defect input files for strips may result in an obscure range exception.
#
# Extra arguments:
#   disable histogramming:  HistogramFileName=None
#   decorrelate random numbers: RngPerDefectType=True will use one random generator per defect type, however some correlation is still
#                                                     present because for example strip defects are not created for dead modules.
#  persistify defects conditions data:
#    DefectsOutputFile='defects.root'  Depending on the extension .root or .json, will write all defects into a root RNTuple or json file.
#                                      These files may get big. ".json" is not recommended unless there are only module defects.
#  read defects from input files:
#    DefectsInputFiles=['moduleDefects.root','cellDefects.root'] Read one or multiple defect files and merge defects. The file can be
#                                                                Either a root RNTuple or a .json file, which can be created by the
#                                                                conditions algorithm by specifying the option e.g.
#                                                                DefectsOutputFile='defects.root'
#                                                                Reading defects from input file(s) will not disable the generation of
#                                                                random defects. The latter will be added on top unless the corresponding
#                                                                probabilities are set to zero.
#                                                                For efficiency files should be ordered by the significance of the
#                                                                defects i.e. files with more module defects should precede files
#                                                                with mostly single strip defects
import math

def poissonFractions(expectation=4, max_n=12) :
    """
    Create fractions for exactly 1..max_n defects, assuming a Poisson
    distribution with an expected value of expect_n.
    """
    def PoissonProb(expected, n) :
        return math.pow(expected,n)*math.exp(-expected)/math.gamma(n+1)
    def norm(fractions) :
        Norm = 1./sum (fractions)
        return [Norm*elm for elm in fractions ]
    return norm([ PoissonProb(expectation,i) for i in range(1,max_n+1) ])


def mergeFractions(probA,fractionA, probB, fractionB) :
    if probA <= 0. :
        return fractionB
    if probB <= 0. :
        return fractionA

    if len(fractionA) < len(fractionB) :
        min_n=len(fractionA)
        scaleA = probA/probB
        scaleB = 1.
        fraction=fractionB
    else :
        min_n=len(fractionA)
        scaleB = probA/probB
        scaleA = 1.
        fraction=fractionA
    for idx in range(0,min_n) :
        fraction[idx]=fractionA[idx]*scaleA + fractionB[idx]*scaleB
    norm=1./sum(fraction)
    fraction = [elm*norm for elm in fraction]
    return fraction

def expectationValue(fractions) :
    expectation_value=0
    for idx in range(0,len(fractions)) :
        expectation_value += (idx+1)*fractions[idx]
    return expectation_value


def emulateITkStripDefects(flags,
                           cfg,
                           StripDefectProb: float=1e-2,
                           ModuleDefectProb: float=1e-2,
                           NoiseProb: float=0.,
                           MaxRandomPositionAttempts: int=10,
                           RngPerDefectType=False,
                           DefectsInputFiles=[],
                           DefectsOutputFile=None,
                           coldNoiseDefectProb: float=0.,
                           coldNoiseNDefectFractions=None,
                           coldNoiseDefectsEvenOnly: bool=False,
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
        combineModuleDefects,
        # ODD_INDEX,
        EVEN_INDEX
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    # coldNoiseDefectFractions=poissonFractions(4,16)
    fractions=mergeFractions(StripDefectProb,[1.],coldNoiseDefectProb,coldNoiseNDefectFractions)
    # total number of defects due to cold noise should be steered just by the coldNoiseDefectProb
    # so in case of defect groups reduce this probability according to the defect group size created
    # in average by each generated defect:
    expectation_value = expectationValue(fractions)
    coldNoiseDefectProb/=expectation_value

    cold_noise_phi_range=EVEN_INDEX if coldNoiseDefectsEvenOnly else [-99,99]

    # dummy "fractions" for strips, module pattern are just examples, and currently all
    # modules get the same defect probabilities irrespectively of e.g. strip length.
    if True :
        module_pattern, module_defect_prob, fractions, ignore_NoiseProbability,ignore_NoiseShape, cornerDefectParam, cornerDefectFractions  = combineModuleDefects([
                                         # coldNoise defects show up in petal modules 4 and 5 (endcap eta index 14-17), and only on
                                         # the lower module part which have even index in phi ?
                                         moduleDefect(bec=[-2,-2],layer=[0,99], phi_range=cold_noise_phi_range,eta_range=[14,17],
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb+coldNoiseDefectProb,  # probability of a strip to be defect
                                                                   ],
                                                      fractionsOfNDefects=[fractions]),
                                         moduleDefect(bec=[2,2],layer=[0,99], phi_range=EVEN_INDEX,eta_range=[14,17], # select endcap C4,5 even modules,
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb+coldNoiseDefectProb  # probability of a strip to be defect
                                                                   ],
                                                      fractionsOfNDefects=[fractions]),
                                         moduleDefect(bec=[-2,-2],layer=[0,99], phi_range=[-99,99],eta_range=[14,17], # select endcap A4,5 modules,
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb   # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[2,2],layer=[0,99], phi_range=[-99,99],eta_range=[14,17], # select endcap C4,5 modules,
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   StripDefectProb   # probability of a strip to be defect
                                                                   ]),
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
    NDefectFractionsPerPattern = fractions

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
            module_pattern_noise, ignore_module_defect_prob, ignore_fractions, NoiseProbability,NoiseShape,cornerDefectParam,cornerDefectFractions = combineModuleDefects(defects)
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
                                                 NDefectFractionsPerPattern = NDefectFractionsPerPattern,
                                                 CornerDefectParamsPerPattern=[],
                                                 NCornerDefectFractionsPerPattern=[],
                                                 RngPerDefectType=RngPerDefectType,
                                                 DefectsInputFiles=DefectsInputFiles,
                                                 DefectsOutputFile=DefectsOutputFile,
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

def emulateITkStripDefectsDefault(flags,
                                  cfg,
                                  StripDefectProb: float=1e-2,
                                  ModuleDefectProb: float=1e-2,
                                  NoiseProb: float=0.,
                                  RngPerDefectType=False,
                                  DefectsInputFiles=[],
                                  DefectsOutputFile=None,
                                  HistogramFileName: str=None,
                                  PropagateDefectsToStatus=True) :
            emulateITkStripDefects(flags,cfg,
                               StripDefectProb=StripDefectProb,
                               ModuleDefectProb=ModuleDefectProb,
                               NoiseProb=NoiseProb,
                               RngPerDefectType=RngPerDefectType,
                               DefectsInputFiles=DefectsInputFiles,
                               DefectsOutputFile=DefectsOutputFile,
                               HistogramFileName=HistogramFileName,
                               PropagateDefectsToStatus=PropagateDefectsToStatus)
