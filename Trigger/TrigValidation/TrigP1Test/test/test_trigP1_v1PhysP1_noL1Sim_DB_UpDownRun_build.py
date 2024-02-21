#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger athenaHLT test of DB upload (inlcuding duplicate), download and running.
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps


# Step 1 - create json files
genJSON = ExecStep.ExecStep("GenJSON")
genJSON.type = 'athenaHLT'
genJSON.job_options = 'TriggerJobOpts.runHLT'
genJSON.input = 'data'
genJSON.flags = ['Trigger.triggerMenuSetup="PhysicsP1_pp_run3_v1_HLTReprocessing_prescale"','Trigger.L1.errorOnMissingTOB=False','Trigger.Offline.SA.Muon.runCommissioningChain=True']
genJSON.args = ' -M --dump-config-exit'
genJSON.perfmon = False
genJSON.fpe_auditor = False


# Step 2 - copy files for clarity
moveJSON = ExecStep.ExecStep("MoveJSON")
moveJSON.type = 'other'
moveJSON.input = ''
moveJSON.executable = '(mkdir filesForUpload && mv L1*.json HLT*.json filesForUpload \
  && echo "*** json files created: ***" \
  && ls filesForUpload \
  && echo "*** summary of HLTJobOptions.db.json HLTJobOptions.json (intentional) differences" \
  && (diff -y --suppress-common-lines filesForUpload/HLTJobOptions.db.json filesForUpload/HLTJobOptions.json || true) \
  && echo "*** check no filename references in HLTJobOptions.db.json" \
  && grep ".json" filesForUpload/HLTJobOptions.db.json | if grep JsonFileName; then false; else true; fi )'
moveJSON.prmon = False


# Common DB commands
# Setup
# - setup TDAQ release to access TriggerDB scripts
# - copy authentication files as can't be read on afs/eos, set TRIGGER_DB_ART to use these files
# - copy TNS lookup file as not in ART container
# - export missing ART username into env
trigDBsetup =  '&& source /cvmfs/atlas.cern.ch/repo/sw/tdaq/tdaq/prod/installed/setup.sh \
  && export TRIGGER_DB_ART=1 \
  && export EOS_MGM_URL=root://eosuser.cern.ch \
  && xrdcp root://eosuser.cern.ch//eos/user/t/trigcomm/.dbauth/run3/write/dblookup.xml . \
  && xrdcp root://eosuser.cern.ch//eos/user/t/trigcomm/.dbauth/run3/write/authentication.xml . \
  && export TNS_ADMIN="." \
  && xrdcp root://eosuser.cern.ch//eos/project/o/oracle/public/admin/tnsnames.ora .\
  && export USER="artprod"'
# Cleanup
# - remove copied files
trigDBcleanup = '&& rm authentication.xml && rm dblookup.xml && rm tnsnames.ora'
# Export
# - create file containing keys to be used to set the env in later steps
trigDBexport = '&& grep smk uploadRecord*.json | awk \'{print "SMK=" substr($2, 1, length($2)-1)}\' > exportMenuKeys.sh \
  && grep l1PSK uploadRecord*.json | awk \'{print "L1PSK=" substr($2, 1, length($2)-1)}\' >> exportMenuKeys.sh \
  && grep hltPSK uploadRecord*.json | awk \'{print "HLTPSK=" substr($2, 1, length($2)-1)}\' >> exportMenuKeys.sh'


# Step 3 - upload
uploadDB = ExecStep.ExecStep("UploadDB")
uploadDB.type = 'other'
uploadDB.input = ''
uploadDB.executable = '(cd filesForUpload'
uploadDB.executable += trigDBsetup
uploadDB.executable += '&& insertAll.py --dbalias TRIGGERDBART --comment "ART test SMK upload `date +%F` release ${AtlasVersion} ${AtlasBuildStamp}" --directory ./ '
uploadDB.executable += trigDBcleanup
uploadDB.executable += trigDBexport
uploadDB.executable += '&& cd ..)'
uploadDB.prmon = False


# Step 4 - download and compare to originals
downloadDB = ExecStep.ExecStep("DownloadDB")
downloadDB.type = 'other'
downloadDB.input = ''
downloadDB.executable = '(mkdir filesDownloaded && cd filesDownloaded'
downloadDB.executable += trigDBsetup
downloadDB.executable += '&& source ../filesForUpload/exportMenuKeys.sh \
  && extractMenu.py --dbalias TRIGGERDBART --smk $SMK \
  && extractPrescales.py --dbalias TRIGGERDBART --l1psk $L1PSK --hltpsk $HLTPSK'
