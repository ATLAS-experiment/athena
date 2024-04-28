/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CacheCreator.h"

namespace ActsTrk::Cache {
    CreatorAlg::CreatorAlg(const std::string &name,ISvcLocator *pSvcLocator):IDCCacheCreatorBase(name,pSvcLocator)
    {
    }

    StatusCode CreatorAlg::initialize(){
        ATH_CHECK(m_pixelClusterCacheKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_stripClusterCacheKey.initialize(SG::AllowEmpty));

        ATH_CHECK(m_pixelSPCacheKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_stripSPCacheKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_stripOSPCacheKey.initialize(SG::AllowEmpty));

        m_do_pixClusters = !m_pixelClusterCacheKey.key().empty();
        m_do_stripClusters = !m_stripClusterCacheKey.key().empty();

        m_do_pixSP = !m_pixelSPCacheKey.key().empty();
        m_do_stripSP = !m_stripSPCacheKey.key().empty();

        ATH_CHECK(detStore()->retrieve(m_pix_idHelper, "PixelID"));
        ATH_CHECK(detStore()->retrieve(m_strip_idHelper, "SCT_ID"));
        
        return StatusCode::SUCCESS;
    }

    StatusCode CreatorAlg::execute(const EventContext& ctx) const{
        if(m_do_pixClusters){
            ATH_CHECK(createContainer(m_pixelClusterCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));
        }

        if(m_do_stripClusters){
            ATH_CHECK(createContainer(m_stripClusterCacheKey, m_strip_idHelper->wafer_hash_max(), ctx));
        }

        if(m_do_pixSP){
            ATH_CHECK(createContainer(m_pixelSPCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));
        }

        if(m_do_stripSP){
            ATH_CHECK(createContainer(m_stripSPCacheKey, m_strip_idHelper->wafer_hash_max(), ctx));
            ATH_CHECK(createContainer(m_stripOSPCacheKey, m_strip_idHelper->wafer_hash_max(), ctx));
	    ATH_MSG_DEBUG("created strip sp containers");
        }

        return StatusCode::SUCCESS;
    }
}
