#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


# Run over AOD or TRIG8 to produce the TIDA track ntuple
# Example: 
# python -m TrigInDetValidation.TrigInDetValidation_AODtoTrkNtuple_CA --filesInput AOD.root

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

# Add custom options
parser = flags.getArgumentParser()
parser.add_argument('--LRT', default=False, help='Turn on LRT flag')
parser.add_argument('--doTIDATier0', default=False, help='Turn on Tier-0 Analysis')
parser.add_argument('--doNewTIDATier0', default=False, help='Turn on Tier-0 Analysis')
parser.add_argument('--parentpdgid', default=0, help='Set parentpdgid in TIDA')
parser.add_argument('--pdgid', default=0, help='Set pdgid in TIDA')
parser.add_argument('--ptmin', default=1000, help='Set parentpdgid in TIDA')


args, _ = parser.parse_known_args()
args = flags.fillFromArgs(parser=parser)


if hasattr(args, "filesInput") and args.filesInput:
    FilesInput = args.filesInput
else:
    FilesInput = ["AOD.pool.root"]

flags.Input.Files = FilesInput
flags.lock()

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
acc.EvtMax = -1 # Set to -1 for all events


# Setup to read xAOD
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg 
acc.merge(PoolReadCfg(flags))

# Setup Trigger Decision Tool
from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))


############ TrigInDetMonitoring part ################################


if args.doTIDATier0 or args.doNewTIDATier0:   

  # this is the new location ...
  from TrigInDetMonitoring.TIDAMonitoring import TIDAMonitoring
  for git in TIDAMonitoring( flags, "idtrigger" ):
    acc.addEventAlgo(git)

  THistSvc=CompFactory.THistSvc()
  THistSvc.Output = ["EXPERT DATAFILE='data-hists-tier0.root' OPT='RECREATE'"]
  acc.addService(THistSvc)


############ TrigInDetAnalysis part ################################

if ( True ) :
  TestMonTool = CompFactory.TrigR3Mon('TrigR3Mon')
  TestMonTool.buildNtuple = True
  TestMonTool.AnalysisConfig = "nTuple" #Change to Tier0 for T0 Analysis
  TestMonTool.EnableLumi = False
