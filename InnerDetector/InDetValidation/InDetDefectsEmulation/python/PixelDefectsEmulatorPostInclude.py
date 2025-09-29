# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# can be added as a post include to a Reco_tf
#   --postInclude "PixelDefectsEmulatorPostInclude.emulateITkPixelDefectsDefault"
# or (with noise 1e-5)
#  --postExec "from InDetDefectsEmulation.PixelDefectsEmulatorPostInclude import emulateITkPixelDefectsDefault;
#              emulateITkPixelDefectsDefault(flags,cfg,HistogramFileName='itk_pixel_defects.root',FrontEndCCDefectProb=1e-1,PixelDefectProb=1e-2,ModuleDefectProb=1e-2,CornerDefectProb=15e-2,NoiseProb=1e-5,PropagateDefectsToStatus=True);"
#
# NOTES: - Will only work with the athena MT scheduler e.g. --threads=n with n>=1 otherwise the algorithms will not be scheduled in the
#          correct order by this post include
#        - accidentally using strip defect input files for pixel may silently "work"
#
# Extra arguments:
#   disable histogramming:  HistogramFileName=None
#   decorrelate random numbers: RngPerDefectType=True will use one random generator per defect type, however some correlation is still
#                                                     present because for example pixel defects are not created for dead modules.
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
#                                                                with mostly single pixel or CC defects

import math
from InDetDefectsEmulation.StripDefectsEmulatorConfig import (moduleDefect,
                                                              combineModuleDefects,
                                                              makeCornerDefectParam
                                                              )

