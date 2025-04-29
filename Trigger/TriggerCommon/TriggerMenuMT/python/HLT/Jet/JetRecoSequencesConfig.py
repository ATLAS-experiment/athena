#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    getDecorList,
    getHLTPrefix,
    getClustersKey,
    isPFlow,
    doTracking,
    doFSTracking,
    getJetCalibDefaultString,
    jetDefToString,
    jetCalibFromJetDef,
)
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import parOR
from ..CommonSequences.FullScanDefs import fs_cells
from ..Bjet.BjetFlavourTaggingConfig import fastFlavourTaggingCfg
from .JetTrackingConfig import JetRoITrackingCfg
from .JetHIConfig import HeavyIonJetRecoDataDeps

from JetRecConfig import JetRecConfig
from JetRecConfig import JetInputConfig
from JetRecConfig.DependencyHelper import solveDependencies, solveGroomingDependencies

from JetRecTools import OnlineMon
from JetRec import JetOnlineMon

from EventShapeTools.EventDensityConfig import getEventShapeName

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from typing import Final

# Default threshold for filtering jets for input to hypo
JET_DEFAULT_VIEW_PT_MIN_GEV : Final[int] = 10

def formatFilteredJetsName(jetsIn, jetPtMinGeV):
    return f"{jetsIn}_pt{int(jetPtMinGeV)}"

##################################################################################################
# Data dependency generation for the stages of jet reconstruction
# This is used to precompute the collections that will be generated,
# which can be passed as needed out to other steps
def JetRecoDataDeps(flags, **jetRecoDict):

    jetalg, jetradius, extra = interpretRecoAlg(jetRecoDict["recoAlg"])

    if jetRecoDict['ionopt']=='ion':
        jetDefDict = HeavyIonJetRecoDataDeps(
            flags, **jetRecoDict
        )
    elif extra == "r":
        jetDefDict = ReclusteredJetRecoDataDeps(
            flags, **jetRecoDict
        )
        jetDefDict['final'] = jetDefDict['reclustered']
    elif extra in ["t", "sd"]:
        jetDefDict = GroomedJetRecoDataDeps(
            flags, **jetRecoDict
        )
        jetDefDict['final'] = jetDefDict['groomed']
    else:
        jetDefDict = StandardJetRecoDataDeps(
            flags, **jetRecoDict
        )
        jetDefDict['final'] = jetDefDict['calib']

    # Consistency check that we generated what we intended to generate
    gen_jetDefStr = jetDefToString(jetDefDict['final'][1])
    assert jetRecoDict['jetDefStr'] == gen_jetDefStr, (
        f"Expected jetDefStr {jetRecoDict['jetDefStr']} from reco dict, generated {gen_jetDefStr}"
    )

    return jetDefDict


def StandardJetBuildDataDeps(flags, **jetRecoDict):
    """Jet clustering step -- generally only called through StandardJetRecoDataDeps"""
    use_FS_tracking = doFSTracking(jetRecoDict)
    trkopt = jetRecoDict['trkopt']
    clustersKey = getClustersKey(jetRecoDict)

    is_pflow = isPFlow(jetRecoDict)
    if is_pflow:
        jetDef = defineJets(
            flags,
            jetRecoDict,
            pfoPrefix=f"HLT_{trkopt}",
            prefix=getHLTPrefix(),
        )
        # Record which clusters should be used to form PFOs
        # Purely for trigger usage
        jetDef.inputdef.prereqs = [f'extinput:{clustersKey}']
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
        jetDef.modifiers += ["TrackMoments", "JVF", "JVT", "NNJVT_TrkAugV1"] # (NN)JVT not strictly needed for no-calib jets
    
    pj_name = JetRecConfig.getPJContName(jetDef.inputdef)
    if use_FS_tracking:
        pj_name = f"{pj_name}MergedWithGhostTracks"
    jetDef._internalAtt["finalPJContainer"] = pj_name

    jetsOut = jetDef.fullname()
    jetDef = solveDependencies(jetDef,flags)
    jetDef.lock()

    return {'build': (jetsOut, jetDef)}


