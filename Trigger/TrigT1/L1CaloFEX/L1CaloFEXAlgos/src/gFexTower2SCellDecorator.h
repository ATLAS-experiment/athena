/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GFEXL1CALO2SCELLDECORATORTOOL_H
#define GFEXL1CALO2SCELLDECORATORTOOL_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "PathResolver/PathResolver.h"

#include "xAODTrigL1Calo/gFexTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerAuxContainer.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include <vector>
#include <unordered_map>
#include <string>


namespace LVL1 {
    
class gFexTower2SCellDecorator : public AthReentrantAlgorithm{
    public:
        gFexTower2SCellDecorator(const std::string& name, ISvcLocator* svc);
        
        /// Function initialising the algorithm
        virtual StatusCode initialize() override;
        /// Function executing the algorithm
        virtual StatusCode execute( const EventContext& ) const override;
        
    private:
        // ------------------------- Properties --------------------------------------
        //Readhandle for Scell container
        SG::ReadHandleKey<CaloCellContainer> m_SCellKey {this, "SCell", "SCell", "SCell container"};
        
        //Readhandle for TriggerTower container
        SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_triggerTowerKey {this, "xODTriggerTowers", "xAODTriggerTowers", "xAODTriggerTowers container"};
        
        const std::string m_ReadKey_name = "L1_gFexDataTowers";

        //Readhandle for Scell container
        SG::ReadHandleKey < xAOD::gFexTowerContainer > m_gTowersReadKey    {this,"gTowersReadKey"   ,"L1_gFexDataTowers", "Read gFex EDM Trigger Tower container"};
        
        //WriteDecorHandle
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gSCellEtdecorKey    { this, "gSCellEtdecorKey"    , "L1_gFexDataTowers.SCellEt"      , "SCell Et information of the gTower"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gSCellEtadecorKey   { this, "gSCellEtadecorKey"   , "L1_gFexDataTowers.SCellEta"     , "SCell Eta information of the gTower"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gSCellPhidecorKey   { this, "gSCellPhidecorKey"   , "L1_gFexDataTowers.SCellPhi"     , "SCell Phi information of the gTower"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gSCellIDdecorKey    { this, "gSCellIDdecorKey"    , "L1_gFexDataTowers.SCellID"      , "SCell IDs information of the gTower"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gSCellSampledecorKey{ this, "gSCellSampledecorKey", "L1_gFexDataTowers.SCellSample"  , "SCell Samples information of the gTower"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gtowerEtMeVdecorKey { this, "gtowerEtMeVdecorKey" , "L1_gFexDataTowers.gtowerEtMeV"  , "gFex Tower Et information in MeV"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gTowerEtdecorKey    { this, "gTowerEtdecorKey"    , "L1_gFexDataTowers.SCSumEncoded" , "SCell sum Et. ENCODED!"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gTileEtMeVdecorKey  { this, "gTileEtMeVdecorKey"  , "L1_gFexDataTowers.TileEt"       , "Tile Tower Et information in Encoded from jepET"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gTileEtadecorKey    { this, "gTileEtadecorKey"    , "L1_gFexDataTowers.TileEta"      , "Tile Tower Eta information in MeV"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gTilePhidecorKey    { this, "gTilePhidecorKey"    , "L1_gFexDataTowers.TilePhi"      , "Tile Tower Phi information in MeV"};
        SG::WriteDecorHandleKey<xAOD::gFexTowerContainer> m_gTileIDdecorKey     { this, "gTileIDdecorKey"     , "L1_gFexDataTowers.TileID"       , "Tile Tower ID information"};

        
        //property for gFEX mapping
        Gaudi::Property<std::string> m_gFEX2Scellmapping {this, "gFEX2SCmapping"  , "L1CaloFEXByteStream/gFEX_maps/2023_02_23/gfexSuperCellMap.txt" , "Text file to convert from simulation ID to SuperCell Identifier"};
        Gaudi::Property<std::string> m_gFEX2Tilemapping  {this, "gFEX2Tilemapping", "L1CaloFEXByteStream/gFEX_maps/2023_02_23/gfexTileMap.txt", "Text file to convert from simulation ID to Tile Identifier"};

        StatusCode ReadSCfromFile(const std::string& );
        StatusCode ReadTilefromFile(const std::string& );
        bool isBadSCellID(const std::string&) const;
        
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCells;
        std::unordered_map< uint32_t, std::vector<uint32_t> > m_map_TTower2Tile;

};
}
#endif
