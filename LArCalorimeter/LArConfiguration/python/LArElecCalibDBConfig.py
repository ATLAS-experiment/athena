# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator,ConfigurationError
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg,addFolderList, getSqliteContent


#Import LArSymConditionsAlgs: Templated on the payload-type they handle
#These algs are mostly needed for MC processing
LArDAC2uASymAlg         =  CompFactory.getComp("LArSymConditionsAlg<LArDAC2uAMC, LArDAC2uASym>")                     
LArMinBiasAverageSymAlg =  CompFactory.getComp("LArSymConditionsAlg<LArMinBiasAverageMC, LArMinBiasAverageSym>")     
LArMinBiasSymAlg        =  CompFactory.getComp("LArSymConditionsAlg<LArMinBiasMC, LArMinBiasSym>")                   
LArNoiseSymAlg          =  CompFactory.getComp("LArSymConditionsAlg<LArNoiseMC, LArNoiseSym>")                       
LArRampSymAlg           =  CompFactory.getComp("LArSymConditionsAlg<LArRampMC, LArRampSym>")                         
LArfSamplSymAlg         =  CompFactory.getComp("LArSymConditionsAlg<LArfSamplMC, LArfSamplSym>")                     
LArAutoCorrSymAlg       =  CompFactory.getComp("LArSymConditionsAlg<LArAutoCorrMC, LArAutoCorrSym>")                 
LAruA2MeVSymAlg         =  CompFactory.getComp("LArSymConditionsAlg<LAruA2MeVMC, LAruA2MeVSym>")                     
LArShapeSymAlg          =  CompFactory.getComp("LArSymConditionsAlg<LArShape32MC, LArShape32Sym>")                   
LArMPhysOverMcalSymAlg  =  CompFactory.getComp("LArSymConditionsAlg<LArMphysOverMcalMC, LArMphysOverMcalSym>")       


LArHVScaleCorrCondFlatAlg  =  CompFactory.getComp("LArFlatConditionsAlg<LArHVScaleCorrFlat>")
LAruA2MeVCondAlg           =  CompFactory.getComp("LArFlatConditionsAlg<LAruA2MeVFlat>")
LArDAC2uACondAlg           =  CompFactory.getComp("LArFlatConditionsAlg<LArDAC2uAFlat>")
LArPedestalCondAlg         =  CompFactory.getComp("LArFlatConditionsAlg<LArPedestalFlat>")
LArRampCondAlg             =  CompFactory.getComp("LArFlatConditionsAlg<LArRampFlat>")
LArMphysOverMcalCondAlg    =  CompFactory.getComp("LArFlatConditionsAlg<LArMphysOverMcalFlat>")
LArOFCCondAlg              =  CompFactory.getComp("LArFlatConditionsAlg<LArOFCFlat>")
LArShapeCondAlg            =  CompFactory.getComp("LArFlatConditionsAlg<LArShapeFlat>")

LArHVScaleCorrSCCondFlatAlg  =  CompFactory.getComp("LArFlatConditionsAlg<LArHVScaleCorrSC>")
LArDAC2uASCCondAlg           =  CompFactory.getComp("LArFlatConditionsAlg<LArDAC2uASC>")
LArRampSCCondAlg             =  CompFactory.getComp("LArFlatConditionsAlg<LArRampSC>")
LAruA2MeVSCCondAlg           =  CompFactory.getComp("LArFlatConditionsAlg<LAruA2MeVSC>")
LArfSamplSCCondAlg           =  CompFactory.getComp("LArFlatConditionsAlg<LArfSamplSC>")
LArShapeSCCondAlg            =  CompFactory.getComp("LArFlatConditionsAlg<LArShapeSC>")
LArPedestalSCCondAlg         =  CompFactory.getComp("LArFlatConditionsAlg<LArPedestalSC>")
LArNoiseSCCondAlg            =  CompFactory.getComp("LArFlatConditionsAlg<LArNoiseSC>")
LArMinBiasSCCondAlg          =  CompFactory.getComp("LArFlatConditionsAlg<LArMinBiasSC>")
LArAutoCorrSCCondAlg         =  CompFactory.getComp("LArFlatConditionsAlg<LArAutoCorrSC>")
LArPileupAverageSCCondAlg    =  CompFactory.getComp("LArFlatConditionsAlg<LArMinBiasAverageSC>")
LArMphysOverMcalSCCondAlg    =  CompFactory.getComp("LArFlatConditionsAlg<LArMphysOverMcalSC>")
LArOFCSCCondAlg              =  CompFactory.getComp("LArFlatConditionsAlg<LArOFCSC>")


