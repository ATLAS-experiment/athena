#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


# Run over AOD or TRIG8 to produce the TIDA track ntuple
# Note, to run using the python file in your current directory you need to drop 'TrigInDetValidation.'
# Local example: 
# python -m TrigInDetValidation.TrigInDetValidation_AODtoTrkNtuple_CA --filesInput AOD.root
# Grid example:
# prun --exec="python -m TrigInDetValidation.TrigInDetValidation_AODtoTrkNtuple_CA --LRT=True --filesInput=%IN" --inDS=DATASETNAME --outDS=OUTPUTNAME --useAthenaPackage --outputs="TrkNtuple-0000.root" 

import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

# Add custom options
parser = flags.getArgumentParser()
parser.add_argument('--LRT', default=False, help='Turn on LRT flag')
parser.add_argument('--doTIDATier0', default=False, help='Turn on Tier-0 Analysis')
parser.add_argument('--doNewTIDATier0', default=False, help='Turn on Tier-0 Analysis')
parser.add_argument('--parentpdgid', type=int, default=0, help='Set parentpdgid in TIDA')
parser.add_argument('--pdgid', type=int, default=0, help='Set pdgid in TIDA')
parser.add_argument('--ptmin', type=float, default=1000, help='Set min pT for tracks in TIDA')


args, _ = parser.parse_known_args()
args = flags.fillFromArgs(parser=parser)


if not (hasattr(args, "filesInput") and args.filesInput):
    flags.Input.Files = ["AOD.pool.root"]

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

  histsvc=CompFactory.THistSvc()
  histsvc.Output = ["CombinedMonitoring DATAFILE='data-hists-tier0.root' OPT='RECREATE'"]
  acc.addService(histsvc)

  # this is the new location ...
  from TrigInDetMonitoring.TIDAMonitoring import TrigInDetMonConfig
  acc.merge( TrigInDetMonConfig( flags ) )

  
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

    "Electrons",

    "Electrons:MediumCB",
    "Electrons:TightCB",
    "Electrons:MediumLH",
    "Electrons:TightLH",

    # "Muons",
    "Muons:Tight",
    "Muons:Medium",


    # "Taus",

    "Taus:Medium:1Prong",
    "Taus:Tight:1Prong",

    #    ":HLT_IDTrack_FS_FTF",
    #    ":HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",


    "HLT_j.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",

    "HLT_.*_presel.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper",
    "HLT_.*_presel.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",

      
    # the FSLRT runs at global scope
    "HLT_fslrt.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI",
    "HLT_fslrt.*:key=HLT_IDTrack_FSLRT_FTF:roi=HLT_FSRoI",
    "HLT_fslrt.*:key=HLT_IDTrack_FSLRT_IDTrig:roi=HLT_FSRoI",

    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_FTF",
    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_IDTrig",
    "HLT_mu.*_idperf.*:HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
    "HLT_mu.*iv.*:HLT_IDTrack_MuonIso_FTF:roi=HLT_Roi_MuonIso",
    "HLT_mu.*iv.*:HLT_IDTrack_MuonIso_IDTrig:roi=HLT_Roi_MuonIso",

    "HLT_mu.*_LRT_idperf_.*:HLT_IDTrack_MuonLRT_FTF:HLT_Roi_L2SAMuon_LRT",
    "HLT_mu.*_LRT_idperf_.*:HLT_IDTrack_MuonLRT_IDTrig:HLT_Roi_L2SAMuon_LRT",

    "HLT_b.*perf.*:HLT_IDTrack_Bjet_FTF:HLT_Roi_Bjet",
    "HLT_b.*perf.*:HLT_IDTrack_Bjet_IDTrig:HLT_Roi_Bjet",
    "HLT_j.*perf.*:HLT_IDTrack_Bjet_FTF:HLT_Roi_Bjet",
    "HLT_j.*perf.*:HLT_IDTrack_Bjet_IDTrig:HLT_Roi_Bjet",

      