# TestMonTool.RequireDecision = False
  TestMonTool.mcTruth = True
  TestMonTool.pTCutOffline = args.ptmin
  TestMonTool.ntupleChainNames = []
  TestMonTool.SelectTruthPdgId=args.pdgid
  TestMonTool.SelectParentTruthPdgId=args.parentpdgid

  if args.LRT:
    TestMonTool.FiducialRadius = 500.

    TestMonTool.ntupleChainNames += [
      ":InDetLargeD0TrackParticles",
      "Muons::LRT",
      "Muons",
      "Electrons::LRT"
    ]


  TestMonTool.KeepAllEvents = False
  # TestMonTool.TrigConfigTool = "TrigConf::xAODConfigTool"
  TestMonTool.ntupleChainNames += [
    "Offline",
    "Truth",
    "Vertex",
    "Vertex:BTagging_AntiKt4EMPFlowSecVtx",
    # "Muons",
    # "Muons:Tight",
    # "Muons:Medium",

    # "HLT_j.*bperf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi:vtx=xPrimVx",
    # "HLT_j.*bperf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi",
    # "HLT_j.*bperf_split:key=InDetTrigTrackingxAODCnv_Bjet_IDTrig",
    # "HLT_j.*bperf_split:key=InDetTrigTrackingxAODCnv_Bjet_FTF",

    #    "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi:vtx=xPrimVx",
    #    "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi:vtx=EFHistoPrmVtx",
    # "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi:vtx=xPrimVx",
    # "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi:vtx=EFHistoPrmVtx",
    # "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=SuperRoi",
    #    "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_Bjet_IDTrig",
    #    "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_Bjet_FTF",

    "Electrons",

    "Electrons:MediumCB",
    "Electrons:TightCB",
    "Electrons:MediumLH",
    "Electrons:TightLH",

    # "Taus",

    # "Taus_1Prong",
    # "Taus_Loose_1Prong",
    # "Taus_Medium_1Prong",
    # "Taus_Tight_1Prong",
    # "Taus_3Prong",
    # "Taus_Loose_3Prong",
    # "Taus_Medium_3Prong",
    # "Taus_Tight_3Prong",

    "Taus:Medium:1Prong",
    "Taus:Tight:1Prong",

    #    ":HLT_IDTrack_FS_FTF",
    #    ":HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",

    "HLT_j45_subjesgscIS_ftf_boffperf_split_L1J20:key=HLT_IDTrack_Bjet_FTF",
    "HLT_j45_subjesgscIS_ftf_boffperf_split_L1J20:key=HLT_IDTrack_Bjet_IDTrig",

    "HLT_j45_ftf_subjesgscIS_boffperf_split_L1J20:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",

    "HLT_.*_presel.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper",
    "HLT_.*_presel.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",

    "HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1J20:key=HLT_IDTrack_Bjet_IDTrig:roi=HLT_Roi_Bjet:vtx=HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingSecVtx",
    # don't use FSJet any longer
    # "HLT_j45_ftf_subjesgscIS_boffperf_split_L1J20:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FSJet",
    "HLT_j45_pf_ftf_preselj20_L1J15:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
    # don't use FSJet any longer
    # "HLT_j45_pf_ftf_preselj20_L1J15:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FSJet",
    "HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1J20:key=HLT_IDTrack_Bjet_IDTrig:roi=HLT_Roi_Bjet:vtx=HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingSecVtx",

    "HLT_fslrt.*:HLT_IDTrack_FSLRT_FTF;DTE",
    "HLT_fslrt.*:HLT_IDTrack_FS_FTF;DTE",
    "HLT_fslrt.*:HLT_IDTrack_FSLRT_IDTrig;DTE",

    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_FTF",
    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_IDTrig",
    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
    "HLT_mu.*iv.*:HLT_IDTrack_MuonIso_FTF:roi=HLT_Roi_MuonIso",
    "HLT_mu.*iv.*:HLT_IDTrack_MuonIso_IDTrig:roi=HLT_Roi_MuonIso",

    "HLT_mu.*_LRT_idperf_.*:HLT_IDTrack_MuonLRT_FTF:HLT_Roi_L2SAMuon_LRT",
    "HLT_mu.*_LRT_idperf_.*:HLT_IDTrack_MuonLRT_IDTrig:HLT_Roi_L2SAMuon_LRT",
    "HLT_mu.*_LRT_idperf_.*:HLT_IDTrack_MuonLRT_FTF:HLT_Roi_L2SAMuon_LRT",

    "HLT_b.*perf.*:HLT_IDTrack_Bjet_FTF",
    "HLT_b.*perf.*:HLT_IDTrack_Bjet_IDTrig",
    "HLT_j.*perf.*:HLT_IDTrack_Bjet_FTF",
    "HLT_j.*perf.*:HLT_IDTrack_Bjet_IDTrig",

#    "HLT_e.*_etcut.*:HLT_IDTrack_Electron_FTF",
#    "HLT_e.*_etcut.*:HLT_IDTrack_Electron_IDTrig",
    "HLT_e.*:HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",
    "HLT_e.*:HLT_IDTrack_Electron_IDTrig",
    "HLT_e.*:HLT_IDTrack_Electron_GSF",

    "HLT_e20_idperf_loose_lrtloose_L1eEM18L:HLT_IDTrack_ElecLRT_FTF:HLT_Roi_FastElectron_LRT",
    "HLT_e30_idperf_loose_lrtloose_L1eEM26M:HLT_IDTrack_ElecLRT_FTF:HLT_Roi_FastElectron_LRT",
    "HLT_e20_idperf_loose_lrtloose_L1eEM18L:HLT_IDTrack_ElecLRT_IDTrig:HLT_Roi_FastElectron_LRT",
    "HLT_e30_idperf_loose_lrtloose_L1eEM26M:HLT_IDTrack_ElecLRT_IDTrig:HLT_Roi_FastElectron_LRT",

    # electron lrt e tag
    "HLT_e26_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM26M:HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT:te=1",
    "HLT_e26_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM26M:HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT:te=1",

    "HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_lrtmedium_probe_L1eEM26M:HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT:te=1",
    "HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_lrtmedium_probe_L1eEM26M:HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT:te=1",

    # electron lrt photon tag
    "HLT_e5_idperf_loose_lrtloose_probe_g25_medium_L1eEM24L:HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT:te=0",
    "HLT_e5_idperf_loose_lrtloose_probe_g25_medium_L1eEM24L:HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT:te=0",

    "HLT_e30_lhloose_nopix_lrtmedium_probe_g25_medium_L1eEM24L:HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT:te=0",
    "HLT_e30_lhloose_nopix_lrtmedium_probe_g25_medium_L1eEM24L:HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT:te=0",

    # double electron chains for tag and probe analysis
    "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_FTF:extra=el_tag:roi=HLT_Roi_FastElectron:te=0",
    "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_FTF:extra=el_probe:roi=HLT_Roi_FastElectron:te=1",
    "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron:te=1",

    "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag:te=0",
    "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe:te=1",
    "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_IDTrig:te=1",

    "HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_GSF:extra=el_tag:te=0",
    "HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_GSF:extra=el_probe:te=1",
    "HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_GSF:te=1",

    "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
    "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
    "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_FTF::roi=HLT_Roi_L2SAMuon:te=1",

    "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
    "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
    "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:te=1",

    # two stage tau FTF
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIsoBDT",

    # two stage tau precision tracking - empty ???
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:vtx=HLT_IDVertex_Tau",


    # should get single stage tau
    "HLT_tau.*_idperf.*_track_.*:HLT_IDTrack_Tau_FTF:roi=HLT_Roi_Tau",
    "HLT_tau.*_idperf.*_track_.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_Tau",

    # LRT tau
    "HLT_tau.*trackLRT.*:HLT_IDTrack_TauLRT_FTF:roi=HLT_Roi_TauLRT",
    "HLT_tau.*trackLRT.*:HLT_IDTrack_TauLRT_IDTrig:roi=HLT_Roi_TauLRT",

    # none of these will work
    "HLT_tau.*_idperf.*:HLT_IDTrack_Tau_IDTrig",


    # only need one instance for the tag of the tag and probe, because we
    # don't actually use the triggder collections at all, only the Roi and 
    # offline objects in the roi 
    # "HLT_mu.*tau.*_idperf.*:HLT_IDTrack_Muon_FTF:HLT_Roi_L2SAMuon:te=0",
    "HLT_mu.*tau.*_idperf.*:HLT_IDTrack_Muon_IDTrig:HLT_Roi_L2SAMuon:te=0",

    "HLT_mu.*tau.*_idperf.*:HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:te=1",
    "HLT_mu.*tau.*_idperf.*:HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:te=1",
    "HLT_mu.*tau.*_idperf.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:te=1",

    # should work for single stage tau ???
    # "HLT_tau.*_idperf.*:HLT_IDTrack_Tau_FTF",

    # should work for a two stage tau ??
    # "HLT_tau.*_idperf.*:HLT_IDTrack_TauIso_FTF",

    "HLT_mu4.*_cosmic.*:HLT_IDTrack_Cosmic_FTF",
    "HLT_mu4.*_cosmic.*:HLT_IDTrack_Cosmic_IDTrig",
    "HLT_mu4.*_cosmic.*:HLT_IDTrack_Cosmic_EFID",

    #"HLT_mb.*:HLT_IDTrack_Cosmic_EFID",
    #"HLT_mb.*:HLT_IDTrack_MinBias_FTF",  #There are no tracks here
    "HLT_mb.*:HLT_IDTrack_MinBias_IDTrig",
    #"HLT_mb.*:HLT_IDTrack_MinBias_EFID"  #There are no tracks here
    
    "HLT_2mu4_bBmumux_BsmumuPhi_L12MU3V:HLT_IDTrack_Bmumux_FTF",
    "HLT_2mu4_bBmumux_BsmumuPhi_L12MU3V:HLT_IDTrack_Bmumux_IDTrig",
    "HLT_mu11_mu6_bBmumux_Bidperf_L1MU8VF_2MU5VF:HLT_IDTrack_Bmumux_FTF",
    "HLT_mu11_mu6_bBmumux_Bidperf_L1MU8VF_2MU5VF:HLT_IDTrack_Bmumux_IDTrig"

    ]

  from PyUtils.Helpers import release_metadata
  d = release_metadata()
  TestMonTool.releaseMetaData = d['nightly name'] + " " + d['nightly release'] + " " + d['date'] + " " + d['platform'] + " " + d['release']
  TestMonTool.outputFileName="TrkNtuple.root"
  acc.addEventAlgo(TestMonTool)
  print (TestMonTool)


print ("configured everything")

# include("AthAnalysisBaseComps/SuppressLogging.py")              #Optional include to suppress as much athena output as possible. Keep at bottom of joboptions so that it doesn't suppress the logging of the things you have configured above

# Adjust message limits
acc.getService("MessageSvc").infoLimit = 100000000

acc.run()