def LArElecCalibDBCfg(flags,condObjs):
    
    #Check MC case
    if flags.Input.isMC:
        return LArElecCalibDBMCCfg(flags,condObjs)
    
    from AthenaConfiguration.Enums import LHCPeriod

    #Check run 1 case:    
    if flags.GeoModel.Run < LHCPeriod.Run2 :
        return LArElecCalibDBRun1Cfg(flags,condObjs)

    #Everything else, eg run 2 (and 3?) data
    return LArElecCalibDBRun2Cfg(flags,condObjs)
    

    

def LArElecCalibDBRun2Cfg(flags,condObjs):

    sqliteFolders=getSqliteContent(flags.IOVDb.SqliteInput,flags.IOVDb.SqliteFolders,flags.IOVDb.DatabaseInstance)

    _larCondDBFoldersDataR2 = {"Ramp":("LArRamp","/LAR/ElecCalibFlat/Ramp", LArRampCondAlg ),
                               "DAC2uA":("LArDAC2uA","/LAR/ElecCalibFlat/DAC2uA",LArDAC2uACondAlg),
                               "Pedestal":("LArPedestal","/LAR/ElecCalibFlat/Pedestal",LArPedestalCondAlg),
                               "uA2MeV":("LAruA2MeV","/LAR/ElecCalibFlat/uA2MeV", LAruA2MeVCondAlg),
                               "MphysOverMcal":("LArMphysOverMcal","/LAR/ElecCalibFlat/MphysOverMcal",LArMphysOverMcalCondAlg),
                               "OFC":("LArOFC","/LAR/ElecCalibFlat/OFC",LArOFCCondAlg),
                               "Shape":("LArShape","/LAR/ElecCalibFlat/Shape",LArShapeCondAlg),
                               "HVScaleCorr":("LArHVScaleCorr","/LAR/ElecCalibFlat/HVScaleCorr",LArHVScaleCorrCondFlatAlg),
                               "fSampl":("LArfSamplSym","/LAR/ElecCalibMC/fSampl",LArfSamplSymAlg),
                           }

    result=IOVDbSvcCfg(flags)
    iovDbSvc=result.getService("IOVDbSvc")
    condLoader=result.getCondAlgo("CondInputLoader")

    for condData in condObjs:
        try:
            outputKey,fldr,calg=_larCondDBFoldersDataR2[condData]
        except KeyError:
            raise ConfigurationError("No conditions data %s found for Run-2 data" % condData)
        
        if fldr in sqliteFolders:
            sqlite=sqliteFolders[fldr]
        else:
            sqlite=None
        

        persClass="CondAttrListCollection"
        # Potential special treatment for OFC/Shape: Load them from offline DB
        if flags.LAr.OFCShapeFolder and condData == "OFC":
            fldr="/LAR/ElecCalibOfl/OFC/PhysWave/RTM/"+flags.LAr.OFCShapeFolder
            dbString=sqlite or "<db>COOLOFL_LAR/CONDBR2</db>"
            persClass="LArOFCComplete"
            calg = None
        elif flags.LAr.OFCShapeFolder and condData == "Shape":
            fldr="/LAR/ElecCalibOfl/Shape/RTM/"+flags.LAr.OFCShapeFolder
            dbString=sqlite or "<db>COOLOFL_LAR/CONDBR2</db>"
            persClass="LArShapeComplete"
            calg = None
        elif condData == "fSampl":
            if not flags.Overlay.DataOverlay:
                raise ConfigurationError("fSampl is only supported for data overlay")
            dbString="<db>COOLOFL_LAR/OFLP200</db>"
            persClass="LArfSamplMC"
            result.addCondAlgo(CompFactory.LArMCSymCondAlg(ReadKey="LArOnOffIdMap"))
            result.addCondAlgo(calg(ReadKey="LArfSampl", WriteKey=outputKey))
            calg = None
        else:
            dbString=sqlite or "<db>COOLONL_LAR/CONDBR2</db>"            

        iovDbSvc.Folders.append(fldr+dbString)# (addFolder(flags,fldr,"LAR_ONL",'CondAttrListCollection'))
        condLoader.Load.add((persClass,fldr))
        if calg is not None:
            result.addCondAlgo(calg (ReadKey=fldr, WriteKey=outputKey))

    return result

