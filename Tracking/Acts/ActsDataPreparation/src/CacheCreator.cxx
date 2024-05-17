/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CacheCreator.h"

namespace ActsTrk::Cache {
    CreatorAlg::CreatorAlg(const std::string &name,
			   ISvcLocator *pSvcLocator)
      : IDCCacheCreatorBase(name,pSvcLocator)
    {}

    StatusCode CreatorAlg::initialize() {
        ATH_MSG_DEBUG("Initializing " << name() << " ...");
	
        m_do_pixClusters = not m_pixelClusterCacheKey.empty();
        m_do_stripClusters = not m_stripClusterCacheKey.empty();

        m_do_pixSpacePoints = not m_pixelSPCacheKey.empty();
        m_do_stripSpacePoints = not m_stripSPCacheKey.empty();
        m_do_stripOverlapSpacePoints = not m_stripOSPCacheKey.empty();
	
        ATH_CHECK(m_pixelClusterCacheKey.initialize(m_do_pixClusters));
        ATH_CHECK(m_stripClusterCacheKey.initialize(m_do_stripClusters));

        ATH_CHECK(m_pixelSPCacheKey.initialize(m_do_pixSpacePoints));
        ATH_CHECK(m_stripSPCacheKey.initialize(m_do_stripSpacePoints));
        ATH_CHECK(m_stripOSPCacheKey.initialize(m_do_stripOverlapSpacePoints));

        ATH_CHECK(detStore()->retrieve(m_pix_idHelper, "PixelID"));
        ATH_CHECK(detStore()->retrieve(m_strip_idHelper, "SCT_ID"));
        
        return StatusCode::SUCCESS;
    }

    StatusCode CreatorAlg::execute(const EventContext& ctx) const {
      ATH_MSG_DEBUG("Executing " << name() << " ...");
      
        if (m_do_pixClusters) {
            ATH_CHECK(createContainer(m_pixelClusterCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));
	    ATH_MSG_DEBUG("created pixel cluster container");
        }

        if (m_do_stripClusters) {
            ATH_CHECK(createContainer(m_stripClusterCacheKey, m_strip_idHelper->wafer_hash_max(), ctx));
	    ATH_MSG_DEBUG("created strip cluster container");
        }

        if (m_do_pixSpacePoints) {
            ATH_CHECK(createContainer(m_pixelSPCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));
	    ATH_MSG_DEBUG("created pixel space point container");
        }

        if (m_do_stripSpacePoints) {
            ATH_CHECK(createContainer(m_stripSPCacheKey, m_strip_idHelper->wafer_hash_max(), ctx));
	    ATH_MSG_DEBUG("created strip space point container");
        }

	if (m_do_stripOverlapSpacePoints) {
	  ATH_CHECK(createContainer(m_stripOSPCacheKey, m_strip_idHelper->wafer_hash_max(), ctx));
	  ATH_MSG_DEBUG("created overlap strip space point container");
	}

        return StatusCode::SUCCESS;
    }
}