def StandardJetRecoDataDeps(flags, **jetRecoDict):
    """Jet clustering + calibration (via shallow copy)"""
    if jetRecoDict["jetCalib"] == "nojcalib":
        # If we don't calibrate, we need to return the clustered jets with filter
        jetDefDict = StandardJetBuildDataDeps(
            flags, **jetRecoDict
        )
        jetsNoCalib, jetDef = jetDefDict['build']
        jetDef.lock()
        jetsNoCalibFiltered = formatFilteredJetsName(jetsNoCalib, jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV)
        # Inelegantly repeat the values because the requested calibration is null
        jetDefDict.update({'nojcalib': (jetsNoCalibFiltered, jetDef), 'calib': (jetsNoCalibFiltered, jetDef)})
        return jetDefDict
    else:
        # If we do calibrate, then the process is to copy+calibrate, and return those jets
        jrdNoJCalib = cloneAndUpdateJetRecoDict(
            jetRecoDict,
            jetCalib="nojcalib"
        )
        jetDefDict = StandardJetBuildDataDeps(
            flags, **jrdNoJCalib
        )
        jetsNoCalib, jetDefNoCalib = jetDefDict['build']
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
            jetDef.modifiers += ["JVT", "NNJVT_TrkAugV1"]

        if jetRecoDict["recoAlg"] == "a4":
            jetDef.modifiers += ["CaloQuality"]
            
        if not is_pflow and jetRecoDict["recoAlg"] == "a4":
            from TriggerMenuMT.HLT.Jet.JetRecoCommon import cleaningDict
            jetDef.modifiers += [f'Cleaning:{clean_wp}' for _,clean_wp in cleaningDict.items()]

        jetDef = solveDependencies(jetDef,flags)
        jetDef.lock()
        jetsOut = formatFilteredJetsName(jetDef.fullname(),jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV)
        jetDefDict.update({'nojcalib':(jetsViewNoCalib, jetDefNoCalib), 'calib':(jetsOut, jetDef)})
        return jetDefDict

def ReclusteredJetRecoDataDeps(flags, **jetRecoDict):
    basicJetRecoDict = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        # Standard size for reclustered inputs
        recoAlg = "a4",
    )

    jetDefDict = StandardJetRecoDataDeps(
        flags, **basicJetRecoDict
    )
    basicJetsName, basicJetDef = jetDefDict['calib']

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

    rcJetsOut = rcJetDef.fullname()
    jetDefDict['reclustered'] = (rcJetsOut, rcJetDef)
    return jetDefDict


def GroomedJetRecoDataDeps(flags, **jetRecoDict):
    ungroomedJRD = cloneAndUpdateJetRecoDict(
        jetRecoDict,
        # Drop grooming spec
        recoAlg=jetRecoDict["recoAlg"].rstrip("tsd"),
        # No need to calibrate
        jetCalib = "nojcalib",
    )

    jetDefDict = StandardJetBuildDataDeps(flags, **ungroomedJRD)
    ungroomedJetsName, ungroomedDef = jetDefDict['build']

    groomDef = defineGroomedJets(jetRecoDict, ungroomedDef)
    groomedJetsName = groomDef.fullname()
    groomDef.modifiers = getCalibMods(flags,jetRecoDict)
    groomDef.modifiers += [
        "Sort",
        "Filter:{}".format(getFilterCut(jetRecoDict["recoAlg"])),
    ]
    groomDef = solveGroomingDependencies(groomDef, flags)
    groomDef.lock()

    return {'ungroomed':(ungroomedJetsName, ungroomedDef), 'groomed':(groomedJetsName, groomDef)}