def LArElecCalibDBSCCfg(flags,condObjs,sqlite=None):

    sqliteFolders=getSqliteContent(flags.IOVDb.SqliteInput,tuple(flags.IOVDb.SqliteFolders),flags.IOVDb.DatabaseInstance)

    _larCondDBFoldersDataSC = {"Ramp":("LArRampSC","/LAR/ElecCalibFlatSC/Ramp", LArRampSCCondAlg ),
                               "DAC2uA":("LArDAC2uASC","/LAR/ElecCalibFlatSC/DAC2uA",LArDAC2uASCCondAlg),
                               "Pedestal":("LArPedestalSC","/LAR/ElecCalibFlatSC/Pedestal",LArPedestalSCCondAlg),
                               "uA2MeV":("LAruA2MeVSC","/LAR/ElecCalibFlatSC/uA2MeV", LAruA2MeVSCCondAlg),
                               "MphysOverMcal":("LArMphysOverMcalSC","/LAR/ElecCalibFlatSC/MphysOverMcal",LArMphysOverMcalSCCondAlg),
                               "OFC":("LArOFCSC","/LAR/ElecCalibFlatSC/OFC",LArOFCSCCondAlg),
                               "Shape":("LArShapeSC","/LAR/ElecCalibFlatSC/Shape",LArShapeSCCondAlg),
                               "HVScaleCorr":("LArHVScaleCorrSC","/LAR/ElecCalibFlatSC/HVScaleCorr",LArHVScaleCorrSCCondFlatAlg),
                               "fSampl":("LArfSamplSC","/LAR/ElecCalibMCSC/fSampl",LArfSamplSCCondAlg),
                           }

    result=IOVDbSvcCfg(flags)
    iovDbSvc=result.getService("IOVDbSvc")
    condLoader=result.getCondAlgo("CondInputLoader")

    for condData in condObjs:
        try:
            outputKey,fldr,calg=_larCondDBFoldersDataSC[condData]
        except KeyError:
            raise ConfigurationError("No conditions data %s found for SCdata" % condData)

        if fldr in sqliteFolders:
            sqlite=sqliteFolders[fldr]
        else:
            sqlite=None
            
        persClass="CondAttrListCollection"
        if condData == "fSampl":
            #Sampling Fraction is always read from MC-db
            dbString="<db>COOLOFL_LAR/OFLP200</db>"
            result.addCondAlgo(calg(ReadKey="LArfSampl", WriteKey=outputKey))
            calg = None
        else:
            dbString=sqlite or "<db>COOLONL_LAR/CONDBR2</db>"
        iovDbSvc.Folders.append(fldr+dbString)# (addFolder(flags,fldr,"LAR_ONL",'CondAttrListCollection'))
        condLoader.Load.add((persClass,fldr))
        if calg is not None:
            result.addCondAlgo(calg (ReadKey=fldr, WriteKey=outputKey))

    return result



