# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# can be added as a post include to a Reco_tf
#   --postInclude "PixelDefectsEmulatorPostInclude.emulateITkPixelDefects"
# or
#  --postExec "from InDetDefectsEmulation.PixelDefectsEmulatorPostInclude import emulateITkPixelDefects;
#              emulateITkPixelDefects(flags,cfg,DefectProbability=1e-3,CoreColumnDefectProbability=0.01);"
import math

def emulateITkPixelDefects(flags,
                           cfg,
                           DefectProbability: float=1e-3,
                           MatrixColumns: list=[800,384,200],
                           ProbabilityOfModuleWithCoreColumnDefects: list=[.05,.05,.05],
                           CoreColumnDefectProbability: list=[1.,-1, 1.,-1, 1.,-1],
                           MaxRandomPositionAttempts: int=10,
                           HistogramGroupName: str="ITkPixelDefects",
                           HistogramFileName: str="itk_pixel_defects_opt1.root") :
    """
    MatrixColumns : list which contains the number of columns of the pixel modules for which probabilities are given
    ProbabilityOfModuleWithCoreColumnDefects: list of probabilities of a module of the coresponding pixel module to have
                                              at least one core column defect
    CoreColumnDefectProbability: list which contains a series of conditional defect probabilities per pixel module, where the last number
                                 of the series must be -1. (end-marker). The numbers mean the conditional probabilities of a module to have
                                 exactly 1, 2, 3 ... n core column defects under the condition that the module has at least one core column
                                 defect. Thses numbers should add up to one.
    The default argeument will configure exactly 1 core column defect per pixel module, for about 5% of the pixel modules which have
    800 (quad), 384 (ring triplet module), 200 (barrel tripplet module) offline columns.
    """

    from InDetDefectsEmulation.PixelDefectsEmulatorConfig import (
        ITkPixelDefectsEmulatorCondAlgCfg,
        ITkPixelDefectsEmulatorAlgCfg,
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
                                                 MatrixColumns=MatrixColumns,
                                                 ProbabilityOfModuleWithCoreColumnDefects=ProbabilityOfModuleWithCoreColumnDefects,
                                                 CoreColumnDefectProbability=CoreColumnDefectProbability,
                                                 DefectProbability=DefectProbability,
                                                 MaxRandomPositionAttempts=MaxRandomPositionAttempts,
                                                 CheckerBoardDefects=False,
                                                 OddColToggle=False,
                                                 OddRowToggle=False,
                                                 WriteKey="ITkPixelEmulatedDefects", # the default should match the key below
                                                 ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( ITkPixelDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="ITkPixelEmulatedDefects",
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/RejectedRDOs/" if HistogramGroupName is not None else "",
                                             # (prefix "/PixelDefects/" is assumed by THistSvc config)
                                             OutputLevel=INFO))


