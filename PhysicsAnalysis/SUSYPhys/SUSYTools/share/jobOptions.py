""" 
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
""" 

# Author: Will Buttinger

# run this with:
# athena SUSYTools/jobOptions.py [athArgs] - [susyArgs]
# do: athena SUSYTools/jobOptions.py - --help
# to see help messages
# to run on the grid do:
# pathena SUSYTools/jobOptions.py --inDS list,of,ds --useContElementBoundary --addNthFieldOfINDSToLFN=2 - [susyArgs]

from AthenaCommon.AthArgumentParser import AthArgumentParser
susyArgsParser = AthArgumentParser()
susyArgsParser.add_argument("--testCampaign",action="store",default=None,choices=["mc20e","mc23a","mc23d","data23","data22","data18"],help="Specify to select a test campaign")
susyArgsParser.add_argument("--testFormat",action="store",default="PHYS",choices=["PHYS","PHYSLITE"],help="Specify to select a test format")
susyArgsParser.add_argument("--accessMode",action="store",choices=["POOLAccess","ClassAccess"],default="POOLAccess",help="xAOD read mode - Class is faster, POOL is more robust")
susyArgsParser.add_argument("--configFile",action="store",default=None,help="Name of the SUSYTools config file, leave blank for auto-config")
susyArgsParser.add_argument("--prwFiles",action="store",nargs="+",default=None,help="Name of prw files")
susyArgsParser.add_argument("--lumicalcFiles",action="store",nargs="+",default=None,help="Name of lumicalc files")
susyArgsParser.add_argument("--noSyst",action="store_true",help="include to disable systematics")
susyArgsParser.add_argument("--fileOutput",default=None,help="Name of output file")

susyArgs = susyArgsParser.parse_args()

if susyArgs.testCampaign:
    pTag = 'p6269' if ('data2' in susyArgs.testCampaign) else 'p6266'
    inputDir = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools'
    inputFiles = {}
    inputFiles['data18'] = f'data18_13TeV.39757132_{pTag}.{susyArgs.testFormat}.pool.root'
    inputFiles['data22'] = f'data22_13p6TeV.39672246_{pTag}.{susyArgs.testFormat}.pool.root'
    inputFiles['data23'] = f'data23_13p6TeV.39756993_{pTag}.{susyArgs.testFormat}.pool.root'
    inputFiles['mc20e']  = f'DAOD_{susyArgs.testFormat}.mc20_13TeV.410470.FS_mc20e_{pTag}.{susyArgs.testFormat}.pool.root'
    inputFiles['mc23a']  = f'mc23_13p6TeV.601229.FS_mc23a_{pTag}.{susyArgs.testFormat}.pool.root'
    inputFiles['mc23d']  = f'mc23_13p6TeV.601229.FS_mc23d_{pTag}.{susyArgs.testFormat}.pool.root'
    jps.AthenaCommonFlags.FilesInput = [f'{inputDir}/{inputFiles[susyArgs.testCampaign]}']
    if susyArgs.fileOutput is None: 
        susyArgs.fileOutput = f"hist-Ath_{susyArgs.testCampaign}_DAOD_{susyArgs.testFormat}.root"

# setup xAOD file reading
jps.AthenaCommonFlags.AccessMode = susyArgs.accessMode
# setup ROOT file outputting
if susyArgs.fileOutput: jps.AthenaCommonFlags.HistOutputs = [f"ANALYSIS:{susyArgs.fileOutput}"]

print("INFO: Processing:",jps.AthenaCommonFlags.FilesInput())
print("INFO: Outputting:",jps.AthenaCommonFlags.HistOutputs())

# read input file metadata: https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/AthAnalysis#How_to_access_file_metadata_at_t
from PyUtils.MetaReader import read_metadata
inp_filename = jps.AthenaCommonFlags.FilesInput()[0]
metadata = read_metadata(inp_filename, None, 'peeker')[inp_filename]
isMC = 'IS_SIMULATION' in metadata['eventTypes']
isFastSim = isMC and ('ATLFASTII' in metadata['SimulationFlavour'].upper()) #full sim or atlfast
print("INFO: Format: "," isMC: ",isMC," isFastSim: ",isFastSim)
if isMC:
    MCCampaign = metadata["mc_campaign"]

# configure SUSYTools algorithm and its tool
susyAlg = CfgMgr.SUSYToolsAlg(DoSyst = isMC and not susyArgs.noSyst)

if susyArgs.configFile:
    susyAlg.SUSYTools.ConfigFile = susyArgs.configFile
else:
    # select config file based on whether we are run3 or run2
    if (isMC and ((MCCampaign in ["mc23a"]) or (MCCampaign in ["mc23d"]) )) or (not isMC and metadata["runNumbers"][0]>400000):
        susyAlg.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default_Run3.conf"   # run3
    else:
        susyAlg.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default.conf"        # run2
    if susyArgs.testFormat == "PHYSLITE":
        STconfig_lite = str(susyAlg.SUSYTools.ConfigFile).replace(".conf","_LITE.conf")
        susyAlg.SUSYTools.ConfigFile = STconfig_lite
        susyAlg.SUSYTools.IsPHYSLITE = True

print("INFO: Configuration file:",susyAlg.SUSYTools.ConfigFile)

susyAlg.SUSYTools.DataSource = 0 if not isMC else (1 if not isFastSim else 2) # data/FS/atlfast

print("INFO: Configuration SUSYTools.DataSource: ",susyAlg.SUSYTools.DataSource)

if isMC:
    if susyArgs.prwFiles:
        susyAlg.SUSYTools.PRWConfigFiles = susyArgs.prwFiles
    else:
        susyAlg.SUSYTools.AutoconfigurePRWTool = True
        susyAlg.SUSYTools.PRWUseCommonMCFiles = True
    # set lumicalc info based on the campaign, if running on MC
    if susyArgs.lumicalcFiles:
        susyAlg.SUSYTools.PRWLumiCalcFiles = susyArgs.lumicalcFiles
    else:
        PRWLumiCalc = {}
        PRWLumiCalc['mc20a'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root',
                                '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root']
        PRWLumiCalc['mc20d'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root']
        PRWLumiCalc['mc20e'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root']
        PRWLumiCalc['mc23a'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data22_13p6TeV/20250321/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-004.root']
        PRWLumiCalc['mc23d'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data23_13p6TeV/20250321/ilumicalc_histograms_None_451587-456749_OflLumi-Run3-004.root']

        susyAlg.SUSYTools.PRWLumiCalcFiles = PRWLumiCalc[MCCampaign]


# schedule alg:
athAlgSeq += susyAlg

include("AthAnalysisBaseComps/SuppressLogging.py")              #Optional include to suppress as much athena output as possible. Keep at bottom of joboptions so that it doesn't suppress the logging of the things you have configured above
svcMgr+=CfgMgr.AthenaEventLoopMgr(IntervalInSeconds = 10,OutputLevel=INFO)  # print progress in fixed period intervals
svcMgr.MessageSvc.Format = "% F%50W%S%7W%R%T %0W%M" #Creates more space for displaying tool names
