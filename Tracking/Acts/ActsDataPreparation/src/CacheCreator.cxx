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

        m_do_pixClusters = !m_pixelClusterCacheKey.key().empty();
        m_do_stripClusters = !m_stripClusterCacheKey.key().empty();

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

        return StatusCode::SUCCESS;
    }
}