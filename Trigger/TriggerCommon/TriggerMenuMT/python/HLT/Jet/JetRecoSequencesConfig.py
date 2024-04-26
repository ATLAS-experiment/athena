#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

""" ComponentAccumulator equivalents for the functions in JetRecoSequences """

from .JetRecoCommon import (
    interpretRecoAlg,
    cloneAndUpdateJetRecoDict,
    defineJets,
    defineGroomedJets,
    defineReclusteredJets,
    getFilterCut,
    getCalibMods,
    getClustersKey,
    getDecorList,
    getHLTPrefix,
    isPFlow,
    doTracking,
    doFSTracking,
    getJetCalibDefaultString,
    jetRecoDictFromString
)
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import parOR
from ..CommonSequences.FullScanDefs import fs_cells
from ..Bjet.BjetFlavourTaggingConfig import fastFlavourTaggingCfg
from .JetTrackingConfig import JetRoITrackingCfg

from JetRecConfig import JetRecConfig
from JetRecConfig import JetInputConfig
from JetRecConfig.DependencyHelper import solveDependencies, solveGroomingDependencies

from JetRecTools import OnlineMon
from JetRec import JetOnlineMon

from EventShapeTools.EventDensityConfig import getEventShapeName

from TrigEDMConfig.TriggerEDM import recordable

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from typing import Final

# Default threshold for filtering jets for input to hypo
JET_DEFAULT_VIEW_PT_MIN_GEV : Final[int] = 10

def formatFilteredJetsName(jetsIn, jetPtMinGeV):
    return f"{jetsIn}_pt{int(jetPtMinGeV)}"

# Prototype data dependency generation for the stages of jet reconstruction
def JetRecoDataDeps(flags, clustersKey, **jetRecoDict):
    jetalg, jetradius, extra = interpretRecoAlg(jetRecoDict["recoAlg"])

    if extra == "r":
        jetOutputDict = ReclusteredJetRecoDataDeps(
            flags, clustersKey, **jetRecoDict
        )
        return jetOutputDict['reclustered']
    elif extra in ["t", "sd"]:
        jetOutputDict = GroomedJetRecoDataDeps(
            flags, clustersKey, **jetRecoDict
        )
        return jetOutputDict['groomed']
    else:
        jetOutputDict = StandardJetRecoDataDeps(
            flags, clustersKey, **jetRecoDict
        )
        return jetOutputDict['calib']


def StandardJetBuildDataDeps(flags, clustersKey, **jetRecoDict):
    use_FS_tracking = doFSTracking(jetRecoDict)
    trkopt = jetRecoDict['trkopt']
    
    is_pflow = isPFlow(jetRecoDict)
    if is_pflow:
        jetDef = defineJets(
            flags,
            jetRecoDict,
            pfoPrefix=f"HLT_{trkopt}",
            prefix=getHLTPrefix(),
        )
    else:
        jetDef = defineJets(
            flags,
            jetRecoDict,
            clustersKey=clustersKey,
            prefix=getHLTPrefix(),
        )
    # Sort and filter
    jetDef.modifiers = [
        "Sort",
        "Filter:{}".format(getFilterCut(jetRecoDict["recoAlg"])),
        "ConstitFourMom_copy",
    ]
    if jetRecoDict["recoAlg"] == "a4":
        jetDef.modifiers += ["CaloEnergies"]  # needed for GSC
        if is_pflow:
            jetDef.modifiers += ["CaloEnergiesClus"] # Needed for FlowElement GSC
    if use_FS_tracking:
        jetDef.modifiers += ["TrackMoments", "JVF", "JVT"]
    
    pj_name = JetRecConfig.getPJContName(jetDef.inputdef)
    if use_FS_tracking:
        pj_name = f"{pj_name}MergedWithGhostTracks"
    jetDef._internalAtt["finalPJContainer"] = pj_name

    jetsOut = recordable(jetDef.fullname())
    jetDef = solveDependencies(jetDef,flags)
    jetDef.lock()

    return {'build': (jetsOut, jetDef)}

