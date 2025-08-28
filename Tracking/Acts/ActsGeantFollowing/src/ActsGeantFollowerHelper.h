/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ActsGeantFollowerHelper_H
#define ActsGeantFollowerHelper_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkParameters/TrackParameters.h" //typedef, can't fwd declare
#include "IActsGeantFollowerHelper.h"
#include "G4ThreeVector.hh" //typedef, can't fwd declare
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Propagator/SurfaceCollector.hpp"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "TrkExInterfaces/IExtrapolationEngine.h"
// FIXME: header-global macro for an array size constant!
#ifndef MAXPROBES
#define MAXPROBES 50000
#endif

class TTree;

namespace Trk {
  class IExtrapolator;
} // namespace Trk


class ActsGeantFollowerHelper : public extends<AthAlgTool, IActsGeantFollowerHelper> {

  public:
    // constructor, destructor
    ActsGeantFollowerHelper(const std::string&,const std::string&,const IInterface*);
    virtual ~ActsGeantFollowerHelper() = default;

    // Athena hooks
    virtual StatusCode initialize() override;
    virtual StatusCode finalize  () override;

    // Follower interface
    // a) begin event - initialize follower process
    virtual void beginEvent() override;
    // b) track the particle
    virtual void trackParticle(const G4ThreeVector& pos, const G4ThreeVector& mom, int pdg, double charge, float t, float X0, bool isSensitive) override;
    // c) end event - ntuple writing
    virtual void endEvent() override;

  private:

    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};


    ToolHandle<Trk::IExtrapolationEngine>     m_extrapolationEngine{this, "ExtrapolationEngine", ""};
    ToolHandle<IActsExtrapolationTool>   m_actsExtrapolator{this, "ActsExtrapolator", ""};
    Gaudi::Property<bool> m_extrapolateDirectly{this, "ExtrapolateDirectly", true};
    Gaudi::Property<bool> m_extrapolateIncrementally{this, "ExtrapolateIncrementally", true};

    Trk::TrackParameters* m_parameterCache{nullptr};
    std::optional<Acts::BoundTrackParameters> m_actsParameterCache{std::nullopt};
    std::unique_ptr<std::vector<Acts::SurfaceHit>> m_actsSurfaceCache{};
    std::vector<Acts::SurfaceHit>::iterator m_actsSurfaceIterator{};
    // Hypothesis to pdg converter
    Trk::PdgToParticleHypothesis m_pdgToParticleHypothesis;
    float m_tX0Cache{0.f};
    float m_tX0NonSensitiveCache{0.f};
    float m_tNonSensitiveCache{0.f};
    float m_tX0CacheActs{0.f};
    float m_tX0CacheATLAS{0.f};

    // put some validation code is
    std::string                    m_validationTreeName;        //!< validation tree name - to be acessed by this from root
    std::string                    m_validationTreeDescription; //!< validation tree description - second argument in TTree
    std::string                    m_validationTreeFolder;      //!< stream/folder to for the TTree to be written out

    TTree*                         m_validationTree{nullptr};            //!< Root Validation Tree
    /** Ntuple variables : initial parameters
        Split this out into a separate, dynamically-allocated block.
        Otherwise, the CaloCellNoiseAlg is so large that it violates
        the ubsan sanity checks. **/
    struct TreeData {
        float                  m_t_x {0};
        float                  m_t_y {0};
        float                  m_t_z {0};
        float                  m_t_theta {0};
        float                  m_t_eta {0};
        float                  m_t_phi {0};
        float                  m_t_p {0};
        float                  m_t_charge {0};
        int                    m_t_pdg {0};
        /** Ntuple variables : g4 step parameters */
        int                    m_g4_steps {0};
        float                  m_g4_pt[MAXPROBES] {0};
        float                  m_g4_eta[MAXPROBES] {0};
        float                  m_g4_theta[MAXPROBES] {0};
        float                  m_g4_phi[MAXPROBES] {0};
        float                  m_g4_x[MAXPROBES] {0};
        float                  m_g4_y[MAXPROBES] {0};
        float                  m_g4_z[MAXPROBES] {0};
        float                  m_g4_tX0[MAXPROBES] {0};
        float                  m_g4_accX0[MAXPROBES] {0};
        float                  m_g4_t[MAXPROBES] {0};
        float                  m_g4_X0[MAXPROBES] {0};
        /** Ntuple variables : trk follow up parameters */
        int                    m_trk_status[MAXPROBES] {0};
        float                  m_trk_pt[MAXPROBES] {0};
        float                  m_trk_eta[MAXPROBES] {0};
        float                  m_trk_theta[MAXPROBES] {0};
        float                  m_trk_phi[MAXPROBES] {0};
        float                  m_trk_x[MAXPROBES] {0};
        float                  m_trk_y[MAXPROBES] {0};
        float                  m_trk_z[MAXPROBES] {0};
        float                  m_trk_lx[MAXPROBES] {0};
        float                  m_trk_ly[MAXPROBES] {0};
        float                  m_trk_tX0[MAXPROBES] {0};
        float                  m_trk_accX0[MAXPROBES] {0};
        float                  m_trk_t[MAXPROBES] {0};
        float                  m_trk_X0[MAXPROBES] {0};
        /** Ntuple variables : acts follow up parameters */
        int                    m_acts_status[MAXPROBES] {0};
        int                    m_acts_volumeID[MAXPROBES] {0};
        float                  m_acts_pt[MAXPROBES] {0};
        float                  m_acts_eta[MAXPROBES] {0};
        float                  m_acts_theta[MAXPROBES] {0};
        float                  m_acts_phi[MAXPROBES] {0};
        float                  m_acts_x[MAXPROBES] {0};
        float                  m_acts_y[MAXPROBES] {0};
        float                  m_acts_z[MAXPROBES] {0};
        float                  m_acts_tX0[MAXPROBES] {0};
        float                  m_acts_accX0[MAXPROBES] {0};
        float                  m_acts_t[MAXPROBES] {0};
        float                  m_acts_X0[MAXPROBES] {0};
    };
    std::unique_ptr<TreeData> m_treeData;
};

#endif
