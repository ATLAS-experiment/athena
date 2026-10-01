/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMONITORING_OBJTRACKVISUALIZATIONALG_H
#define ACTSMONITORING_OBJTRACKVISUALIZATIONALG_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "ActsEvent/ContextUtility.h"
namespace ActsTrk{
    /** @brief Dump each track trajectory into an obj file */
    class ObjTrackVisualizationAlg : public AthAlgorithm {
        public:
            using AthAlgorithm::AthAlgorithm;
            virtual StatusCode execute(const EventContext& ctx) override final;
            virtual StatusCode initialize() override final;
        private:
            ContextUtility m_ctxProvider{this};
            /** @brief The track particle container to be dumped */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_readKey{this, "ReadKey", "InDetTrackParticles"};
            /** @brief Track extrapolation tool */
            ToolHandle<IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            Gaudi::Property<std::string> m_outPath{this, "outPath", "ObjPlots"};


    };
}



#endif