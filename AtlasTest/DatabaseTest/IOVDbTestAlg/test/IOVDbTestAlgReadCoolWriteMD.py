# Copyright (C) 2002-2024 by CERN for the benefit of the ATLAS collaboration

from IOVDbTestAlg.IOVDbTestAlgConfig import IOVDbTestAlgFlags, IOVDbTestAlgReadCfg

flags = IOVDbTestAlgFlags()
flags.Exec.MaxEvents = 30

flags.addFlag("Output.Stream1FileName", "SimpleEventPoolFile.root")
flags.addFlag("Output.doWriteStream1", True)
flags.lock()

acc = IOVDbTestAlgReadCfg(flags)

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge( OutputStreamCfg(flags, "Stream1",
                           disableEventTag = True,
                           MetadataItemList = ['xAOD::EventFormat#EventFormatStream1']) )

from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge( EventInfoCnvAlgCfg(flags, disableBeamSpot = True) )

from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
acc.merge( SetupMetaDataForStreamCfg(flags, "Stream1") )

#--------------------------------------------------------------
# Create simulation and digitization parameters and add them to the
# file meta data
#--------------------------------------------------------------

from IOVDbMetaDataTools import ParameterDbFiller
# Set run numbers and parameters
beginRun    = 0
endRun      = 99999
simParams   = []
digitParams = []
simParams  += ['k_a']
simParams  += ['v_a']
simParams  += ['k_b']
simParams  += ['v_b']
simParams  += ['k_c']
simParams  += ['v_c1 v_c1 v_c1']
digitParams  += ['k_d_a']
digitParams  += ['v_d_a']
digitParams  += ['k_d_b']
digitParams  += ['v_d_b']
digitParams  += ['k_d_c']
digitParams  += ['v_d_c1 v_d_c1 v_d_c1']

# Create simulation parameters db
dbFiller = ParameterDbFiller.ParameterDbFiller()
# set iov
dbFiller.setBeginRun(beginRun)
dbFiller.setEndRun(endRun)
# set parameters
for i in range(len(simParams)//2):
    dbFiller.addSimParam(simParams[2*i], simParams[2*i+1])
# generate db
dbFiller.genSimDb()

# create digit parameters db
# set parameters
for i in range(len(digitParams)//2):
    dbFiller.addDigitParam(digitParams[2*i], digitParams[2*i+1])
# generate db
dbFiller.genDigitDb()

# List of folders to  write to file meta data
from IOVDbSvc.IOVDbSvcConfig import addFolders
acc.getService("IOVDbSvc").FoldersToMetaData = [ "/IOVDbTest/*" ]

for f, db in (("/Simulation/Parameters", "sqlite://;schema=SimParams.db;dbname=SIMPARAM"),
              ("/Digitization/Parameters", "sqlite://;schema=DigitParams.db;dbname=DIGPARAM")):

    acc.merge( addFolders(flags, f, modifiers=f"<dbConnection>{db}</dbConnection>") )
    acc.getService("IOVDbSvc").FoldersToMetaData.append(f)


import sys
sc = acc.run(flags.Exec.MaxEvents)

sys.exit(sc.isFailure())