def StandardJetRecoDataDeps(flags, clustersKey, **jetRecoDict):
    if jetRecoDict["jetCalib"] == "nojcalib":
        # If we don't calibrate, we need to return the clustered jets with filter
        jetOutputDict = StandardJetBuildDataDeps(
            flags, clustersKey, **jetRecoDict
        )
        jetsNoCalib, jetDef = jetOutputDict['build']
        jetDef.lock()
        jetsNoCalibFiltered = formatFilteredJetsName(jetsNoCalib, jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV)
        # Inelegantly repeat the values because the requested calibration is null
        jetOutputDict.update({'nojcalib': (jetsNoCalibFiltered, jetDef), 'calib': (jetsNoCalibFiltered, jetDef)})
        return jetOutputDict
    else:
        # If we do calibrate, then the process is to copy+calibrate, and return those jets
        jrdNoJCalib = cloneAndUpdateJetRecoDict(
            jetRecoDict,
            jetCalib="nojcalib"
        )
        jetOutputDict = StandardJetBuildDataDeps(
            flags, clustersKey, **jrdNoJCalib
        )
        jetsNoCalib, jetDefNoCalib = jetOutputDict['build']
        jetsViewNoCalib = formatFilteredJetsName(jetsNoCalib, jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV)

        jetDef = jetDefNoCalib.clone()
        jetDef.suffix = jetDefNoCalib.suffix.replace("nojcalib", jetRecoDict["jetCalib"])

        if "sub" in jetRecoDict["jetCalib"]:
            rhoKey = getEventShapeName(jetDef, nameprefix=getHLTPrefix())
        else:
            rhoKey = "auto"

        # If we need JVT rerun the JVT modifier
        use_FS_tracking = doFSTracking(jetRecoDict)
        is_pflow = isPFlow(jetRecoDict)
        
        jetDef.modifiers = getCalibMods(flags, jetRecoDict, rhoKey)
        if use_FS_tracking:
            jetDef.modifiers += ["JVT"]

        if jetRecoDict["recoAlg"] == "a4":
            jetDef.modifiers += ["CaloQuality"]
            
        if not is_pflow and jetRecoDict["recoAlg"] == "a4":
            from TriggerMenuMT.HLT.Jet.JetRecoCommon import cleaningDict
            jetDef.modifiers += [f'Cleaning:{clean_wp}' for _,clean_wp in cleaningDict.items()]
   
        jetDef = solveDependencies(jetDef,flags)
        jetDef.lock()
        jetsOut = formatFilteredJetsName(jetDef.fullname(),jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV)
        jetOutputDict.update({'nojcalib':(jetsViewNoCalib, jetDefNoCalib), 'calib':(jetsOut, jetDef)})
        return jetOutputDict

def ReclusteredJetRecoDataDeps(flags, clustersKey, **jetRecoDict):
    basicJetRecoDict = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        # Standard size for reclustered inputs
        recoAlg = "a4",
    )

    jetOutputDict = StandardJetRecoDataDeps(
        flags, clustersKey, **basicJetRecoDict
    )
    basicJetsName, basicJetDef = jetOutputDict['calib']

    rcJetPtMinGeV = 15 # 15 GeV minimum pt for jets to be reclustered
    rcInputJetsName = formatFilteredJetsName(basicJetDef.fullname(), rcJetPtMinGeV)
    rc_suffix = f"_{jetRecoDict['jetCalib']}" + (f"_{jetRecoDict['trkopt']}" if doTracking(jetRecoDict) else "")

    rcJetDef = defineReclusteredJets(
        jetRecoDict,
        rcInputJetsName,
        basicJetDef.inputdef.label,
        getHLTPrefix(),
        rc_suffix,
    )

    rcConstitPJKey = JetRecConfig.getPJContName(rcJetDef.inputdef, suffix=jetRecoDict['jetDefStr'])
    rcJetDef._internalAtt["finalPJContainer"] = rcConstitPJKey
    rcJetDef.lock()

    rcJetsOut = recordable(rcJetDef.fullname())
    jetOutputDict['reclustered'] = (rcJetsOut, rcJetDef)
    return jetOutputDict


