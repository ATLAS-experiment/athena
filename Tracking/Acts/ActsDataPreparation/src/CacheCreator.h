/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRACKING_ACTS_CACHECREATOR_H
#define TRACKING_ACTS_CACHECREATOR_H

#include "ViewAlgs/IDCCacheCreatorBase.h"
#include "Cache.h"
#include "StoreGate/WriteHandleKey.h"

#include <InDetIdentifier/PixelID.h>
#include <InDetIdentifier/SCT_ID.h>
#include "details/PixelClusterCacheId.h"
#include "details/StripClusterCacheId.h"
#include "details/SPCacheId.h"

namespace ActsTrk::Cache{
    class CreatorAlg: public IDCCacheCreatorBase{
    public:
        CreatorAlg(const std::string &name,ISvcLocator *pSvcLocator);
        virtual ~CreatorAlg() = default;

        virtual StatusCode initialize () override;
        virtual StatusCode execute (const EventContext& ctx) const override;

    protected:		
        SG::WriteHandleKey<Handles<xAOD::PixelCluster>::IDCBackend> m_pixelClusterCacheKey{this, "PixelClustersCacheKey", ""};
        SG::WriteHandleKey<Handles<xAOD::StripCluster>::IDCBackend> m_stripClusterCacheKey{this, "StripClustersCacheKey", ""};

        SG::WriteHandleKey<Handles<xAOD::SpacePoint>::IDCBackend> m_pixelSPCacheKey{this, "PixelSpacePointCacheKey", ""};
        SG::WriteHandleKey<Handles<xAOD::SpacePoint>::IDCBackend> m_stripSPCacheKey{this, "StripSpacePointCacheKey", ""};
        SG::WriteHandleKey<Handles<xAOD::SpacePoint>::IDCBackend> m_stripOSPCacheKey{this, "StripOverlapSpacePointCacheKey", ""};

        const PixelID* m_pix_idHelper{};
        const SCT_ID*  m_strip_idHelper{};

        bool m_do_pixClusters{false};
        bool m_do_stripClusters{false};

        bool m_do_pixSpacePoints{false};
        bool m_do_stripSpacePoints{false};
        bool m_do_stripOverlapSpacePoints{false};
};
}
#endif
