/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPrepRawDataProviderTools/MuonLayerHashProviderTool.h"


#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"

#include "MuonDetDescrUtils/MuonSectorMapping.h"

namespace Muon {
  using namespace MuonStationIndex;
  MuonLayerHashProviderTool::MuonLayerHashProviderTool(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type,name,parent) {
    declareInterface<MuonLayerHashProviderTool>(this);

  }

  StatusCode MuonLayerHashProviderTool::initialize() {

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_detMgrKey.initialize());

    if( !initializeSectorMapping() ){
      ATH_MSG_ERROR("Failed to initialize sector mapping");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }


  void MuonLayerHashProviderTool::insertHash( const IdentifierHash& hash, const Identifier& id ) const{
    insertHash(m_idHelperSvc->sector(id),hash,id);
  }

  void MuonLayerHashProviderTool::insertHash( int sector, const IdentifierHash& hash, const Identifier& id ) const {
    const int techIndex = toInt(m_idHelperSvc->technologyIndex(id));
    const int layHash = sectorLayerHash(m_idHelperSvc->regionIndex(id),m_idHelperSvc->layerIndex(id));
    m_regionHashesPerSector[sector-1].technologyRegionHashVecs[techIndex][layHash].push_back(hash);
  }

  void MuonLayerHashProviderTool::insertTechnology( const MuonIdHelper& idHelper ){

    // loop over all available STGC collection identifiers and order them per sector
    MuonIdHelper::const_id_iterator it = idHelper.module_begin();
    MuonIdHelper::const_id_iterator it_end = idHelper.module_end();
    for( ;it!=it_end; ++it ){
      IdentifierHash hash;
      idHelper.get_module_hash(*it,hash);
      insertHash(hash,*it);
    }

  }

  void MuonLayerHashProviderTool::insertTgcs() const {
    std::lock_guard guard{m_tgcHash};
    if (m_hashLoaded) return;
    // the tgc's can be in multiple sectors so we need to do something special here
    SG::ReadCondHandle detMgr{m_detMgrKey};
  
    MuonSectorMapping sectorMapping;

    // loop over all available TGC collection identifiers and order them per sector
    MuonIdHelper::const_id_iterator it = m_idHelperSvc->tgcIdHelper().module_begin();
    MuonIdHelper::const_id_iterator it_end = m_idHelperSvc->tgcIdHelper().module_end();
    for( ;it!=it_end; ++it ){
      const MuonGM::TgcReadoutElement* detEl = detMgr->getTgcReadoutElement(*it);
      if( !detEl ) {
        ATH_MSG_DEBUG(" No detector element found for " << m_idHelperSvc->toString(*it) );
        continue;
      }
      const IdentifierHash hash = m_idHelperSvc->moduleHash(*it);
      int nstrips = detEl->nStrips(1);
      Amg::Vector3D p1 = detEl->channelPos(1,1,1);
      Amg::Vector3D p2 = detEl->channelPos(1,1,nstrips);
      std::vector<int> sectors1;
      sectorMapping.getSectors(p1.phi(),sectors1);
      std::set<int> added;
      std::vector<int>::iterator sit = sectors1.begin();
      std::vector<int>::iterator sit_end = sectors1.end();
      for( ;sit!=sit_end; ++sit ){
        insertHash(*sit,hash,*it);
        added.insert(*sit);
      }

      std::vector<int> sectors2;
      sectorMapping.getSectors(p2.phi(),sectors2);
      sit = sectors2.begin();
      sit_end = sectors2.end();
      for( ;sit!=sit_end; ++sit ){
        if( added.count(*sit) ) continue;
        added.insert(*sit);
        insertHash(*sit,hash,*it);
      }
    }
    m_hashLoaded = true;
  }

  // all chambers are mapped onto a layer and sector map
  bool MuonLayerHashProviderTool::initializeSectorMapping() {
    // set sector numbers
    for( unsigned int i=0;i<m_regionHashesPerSector.size();++i ) {
      m_regionHashesPerSector[i].sector=i+1;      
    }
    ATH_MSG_DEBUG("Initializing hashes: number of sectors " << numberOfSectors() 
                 << " sectorLayers " << sectorLayerHashMax() );

    // add technologies
    if (m_idHelperSvc->hasMDT()) insertTechnology(m_idHelperSvc->mdtIdHelper());
    if (m_idHelperSvc->hasRPC()) insertTechnology(m_idHelperSvc->rpcIdHelper());
    if (m_idHelperSvc->hasCSC()) insertTechnology(m_idHelperSvc->cscIdHelper());
    if (m_idHelperSvc->hasMM()) insertTechnology(m_idHelperSvc->mmIdHelper());
    if (m_idHelperSvc->hasSTGC()) insertTechnology(m_idHelperSvc->stgcIdHelper());
    return true;
  }

}