def LArElecCalibDBRun1Cfg(flags,condObjs):

    _larCondDBFoldersDataR1 = {"Ramp":("/LAR/ElecCalibOnl/Ramp","LAR_ONL","LArRampComplete",None),
                               "DAC2uA":("/LAR/ElecCalibOnl/DAC2uA","LAR_ONL","LArDAC2uAMC",LArDAC2uASymAlg),
                               "Pedestal":("/LAR/ElecCalibOnl/Pedestal<key>LArPedestal</key>","LAR_ONL","LArPedestalComplete",None),
                               "uA2MeV":("/LAR/ElecCalibOfl/uA2MeV/Symmetry","LAR_OFL", "LAruA2MeVMC",LAruA2MeVSymAlg),
                               "MphysOverMcal":("/LAR/ElecCalibOfl/MphysOverMcal/RTM","LAR_OFL","LArMphysOverMcalComplete",None),
                               "HVScaleCorr":("/LAR/ElecCalibOnl/HVScaleCorr","LAR_ONL","LArHVScaleCorrComplete",None),
                               "OFC":("/LAR/ElecCalibOfl/OFC/PhysWave/RTM/"+ flags.LAr.OFCShapeFolder if len(flags.LAr.OFCShapeFolder)>0 else "/LAR/ElecCalibOfl/OFC/PhysWave/RTM/5samples1phase","LAR_OFL","LArOFCComplete",None),
                               "Shape":("/LAR/ElecCalibOfl/Shape/RTM/"+ flags.LAr.OFCShapeFolder if len(flags.LAr.OFCShapeFolder)>0 else "/LAR/ElecCalibOfl/Shape/RTM/5samples1phase","LAR_OFL","LArShapeComplete",None),
                           }



    result=ComponentAccumulator()
    folderlist=[]
    for condData in condObjs:
        try:
            folder,db,obj,calg=_larCondDBFoldersDataR1[condData]
        except KeyError:
            raise ConfigurationError("No conditions data %s found for Run-1 data" % condData)
        folderlist.append((folder,db,obj))
        if (calg):
            if obj.endswith ('MC'):
                obj = obj[:-2]
            result.addCondAlgo(calg(ReadKey=obj,WriteKey=obj+"Sym"))
    result.merge(addFolderList(flags,folderlist))
                     
    return result


def LArElecCalibDBMCCfg(flags,folders,detdb="LAR_OFL"):
    _larCondDBFoldersMC = {
                           "Ramp":("LArRampMC","/LAR/ElecCalibMC/Ramp","LArRamp", LArRampSymAlg ),
                           "AutoCorr":("LArAutoCorrMC","/LAR/ElecCalibMC/AutoCorr","LArAutoCorr", LArAutoCorrSymAlg),
                           "DAC2uA":("LArDAC2uAMC","/LAR/ElecCalibMC/DAC2uA","LArDAC2uA",LArDAC2uASymAlg),
                           "Pedestal":("LArPedestalMC","/LAR/ElecCalibMC/Pedestal","LArPedestal",None),
                           "Noise":("LArNoiseMC","/LAR/ElecCalibMC/Noise","LArNoise",LArNoiseSymAlg),
                           "fSampl":("LArfSamplMC","/LAR/ElecCalibMC/fSampl","LArfSampl",LArfSamplSymAlg),
                           "uA2MeV":("LAruA2MeVMC","/LAR/ElecCalibMC/uA2MeV","LAruA2MeV", LAruA2MeVSymAlg),
                           "MinBias":("LArMinBiasMC","/LAR/ElecCalibMC/MinBias","LArMinBias",LArMinBiasSymAlg),
                           "Shape":("LArShape32MC","/LAR/ElecCalibMC/Shape","LArShape",LArShapeSymAlg),
                           "MinBiasAvc":("LArMinBiasAverageMC","/LAR/ElecCalibMC/MinBiasAverage","LArMinBiasAverage",LArMinBiasAverageSymAlg),
                           "MphysOverMcal":("LArMphysOverMcalMC","/LAR/ElecCalibMC/MphysOverMcal","LArMphysOverMcal",LArMPhysOverMcalSymAlg),
                           "HVScaleCorr" : ("LArHVScaleCorrComplete", '/LAR/ElecCalibMC/HVScaleCorr',"LArHVScaleCorr",None)
                       }

    result=ComponentAccumulator()
    #Add cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))
    LArMCSymCondAlg=CompFactory.LArMCSymCondAlg
    result.addCondAlgo(LArMCSymCondAlg(ReadKey="LArOnOffIdMap"))
    folderlist=[]
    for folder in folders:
        try:
            classname,fldr,key,calg=_larCondDBFoldersMC[folder]
        except KeyError:
            raise ConfigurationError("No conditions data %s found for Monte Carlo" % folder)

        folderlist+=[(fldr,detdb,classname),]
        if calg is not None:
            result.addCondAlgo(calg(ReadKey=key,WriteKey=key+"Sym"))

    result.merge(addFolderList(flags,folderlist,db="OFLP200"))
    return result


