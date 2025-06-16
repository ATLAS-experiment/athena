#!/usr/bin/env python

# Read the submission directory as a command line argument. You can
# extend the list of arguments with your private ones later on.
import sys
import optparse
from glob import glob
from datetime import date

#import AthenaPoolCnvSvc.ReadAthenaPool #read xAOD files

parser = optparse.OptionParser()
#
parser.add_option('--driver', dest = 'driver', choices = ['direct','grid','lxplus'], default = 'direct')
parser.add_option('--gridTag', dest = 'gridTag')
parser.add_option('--submitFlags', dest = 'submitFlags')
parser.add_option('--express', dest = 'express', default = False, action = 'store_true' )
parser.add_option('-n', '--dryrun', dest = 'dryrun', default=False, action = 'store_true' )
#
parser.add_option('--log-level', dest = 'log_level', default = 'INFO', choices = ['ALWAYS','FATAL','ERROR','WARNING','INFO','DEBUG','VERBOSE']) 
parser.add_option('--dosyst', dest = 'dosyst', default = False, action = 'store_true')
parser.add_option( '-s', '--submission-dir', dest = 'submission_dir', default = 'submitDir', help = 'Submission directory for EventLoop' )
parser.add_option('-t', '--type', dest = 'type', default = 'mc20e', help = 'Job type. (mc20a, mc20d, mc20e, mc23a, mc23d, data18, data22, data23)', choices = ['mc20a', 'mc20d', 'mc20e', 'mc23a', 'mc23d', 'data18', 'data22','data23'])
parser.add_option('--AF', dest = 'AF', default = False, action = 'store_true' )
parser.add_option('-f', '--flav', dest = 'flav', default = 'PHYS', choices = ["PHYS", "PHYSLITE"], help = 'input DAOD flavour' )
parser.add_option('-m', '--maxEvts', dest = 'maxEvts', type = 'int', default = -1, help = 'Max events (-1 is all)' )
parser.add_option('-M', '--maxEvtsManual', dest = 'maxEvtsManual', type = 'int')
parser.add_option('-p', '--ptag', dest = 'ptag', default = 'p6266', help = 'ptag' )
parser.add_option('--grl', dest = 'grl')
parser.add_option('--inputDir', dest = 'inputDir')
parser.add_option('--inputFile', dest = 'inputFile')
parser.add_option('--inputGrid', dest = 'inputGrid')
parser.add_option('--inputXRD', dest = 'inputXRD')
parser.add_option('--overwrite', dest = 'overwrite', default = False, action = 'store_true' )
( options, args ) = parser.parse_args()

# Print configuration
print("Configured input data ptag: %s"%(options.ptag))
ptageqdata = {'p6266' : 'p6269'}
if 'data2' in options.type and options.ptag in ptageqdata: 
   options.ptag = ptageqdata[options.ptag]
   print("Overriding ptag to equivalent data ptag: -> %s"%(options.ptag))
print("Configured input data type: %s"%(options.type))
print("Configured input data DAOD flavour: %s"%(options.flav))
print("Configured input data sim type: %s"%('FullSim' if not options.AF else 'AF'))

# Set up (Py)ROOT.
import ROOT
ROOT.xAOD.Init().ignore()

# for logging
outputlvl = {'INFO':ROOT.MSG.INFO,'DEBUG':ROOT.MSG.DEBUG,'VERBOSE':ROOT.MSG.VERBOSE,'FATAL':ROOT.MSG.FATAL,'ALWAYS':ROOT.MSG.ALWAYS,'ERROR':ROOT.MSG.ERROR,'WARNING':ROOT.MSG.WARNING}

# Set up the sample handler object. See comments from the C++ macro
# for the details about these lines.
import os
sh = ROOT.SH.SampleHandler()
sh.setMetaString( 'nc_tree', 'CollectionTree' )

import shutil
if options.overwrite and os.path.exists(options.submission_dir): shutil.rmtree(options.submission_dir)

# set up file source
cvmfsInputArea = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools'
sim_type = "FS" if not options.AF else "AF3"
inputFiles = {}
inputFiles['mc20e']      = 'DAOD_%s.mc20_13TeV.410470.%s_mc20e_%s.%s.pool.root'%(options.flav, sim_type, options.ptag, options.flav)
inputFiles['mc23a']      = 'mc23_13p6TeV.601229.%s_mc23a_%s.%s.pool.root'%(sim_type, options.ptag, options.flav)
inputFiles['mc23d']      = 'mc23_13p6TeV.601229.%s_mc23d_%s.%s.pool.root'%(sim_type, options.ptag, options.flav)
inputFiles['data18']     = 'data18_13TeV.39757132_%s.%s.pool.root'%(options.ptag,options.flav)
inputFiles['data22']     = 'data22_13p6TeV.39672246_%s.%s.pool.root'%(options.ptag,options.flav)
inputFiles['data23']     = 'data23_13p6TeV.39756993_%s.%s.pool.root'%(options.ptag,options.flav)

