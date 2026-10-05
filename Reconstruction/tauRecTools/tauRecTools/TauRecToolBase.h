/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAURECTOOLBASE_H
#define TAURECTOOLS_TAURECTOOLBASE_H
/**
 * @brief The base class for all tau tools.
 * 
 * @author Lukasz Janyst
 * @author Justin Griffiths
 * Thanks to Lianyou Shan, Lorenz Hauswald
 */

#include <string>

#include "tauRecTools/ITauToolBase.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

class TauRecToolBase : public asg::AsgTool, virtual public ITauToolBase {
 public:

  ASG_TOOL_INTERFACE(TauRecToolBase)
  ASG_TOOL_CLASS1( TauRecToolBase, ITauToolBase )

  TauRecToolBase(const std::string& name);
  virtual ~TauRecToolBase() {}

  //-----------------------------------------------------------------
  //! Tool initializer
  //-----------------------------------------------------------------
  virtual StatusCode initialize() override;

  //-----------------------------------------------------------------
  //! Event initializer - called at the beginning of each event
  //-----------------------------------------------------------------
  virtual StatusCode eventInitialize() override;

  //-----------------------------------------------------------------
  //! Execute - called for each tau candidate
  //-----------------------------------------------------------------
  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ) const override;

#ifdef XAOD_ANALYSIS
  virtual StatusCode executeDev(xAOD::TauJet& ) override;
#else
  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 CaloConstCellContainer& ,
				 boost::dynamic_bitset<>& ) const override;
#endif
  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 const xAOD::VertexContainer* ) const override;

  virtual StatusCode executeTool(xAOD::TauJet& ,
                                 const EventContext& ,
                                 xAOD::VertexContainer& ) const override;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::TauTrackContainer& ) const override;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::CaloClusterContainer& ,
				 xAOD::PFOContainer& ) const override;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::PFOContainer& ,
				 xAOD::PFOContainer& ,
				 const xAOD::CaloClusterContainer& ) const override;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::PFOContainer& ,
				 xAOD::PFOContainer& ) const override;  

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::PFOContainer& ) const override;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::ParticleContainer& ,
				 xAOD::PFOContainer& ) const override;

  //-----------------------------------------------------------------
  //! Event finalizer - called at the end of each event
  //-----------------------------------------------------------------
  virtual StatusCode eventFinalize() override;

  //-----------------------------------------------------------------
  //! Finalizer
  //-----------------------------------------------------------------
  virtual StatusCode finalize() override;

  std::string find_file(const std::string& fname) const;

 protected:
  Gaudi::Property<bool>        m_in_trigger     {this, "inTrigger",   false,                     "Indicate if the tool is running on trigger"};
  Gaudi::Property<bool>        m_in_AOD         {this, "inAOD",       false,                     "Indicate if the tool is running on AOD"};
  Gaudi::Property<bool>        m_in_EleRM       {this, "inEleRM",       false,                     "Indicate if the tool is running on EleRM routine"};
  Gaudi::Property<std::string> m_tauRecToolsTag {this, "calibFolder", "tauRecTools/R22_preprod", "CVMFS path to the tau calibration folder"};

  bool inTrigger() const;
  bool inAOD() const;
  bool inEleRM() const;

};

inline bool TauRecToolBase::inTrigger() const { return m_in_trigger; }
inline bool TauRecToolBase::inAOD() const { return m_in_AOD; }
inline bool TauRecToolBase::inEleRM() const { return m_in_EleRM; }

#endif // TAURECTOOLS_TAURECTOOLBASE_H