def GroomedJetRecoDataDeps(flags, clustersKey, **jetRecoDict):
    ungroomedJRD = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        # Drop grooming spec
        recoAlg=jetRecoDict["recoAlg"].rstrip("tsd"),
        # No need to calibrate
        jetCalib = "nojcalib",
    )

    jetOutputDict = StandardJetBuildDataDeps(
        flags,
        clustersKey,
        **ungroomedJRD,
    )
    ungroomedJetsName, ungroomedDef = jetOutputDict['build']

    groomDef = defineGroomedJets(jetRecoDict, ungroomedDef)
    groomedJetsName = recordable(groomDef.fullname())
    groomDef.modifiers = getCalibMods(flags,jetRecoDict)
    groomDef.modifiers += [
        "Sort",
        "Filter:{}".format(getFilterCut(jetRecoDict["recoAlg"])),
    ]
    groomDef = solveGroomingDependencies(groomDef, flags)
    groomDef.lock()

    return {'ungroomed':(ungroomedJetsName, ungroomedDef), 'groomed':(groomedJetsName, groomDef)}


@AccumulatorCache
def JetRecoCfg(flags, clustersKey, **jetRecoDict):
    """The top-level sequence

    Forwards arguments to the standard jet reco, grooming or reclustering sequences.
    """

    jetalg, jetradius, extra = interpretRecoAlg(jetRecoDict["recoAlg"])

    if extra == "r":
        return ReclusteredJetRecoCfg(
            flags, clustersKey, **jetRecoDict
        )
    elif extra in ["t", "sd"]:
        return GroomedJetRecoCfg(
            flags, clustersKey, **jetRecoDict
        )
    else:
        return StandardJetRecoCfg(
            flags, clustersKey, **jetRecoDict
        )


# Get a configured JetViewAlg that creates a VIEW_ELEMENTS container of jets above a minimum jet pT
# Filtered jets are given to hypo.
# jetPtMinGeV is minimum jet pt in GeV for jets to be seen by hypo
@AccumulatorCache
def JetViewAlgCfg(flags,jetsIn,jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV,**jetRecoDict):

    decorList = getDecorList(jetRecoDict)
    filteredJetsName = f"{jetsIn}_pt{int(jetPtMinGeV)}"
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.JetViewAlg(
            "jetview_"+filteredJetsName,
            InputContainer=jetsIn,
            OutputContainer=filteredJetsName,
            PtMin=jetPtMinGeV*1e3, #MeV
            DecorDeps=decorList
        )
    )
    jetsOut = filteredJetsName

    return acc, jetsOut


@AccumulatorCache
def StandardJetBuildCfg(flags, clustersKey, **jetRecoDict):
    """ Standard jet reconstruction, no reclustering or grooming 
    
    The clusters (and tracks, if necessary) should be built beforehand and passed into this config,
    but this config will build the PFOs if they are needed.

    The configuration then builds the jet definition, applies any required constituent modifiers
    and creates the JetRecAlg
    """

    seqname = "JetBuildSeq_"+jetRecoDict['jetDefStr']
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)
    use_FS_tracking = doFSTracking(jetRecoDict)

    trkopt = jetRecoDict['trkopt']
    context = flags.Jet.Context[trkopt]
    
    is_pflow = isPFlow(jetRecoDict)

    # Add PFlow reconstruction if necessary
    if is_pflow:
        from eflowRec.PFHLTConfig import PFCfg

        acc.merge(
            PFCfg(
                flags,
                trkopt,
                clustersin=clustersKey,
                calclustersin="",
                tracksin=context["Tracks"],
                verticesin=context["Vertices"],
                cellsin=fs_cells,
            ),
            seqname
        )
        jetDef = defineJets(
            flags,
            jetRecoDict,
            pfoPrefix=f"HLT_{trkopt}",
            prefix=getHLTPrefix(),
        )
    else:
        jetDef = defineJets(
            flags,
            jetRecoDict,
            clustersKey=clustersKey,
            prefix=getHLTPrefix(),
        )

    # Sort and filter
    jetDef.modifiers = [
        "Sort",
        "Filter:{}".format(getFilterCut(jetRecoDict["recoAlg"])),
        "ConstitFourMom_copy",
    ]
    if jetRecoDict["recoAlg"] == "a4":
        jetDef.modifiers += ["CaloEnergies"]  # needed for GSC
        if isPFlow(jetRecoDict):
            jetDef.modifiers += ["CaloEnergiesClus"] # Needed for FlowElement GSC
    if use_FS_tracking:
        jetDef.modifiers += ["TrackMoments", "JVF", "JVT"]
        
    jetsOut = recordable(jetDef.fullname())
    jetDef = solveDependencies(jetDef,flags)

    if not (
        jetRecoDict["constitMod"] == ""
        and jetRecoDict["constitType"] == "tc"
        and jetRecoDict["clusterCalib"] == "lcw"
    ):
        alg = JetRecConfig.getConstitModAlg(
            jetDef, jetDef.inputdef,
            monTool=OnlineMon.getMonTool_Algorithm(flags, f"HLTJets/{jetsOut}/"),
        )
        # getConstitModAlg will return None if there's nothing for it to do
        if alg is not None:
            acc.addEventAlgo(alg,seqname)

    pj_alg = JetRecConfig.getConstitPJGAlg(jetDef.inputdef)
    pj_name = pj_alg.OutputContainer.Path
    acc.addEventAlgo(pj_alg,seqname)

    if use_FS_tracking:

        # Make sure that the jets are constructed with the ghost tracks included
        merge_alg = CompFactory.PseudoJetMerger(
            f"PJMerger_{pj_name}MergedWithGhostTracks",
            InputPJContainers=[pj_name, context["GhostTracks"]],
            OutputContainer=f"{pj_name}MergedWithGhostTracks",
        )
        # update the pseudo jet name
        pj_name = merge_alg.OutputContainer.Path
        acc.addEventAlgo(merge_alg,seqname)

    jetDef._internalAtt["finalPJContainer"] = pj_name


    acc.addEventAlgo(
        JetRecConfig.getJetRecAlg(
            jetDef,JetOnlineMon.getMonTool_TrigJetAlgorithm(flags, f"HLTJets/{jetsOut}/")
        ),
        seqname,
    )

    return acc, jetsOut, jetDef