def LArElecCalibDBMCSCCfg(flags,folders,detdb="LAR_OFL"):
    _larCondDBFoldersMC = {
                           "RampSC":('CondAttrListCollection',"/LAR/ElecCalibMCSC/Ramp","LArRampSC", LArRampSCCondAlg ),
                           "DAC2uASC":('CondAttrListCollection',"/LAR/ElecCalibMCSC/DAC2uA","LArDAC2uASC",LArDAC2uASCCondAlg),
                           "uA2MeVSC":('CondAttrListCollection',"/LAR/ElecCalibMCSC/uA2MeV","LAruA2MeVSC",LAruA2MeVSCCondAlg),
                           "fSamplSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/fSampl',"LArfSamplSC",LArfSamplSCCondAlg),
                           "ShapeSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/Shape',"LArShapeSC",LArShapeSCCondAlg),
                           "PileupAverageSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/LArPileupAverage',"LArPileupAverageSC",LArPileupAverageSCCondAlg),
                           "PedestalSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/Pedestal',"LArPedestalSC",LArPedestalSCCondAlg),
                           "NoiseSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/Noise',"LArNoiseSC",LArNoiseSCCondAlg),
                           "MinBiasSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/MinBias',"LArMinBiasSC",LArMinBiasSCCondAlg),
                           "AutoCorrSC":('CondAttrListCollection','/LAR/ElecCalibMCSC/AutoCorr',"LArAutoCorrSC",LArAutoCorrSCCondAlg)
                       }

    result=ComponentAccumulator()
    folderlist=[]
    for folder in folders:
        try:
            classname,fldr,key,calg=_larCondDBFoldersMC[folder]
        except KeyError:
            raise ConfigurationError("No conditions data %s found for Monte Carlo" % folder)

        folderlist+=[(fldr,detdb,classname),]
        if calg is not None:
            result.addCondAlgo(calg(ReadKey=fldr,WriteKey=key))

    result.merge(addFolderList(flags,folderlist,db="OFLP200"))
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- run2')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.lock()
    acc1 = LArElecCalibDBCfg (flags1, ['Ramp', 'Pedestal'])
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- run1')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.RAW_RUN2
    from AthenaConfiguration.Enums import LHCPeriod
    flags2.GeoModel.Run=LHCPeriod.Run1
    flags2.lock()
    acc2 = LArElecCalibDBCfg (flags2, ['Ramp', 'Pedestal'])
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getService('IOVDbSvc').Folders)
    acc2.wasMerged()

    print ('--- mc')
    flags3 = initConfigFlags()
    flags3.Input.Files = defaultTestFiles.ESD
    flags3.lock()
    acc3 = LArElecCalibDBCfg (flags3, ['Ramp', 'Pedestal'])
    acc3.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc3.getService('IOVDbSvc').Folders)
    acc3.wasMerged()
    