def emulatePixelDefects(flags,
                        cfg,
                        DefectProbability: float=1e-4,
                        HistogramGroupName: str="PixelDefects",
                        HistogramFileName: str="pixel_defects.root") :
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

    # schedule custom defects generating conditions alg
    cfg.merge( PixelDefectsEmulatorCondAlgCfg(flags,
                                              # to enable histogramming:
                                              HistogramGroupName=f"/{HistogramGroupName}/EmulatedDefects/" if HistogramGroupName is not None else "",
                                              MatrixColumns=[],  # no core column defects for run3 pixel
                                              ProbabilityOfModuleWithCoreColumnDefects=[],
                                              CoreColumnDefectProbability=[],
                                              DefectProbability=DefectProbability,
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



def poissonDefects(prob_per_frontend=0.05) :
    MatrixColumns=[]
    ProbabilityOfModuleWithCoreColumnDefects=[]
    CoreColumnDefectProbability=[]

    def PoissonProb(expected, n) :
        return math.pow(expected,n)*math.exp(-expected)/math.gamma(n+1)

    # 768x800:
    MatrixColumns+=[800]
    # no defect for quad : 1-(1-prob)^4
    prob_per_quad = 1-math.pow(1-prob_per_frontend,4)
    ProbabilityOfModuleWithCoreColumnDefects+=[prob_per_quad]
    expectation=-math.log(1-prob_per_quad)
    poisson_prob=[ PoissonProb(expectation,i) for i in range(1,6) ]
    Norm=1./sum(poisson_prob)
    CoreColumnDefectProbability+=[p*Norm for p in poisson_prob]+[-1.]

    # 400x384
    # 25% of these modules have exactly 1 core column defect
    MatrixColumns+=[384]
    ProbabilityOfModuleWithCoreColumnDefects+=[prob_per_frontend]
    expectation=-math.log(1-prob_per_frontend)
    poisson_prob=[ PoissonProb(expectation,i) for i in range(1,6) ]
    Norm=1./sum(poisson_prob)
    CoreColumnDefectProbability+=[p*Norm for p in poisson_prob]+[-1.]

    # 768x200
    MatrixColumns+=[200]
    # 25% of these modules have exactly 2 core column defect
    ProbabilityOfModuleWithCoreColumnDefects+=[prob_per_frontend]
    CoreColumnDefectProbability+=[p*Norm for p in poisson_prob]+[-1.]

    return [MatrixColumns,ProbabilityOfModuleWithCoreColumnDefects,CoreColumnDefectProbability]


def exactlyOneCoreColumnDefect(prob=.05) :
    MatrixColumns=[]
    ProbabilityOfModuleWithCoreColumnDefects=[]
    CoreColumnDefectProbability=[]

    # 768x800:
    MatrixColumns+=[800]
    ProbabilityOfModuleWithCoreColumnDefects+=[prob]
    CoreColumnDefectProbability+=[1.,-1]

    # 400x384
    # 25% of these modules have exactly 1 core column defect
    MatrixColumns+=[384]
    ProbabilityOfModuleWithCoreColumnDefects+=[prob]
    CoreColumnDefectProbability+=[1.,-1]

    # 768x200
    MatrixColumns+=[200]
    # 25% of these modules have exactly 2 core column defect
    ProbabilityOfModuleWithCoreColumnDefects+=[prob]
    CoreColumnDefectProbability+=[1.,-1.]
    return [MatrixColumns,ProbabilityOfModuleWithCoreColumnDefects,CoreColumnDefectProbability]




def emulateITkPixelDefectsOneCC(flags,
                                 cfg,
                                 module_prob=0.05) :
    """
    Create exaactly one core column defect per module where the probability is given
    by module_prob.
    """
    MatrixColumns, ProbabilityOfModuleWithCoreColumnDefects, CoreColumnDefectProbability = exactlyOneCoreColumnDefect(module_prob)

    return emulateITkPixelDefects(flags,
                                 cfg,
                                 DefectProbability=1e-3,
                                 MatrixColumns=MatrixColumns,
                                 ProbabilityOfModuleWithCoreColumnDefects=ProbabilityOfModuleWithCoreColumnDefects,
                                 CoreColumnDefectProbability=CoreColumnDefectProbability,
                                 MaxRandomPositionAttempts=10,
                                 HistogramGroupName="ITkPixelDefects",
                                 HistogramFileName="itk_pixel_defects_1cc05.root")


def emulateITkPixelDefectsPoisson(flags,
                                  cfg,
                                  front_end_defect_prob=0.05) :
    """
    Create Poisson distributed  core column defects per module where the probability for a core column defect
    is given by front_end_defect_prob for single chip modules and larger for quads.
    """
    MatrixColumns, ProbabilityOfModuleWithCoreColumnDefects, CoreColumnDefectProbability = poissonDefects(front_end_defect_prob)

    return emulateITkPixelDefects(flags,
                                  cfg,
                                  DefectProbability=1e-3,
                                  MatrixColumns=MatrixColumns,
                                  ProbabilityOfModuleWithCoreColumnDefects=ProbabilityOfModuleWithCoreColumnDefects,
                                  CoreColumnDefectProbability=CoreColumnDefectProbability,
                                  MaxRandomPositionAttempts=10,
                                  HistogramGroupName="ITkPixelDefects",
                                  HistogramFileName="itk_pixel_defects_poisson05.root")
