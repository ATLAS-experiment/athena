/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexTowerBuilder  -  description
//                              -------------------
//         Builds a gFexTowerContainer from CaloCellContainer (for supercells)
//                  TriggerTowerContainer (for ppm tile towers)
//         Information about SCellContainer objects are in:
//         - https://gitlab.cern.ch/atlas/athena/-/blob/22.0/Calorimeter/CaloEvent/CaloEvent/CaloCell.h
//
//     begin                : 22 04 2025
//     email                : jared.little@cern.ch
//***************************************************************************/

#ifndef gFexTowerBuilder_H
#define gFexTowerBuilder_H

#include "AsgTools/ToolHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerAuxContainer.h"
#include "xAODTrigL1Calo/gFexTowerContainer.h"

namespace LVL1 {

    class gFexTowerBuilder : public AthReentrantAlgorithm {
      public:
        gFexTowerBuilder(const std::string& name, ISvcLocator* svc);

        /// Function initialising the algorithm
        virtual StatusCode initialize() override;
        /// Function executing the algorithm
        virtual StatusCode execute(const EventContext&) const override;

      private:
        // ------------------------- Properties --------------------------------------
        // Readhandle for Scell container
        SG::ReadHandleKey<CaloCellContainer> m_SCellKey{this, "SCell", "SCell",
                                                        "SCell container"};

        // Readhandle for TriggerTower container
        SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_triggerTowerKey{
                this, "xODTriggerTowers", "xAODTriggerTowers",
                "xAODTriggerTowers container"};

        // Writehandle for EmulatedTowers container
        SG::WriteHandleKey<xAOD::gFexTowerContainer> m_gTowersWriteKey{
                this, "gTowersWriteKey", "L1_gFexDataTowers",
                "Write gFexEDM Trigger Tower container"};

        // FiberMapping property required by the interface
        Gaudi::Property<std::string> m_FiberMapping{
                this, "gFexFiberTowerMapping",
                // PathResolver::find_calib_file("L1CaloFEXAlgos/gFexFibrTowerMap.txt"),
                PathResolver::find_calib_file("Run3L1CaloSimulation/L1CaloFEXAlgos/gFEX/gFex_gCaloTowerMap_weighted_v1.txt"),
                "Text file to convert from hardware fiber to eta-phi location"};

        // property for gFEX mapping
        Gaudi::Property<bool> m_apply_masking{this, "SCellMasking", true,
                                              "Applies masking. Only use for data"};
        Gaudi::Property<bool> m_isDATA{
                this, "isDATA", true,
                "Tells the algorithm if it is data. Used for xAOD::TriggerTower WARNING "
                "due to the 0 supression"};

        Gaudi::Property<std::string> m_gFEX2Scellmapping{
                this, "gFEX2SCmapping",
                "Run3L1CaloSimulation/L1CaloFEXAlgos/gFEX/gCaloTowers_to_scells_v1.txt",
                "Text file to convert from simulation ID to SuperCell Identifier"};

        Gaudi::Property<std::string> m_gFEX2Tilemapping{
                this, "gFEX2Tilemapping",
                "Run3L1CaloSimulation/L1CaloFEXAlgos/gFEX/gCaloTowers_to_tile_v1.txt",
                "Text file to convert from simulation ID to Tile Identifier"};


        // Read mapping functions
        StatusCode ReadFibersfromFile(const std::string&);
        StatusCode ReadTilefromFile(const std::string&);
        StatusCode ReadSCfromFile(const std::string&);

        bool isBadSCellID(const std::string&) const;

        std::unordered_map<uint32_t, std::vector<uint64_t> > m_map_TTower2SCells;
        std::unordered_map<uint32_t, std::vector<uint32_t> > m_map_TTower2Tile;

        std::unordered_map<unsigned int, std::array<float, 4> > m_Firm2Tower_map;  /// {map index(towerid), {fpga, eta, phi, source}}

    };
}  // namespace LVL1
#endif