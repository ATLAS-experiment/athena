import os

makeDataDAODs=True
makeMCDAODs=True
makeTruthDAODs=True
makeTrains=True

formatList = ["PHYSVAL","PHYS","PHYSLITE",
              "LLP1","HIGG1D1","HIGG1D2",
              "JETM1","JETM2","JETM3","JETM4","JETM5","JETM6","JETM10","JETM11","JETM12","JETM14",
              "IDTR2",
              "EGAM1","EGAM2","EGAM3","EGAM4","EGAM5","EGAM7","EGAM8","EGAM9","EGAM10",
              "FTAG1","FTAG2","FTAG3",
              "BPHY1","BPHY2","BPHY3","BPHY4","BPHY5","BPHY6","BPHY10","BPHY12","BPHY15","BPHY16","BPHY18","BPHY21","BPHY22",
              "BPHY23","BPHY24",
              "STDM7","STDM13",
              "TRIG8","TRIG9",
              "MUON1"
]

truthFormatList = ["TRUTH0", "TRUTH1", "TRUTH3"]

trainList = [
              ["EGAM1","EGAM2","EGAM3","EGAM4","EGAM5","EGAM7","EGAM8","EGAM9","EGAM10","JETM1","JETM3","JETM4","JETM6","FTAG1","FTAG2","FTAG3","IDTR2","TRIG8","TRIG9","LLP1","STDM7","STDM13","HIGG1D1","MUON1"]
]


# Files
com_dir = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/"
mc20File = com_dir+"mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1"
mc23File = com_dir+"mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4162_r14622/1000events.AOD.33799166._000073.pool.root.1"
truthFile = com_dir+"mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"
data18File = com_dir+"data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1"
data22File = com_dir+"data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1"
data23File = com_dir+"data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1"
data23CosFile = com_dir+"data23_cos/AOD/data23_cos.00459152.physics_CosmicMuons.merge.AOD.f1383_m2195/data23_cos.00459152.physics_CosmicMuons.merge.AOD.f1383_m2195._lb0124-lb0126._0001.1"


def generateText(formatName,label,inputFile,isTruth,isMC,nEvents):
   add_str = ""
   outputFileName = "test_"+label+formatName+add_str+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: main/Athena"+"\n")
   if (formatName.find("EGAM")!=-1 or formatName.find("JETM")!=-1 or formatName.find("FTAG")!=-1 or formatName.find("IDTR")!=-1 or formatName.find("TRIG")!=-1 or (formatName.find("PHYS")!=-1 and formatName.find("PHYSLITE")==-1)):
      outputFile.write("# art-include: main/Athena"+"\n")
   outputFile.write("# art-description: DAOD building "+formatName+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("# art-output: checkFile*.txt"+"\n")
   outputFile.write("# art-output: checkxAOD*.txt"+"\n")
   outputFile.write("# art-output: checkIndexRefs*.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if (not isTruth):
      outputFile.write("Derivation_tf.py \\\n")
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

def generateTrains(formatList,label,inputFile,isMC,nEvents):
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

if (makeDataDAODs or makeMCDAODs):
   for formatName in formatList:
      if makeDataDAODs: 
         generateText(formatName,"data18",data18File,False,False,"-1")
         generateText(formatName,"data22",data22File,False,False,"-1")
         generateText(formatName,"data23",data23File,False,False,"-1") 
      if makeMCDAODs:
         generateText(formatName,"mc20",mc20File,False,True,"-1")
         generateText(formatName,"mc23",mc23File,False,True,"-1")
      generateText("NCB1","data23cos",data23CosFile,False,False,"-1")
if makeTruthDAODs:
   for formatName in truthFormatList:
      generateText(formatName,"mc23",truthFile,True,False,"1000")

if makeTrains:
   for train in trainList:
      if makeDataDAODs: 
         generateTrains(train,"data18",data18File,False,"-1")
         generateTrains(train,"data22",data22File,False,"-1")
         generateTrains(train,"data23",data23File,False,"-1")
      if makeMCDAODs:
         generateTrains(train,"mc20",mc20File,True,"-1")
         generateTrains(train,"mc23",mc23File,True,"-1")
