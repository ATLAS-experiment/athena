/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COPYTRUTHPARTICLES_H
#define COPYTRUTHPARTICLES_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "JetInterface/IJetExecuteTool.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "AthContainers/ConstDataVector.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"

// Do I need IAsgTool? I need AsgTool for the eventStore()
class CopyTruthParticles : public IJetExecuteTool, public asg::AsgTool {
ASG_TOOL_INTERFACE(CopyTruthParticles)
ASG_TOOL_CLASS(CopyTruthParticles, IJetExecuteTool)
public:

  /// Constructor
  CopyTruthParticles(const std::string& name);

  /// @name Event loop algorithm methods
  //@{
  virtual int execute() const;

  virtual StatusCode initialize();
  //@}


  /// Classifier function(s)
  virtual bool classify(const xAOD::TruthParticle* tp) const = 0;


protected:
   /// Minimum pT for particle selection (in MeV)
  Gaudi::Property<float> m_ptmin{this, "PtMin", 0. , "Minimum pT of particles to be accepted for tagging (in MeV)"};

  /// Key for input truth event
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventKey{this, "TruthEventKey", "TruthEvents", "SG Key for input truth event container"};

  /// Key for output truth particles
  SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer> > m_outTruthPartKey{this, "OutputName", "TagInputs", "Name of the resulting TruthParticle collection"};

};


#endif