@AccumulatorCache
def StandardJetRecoCfg(flags, clustersKey, **jetRecoDict):
    """ Full reconstruction for 'simple' (ungroomed, not reclustered) jets

    First the uncalibrated jets are built, then (if necessary) the calibrated jets are provided
    as a shallow copy.
    """

    seqname = "JetRecSeq_"+jetRecoDict['jetDefStr']
    if jetRecoDict["jetCalib"] == "nojcalib":        

        reco_acc = ComponentAccumulator()
        reco_acc.addSequence(parOR(seqname))

        build_acc, jetsNoCalib, jetDef = StandardJetBuildCfg(
            flags, clustersKey, **jetRecoDict
        )
        reco_acc.merge(build_acc, seqname)


        # This view alg is added here rather than in StandardJetBuildCfg
        # so that we are able to get the no-calib collection name later
        jetViewAcc, jetsOut = JetViewAlgCfg(
            flags,
            jetDef.fullname(),
            jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV, # GeV converted internally
            **jetRecoDict
        )
        reco_acc.merge(jetViewAcc, seqname)
        return reco_acc, jetsOut, jetDef

    # Schedule reconstruction w/o calibration
    jrdNoJCalib = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        jetCalib="nojcalib"
    )

    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname))

    build_acc, jetsNoCalib, jetDefNoCalib = StandardJetBuildCfg(
        flags, clustersKey, **jrdNoJCalib
    )
    acc.merge(build_acc,seqname)

    jetViewAcc, jetsViewNoCalib = JetViewAlgCfg(
        flags,
        jetDefNoCalib.fullname(),
        jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV, # GeV converted internally
        **jrdNoJCalib
    )
    acc.merge(jetViewAcc, seqname)

    # Get the calibration tool
    jetDef = jetDefNoCalib.clone()
    jetDef.suffix = jetDefNoCalib.suffix.replace("nojcalib", jetRecoDict["jetCalib"])

    if "sub" in jetRecoDict["jetCalib"]:
        # Add the event shape alg for area subtraction
        # WARNING : offline jets use the parameter voronoiRf = 0.9 ! we might want to harmonize this.
        eventShapeAlg = JetInputConfig.buildEventShapeAlg(jetDef, getHLTPrefix(),voronoiRf = 1.0 )
        acc.addEventAlgo(eventShapeAlg,seqname)
        rhoKey = str(eventShapeAlg.EventDensityTool.OutputContainer)
    else:
        rhoKey = "auto"

    # If we need JVT rerun the JVT modifier
    use_FS_tracking = doFSTracking(jetRecoDict)
    is_pflow = isPFlow(jetRecoDict)

    decorList = getDecorList(jetRecoDict)
    
    jetDef.modifiers = getCalibMods(flags, jetRecoDict, rhoKey)
    if use_FS_tracking:
        jetDef.modifiers += ["JVT"]

    if jetRecoDict["recoAlg"] == "a4":
        jetDef.modifiers += ["CaloQuality"]
        
    if not is_pflow and jetRecoDict["recoAlg"] == "a4":
        from TriggerMenuMT.HLT.Jet.JetRecoCommon import cleaningDict
        jetDef.modifiers += [f'Cleaning:{clean_wp}' for _,clean_wp in cleaningDict.items()]

    # getjet context for our trkopt
    context = flags.Jet.Context[jetRecoDict['trkopt']]
    # make sure all modifiers info is ready before passing jetDef to JetRecConfig helpers
    jetDef = solveDependencies(jetDef,flags) 
    # This algorithm creates the shallow copy and then also applies the calibration as part of the
    # modifiers list
    acc.addEventAlgo(
        JetRecConfig.getJetCopyAlg(
            jetsin=jetsNoCalib,
            jetsoutdef=jetDef,
            decorations=decorList,
            monTool=JetOnlineMon.getMonTool_TrigJetAlgorithm(flags,
                "HLTJets/{}/".format(jetDef.fullname())
            ),
        ),
        seqname
    )

    # Check conditions before adding fast flavour tag info to jets
    jetCalibDef=getJetCalibDefaultString(jetRecoDict)
    if(
        flags.Trigger.Jet.fastbtagPFlow
        and isPFlow(jetRecoDict)   # tag only PFlow jets
        and jetRecoDict['recoAlg']=='a4'         # tag only anti-kt with R=0.4
        and jetRecoDict['constitMod']==''        # exclude SK and CSSK chains
        and jetRecoDict['jetCalib']==jetCalibDef # exclude jets with not full default calibration
    ):

        ftagseqname = f"jetFtagSeq_{jetRecoDict['trkopt']}"
        acc.addSequence(parOR(ftagseqname),seqname)

        # Adding Fast flavor tagging
        acc.merge(
            fastFlavourTaggingCfg(
                flags,
                jetDef.fullname(),
                context["Vertices"],
                context["Tracks"],
                isPFlow=True,
            ),
            ftagseqname
        )


    # Filter the copied jet container so we only output jets with pt above jetPtMin
    jetViewAcc, jetsOut = JetViewAlgCfg(
        flags,
        jetDef.fullname(),
        jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV, # GeV converted internally
        **jetRecoDict
    )
    acc.merge(jetViewAcc,seqname)

    return acc, jetsOut, jetDef