##################################################################################################
# Configuration of the HLT jet reconstruction
# Takes as input the dictionaries of JetDefs from the data dependency generators above
@AccumulatorCache
def JetRecoCfg(flags, **jetDefDict):
    """The top-level sequence

    Forwards arguments to the standard jet reco, grooming or reclustering sequences.
    """

    if 'reclustered' in jetDefDict:
        return ReclusteredJetRecoCfg(
            flags, **jetDefDict
        )
    elif 'groomed' in jetDefDict:
        return GroomedJetRecoCfg(
            flags, **jetDefDict
        )
    else:
        return StandardJetRecoCfg(
            flags, **jetDefDict
        )


# Get a configured JetViewAlg that creates a VIEW_ELEMENTS container of jets above a minimum jet pT
# Filtered jets are given to hypo.
# jetPtMinGeV is minimum jet pt in GeV for jets to be seen by hypo
@AccumulatorCache
def JetViewAlgCfg(flags,jetDef,jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV):

    decorList = getDecorList(jetDef)
    filteredJetsName = f"{jetDef.fullname()}_pt{int(jetPtMinGeV)}"
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.JetViewAlg(
            "jetview_"+filteredJetsName,
            InputContainer=jetDef.fullname(),
            OutputContainer=filteredJetsName,
            PtMin=jetPtMinGeV*1e3, #MeV
            DecorDeps=decorList
        )
    )

    return acc


@AccumulatorCache
def StandardJetBuildCfg(flags, jetDef):
    """ Standard jet reconstruction, no reclustering or grooming 
    
    The clusters (and tracks, if necessary) should be built beforehand,
    but this config will build the PFOs if they are needed.

    The reconstruction is configured from the input JetDefinition
    """

    jetDefStr = jetDefToString(jetDef)
    trackColls = jetDef._contextDic

    seqname = "JetBuildSeq_"+jetDefStr
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)

    # Add PFlow reconstruction if necessary
    if 'PFlow' in jetDef.inputdef.label:
        from eflowRec.PFHLTConfig import PFCfg

        clustersKey = jetDef.inputdef.prereqs[0].split(':')[1]
        acc.merge(
            PFCfg(
                flags,
                jetDef.context,
                clustersin=clustersKey,
                calclustersin="",
                tracksin=trackColls["Tracks"],
                verticesin=trackColls["Vertices"],
                cellsin=fs_cells,
            ),
            seqname
        )

    alg = JetRecConfig.getConstitModAlg(
        jetDef, jetDef.inputdef,
        monTool=OnlineMon.getMonTool_Algorithm(flags, f"HLTJets/{jetDef.fullname()}/"),
    )
    # getConstitModAlg will return None if there's nothing for it to do
    if alg is not None:
        acc.addEventAlgo(alg,seqname)

    pj_alg = JetRecConfig.getConstitPJGAlg(jetDef.inputdef)
    acc.addEventAlgo(pj_alg,seqname)

    if jetDef.context=='ftf':
        pj_name = pj_alg.OutputContainer.Path
        # Make sure that the jets are constructed with the ghost tracks included
        merge_alg = CompFactory.PseudoJetMerger(
            f"PJMerger_{pj_name}MergedWithGhostTracks",
            InputPJContainers=[pj_name, trackColls["GhostTracks"]],
            OutputContainer=f"{pj_name}MergedWithGhostTracks",
        )
        # update the pseudo jet name
        acc.addEventAlgo(merge_alg,seqname)

    acc.addEventAlgo(
        JetRecConfig.getJetRecAlg(
            jetDef,JetOnlineMon.getMonTool_TrigJetAlgorithm(flags, f"HLTJets/{jetDef.fullname()}/")
        ),
        seqname,
    )

    return acc


