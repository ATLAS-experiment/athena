import os

makeDataDAODs=True
makeMCDAODs=True
makeTruthDAODs=True
makeTrains=True
makePHYStoPHYSLITE=True

formatList = ["PHYSVAL","PHYS","PHYSLITE",
              "LLP1","LLJ1","HIGG1D1","HIGG1D2", "HIGG9D1"
              "JETM1","JETM2","JETM3","JETM4","JETM5","JETM7","JETM12","JETM42",
              "IDTR2",
              "EGAM1","EGAM2","EGAM3","EGAM4","EGAM5","EGAM7","EGAM8","EGAM9","EGAM10",
              "FTAG1","FTAG1LITE","FTAG2","FTAG3","FTAG4","FTAG5","FTAGPU","FTAGXBB",
              "BPHY1","BPHY2","BPHY3","BPHY4","BPHY5","BPHY6","BPHY10","BPHY12","BPHY14","BPHY15","BPHY16","BPHY18","BPHY21","BPHY22",
              "BPHY23","BPHY24",
              "STDM6","STDM7","STDM13","STDM16","STDM17",
              "SUSY20",
              "TRIG8","TRIG9","TRIG10",
              "MUON1", "MUON5",
              "TCAL1",
              "TOPQ7"
]

truthFormatList = ["TRUTH0", "TRUTH1", "TRUTH3"]

trainList = [
              ["EGAM1","EGAM2","EGAM3","EGAM4","EGAM5","EGAM7","EGAM8","EGAM9","EGAM10","JETM1","JETM3","JETM4","FTAG1","FTAG2","FTAG3","IDTR2","TRIG8","TRIG9","LLP1","STDM7","STDM13","HIGG1D1","MUON1"]
]


# Files
from AthenaConfiguration.TestDefaults import defaultTestFiles
mc20File = defaultTestFiles.AOD_RUN2_MC[0]
mc23aFile = defaultTestFiles.AOD_RUN3_MC[0]
mc21_14TeV_File = defaultTestFiles.AOD_RUN4_MC[0]
truthFile = defaultTestFiles.EVNT[0]
data18File = defaultTestFiles.AOD_RUN2_DATA[0]
data22File = defaultTestFiles.AOD_RUN3_DATA[0]
com_dir = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/"
data23File = com_dir+"data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1"
data23CosFile = com_dir+"data23_cos/AOD/data23_cos.00459152.physics_CosmicMuons.merge.AOD.f1383_m2195/data23_cos.00459152.physics_CosmicMuons.merge.AOD.f1383_m2195._lb0124-lb0126._0001.1"
data24File = com_dir+"data24/AOD/data24_13p6TeV.00486658.physics_Main.recon.AOD.f1522_m2262_r16385_r16377/AOD.43718985._000221.pool.root.1"

data25Input = "user.martindl.data25_13p6TeV.00498515.physics_Main.merge.AOD.r17521_p7232"
data25File = "root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/data25/AOD/data25_13p6TeV.00498515.physics_Main.merge.AOD.r17521_p7232/AOD.49752827._000024.pool.root.1"

mc23dInput = "user.martindl.mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r15530"
mc23dFile = "root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r15530/AOD.38803011._001713.pool.root.1"
mc23eInput = "user.martindl.mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4369_r16083"
mc23eFile = "root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4369_r16083/AOD.41608496._001231.pool.root.1"
mc23gInput = "user.martindl.mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4618_r17610"
mc23gFile = "root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4618_r17610/AOD.50092877._002250.pool.root.1"


import os
mc20PHYSFile = os.getenv('ASG_TEST_FILE_MC')
mc23PHYSFile = os.getenv('ASG_TEST_FILE_RUN3_MC')
data18PHYSFile = os.getenv('ASG_TEST_FILE_DATA')
data23PHYSFile = os.getenv('ASG_TEST_FILE_RUN3_DATA')