#   "HLT_e.*_etcut.*:HLT_IDTrack_Electron_FTF",
#   "HLT_e.*_etcut.*:HLT_IDTrack_Electron_IDTrig",
    "HLT_e.*_idperf.*:HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",
    "HLT_e.*_idperf.*:HLT_IDTrack_Electron_IDTrig",
    "HLT_e.*_idperf.*:HLT_IDTrack_Electron_GSF",


    # LRT electron idperf               
    "HLT_e.*idperf.*lrt.*:HLT_IDTrack_ElecLRT_FTF:HLT_Roi_FastElectron_LRT",
    "HLT_e.*idperf.*lrt.*:HLT_IDTrack_ElecLRT_IDTrig:HLT_Roi_FastElectron_LRT",
    "HLT_e.*idperf.*lrt.*:HLT_IDTrack_Electron_LRTGSF:HLT_Roi_FastElectron_LRT",

    # electron lrt e tag
    "HLT_e.*_e.*_lrt.*:HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT:te=1",
    "HLT_e.*_e.*_lrt.*:HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT:te=1",

    # electron lrt photon tag
    "HLT_e.*_idperf_.*_lrt.*g.*:HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT:te=0",
    "HLT_e.*_idperf_.*_lrt.*g.*:HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT:te=0",

    # double electron chains for tag and probe analysis
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_FTF:extra=el_tag:roi=HLT_Roi_FastElectron:te=0",
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_FTF:extra=el_probe:roi=HLT_Roi_FastElectron:te=1",

    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron:te=0",
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron:te=1",

    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag:te=0",
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe:te=1",

    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_IDTrig:te=0",
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_IDTrig:te=1",

    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_GSF:extra=el_tag:te=0",
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_GSF:extra=el_probe:te=1",

    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_GSF:te=0",
    "HLT_e.*_e.*_idperf.*_tight.*:key=HLT_IDTrack_Electron_GSF:te=1",

      
    # "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag:te=0",
    # "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe:te=1",
    # "HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_IDTrig:te=1",

    # "HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_GSF:extra=el_tag:te=0",
    # "HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_GSF:extra=el_probe:te=1",
    # "HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M:key=HLT_IDTrack_Electron_GSF:te=1",

    "HLT_mu14_mu14_idtp_idZmumu_.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
    "HLT_mu14_mu14_idtp_idZmumu_.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
    "HLT_mu14_mu14_idtp_idZmumu_.*:key=HLT_IDTrack_Muon_FTF::roi=HLT_Roi_L2SAMuon:te=1",

    # "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
    # "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
    # "HLT_mu14_mu14_idtp_idZmumu_L12MU8F:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:te=1",

    # two stage tau FTF
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIsoBDT",

    # two stage tau precision tracking - empty ???
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
    "HLT_tau.*_idperf.*tracktwo.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:vtx=HLT_IDVertex_Tau",


    # should get single stage tau
    # "HLT_tau.*_idperf.*_track_.*:HLT_IDTrack_Tau_FTF:roi=HLT_Roi_Tau",
    # "HLT_tau.*_idperf.*_track_.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_Tau",

    # LRT tau
    "HLT_tau.*trackLRT.*:HLT_IDTrack_TauLRT_FTF:roi=HLT_Roi_TauLRT",
    "HLT_tau.*trackLRT.*:HLT_IDTrack_TauLRT_IDTrig:roi=HLT_Roi_TauLRT",

    # none of these will work
    "HLT_tau.*_idperf.*:HLT_IDTrack_Tau_IDTrig",


    # only need one instance for the tag of the tag and probe, because we
    # don't actually use the trigger collections at all, only the Roi and 
    # offline objects in the roi 
    # "HLT_mu.*tau.*_idperf.*:HLT_IDTrack_Muon_FTF:HLT_Roi_L2SAMuon:te=0",
    "HLT_mu.*_tau.*_idperf.*:HLT_IDTrack_Muon_IDTrig:HLT_Roi_L2SAMuon:te=0",

    "HLT_mu.*_tau.*_idperf.*:HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:te=1",
    "HLT_mu.*_tau.*_idperf.*:HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:te=1",
    "HLT_mu.*_tau.*_idperf.*:HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:te=1",

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
    "HLT_mb.*:HLT_IDTrack_MinBiasPixel_IDTrig",
    #"HLT_mb.*:HLT_IDTrack_MinBias_EFID"  #There are no tracks here
    
    "HLT_2mu4_bBmumux_BsmumuPhi_.*:HLT_IDTrack_Bmumux_FTF",
    "HLT_2mu4_bBmumux_BsmumuPhi_.*:HLT_IDTrack_Bmumux_IDTrig",
    "HLT_mu11_mu6_bBmumux_Bidperf_.*:HLT_IDTrack_Bmumux_FTF",
    "HLT_mu11_mu6_bBmumux_Bidperf_.*:HLT_IDTrack_Bmumux_IDTrig",

    #displaced jet lrt
    "HLT_j180_.*dispjet.*_L1J100:key=HLT_IDTrack_DJLRT_FTF:roi=HLT_Roi_DJ", 
    "HLT_j180_.*dispjet.*_L1jJ160:key=HLT_IDTrack_DJLRT_FTF:roi=HLT_Roi_DJ"

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

# Return a status code
sys.exit(not acc.run().isSuccess())