def StandardJetRecoCfg(flags, **jetDefDict):
    """ Full reconstruction for 'simple' (ungroomed, not reclustered) jets

    First the uncalibrated jets are built, then (if necessary) the calibrated jets are provided
    as a shallow copy.
    """

    jetsOut, jetDef = jetDefDict['calib']
    jetDefStr = jetDefToString(jetDef)
    seqname = "JetRecSeq_"+jetDefStr
    
    # If we want the final jets to be uncalibrated, we can shortcut
    if jetDef.suffix[1:] == "nojcalib":

        reco_acc = ComponentAccumulator()
        reco_acc.addSequence(parOR(seqname))

        buildJets, buildJetDef = jetDefDict['build']
        build_acc = StandardJetBuildCfg(
            flags, buildJetDef
        )
        reco_acc.merge(build_acc, seqname)

        # This view alg is added here rather than in StandardJetBuildCfg
        # so that we are able to get the no-calib collection name later
        jetViewAcc = JetViewAlgCfg(
            flags,
            jetDef,
            jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV, # GeV converted internally
        )
        reco_acc.merge(jetViewAcc, seqname)
        return reco_acc

    # From here we run the full reconstruction including calibration
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname))

    buildJets, buildJetDef = jetDefDict['build']
    build_acc = StandardJetBuildCfg(
        flags, buildJetDef
    )
    acc.merge(build_acc,seqname)

    jetsNoCalib, jetDefNoCalib = jetDefDict['nojcalib']
    jetViewAcc = JetViewAlgCfg(
        flags,
        jetDefNoCalib,
        jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV, # GeV converted internally
    )
    acc.merge(jetViewAcc, seqname)

    # Figure out what jet calibration we are running
    jetCalib = jetCalibFromJetDef(jetDef)

    if "sub" in jetCalib:
        # Add the event shape alg for area subtraction
        # WARNING : offline jets use the parameter voronoiRf = 0.9 ! we might want to harmonize this.
        eventShapeAlg = JetInputConfig.buildEventShapeAlg(jetDef, getHLTPrefix(),voronoiRf = 1.0 )
        acc.addEventAlgo(eventShapeAlg,seqname)

    decorList = getDecorList(jetDef)
    
    # This algorithm creates the shallow copy and then also applies the calibration as part of the
    # modifiers list
    acc.addEventAlgo(
        JetRecConfig.getJetCopyAlg(
            jetsin=jetDefNoCalib.fullname(),
            jetsoutdef=jetDef,
            decorations=decorList,
            monTool=JetOnlineMon.getMonTool_TrigJetAlgorithm(flags,
                f"HLTJets/{jetDef.fullname()}/"
            ),
        ),
        seqname
    )

    # Check conditions before adding fast flavour tag info to jets
    recoAlg, constitType, _ = jetDefStr.split('_',2)
    jetCalibDef = getJetCalibDefaultString(recoAlg,f'{constitType}',jetDef.context)
    if(
        flags.Trigger.Jet.fastbtagPFlow
        and jetDef.basename=="AntiKt4EMPFlow" # Tag only standard small-R PFlow jets
        and jetCalib==jetCalibDef # exclude jets with not full default calibration
    ):

        ftagseqname = f"jetFtagSeq_{jetDef.context}"
        acc.addSequence(parOR(ftagseqname),seqname)
        # getjet context for our trkopt
        trackingColls = jetDef._contextDic

        # Adding Fast flavor tagging
        acc.merge(
            fastFlavourTaggingCfg(
                flags,
                jetDef.fullname(),
                trackingColls["Vertices"],
                trackingColls["Tracks"],
                isPFlow=True,
            ),
            ftagseqname
        )


    # Filter the copied jet container so we only output jets with pt above jetPtMin
    jetViewAcc = JetViewAlgCfg(
        flags,
        jetDef,
        jetPtMinGeV=JET_DEFAULT_VIEW_PT_MIN_GEV, # GeV converted internally
    )
    acc.merge(jetViewAcc,seqname)

    return acc