def generateText(formatName,label,inputFile,isTruth,nEvents,artInput=""):
   add_str = ""
   outputFileName = "test_"+label+formatName+add_str+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: main/Athena"+"\n")
   outputFile.write("# art-description: DAOD building "+formatName+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-memory: 4096"+"\n")
   if artInput:
      outputFile.write("# art-input: "+artInput+"\n")
      outputFile.write("# art-input-nfiles: 1 \n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("# art-output: checkFile*.txt"+"\n")
   outputFile.write("# art-output: checkxAOD*.txt"+"\n")
   outputFile.write("# art-output: checkIndexRefs*.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if artInput:
      outputFile.write("if [[ -z ${ArtInFile} ]]; then\n")
      outputFile.write("    ArtInFile=\""+inputFile+"\"\n")
      outputFile.write("fi\n")
      outputFile.write("\n")
   if (not isTruth):
      outputFile.write("Derivation_tf.py \\\n")
      if artInput:
         outputFile.write("--inputAODFile ${ArtInFile} \\\n")
      else:
         outputFile.write("--inputAODFile "+inputFile+" \\\n")
      outputFile.write("--outputDAODFile art.pool.root \\\n")
      outputFile.write("--formats "+formatName+" \\\n")
      outputFile.write("--maxEvents "+nEvents+" \\\n")
   if isTruth:
      outputFile.write("Derivation_tf.py \\\n")
      outputFile.write("--inputEVNTFile "+inputFile+" \\\n")
      outputFile.write("--outputDAODFile art.pool.root \\\n")
      outputFile.write("--formats "+formatName+" \\\n") 
      outputFile.write("--maxEvents "+nEvents+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $? reco\""+"\n")
   outputFile.write("\n")
   outputFile.write("checkFile.py DAOD_"+formatName+".art.pool.root > checkFile_"+formatName+".txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkfile\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkxAOD.py DAOD_"+formatName+".art.pool.root > checkxAOD_"+formatName+".txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkxAOD\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkIndexRefs.py DAOD_"+formatName+".art.pool.root > checkIndexRefs_"+formatName+".txt 2>&1"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkIndexRefs\""+'\n')
   outputFile.close()
   os.system("chmod +x "+outputFileName)

def generateTrains(formatList,label,inputFile,nEvents):
   add_str = ""
   outputFileName = "test_"+label+"_".join(formatList)+add_str+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: main/Athena"+"\n")
   outputFile.write("# art-description: DAOD building "+" ".join(formatList)+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("# art-output: checkFile*.txt"+"\n")
   outputFile.write("# art-output: checkxAOD*.txt"+"\n")
   outputFile.write("# art-output: checkIndexRefs*.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   outputFile.write("Derivation_tf.py \\\n")
   outputFile.write("--inputAODFile "+inputFile+" \\\n") 
   outputFile.write("--outputDAODFile art.pool.root \\\n")
   outputFile.write("--formats "+" ".join(formatList)+" \\\n")
   outputFile.write("--maxEvents "+nEvents+" \\\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $? reco\""+"\n")
   for formatname in formatList:
      outputFile.write("\n")
      outputFile.write("checkFile.py DAOD_"+formatname+".art.pool.root > checkFile_"+formatname+".txt"+"\n")
      outputFile.write("\n")
      outputFile.write("echo \"art-result: $?  checkfile\""+'\n')
      outputFile.write("\n")
      outputFile.write("checkxAOD.py DAOD_"+formatname+".art.pool.root > checkxAOD_"+formatname+".txt"+"\n")
      outputFile.write("\n")
      outputFile.write("echo \"art-result: $?  checkxAOD\""+'\n')
      outputFile.write("\n")
      outputFile.write("checkIndexRefs.py DAOD_"+formatname+".art.pool.root > checkIndexRefs_"+formatname+".txt 2>&1"+"\n")
      outputFile.write("\n")
      outputFile.write("echo \"art-result: $?  checkIndexRefs\""+'\n')
   outputFile.close()
   os.system("chmod +x "+outputFileName)

def generatePHYStoPHYSLITE(label,inputFile,nEvents):
   add_str = "PHYStoPHYSLITE"
   outputFileName = "test_"+label+"_"+add_str+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: main/Athena"+"\n")
   outputFile.write("# art-description: DAOD building PHYStoPHYSLITE "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("# art-output: checkFile*.txt"+"\n")
   outputFile.write("# art-output: checkxAOD*.txt"+"\n")
   outputFile.write("# art-output: checkIndexRefs*.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   outputFile.write("Derivation_tf.py \\\n")
   outputFile.write("--inputDAOD_PHYSFile "+inputFile+" \\\n")
   outputFile.write("--outputD2AODFile art.pool.root \\\n")
   outputFile.write("--formats PHYSLITE \\\n")
   outputFile.write("--maxEvents "+nEvents+" \\\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $? reco\""+"\n")
   outputFile.write("\n")
   outputFile.write("checkFile.py D2AOD_PHYSLITE.art.pool.root > checkFile_PHYSLITE.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkfile\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkxAOD.py D2AOD_PHYSLITE.art.pool.root > checkxAOD_PHYSLITE.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkxAOD\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkIndexRefs.py D2AOD_PHYSLITE.art.pool.root > checkIndexRefs_PHYSLITE.txt 2>&1"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkIndexRefs\""+'\n')
   outputFile.close()
   os.system("chmod +x "+outputFileName)

if (makeDataDAODs or makeMCDAODs):
   for formatName in formatList:
      # Special cases
      if formatName == "JETM7":
         # JETM7 requires per-vertex jet reconstruction, therefore running only over 100 events
         if makeDataDAODs:
            generateText(formatName,"data18",data18File,False,"100")
            generateText(formatName,"data24",data24File,False,"100")
         if makeMCDAODs:
            generateText(formatName,"mc20",mc20File,False,"100")
            generateText(formatName,"mc23",mc23aFile,False,"100")
            generateText(formatName,"mc21_14TeV_",mc21_14TeV_File,False,"100")
         continue
      if formatName == "JETM42":
         # JETM42 currently only used for upgrade studies
         if makeMCDAODs:
            generateText(formatName,"mc21_14TeV_",mc21_14TeV_File,False,"-1")
         continue
      # End special cases
      if makeDataDAODs: 
         generateText(formatName,"data18",data18File,False,"-1")
         generateText(formatName,"data22",data22File,False,"-1")
         generateText(formatName,"data23",data23File,False,"-1")
         generateText(formatName,"data24",data24File,False,"-1")
         if formatName in ["PHYS", "PHYSLITE"]:
            generateText(formatName,"data25",data25File,False,"1000",data25Input)
      if makeMCDAODs:
         generateText(formatName,"mc20",mc20File,False,"-1")
         if formatName in ["PHYS", "PHYSLITE"]:
            generateText(formatName,"mc23a",mc23aFile,False,"-1")
            generateText(formatName,"mc23d",mc23dFile,False,"1000",mc23dInput)
            generateText(formatName,"mc23e",mc23eFile,False,"1000",mc23eInput)
            generateText(formatName,"mc23g",mc23gFile,False,"1000",mc23gInput)
         else:
            generateText(formatName,"mc23",mc23aFile,False,"-1")
         generateText(formatName,"mc21_14TeV_",mc21_14TeV_File,False,"-1")
      generateText("NCB1","data23cos",data23CosFile,False,"-1")

if makeTruthDAODs:
   for formatName in truthFormatList:
      generateText(formatName,"mc23",truthFile,True,"1000")

if makeTrains:
   for train in trainList:
      if makeDataDAODs:
         generateTrains(train,"data18",data18File,"-1")
         generateTrains(train,"data22",data22File,"-1")
         generateTrains(train,"data23",data23File,"-1")
         generateTrains(train,"data24",data24File,"-1")
      if makeMCDAODs:
         generateTrains(train,"mc20",mc20File,"-1")
         generateTrains(train,"mc23",mc23aFile,"-1")
         generateTrains(train,"mc21_14TeV_",mc21_14TeV_File,"-1")

if makePHYStoPHYSLITE:
   if makeDataDAODs: 
      generatePHYStoPHYSLITE("data18",data18PHYSFile,"-1")
      generatePHYStoPHYSLITE("data23",data23PHYSFile,"-1")
   if makeMCDAODs:
      generatePHYStoPHYSLITE("mc20",mc20PHYSFile,"-1")
      generatePHYStoPHYSLITE("mc23a",mc23PHYSFile,"-1")
