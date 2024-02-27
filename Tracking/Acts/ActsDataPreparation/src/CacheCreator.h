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
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

namespace ActsTrk::Cache{
    class CreatorAlg: public IDCCacheCreatorBase{
    public:
        CreatorAlg(const std::string &name,ISvcLocator *pSvcLocator);
        virtual StatusCode initialize () override;
        virtual StatusCode execute (const EventContext& ctx) const override;

    protected:		
        SG::WriteHandleKey<Handles<xAOD::PixelCluster>::IDCBackend> m_pixelClusterCacheKey{this, "PixelClustersCacheKey", ""};
        SG::WriteHandleKey<Handles<xAOD::StripCluster>::IDCBackend> m_stripClusterCacheKey{this, "StripClustersCacheKey", ""};

        const PixelID* m_pix_idHelper{};
        const SCT_ID*  m_strip_idHelper{};

        bool m_do_pixSP{false};
        bool m_do_stripSP{false};

        bool m_do_pixClusters{false};
        bool m_do_stripClusters{false};
    };
}

CLASS_DEF(ActsTrk::Cache::Handles<xAOD::PixelCluster>::IDCBackend, 70424203, 1);
CLASS_DEF(ActsTrk::Cache::Handles<xAOD::StripCluster>::IDCBackend, 202232989, 1);
#endif