@AccumulatorCache
def GroomedJetRecoCfg(flags, clustersKey, **jetRecoDict):
    """ Create the groomed jets

    First the ungroomed jets are created (using the standard configuration), then the grooming
    is applied
    """
    # Grooming needs the ungroomed jets to be built first,
    # so call the basic jet reco seq, then add a grooming alg

    ungroomedJRD = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        # Drop grooming spec
        recoAlg=jetRecoDict["recoAlg"].rstrip("tsd"),
        # No need to calibrate
        jetCalib = "nojcalib",
    )

    seqname = "JetGroomSeq_"+jetRecoDict['jetDefStr']
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)

    build_acc, ungroomedJetsName, ungroomedDef = StandardJetBuildCfg(
        flags,
        clustersKey,
        **ungroomedJRD,
    )
    acc.merge(build_acc,seqname)

    groomDef = defineGroomedJets(jetRecoDict, ungroomedDef)
    jetsOut = recordable(groomDef.fullname())
    groomDef.modifiers = getCalibMods(flags,jetRecoDict)
    groomDef.modifiers += [
        "Sort",
        "Filter:{}".format(getFilterCut(jetRecoDict["recoAlg"])),
    ]
    groomDef = solveGroomingDependencies(groomDef, flags)

    acc.addEventAlgo( JetRecConfig.getJetRecGroomAlg(
        groomDef,
        monTool=JetOnlineMon.getMonTool_TrigJetAlgorithm(flags, f"HLTJets/{jetsOut}/"),
        ),
        seqname
    )
    jetCalibDef=getJetCalibDefaultString(jetRecoDict)
    if(
        isPFlow(jetRecoDict)   # tag only PFlow jets
        and jetRecoDict['recoAlg'] == 'a10sd'
        and jetRecoDict['constitMod']=='cssk'        # include only CSSK chains
        and jetRecoDict['jetCalib']==jetCalibDef # exclude jets without full default calibration
        ):
        context = flags.Jet.Context[jetRecoDict['trkopt']]

        ftagseqname = f"jetFtagSeq_{jetRecoDict['trkopt']}_largeR"
        acc.addSequence(parOR(ftagseqname), seqname)

        acc.merge(
                fastFlavourTaggingCfg(
                    flags,
                    groomDef.fullname(),
                    context['Vertices'],
                    context['Tracks'],
                    isPFlow=True,
                    doXbbtagLargeRJet = True,
                ),
                ftagseqname 
            )
    return acc, jetsOut, groomDef