downloadDB.executable += trigDBcleanup
downloadDB.executable += '&& cd .. \
  && diff -q -s filesDownloaded/L1Prescale_${L1PSK}.json filesForUpload/L1PrescalesSet_*.json \
  && diff -q -s filesDownloaded/HLTPrescale_${HLTPSK}.json filesForUpload/HLTPrescalesSet_*.json \
  && diff -q -s filesDownloaded/SMK_${SMK}/L1Menu_${SMK}.json filesForUpload/L1Menu_*.json \
  && diff -q -s filesDownloaded/SMK_${SMK}/HLTMenu_${SMK}.json filesForUpload/HLTMenu_*.json \
  && diff -q -s filesDownloaded/SMK_${SMK}/HLTJO_${SMK}.json filesForUpload/HLTJobOptions*.db.json )'
downloadDB.prmon = False


# Step 5 - create duplicate json files
genJSONdupe = ExecStep.ExecStep("GenJSONdupe")
genJSONdupe.type = genJSON.type
genJSONdupe.job_options = genJSON.job_options
genJSONdupe.input = genJSON.input
genJSONdupe.args = genJSON.args
genJSONdupe.flags = genJSON.flags
genJSONdupe.perfmon = genJSON.perfmon
genJSONdupe.fpe_auditor = genJSON.fpe_auditor


# Step 6 - copy files for clarity and compare to downloaded
moveJSONdupe = ExecStep.ExecStep("MoveJSONdupe")
moveJSONdupe.type = 'other'
moveJSONdupe.input = ''
moveJSONdupe.executable = '(mkdir filesDuplicate && mv L1*.json HLT*.json filesDuplicate \
  && echo "*** duplicate json files created: ***" \
  && ls filesDuplicate \
  && source filesForUpload/exportMenuKeys.sh \
  && diff -q -s filesDownloaded/L1Prescale_${L1PSK}.json filesDuplicate/L1PrescalesSet_*.json \
  && diff -q -s filesDownloaded/HLTPrescale_${HLTPSK}.json filesDuplicate/HLTPrescalesSet_*.json \
  && diff -q -s filesDownloaded/SMK_${SMK}/L1Menu_${SMK}.json filesDuplicate/L1Menu_*.json \
  && diff -q -s filesDownloaded/SMK_${SMK}/HLTMenu_${SMK}.json filesDuplicate/HLTMenu_*.json \
  && diff -q -s filesDownloaded/SMK_${SMK}/HLTJO_${SMK}.json filesDuplicate/HLTJobOptions*.db.json )'
moveJSONdupe.prmon = False


# Step 7 - upload duplicate and compare to original upload
uploadDBdupe = ExecStep.ExecStep("UploadDBdupe")
uploadDBdupe.type = 'other'
uploadDBdupe.input = ''
uploadDBdupe.executable = '(cd filesDuplicate'
uploadDBdupe.executable += trigDBsetup
uploadDBdupe.executable += '&& insertAll.py --dbalias TRIGGERDBART --comment "ART test duplicate SMK upload `date +%F` release ${AtlasVersion} ${AtlasBuildStamp}" --directory ./'
uploadDBdupe.executable += trigDBcleanup
uploadDBdupe.executable += trigDBexport
uploadDBdupe.executable += '&& cd .. \
  && diff -q -s filesDuplicate/exportMenuKeys.sh filesForUpload/exportMenuKeys.sh)'
uploadDBdupe.prmon = False


# Step 8 - run from DB using original keys
runDB = ExecStep.ExecStep("RunDB")
runDB.type = 'Trig_reco_tf'
runDB.input = 'data'
runDB.max_events = 50
runDB.args = '--useDB=True --DBserver=TRIGGERDBART'
runDB.args += ' --DBsmkey=`source filesForUpload/exportMenuKeys.sh && echo ${SMK}`' 
runDB.args += ' --DBl1pskey=`source filesForUpload/exportMenuKeys.sh && echo ${L1PSK}`'
runDB.args += ' --DBhltpskey=`source filesForUpload/exportMenuKeys.sh && echo ${HLTPSK}`'
runDB.args += ' --prodSysBSRDO True'
runDB.args += ' --outputHIST_HLTMONFile=hltmon.root'
# potential other outputs not created currently
# runDB.args += ' --outputBSFile=RAW.pool.root'
# runDB.args += ' --outputDRAW_TRIGCOSTFile=TRIGCOST.pool.root'
# runDB.args += ' --outputNTUP_TRIGCOSTFile=cost.ntup.root'


test = Test.Test()
test.art_type = 'build'
test.exec_steps = [genJSON, moveJSON, uploadDB, downloadDB, genJSONdupe, moveJSONdupe, uploadDBdupe, runDB]
test.check_steps = CheckSteps.default_check_steps(test)


import sys
sys.exit(test.run())
