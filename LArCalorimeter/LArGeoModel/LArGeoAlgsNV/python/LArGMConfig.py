# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AtlasGeoModel.GeoModelConfig import GeoModelCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep, Project
from IOVDbSvc.IOVDbSvcConfig import addFolders

def LArGMCfg(configFlags):
    
    result=GeoModelCfg(configFlags)

    doAlignment=configFlags.LAr.doAlign
    activateCondAlgs = configFlags.Common.Project is not Project.AthSimulation
    tool = CompFactory.LArDetectorToolNV(ApplyAlignments=doAlignment, EnableMBTS=configFlags.Detector.GeometryMBTS)
    if configFlags.Common.ProductionStep != ProductionStep.Simulation and configFlags.Common.ProductionStep != ProductionStep.FastChain:
        tool.GeometryConfig = "RECO"

    result.getPrimary().DetectorTools += [ tool ]

    if doAlignment:
        if configFlags.Input.isMC:
            #Monte Carlo case:
            if activateCondAlgs:
                result.merge(addFolders(configFlags,"/LAR/Align","LAR_OFL",className="DetCondKeyTrans"))
                result.merge(addFolders(configFlags,"/LAR/LArCellPositionShift","LAR_OFL",className="CaloRec::CaloCellPositionShift"))
            else:
                result.merge(addFolders(configFlags,"/LAR/Align","LAR_OFL"))
                result.merge(addFolders(configFlags,"/LAR/LArCellPositionShift","LAR_OFL"))
        else:
            result.merge(addFolders(configFlags,"/LAR/Align","LAR_ONL",className="DetCondKeyTrans"))
            result.merge(addFolders(configFlags,"/LAR/LArCellPositionShift","LAR_ONL",className="CaloRec::CaloCellPositionShift"))

        if activateCondAlgs:
            result.addCondAlgo(CompFactory.LArAlignCondAlg())
            result.addCondAlgo(CompFactory.CaloAlignCondAlg())
            AthReadAlg_ExtraInputs = set()
            caloCellsInInput = "CaloCellContainer" in [i.split('#')[0] for i in flags.Input.TypedCollections]
            sCellsInInput = False
            caloCellKeys = []
            if caloCellsInInput:
                from SGComps.AddressRemappingConfig import AddressRemappingCfg
                result.merge(AddressRemappingCfg())

                caloCellKeys = [i.split('#')[1] for i in configFlags.Input.TypedCollections if "CaloCellContainer"==i.split('#')[0] ]
                for key in caloCellKeys:
                    if key != 'AllCalo':
                        sCellsInInput = True

            AthReadAlg_ExtraInputs.add(('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager'))
            if (flags.GeoModel.Run >= LHCPeriod.Run3 and flags.Detector.GeometryTile) or sCellsInInput:
                # TODO: avoid depending on Tile in SuperCell alignment
                from TileGeoModel.TileGMConfig import TileGMCfg
                result.merge(TileGMCfg(configFlags))
                result.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())
                AthReadAlg_ExtraInputs.add(('CaloSuperCellDetDescrManager', 'ConditionStore+CaloSuperCellDetDescrManager'))


            if caloCellsInInput:
                for key in caloCellKeys:
                    AthReadAlg=CompFactory.AthReadAlg
                    AthReadAlg_CaloCellCont = AthReadAlg (f'AthReadAlg_{key}',
                                                          Key = f'CaloCellContainer/{key}',
                                                          Aliases = [],
                                                          ExtraInputs = AthReadAlg_ExtraInputs)
                    result.addCondAlgo(AthReadAlg_CaloCellCont)
    else:
        # Build unalinged CaloDetDescrManager instance in the Condition Store
        if activateCondAlgs:
            result.addCondAlgo(CompFactory.CaloAlignCondAlg(LArAlignmentStore="",CaloCellPositionShiftFolder=""))
            if flags.GeoModel.Run >= LHCPeriod.Run3 and flags.Detector.GeometryTile and flags.Common.ProductionStep != ProductionStep.Overlay:
                # TODO: avoid depending on Tile in SuperCell alignment
                from TileGeoModel.TileGMConfig import TileGMCfg
                result.merge(TileGMCfg(configFlags))
                result.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())
            
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    ConfigFlags.Input.Files = defaultTestFiles.RAW_RUN2
    ConfigFlags.lock()

    acc = LArGMCfg(ConfigFlags)
    f=open('LArGMCfg.pkl','wb')
    acc.store(f)
    f.close()