@AccumulatorCache
def ReclusteredJetRecoCfg(flags, clustersKey, **jetRecoDict):
    """ Create the reclustered jets

    First the input jets are built, then the reclustering algorithm is run
    """
    seqname = "JetReclusterSeq_"+jetRecoDict['jetDefStr']
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)

    basicJetRecoDict = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        # Standard size for reclustered inputs
        recoAlg = "a4",
    )

    basic_acc, basicJetsFiltered, basicJetDef = StandardJetRecoCfg(
        flags, clustersKey, **basicJetRecoDict
    )
    acc.merge(basic_acc,seqname)

    rcJetPtMin = 15 # 15 GeV minimum pt for jets to be reclustered
    jetViewAcc, jetsOut = JetViewAlgCfg(
        flags,
        basicJetDef.fullname(),
        jetPtMinGeV=rcJetPtMin, # GeV converted internally
        **jetRecoDict
    )
    acc.merge(jetViewAcc,seqname)

    rc_suffix = f"_{jetRecoDict['jetCalib']}" + (f"_{jetRecoDict['trkopt']}" if doTracking(jetRecoDict) else "")
    rcJetDef = defineReclusteredJets(
        jetRecoDict,
        jetsOut,
        basicJetDef.inputdef.label,
        getHLTPrefix(),
        rc_suffix,
    )

    rcJetDef.modifiers = []

    rcConstitPJAlg = JetRecConfig.getConstitPJGAlg(rcJetDef.inputdef, suffix=jetRecoDict['jetDefStr'])
    rcConstitPJKey = str(rcConstitPJAlg.OutputContainer)
    acc.addEventAlgo(rcConstitPJAlg,seqname)

    rcJetDef._internalAtt["finalPJContainer"] = rcConstitPJKey

    acc.addEventAlgo(
        JetRecConfig.getJetRecAlg(
            rcJetDef,
            JetOnlineMon.getMonTool_TrigJetAlgorithm(flags,
                "HLTJets/{}/".format(rcJetDef.fullname())
            )
        ),
        seqname
    )

    return acc, recordable(rcJetDef.fullname()), rcJetDef


@AccumulatorCache
def FastFtaggedJetCopyAlgCfg(flags,jetsIn,**jetRecoDict):

    acc = ComponentAccumulator()
    caloJetRecoDict = jetRecoDictFromString(jetsIn)
    caloJetDef = defineJets(flags,caloJetRecoDict,clustersKey=getClustersKey(caloJetRecoDict),prefix=getHLTPrefix(),suffix='fastftag')
    decorList = getDecorList(jetRecoDict)
    acc.addEventAlgo(JetRecConfig.getJetCopyAlg(jetsin=jetsIn,jetsoutdef=caloJetDef,decorations=decorList))
    ftaggedJetsIn = caloJetDef.fullname()
    return acc,ftaggedJetsIn

# Returns reco sequence for RoI-based track reco + low-level flavour tagging
@AccumulatorCache
def JetRoITrackJetTagSequenceCfg(flags,jetsIn,trkopt,RoIs):

    acc = ComponentAccumulator()
    
    trkcfg, trkmap = JetRoITrackingCfg(flags, jetsIn, trkopt, RoIs)
    acc.merge( trkcfg )

    acc.merge(
        fastFlavourTaggingCfg(
            flags,
            jetsIn,
            trkmap['Vertices'],
            trkmap['Tracks']
        )
    )

    return acc
