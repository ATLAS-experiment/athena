// -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASSIMTOOL_h
#define ISF_FATRASSIMTOOL_h

//Gaudi
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"  // for ToolHandleArray
#include "CxxUtils/checker_macros.h"

// ISF
#include "ISF_Interfaces/BaseSimulatorTool.h"
#include "ISF_Interfaces/IParticleFilter.h"
#include "ISF_Interfaces/IParticleProcessor.h"

namespace ISF {

  class ATLAS_NOT_THREAD_SAFE FatrasSimTool : public BaseSimulatorTool  {  // deprecated: ATLASSIM-6020
  public:
    FatrasSimTool( const std::string& type, const std::string& name,  const IInterface* parent);

    ~FatrasSimTool();

    virtual StatusCode initialize() override;

    virtual StatusCode simulate( const EventContext& ctx, ISFParticle& isp, ISFParticleContainer&, McEventCollection* ) override;

    virtual StatusCode setupEvent(const EventContext&) override { return StatusCode::SUCCESS; };

    virtual StatusCode releaseEvent(const EventContext&) override { return StatusCode::SUCCESS; };

    virtual ISF::SimulationFlavor simFlavor() const override { return ISF::Fatras; };

  private:

    /** Track Creation & transport */
    PublicToolHandle<ISF::IParticleProcessor>  m_IDsimulationTool{this, "IDSimulationTool", "", ""};   //!< Pointer to the transport AlgTool
    bool                                 m_useExtrapolator{true};  //!< Boolean used to run with the old extrapolator setup
    PublicToolHandle<ISF::IParticleProcessor>  m_simulationTool{this, "SimulationTool", "", ""};   //!< Pointer to the transport AlgTool
    PublicToolHandle<ISF::IParticleFilter>     m_particleFilter{this, "ParticleFilter", "", ""};   //!< the particle filter concerning kinematic cuts, etc.

  };

}

#endif