def emulateITkPixelDefects(flags,
                           cfg,
                           ModulePatterns=[[-2,2,0,99,-99,99,-99,99,0,9999,0,1,0]],
                           DefectProbabilities=[[0.,1e-2,1e-1,0.]],
                           NDefectFractionsPerPattern=[[1.,-1, 1.]],
                           NoiseProbability=[],
                           NoiseShape=[],
                           CornerDefectParamsPerPattern=[],
                           NCornerDefectFractionsPerPattern=[],
                           RngPerDefectType=False,
                           DefectsInputFiles=[],
                           DefectsOutputFile=None,
                           FillHistogramsPerPattern=False,
                           FillEtaPhiHistogramsPerPattern=False,
                           MaxRandomPositionAttempts: int=10,
                           HistogramGroupName: str="ITkPixelDefects",
                           HistogramFileName: str="itk_pixel_defects_opt1.root",
                           PropagateDefectsToStatus=True) :
    """
    Schedule conditions algorithm for emulated ITk pixel defects, and algorithms to drop RDOs overlapping with the emulated defects.
    ModulePattern: criteria to match modules, which are lists of n-tuples where every two numbers of each n-tuple define a range
                   to match parts of module identifiers (barrel/ec, layer/disk, eta., phi, columns, side) plus a flag (last
                   element of each n-tuple), which is unused for pixel.
    DefectProbabilities: list of n-tuples per criterion containing 4 probabilities: module, pixel to be defect, a module
                         to have at least one core column defect, circuit defect.
    NDefectFractionsPerPattern: Fractions of exactly n-group defects (core-column, circuit) under condition that there is at least one
                                such group defect. There must be one n-tuple with fractions per criterion, and each fraction
                                n-tuple must contain for each group defect probability a non empty set of fractions of exactly 1, ... n defects.
                                Fractions for different group defects are separated by -1. There should be two sequences of
                                positive fractions separated by -1.
    NoiseProbability: Empty or probabilities of a pixel to produce a spurious hit.
    NoiseShape: binned PDF (i.e. list of fractions) to create tot values for spurious hits.
    CornerDefectParamsPerPattern: parameters for corner defects per pattern can be created with e.g. makeCornerDefectParam
    NCornerDefectFractionsPerPattern: fractions of 1,..4 corner defects
    FillHistogramsPerPattern: if True histograms are filled per module pattern
    FillEtaPhiHistogramsPerPattern: if True also fill xy, and rz histograms of defects per module.
    HistogramGroupName: None (disables histogramming) or the histogram group name must be unique
    HistogramFileName: None (disables histogramming) or a file name for writing the histograms.
    """

    from InDetDefectsEmulation.PixelDefectsEmulatorConfig import (
        ITkPixelDefectsEmulatorCondAlgCfg,
        ITkPixelDefectsEmulatorAlgCfg,
        ITkPixelDefectsEmulatorToDetectorElementStatusCondAlgCfg,
        DefectsHistSvcCfg
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    # schedule custom defects generating conditions alg
    cfg.merge( ITkPixelDefectsEmulatorCondAlgCfg(flags,
                                                 # to enable histogramming:
                                                 HistogramGroupName=f"/{HistogramGroupName}/EmulatedDefects/" if HistogramGroupName is not None else "",
                                                 ModulePatterns = ModulePatterns,
                                                 DefectProbabilities = DefectProbabilities,
                                                 NDefectFractionsPerPattern = NDefectFractionsPerPattern,
                                                 MaxRandomPositionAttempts=MaxRandomPositionAttempts,
                                                 CornerDefectParamsPerPattern=CornerDefectParamsPerPattern,
                                                 NCornerDefectFractionsPerPattern=NCornerDefectFractionsPerPattern,
                                                 RngPerDefectType=RngPerDefectType,
                                                 DefectsInputFiles=DefectsInputFiles,
                                                 DefectsOutputFile=DefectsOutputFile,
                                                 FillHistogramsPerPattern=FillHistogramsPerPattern,
                                                 FillEtaPhiHistogramsPerPattern=FillEtaPhiHistogramsPerPattern,
                                                 CheckerBoardDefects=False,
                                                 OddColToggle=False,
                                                 OddRowToggle=False,
                                                 WriteKey="ITkPixelEmulatedDefects", # the default should match the key below
                                                 ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( ITkPixelDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="ITkPixelEmulatedDefects",
                                             ModulePatterns = ModulePatterns,
                                             NoiseProbability = NoiseProbability,
                                             NoiseShape = NoiseShape,
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/RejectedRDOs/" if HistogramGroupName is not None else "",
                                             OutputLevel=INFO))

    if PropagateDefectsToStatus :
        # propagate defects to detector element status
        cfg.merge(ITkPixelDefectsEmulatorToDetectorElementStatusCondAlgCfg(flags,
                                                                           name="ITkPixelDefectsEmulatorToDetectorElementStatusCondAlg",
                                                                           EmulatedDefectsKey="ITkPixelEmulatedDefects",
                                                                           WriteKey="ITkPixelDetectorElementStatusFromEmulatedDefects"))
        pixel_det_el_status_cond_alg=cfg.getCondAlgo("ITkPixelDetectorElementStatusCondAlgNoByteStreamErrors")
        pixel_det_el_status_cond_alg.ConditionsSummaryTool.PixelDetElStatusCondDataBaseKey="ITkPixelDetectorElementStatusFromEmulatedDefects"

def emulatePixelDefects(flags,
                        cfg,
                        ModulePatterns=None,
                        DefectProbabilities=None,
                        NDefectFractionsPerPattern=None,
                        RngPerDefectType=False,
                        DefectsInputFiles=[],
                        DefectsOutputFile=None,
                        FillHistogramsPerPattern=False,
                        FillEtaPhiHistogramsPerPattern=False,
                        HistogramGroupName: str="PixelDefects",
                        HistogramFileName: str="pixel_defects.root") :
    """
    Schedule algorithms to emulate run3 pixel defects, and algorithm to drop RDOs which overlap with these defects
    """
    from InDetDefectsEmulation.PixelDefectsEmulatorConfig import (
        PixelDefectsEmulatorCondAlgCfg,
        PixelDefectsEmulatorAlgCfg,
        DefectsHistSvcCfg
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    if ModulePatterns is None and DefectProbabilities is None and NDefectFractionsPerPattern is None:
        a_module_pattern_list, a_prob_list,fractions,ignore_NoiseProbability,ignore_NoiseShape, cornerDefectParam, cornerDefectFractions = combineModuleDefects([
                                         moduleDefect(bec=[-2,-2],layer=[0,99], phi_range=[-99,99],eta_range=[-99,99], # select all modules
                                                      columns_or_strips=[0,9999], # sensors with all kind of numbers of columns
                                                      side_range=[0,0],     # there is only a single side
                                                      all_rows=False,       # there is no connection between modules
                                                      probability=[1e-2,    # probability of a module to be defect
                                                                   1e-2,    # probability of a pixel to be defect
                                                                   0.,      # probability of a module to have at least one core-column defect
                                                                   0.       # probability of a module to have at least one defect circuit
                                                                   ])
                                                                   ])
        ModulePatterns =  a_module_pattern_list
        DefectProbabilities =  a_prob_list
        NDefectFractionsPerPattern = fractions

    # schedule custom defects generating conditions alg
    cfg.merge( PixelDefectsEmulatorCondAlgCfg(flags,
                                              ModulePatterns = ModulePatterns,
                                              DefectProbabilities = DefectProbabilities,
                                              NDefectFractionsPerPattern = NDefectFractionsPerPattern,
                                              CornerDefectParamsPerPattern=cornerDefectParam,
                                              NCornerDefectFractionsPerPattern=cornerDefectFractions,
                                              RngPerDefectType=RngPerDefectType,
                                              DefectsInputFiles=DefectsInputFiles,
                                              DefectsOutputFile=DefectsOutputFile,
                                              FillHistogramsPerPattern=FillHistogramsPerPattern,
                                              FillEtaPhiHistogramsPerPattern=FillEtaPhiHistogramsPerPattern,
                                              # to enable histogramming:
                                              HistogramGroupName=f"/{HistogramGroupName}/EmulatedDefects/" if HistogramGroupName is not None else "",
                                              WriteKey="PixelEmulatedDefects", # the default should match the key below
                                              ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( PixelDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="PixelEmulatedDefects",
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/RejectedRDOs/" if HistogramGroupName is not None else "",
                                             # (prefix "/PixelDefects/" is assumed by THistSvc config)
                                             OutputLevel=INFO))


def poissonFractions(cc_defect_prob=1e-1, max_n=5) :
    """
    Create fractions for exactly 1..max_n defects, under the condition that
    the probability for at least one such defects is cc_defect_prob, and
    assuming that the fractions are Poisson distributed.
    """
    if cc_defect_prob > 0. :
        def PoissonProb(expected, n) :
            return math.pow(expected,n)*math.exp(-expected)/math.gamma(n+1)
        def norm(fractions) :
            Norm = 1./sum (fractions)
            return [Norm*elm for elm in fractions ]
        expectation=-math.log(1-cc_defect_prob)
        return norm([ PoissonProb(expectation,i) for i in range(1,max_n+1) ])
    else :
        return [1.]

def quadProb(circuit_prob) :
    """
    Compute the defect probability for a quad module from the probabilities
    of a single chip module, assuming that the defect is a "feature" of the
    chip.
    """
    return 1-math.pow(1-circuit_prob,4) # not (no defect core column on none of the sensors)

def fractionsForExactlyNCoreColumnDefects(ExactlyNCoreColumnDefects=1) :
    """
    Create fraction n-tuple, containing zero except for the element corresponding
    to exactly n defects, which is set to 1.
    """
    return [ 0. if idx != ExactlyNCoreColumnDefects else 1. for idx in range(1,ExactlyNCoreColumnDefects+1) ]

def makeITkDefectsParams( quad_cc_defect_prob,
                          quad_fractions,
                          circuit_cc_defect_prob,
                          circuit_fractions,
                          pixel_defect_prob=1e-2,
                          module_defect_prob=0.,
                          cornerDefectProb=0.,
                          noiseProbability=None,
                          noiseShape=[]) :
    """
    Create different defects for quads and single chip modules, where the corresponding modules are selected by
    the number of offline columns
    """
    # corner defects:
    # - 50% of ~30% of the 3D modules have disconnected corners area 2-4mm x 2-2.5mm,
    # - roughly circular shape with a sagitta (defined by circle crossing sensor edges) of ~0-2mm,
    # - impact on all 3D sensors which are all single chip modules i.e. modules with 384 or 400 columns
    # Update: after defect mitigation, new measurement of remaining defects: 4.1mm x 1.3 sagitta 0.44 (representative?)
    corner_defects=makeCornerDefectParam(probability=cornerDefectProb,
                                         min_rx=2.,max_rx=5.,
                                         min_ry=0.5,max_ry=1.5,
                                         min_sagitta=0.2,max_sagitta=0.5)
    # assume each corner has equal probability to suffer corner defects
    # Assume number of corners with defects Poisson distributed
    # According to the information on JIRA https://its.cern.ch/jira/browse/ATLITKSW-302 only bottom of
    # sensors suffers such defects (currently not taken into account
    corner_defect_n_defect_corners=poissonFractions(cc_defect_prob=15e-2, max_n=4)

    return combineModuleDefects([
            moduleDefect(bec=[-2,2],layer=[0,99], phi_range=[-99,99],eta_range=[-99,99], # select all modules
                         columns_or_strips=[800,800], # but only quads
                         side_range=[0,0],     # there is only a single side
                         all_rows=False,       # there is no connection between modules
                         probability=[module_defect_prob,      # probability of a module to be defect
                                      pixel_defect_prob,    # probability of a pixel to be defect
                                      quad_cc_defect_prob, # probability of a module to have at least one core-column defect
                                      0.       # probability of a module to have at least one defect circuit
                                      ],
                         fractionsOfNDefects=[quad_fractions,[1.]], # dummy fractions for circuit defects
                         noiseProbability=noiseProbability,
                         noiseShape=noiseShape,
                         cornerDefectParam=[], # need empty lists here
                         cornerDefectNCornerFractions=[] # and here, otherwise resulting property will be non-conform,
                         ),
            moduleDefect(bec=[-2,2],layer=[0,1], phi_range=[-99,99],eta_range=[-99,99], # select all modules
                         columns_or_strips=[384,384], # but only ring triplet modules
                         side_range=[0,0],     # there is only a single side
                         all_rows=False,       # there is no connection between modules
                         probability=[module_defect_prob,      # probability of a module to be defect
                                      pixel_defect_prob,    # probability of a pixel to be defect
                                      circuit_cc_defect_prob,    # probability of a module to have at least one core-column defect
                                      0.       # probability of a module to have at least one defect circuit
                                      ],
                         fractionsOfNDefects=[circuit_fractions,[1.]], # dummy fractions for circuit defects
                         noiseProbability=noiseProbability,
                         noiseShape=noiseShape,
                         cornerDefectParam=corner_defects,
                         cornerDefectNCornerFractions=corner_defect_n_defect_corners,
                         ),
            moduleDefect(bec=[-2,2],layer=[0,1], phi_range=[-99,99],eta_range=[-99,99], # select all modules
                         columns_or_strips=[200,200], # but only barrel triplet modules
                         side_range=[0,0],     # there is only a single side
                         all_rows=False,       # there is no connection between modules
                         probability=[module_defect_prob,      # probability of a module to be defect
                                      pixel_defect_prob,    # probability of a pixel to be defect
                                      circuit_cc_defect_prob,    # probability of a module to have at least one core-column defect
                                      0.       # probability of a module to have at least one defect circuit
                                    ],
                         fractionsOfNDefects=[circuit_fractions,[1.]], # dummy fractions for circuit defects
                         noiseProbability=noiseProbability,
                         noiseShape=noiseShape,
                         cornerDefectParam=corner_defects,
                         cornerDefectNCornerFractions=corner_defect_n_defect_corners,
                         )
        ])

def makeITkPixelNoise(noiseProb=0.) :
    """
    For NoiseProb>0. create phantasie noise tot shape, with mean at zero width of 0.7
    truncated below a tot of 1.
    """
    if noiseProb > 0. :
        import math
        def norm(dist) :
            scale = sum(dist)
            return [elm/scale if scale > 0. else 0. for elm in dist]
        def gaussDist(mean, width, n) :
            scale = -1./(2*width)
            return norm([ math.exp( scale *math.pow(i - mean,2.) ) for i in range(0,n) ])

        # Guassian tot distribution with mean of 0 but tot >=1
        noiseShape=[ 0. ] + gaussDist(-1,0.7,4)
    else :
        noiseProb=None
        noiseShape=[]
    return noiseProb,noiseShape


def emulateITkPixelDefectsOneCC(flags,
                                cfg,
                                FrontEndCCDefectProb=0.1,
                                PixelDefectProb=1e-2,
                                NumberOfCCDefects=1,
                                ModuleDefectProb=0.,
                                CornerDefectProb=0.,
                                NoiseProb=0.,
                                RngPerDefectType=False,
                                DefectsInputFiles=[],
                                DefectsOutputFile=None,
                                PropagateDefectsToStatus=True,
                                HistogramFileName=None) :
    """
    Create exactly one core column defect per module where the probability is given
    by module_prob, and create single pixel defects according to pixel_defect_prob
    """
    noiseProbabilitySingle,noiseShapeSingle = makeITkPixelNoise(NoiseProb)
    fractions=fractionsForExactlyNCoreColumnDefects(NumberOfCCDefects)
    ModulePatterns, DefectProbabilities, NDefectFractionsPerPattern, NoiseProbability, NoiseShape,cornerDefectParam,cornerDefectFractions = makeITkDefectsParams(
        quad_cc_defect_prob=quadProb(FrontEndCCDefectProb),
        quad_fractions=fractions,    # same fractions for quads and other modules
        circuit_cc_defect_prob=FrontEndCCDefectProb,
        circuit_fractions=fractions,
        pixel_defect_prob=PixelDefectProb,
        module_defect_prob=ModuleDefectProb,
        cornerDefectProb=CornerDefectProb,
        noiseProbability=noiseProbabilitySingle,
        noiseShape=noiseShapeSingle)

    return emulateITkPixelDefects(flags,
                                  cfg,
                                  ModulePatterns=ModulePatterns,
                                  DefectProbabilities=DefectProbabilities,
                                  NDefectFractionsPerPattern=NDefectFractionsPerPattern,
                                  NoiseProbability=NoiseProbability,
                                  NoiseShape=NoiseShape,
                                  CornerDefectParamsPerPattern=cornerDefectParam,
                                  NCornerDefectFractionsPerPattern=cornerDefectFractions,
                                  RngPerDefectType=RngPerDefectType,
                                  DefectsInputFiles=DefectsInputFiles,
                                  DefectsOutputFile=DefectsOutputFile,
                                  FillHistogramsPerPattern=True,
                                  FillEtaPhiHistogramsPerPattern=True,
                                  PropagateDefectsToStatus=PropagateDefectsToStatus,
                                  HistogramFileName=HistogramFileName)


def emulateITkPixelDefectsPoisson(flags,
                                  cfg,
                                  FrontEndCCDefectProb=0.1,
                                  PixelDefectProb=1e-2,
                                  ModuleDefectProb=0.,
                                  CornerDefectProb=0.,
                                  NoiseProb=0.,
                                  RngPerDefectType=False,
                                  DefectsInputFiles=[],
                                  DefectsOutputFile=None,
                                  PropagateDefectsToStatus=True,
                                  HistogramFileName=None) :
    """
    Create Poisson distributed  core column defects per module where the probability for at least one
    core column defect is given by front_end_cc_defect_prob for single chip modules and larger for
    quads, respectively. Single pixel defects are controlled by pixel_defect_prob
    """
    noiseProbabilitySingle,noiseShapeSingle = makeITkPixelNoise(NoiseProb)
    quad_prob=quadProb(FrontEndCCDefectProb)
    ModulePatterns, DefectProbabilities, NDefectFractionsPerPattern, NoiseProbability, NoiseShape,cornerDefectParam,cornerDefectFractions = makeITkDefectsParams(
        quad_cc_defect_prob=quad_prob,
        quad_fractions=poissonFractions(quad_prob),
        circuit_cc_defect_prob=FrontEndCCDefectProb,
        circuit_fractions=poissonFractions(FrontEndCCDefectProb),
        pixel_defect_prob=PixelDefectProb,
        module_defect_prob=ModuleDefectProb,
        cornerDefectProb=CornerDefectProb,
        noiseProbability=noiseProbabilitySingle,
        noiseShape=noiseShapeSingle)

    return emulateITkPixelDefects(flags,
                                  cfg,
                                  ModulePatterns=ModulePatterns,
                                  DefectProbabilities=DefectProbabilities,
                                  NDefectFractionsPerPattern=NDefectFractionsPerPattern,
                                  NoiseProbability=NoiseProbability,
                                  NoiseShape=NoiseShape,
                                  CornerDefectParamsPerPattern=cornerDefectParam,
                                  NCornerDefectFractionsPerPattern=cornerDefectFractions,
                                  RngPerDefectType=RngPerDefectType,
                                  DefectsInputFiles=DefectsInputFiles,
                                  DefectsOutputFile=DefectsOutputFile,
                                  FillHistogramsPerPattern=True,
                                  FillEtaPhiHistogramsPerPattern=True,
                                  PropagateDefectsToStatus=PropagateDefectsToStatus,
                                  HistogramFileName=HistogramFileName)

def emulateITkPixelDefectsDefault(flags,
                                  cfg,
                                  FrontEndCCDefectProb=0.1,
                                  PixelDefectProb=1e-2,
                                  ModuleDefectProb=1e-2,
                                  CornerDefectProb=15e-2,
                                  NoiseProb=0.,
                                  RngPerDefectType=False,
                                  DefectsInputFiles=[],
                                  DefectsOutputFile=None,
                                  PropagateDefectsToStatus=True,
                                  HistogramFileName=None) :
    """
    Enables all defects according to the current expectation.
    """
    emulateITkPixelDefectsPoisson(flags,cfg,
                                  FrontEndCCDefectProb,
                                  PixelDefectProb,
                                  ModuleDefectProb,
                                  CornerDefectProb,
                                  NoiseProb,
                                  RngPerDefectType=RngPerDefectType,
                                  DefectsInputFiles=DefectsInputFiles,
                                  DefectsOutputFile=DefectsOutputFile,
                                  PropagateDefectsToStatus=PropagateDefectsToStatus,
                                  HistogramFileName=HistogramFileName)
