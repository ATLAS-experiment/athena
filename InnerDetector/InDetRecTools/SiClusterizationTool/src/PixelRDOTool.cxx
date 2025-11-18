/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelRDOTool.cxx
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "SiClusterizationTool/PixelRDOTool.h"
#include "SiClusterizationTool/ClusterMakerTool.h"
#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetIdentifier/PixelID.h"
#include "TrkSurfaces/RectangleBounds.h"
#include <boost/container/flat_set.hpp>

namespace InDet
{

  PixelRDOTool::PixelRDOTool(const std::string &type,
      const std::string &name,
      const IInterface *parent) :
      AthAlgTool(type,name,parent)
  {
  }

  StatusCode PixelRDOTool::initialize()
  {
    ATH_CHECK(m_pixelDetEleCollKey.initialize());

    bool disable_smry = 
	!m_useModuleMap ||
	(!m_pixelDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED);

    ATH_CHECK(m_summaryTool.retrieve(DisableTool{disable_smry}));

    ATH_CHECK( m_pixelDetElStatus.initialize( !m_pixelDetElStatus.empty()) );
    ATH_CHECK( detStore()->retrieve(m_pixelId, "PixelID") );

    return StatusCode::SUCCESS;
  }


  std::optional<Identifier>
  PixelRDOTool::isGanged(const Identifier& rdoID,
                         const InDetDD::SiDetectorElement* element)
  {
    // If the pixel is ganged, returns a new identifier for it
    InDetDD::SiCellId cellID = element->cellIdFromIdentifier(rdoID);
    if (element->numberOfConnectedCells(cellID) > 1) {
      InDetDD::SiCellId gangedCellID = element->connectedCell(cellID,1);
      return element->identifierFromCellId(gangedCellID);
    } 
    return {};
  }
  
  bool PixelRDOTool::isGoodRDO(const InDet::SiDetectorElementStatus *pixelDetElStatus,
		 const IdentifierHash& moduleHash,
		 const InDetDD::PixelModuleDesign &design,
		 const Identifier& rdoID, const EventContext& ctx,
                 const IInDetConditionsTool::IDCCacheEntry* cacheEntry) const
  {

    std::array<InDetDD::PixelDiodeTree::CellIndexType,2> diode_idx
       = InDetDD::PixelDiodeTree::makeCellIndex(m_pixelId->phi_index(rdoID),
                                                m_pixelId->eta_index(rdoID));
    InDetDD::PixelDiodeTree::DiodeProxyWithPosition si_param ( design.diodeProxyFromIdxCachePosition(diode_idx));
    std::uint32_t feValue = design.getFE(si_param);
    
    VALIDATE_STATUS_ARRAY(
      m_useModuleMap && pixelDetElStatus,
      pixelDetElStatus ? pixelDetElStatus->isChipGood(moduleHash,feValue) : false,
      m_summaryTool->isGood(moduleHash, rdoID, ctx, cacheEntry));

    if (!m_useModuleMap) {
      return true;
    }
    
    if (pixelDetElStatus) {
      
      return pixelDetElStatus->isChipGood(moduleHash, feValue);
    } else {
      return m_summaryTool->isGood(moduleHash, rdoID, ctx, cacheEntry);
    }
  }
  
  
  bool PixelRDOTool::checkDuplication(const PixelID& pixelID,
				      const Identifier& rdoID, 
				      const int lvl1, 
				      std::vector<UnpackedPixelRDO>& collectionID) const
  {
    if (!m_checkDuplicatedRDO)
      return false;

    auto isDuplicate = [&pixelID,rdoID](const UnpackedPixelRDO& rc) -> bool {
	return (pixelID.phi_index(rdoID) == pixelID.phi_index(rc.ID)) &&
      	       (pixelID.eta_index(rdoID) == pixelID.eta_index(rc.ID));
    };
    
    const auto pDuplicate = std::find_if(collectionID.begin(), collectionID.end(),isDuplicate); 
    const bool foundDuplicate {pDuplicate != collectionID.end()}; 
    if (foundDuplicate)
      pDuplicate->LVL1 = std::max(pDuplicate->LVL1, lvl1); 

    return foundDuplicate;
  }                                            


  const InDet::SiDetectorElementStatus* PixelRDOTool::getPixelDetElStatus(const EventContext& ctx) const
  {
    if (m_pixelDetElStatus.empty()) {
      return nullptr;
    }

    SG::ReadHandle<InDet::SiDetectorElementStatus> status = SG::makeHandle<InDet::SiDetectorElementStatus>(m_pixelDetElStatus, ctx);
    if (!status.isValid()) {
      std::stringstream msg;
      msg << "Failed to get " << m_pixelDetElStatus.key() << " from StoreGate in " << name();
      throw std::runtime_error(msg.str());
    }
    return status.cptr();
  }



