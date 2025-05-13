/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONLAYERHASHPROVIDERTOOL_H
#define MUON_MUONLAYERHASHPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <mutex>
#include <string>
#include <vector>
#include <array>


namespace Muon {

  class MuonLayerHashProviderTool :  public AthAlgTool {
  public:
    /** define data structure 
        - sector 
          - technology
            - hashes
    */
    using HashVec = std::vector<IdentifierHash>;
    
    using RegionHashVec = std::array<HashVec, MuonStationIndex::sectorLayerHashMax()>;
    using TechnologyIndex = MuonStationIndex::TechnologyIndex;
    static constexpr unsigned techMax = MuonStationIndex::toInt(TechnologyIndex::TechnologyIndexMax);
    using TechnologyRegionHashVec = std::array< RegionHashVec, techMax>;

    struct RegionHashesPerSector {
      int sector{-1};
      TechnologyRegionHashVec technologyRegionHashVecs{};
    };
    using RegionHashesPerSectorVec =  std::array<RegionHashesPerSector, MuonStationIndex::numberOfSectors()>;

    /** access hashes for a given sector, technology and layer hash */
    const HashVec& getHashes( int sector, TechnologyIndex technologyIndex, unsigned int sectorLayerHash ) const;

    /** Default AlgTool functions */
    MuonLayerHashProviderTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~MuonLayerHashProviderTool()=default;
    StatusCode initialize() override;

    /** @brief access to tool interface */
    static const InterfaceID& interfaceID() { 
      static const InterfaceID IID_MuonLayerHashProviderTool("Muon::MuonLayerHashProviderTool",1,0);
      return IID_MuonLayerHashProviderTool; 
    }


  private:
    
    /** insert hashes of a given technology */
    void insertTechnology( const MuonIdHelper& idHelper );

    /** insert hashes for the tgcs */
    void insertTgcs() const;

    /** insert a single hash for a given identifier */
    void insertHash( const IdentifierHash& hash, const Identifier& id ) const;

    /** insert a single hash for a given identifier and sector */
    void insertHash( int sector, const IdentifierHash& hash, const Identifier& id ) const;

    /** initialize the mapping structure */
    bool initializeSectorMapping();

    /** cachaed hash data structure */
    mutable RegionHashesPerSectorVec m_regionHashesPerSector ATLAS_THREAD_SAFE{};
    mutable std::atomic<bool> m_hashLoaded ATLAS_THREAD_SAFE{false};
    mutable std::mutex m_tgcHash{};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "DetMgr", "MuonDetectorManager", 
                                                                "MuonManager ReadKey for IOV Range intersection"};

    
  };

 
  inline const std::vector<IdentifierHash>& MuonLayerHashProviderTool::getHashes( int sector, TechnologyIndex technologyIndex, 
                                                                                  unsigned int sectorLayerHash ) const {
    if (!m_hashLoaded){
      insertTgcs();
    }
    using namespace MuonStationIndex;
    return m_regionHashesPerSector[sector-1].technologyRegionHashVecs[toInt(technologyIndex)][sectorLayerHash];
  }
}



#endif
