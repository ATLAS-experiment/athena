// This is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_PRIMARYVERTEXREFITTER_H
#define PROMPT_PRIMARYVERTEXREFITTER_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : PrimaryVertexReFitter
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 * @Author : Kees Benkendorfer
 *
 * @Brief  :
 *
 *  Decorate leptons with secondary vertex algorithem output
 *
 **********************************************************************************/

// Local
#include "VertexFittingTool.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/Accessor.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

// xAOD
#include "xAODTracking/VertexContainer.h"

// ROOT
#include "TStopwatch.h"

#include <optional>

namespace Prompt
{
    class PrimaryVertexReFitter: public AthAlgorithm
    {
    public:

        PrimaryVertexReFitter(const std::string& name, ISvcLocator* pSvcLocator);

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;
        virtual StatusCode finalize() override;

    private:

        using accessorFloat_t = SG::Accessor<float>;
        using decoratorHandElemVtx_t = SG::WriteDecorHandle<xAOD::IParticleContainer, ElementLink<xAOD::VertexContainer> >;


        bool decorateLepWithReFitPrimaryVertex(const FittingInput &input,
            const xAOD::TrackParticle* tracklep,
            const xAOD::IParticle *lep,
            const std::vector<const xAOD::TrackParticle*> &tracks,
            xAOD::VertexContainer &refitVtxContainer,
            decoratorHandElemVtx_t& lepRefittedRMVtxLinkDec);


    private:

        //
        // Tools and services:
        //
        ToolHandle<Prompt::VertexFittingTool> m_vertexFitterTool {
            this, "VertexFittingTool", "Prompt::VertexFittingTool/VertexFittingTool"
        };

        //
        // Properties:
        //
        Gaudi::Property<bool> m_printTime {this, "PrintTime", false};

        Gaudi::Property<std::string> m_distToRefittedPriVtxName {
            this, "DistToRefittedPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_normDistToRefittedPriVtxName {
            this, "NormDistToRefittedPriVtxName", "default"
        };

        TStopwatch m_timerAll;
        TStopwatch m_timerExec;

        // Read/write handles
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inDetTracksKey{
            this, "InDetTrackParticlesKey", "InDetTrackParticles"
        };

        SG::ReadHandleKey<xAOD::IParticleContainer> m_leptonContainerKey {
            this,
            "LeptonContainerName",
            "lepContainerNameDefault", "Name of lepton container"
        };
        SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexContainerKey {
            this, "PriVertexContainerName", "PrimaryVertices",
            "Name of primary vertex container"
        };
        SG::WriteHandleKey<xAOD::VertexContainer> m_reFitPrimaryVertexKey {
            this, "ReFitPriVtxName", "default"
        };

        //
        // Accessors/Decorators
        //
        std::optional<accessorFloat_t> m_distToRefittedPriVtx;
        std::optional<accessorFloat_t> m_normdistToRefittedPriVtx;

        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_lepRefittedVtxWithoutLeptonLinkName
          { this, "RefittedVtxWithoutLeptonLinkName", m_leptonContainerKey, "default", "" };
    };
}

#endif // PROMPT_PRIMARYVERTEXREFITTER_H