inputDir = ''
inputFile = ''

# configure test type
if options.inputXRD:
   print("Using inputXRD: ",options.inputXRD)
   pref,server,fname = options.inputXrootd.plit('//')
   dl = ROOT.SH.DiskListXRD(server,fname)
   ROOT.SH.ScanDir().scan(sh,dl)
elif options.inputGrid:
   print("Using inputGrid: ",options.inputGrid)
   dsname = options.inputGrid.split(':')[-1].rstrip() # drop scope if present, and not trailing spaces
   ROOT.SH.addGrid(sh,dsname)
else:
   inputDir = cvmfsInputArea
   inputFile = inputFiles[options.type] if options.type in inputFiles else ''

   if options.inputDir: inputDir = options.inputDir
   if options.inputFile: inputFile = options.inputFile

   print("Using inputDir: ",inputDir)
   print("Using inputFile:",inputFile)
   ROOT.SH.ScanDir().filePattern(inputFile).scan(sh, inputDir)
sh.printContent()

# Create an EventLoop job.
job = ROOT.EL.Job()
job.sampleHandler( sh )
job.options().setDouble( ROOT.EL.Job.optMaxEvents, options.maxEvts )

# Create the algorithm's configuration. Note that we'll be able to add
# algorithm property settings here later on.
from AnaAlgorithm.AnaAlgorithmConfig import AnaAlgorithmConfig
config = AnaAlgorithmConfig( 'SUSYToolsAlg' )
config.addPrivateTool("SUSYTools","ST::SUSYObjDef_xAOD")

config.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default.conf"
if ("data2" in options.type or "mc23" in options.type): config.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default_Run3.conf"
if ("data" not in options.type): config.DoSyst = options.dosyst
config.SUSYTools.DataSource = 1
config.OutputLevel = outputlvl[options.log_level]
config.SUSYTools.OutputLevel = outputlvl[options.log_level]
config.SUSYTools.PRWLumiCalcFiles = []
config.SUSYTools.AutoconfigurePRWTool = True
config.SUSYTools.PRWUseCommonMCFiles = True
if options.flav == "PHYSLITE": 
   print("Running on PHYSLITE : ", inputFile)
   STconfig_lite = str(config.SUSYTools.ConfigFile).replace(".conf","_LITE.conf")
   config.SUSYTools.ConfigFile = STconfig_lite
   config.SUSYTools.IsPHYSLITE = True

# set datasource if AF3
if options.AF:
   config.SUSYTools.DataSource = 2

# set mcCampaign
if 'mc' in options.type:
   mcCampaign = options.type
   config.SUSYTools.mcCampaign = options.type
elif options.type == 'data18':
   mcCampaign = 'mc20e'
   config.SUSYTools.mcCampaign = options.type
   config.SUSYTools.DataSource = 0
elif options.type == 'data22':
   mcCampaign = 'mc23a'
   config.SUSYTools.mcCampaign = options.type
   config.SUSYTools.DataSource = 0
elif options.type == 'data23':
   mcCampaign = 'mc23d'
   config.SUSYTools.mcCampaign = options.type
   config.SUSYTools.DataSource = 0


# set lumicalc info
PRWLumiCalc = {}
PRWLumiCalc['mc20a'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root',
                        '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root']
PRWLumiCalc['mc20d'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root']
PRWLumiCalc['mc20e'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root']
PRWLumiCalc['mc23a'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data22_13p6TeV/20250321/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-004.root']
PRWLumiCalc['mc23d'] = ['/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data23_13p6TeV/20250321/ilumicalc_histograms_None_451587-456749_OflLumi-Run3-004.root']

config.SUSYTools.PRWLumiCalcFiles = PRWLumiCalc[mcCampaign]

if options.grl: config.GRLFiles = options.grl.split(',')
if options.maxEvtsManual: config.maxEvts = options.maxEvtsManual

# submit
job.algsAdd( config )
if options.driver == 'direct':
   # Run the job using the direct driver.
   driver = ROOT.EL.DirectDriver()
   if not options.dryrun:
      driver.submit( job, options.submission_dir )
elif options.driver == 'grid':
   driver = ROOT.EL.PrunDriver()
   submitFlags = ['--addNthFieldOfInDSToLFN=2,6']
   if options.submitFlags: submitFlags += [options.submitFlags]
   if not options.gridTag: options.gridTag = date.today().strftime('%y%m%d')
   outName = 'user.%s.%%in:name[1]%%.%%in:name[2]%%.%%in:name[3]%%.STAlg_%s'%(os.environ['USER'],options.gridTag)
   driver.options().setString("nc_outputSampleName", outName );
   driver.options().setString("nc_EventLoop_SubmitFlags", ' '.join(submitFlags))
   driver.options().setDouble(ROOT.EL.Job.optGridMergeOutput, 0)
   if not options.dryrun:
      if options.express: driver.options().setDouble(ROOT.EL.Job.optGridExpress, 1)
      driver.submitOnly( job, options.submission_dir )
