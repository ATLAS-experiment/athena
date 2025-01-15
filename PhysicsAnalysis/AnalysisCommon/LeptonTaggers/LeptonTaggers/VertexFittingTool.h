/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LEPTONTAGGERS_VertexFittingTool_H
#define LEPTONTAGGERS_VertexFittingTool_H

// Local
#include "IVertexFittingTool.h"

// Reconstruction
#include "TrkVertexFitterInterfaces/IVertexFitter.h"

// Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

// xAOD
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"

// ROOT
#include "TStopwatch.h"

namespace Prompt {
    class VertexFittingTool :   public AthAlgTool,
                                virtual public IVertexFittingTool
{
    public:
        VertexFittingTool(const std::string& t, const std::string &name, const IInterface* p);

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

        virtual std::unique_ptr<xAOD::Vertex> fitVertexWithPrimarySeed(
            const FittingInput &input,
            const std::vector<const xAOD::TrackParticle* > &tracks,
            VtxType vtx
        ) override;

        virtual std::unique_ptr<xAOD::Vertex> fitVertexWithSeed(
            const FittingInput &input,
            const std::vector<const xAOD::TrackParticle* > &tracks,
            const Amg::Vector3D& seed,
            VtxType vtxType
        ) override;

        virtual bool isValidVertex(const xAOD::Vertex *vtx) const override;

    private:

        void removeDoubleEntries(std::vector<const xAOD::TrackParticle*> &tracks);

        bool decorateNewSecondaryVertex(const FittingInput &input, xAOD::Vertex *secVtx);

        std::unique_ptr<xAOD::Vertex> getSecondaryVertexWithSeed(
            const std::vector<const xAOD::TrackParticle*> &tracks,
            const xAOD::TrackParticleContainer *inDetTracks,
            const Amg::Vector3D& seed
        );

    private:

        typedef std::unique_ptr<xAOD::Vertex> vtxPtr_t;

    private:

        //
        // Properties:
        //
        ToolHandle<Trk::IVertexFitter> m_vertexFitter {
            this, "vertexFitterTool", "Trk::FastVertexFitter/FastVertexFitterTool"
        };
        ToolHandle<Trk::IVertexFitter> m_seedVertexFitter {
            this, "seedVertexFitterTool",
            "Trk::FastVertexFitter/FastVertexFitterTool"
        };

        Gaudi::Property<bool> m_doSeedVertexFit {
        this, "doSeedVertexFit", false
        };

        Gaudi::Property<std::string> m_distToPriVtxName {
        this, "DistToPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_normDistToPriVtxName {
        this, "NormDistToPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_distToRefittedPriVtxName {
        this, "DistToRefittedPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_normDistToRefittedPriVtxName {
        this, "NormDistToRefittedPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_distToRefittedRmLepPriVtxName {
        this, "DistToRefittedRmLepPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_normDistToRefittedRmLepPriVtxName {
        this, "NormDistToRefittedRmLepPriVtxName", "default"
        };

        //
        // Variables
        //
        TStopwatch                                             m_timer;
        int                                                    m_countNumberOfFits{0};
        int                                                    m_countNumberOfFitsFailed{0};
        int                                                    m_countNumberOfFitsInvalid{0};

        int                                                    m_secondaryVertexIndex{0};

        std::unique_ptr<SG::AuxElement::Decorator<float> >     m_distToPriVtx;
        std::unique_ptr<SG::AuxElement::Decorator<float> >     m_normDistToPriVtx;
        std::unique_ptr<SG::AuxElement::Decorator<float> >     m_distToRefittedPriVtx;
        std::unique_ptr<SG::AuxElement::Decorator<float> >     m_normDistToRefittedPriVtx;
        std::unique_ptr<SG::AuxElement::Decorator<float> >     m_distToRefittedRmLepPriVtx;
        std::unique_ptr<SG::AuxElement::Decorator<float> >     m_normDistToRefittedRmLepPriVtx;
    };
} // namespace DerivationFramework
#endif  // LEPTONTAGGERS_VertexFittingTool_H