  const InDetDD::SiDetectorElement*
  PixelRDOTool::checkCollection(const InDetRawDataCollection<PixelRDORawData> &collection, const EventContext& ctx) const
  {
    const unsigned int RDO_size = collection.size();
    if ( RDO_size==0) {
        ATH_MSG_DEBUG("Empty RDO collection");
        return nullptr;
    }

    IdentifierHash idHash = collection.identifyHash();

    const InDet::SiDetectorElementStatus *pixelDetElStatus = getPixelDetElStatus(ctx);
    if (pixelDetElStatus){
      // If module is bad, do not create a cluster collection
      VALIDATE_STATUS_ARRAY (
        m_useModuleMap,
        pixelDetElStatus->isGood(idHash),
        m_summaryTool->isGood(idHash,ctx)
      );
    }
    //
    if (m_useModuleMap && (pixelDetElStatus ? !pixelDetElStatus->isGood(idHash): !(m_summaryTool->isGood(idHash, ctx)))) {
      return nullptr;
    }

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
    const InDetDD::SiDetectorElementCollection* pixelDetEle(*pixelDetEleHandle);
    if (not pixelDetEleHandle.isValid() or pixelDetEle == nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return nullptr;
    }

    const InDetDD::SiDetectorElement* element = pixelDetEle->getDetectorElement(idHash);
    const Trk::RectangleBounds *mybounds =
      dynamic_cast<const Trk::RectangleBounds *>(&element->surface().bounds());
    if (not mybounds) {
      ATH_MSG_ERROR("Dynamic cast failed at "<<__LINE__<<" of PixelRDOTool.cxx.");
      return nullptr;
    }
 
    return element;
  }

  std::vector<UnpackedPixelRDO>
  PixelRDOTool::getUnpackedPixelRDOs(const InDetRawDataCollection<PixelRDORawData> &collection,
				     const PixelID& pixelID,
				     const InDetDD::SiDetectorElement* element,
				     const EventContext& ctx,
				     int defaultLabel) const
  {
    std::vector<UnpackedPixelRDO> unpacked;
    unpacked.reserve(collection.size());
    
    const IdentifierHash idHash = collection.identifyHash();
    const InDet::SiDetectorElementStatus *pixelDetElStatus = getPixelDetElStatus(ctx);
    IInDetConditionsTool::IDCCacheEntry* cacheEntry = (pixelDetElStatus ? nullptr : m_summaryTool->getCacheEntryOut(ctx));
    const InDetDD::PixelModuleDesign &pixel_design=dynamic_cast<const InDetDD::PixelModuleDesign &>(element->design());

    // For ttbar200 we have ~70 RDO in average per element, with a max of ~1000.
    // A flat set brings marginal improvements wrt unordered set for those sizes.
    //std::unordered_set<Identifier> > idset;

    // This is not used for ITk, still we are creating it for the moment and could be optimized away in the future
    boost::container::flat_set<Identifier> idset;

    for(const auto *const rdo : collection) {
      const Identifier rdoID = rdo->identify();
      
      if (!isGoodRDO(pixelDetElStatus,
		     idHash,
		     pixel_design,
		     rdoID,
		     ctx,
		     cacheEntry))
	continue;
      
      const int lvl1 = rdo->getLVL1A();
      
      if (!m_isITk) {
	
	if (not idset.insert(rdoID).second) {
	  if (m_printDuplicate) ATH_MSG_WARNING("Discarded a duplicated RDO");
	  continue;
	}
	
	if (m_checkDuplicatedRDO) {
	  if (checkDuplication(pixelID, rdoID, lvl1, unpacked))
	    continue;
	}
      }
      
      const int tot = rdo->getToT();

      unpacked.emplace_back(
	  defaultLabel,
	  pixelID.phi_index(rdoID),
	  pixelID.eta_index(rdoID),
	  tot,
	  lvl1,
	  rdoID
	
      );

      if (m_checkGanged) {
	std::optional<Identifier> gangedID = isGanged(rdoID, element);
	if (gangedID.has_value()) {
	  unpacked.emplace_back(
	      defaultLabel,
	      pixelID.phi_index(*gangedID),
	      pixelID.eta_index(*gangedID),
	      tot,
	      lvl1,
	      *gangedID
	    
	  );
	}
      }
    }
    return unpacked;
  }



}