def GroomedJetRecoCfg(flags, **jetDefDict):
    """ Create the groomed jets

    First the ungroomed jets are created (using the standard configuration), then the grooming
    is applied
    """
    # Grooming needs the ungroomed jets to be built first,
    # so call the basic jet reco seq, then add a grooming alg

    ungroomedJetsName, ungroomedDef = jetDefDict['ungroomed']
    groomedJets, groomDef = jetDefDict['groomed']
    jetDefStr = jetDefToString(groomDef)
    
    seqname = "JetGroomSeq_"+jetDefStr
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)

    build_acc = StandardJetBuildCfg(
        flags,
        ungroomedDef,
    )
    acc.merge(build_acc,seqname)

    acc.addEventAlgo( JetRecConfig.getJetRecGroomAlg(
        groomDef,
        monTool=JetOnlineMon.getMonTool_TrigJetAlgorithm(flags, f"HLTJets/{groomedJets}/"),
        ),
        seqname
    )

    recoAlg, constitType, _ = jetDefStr.split('_',2)
    jetCalib = jetCalibFromJetDef(groomDef)
    jetCalibDef = getJetCalibDefaultString(recoAlg,f'{constitType}',groomDef.context)
    if(
        groomDef.basename.startswith("AntiKt10EMPFlowCSSKSoftDrop") # Tag only groomed large-R PFlow jets
        and jetCalib==jetCalibDef # exclude jets with not full default calibration
        ):
        trackingColls = groomDef._contextDic

        ftagseqname = f"jetFtagSeq_{groomDef.context}_largeR"
        acc.addSequence(parOR(ftagseqname), seqname)

        acc.merge(
                fastFlavourTaggingCfg(
                    flags,
                    groomDef.fullname(),
                    trackingColls['Vertices'],
                    trackingColls['Tracks'],
                    isPFlow=True,
                    doXbbtagLargeRJet = True,
                ),
                ftagseqname 
            )
    return acc


def ReclusteredJetRecoCfg(flags, **jetDefDict):
    """ Create the reclustered jets

    First the input jets are built, then the reclustering algorithm is run
    """

    basicJetsFiltered, basicJetDef = jetDefDict['calib']
    rcJets, rcJetDef = jetDefDict['reclustered']
    jetDefStr = jetDefToString(rcJetDef)

    seqname = "JetReclusterSeq_"+jetDefStr
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)

    basic_acc = StandardJetRecoCfg(
        flags, **jetDefDict
    )
    acc.merge(basic_acc,seqname)

    rcJetPtMin = 15 # 15 GeV minimum pt for jets to be reclustered
    jetViewAcc = JetViewAlgCfg(
        flags,
        basicJetDef,
        jetPtMinGeV=rcJetPtMin, # GeV converted internally
    )
    acc.merge(jetViewAcc,seqname)

    rcConstitPJAlg = JetRecConfig.getConstitPJGAlg(rcJetDef.inputdef, suffix=jetDefStr)
    acc.addEventAlgo(rcConstitPJAlg,seqname)

    acc.addEventAlgo(
        JetRecConfig.getJetRecAlg(
            rcJetDef,
            JetOnlineMon.getMonTool_TrigJetAlgorithm(flags,
                f"HLTJets/{rcJetDef.fullname()}/"
            )
        ),
        seqname
    )

    return acc


@AccumulatorCache
def FastFtaggedJetCopyAlgCfg(flags,jetDef):

    acc = ComponentAccumulator()
    jetsIn = jetDef.fullname()
    # Don't need modifiers here, we won't rerun calibration
    ftagJetDef = jetDef.clone(suffix=jetDef.suffix+'_fastftag',modifiers=[],lock=True)
    decorList = getDecorList(jetDef)
    acc.addEventAlgo(JetRecConfig.getJetCopyAlg(jetsin=jetsIn,jetsoutdef=ftagJetDef,decorations=decorList))
    return acc, ftagJetDef

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
