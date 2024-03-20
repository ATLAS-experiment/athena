/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak <tadej@cern.ch>

#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_CLASSIFICATION_DECORATION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_CLASSIFICATION_DECORATION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODBase/IParticle.h>
#include <xAODBase/IParticleContainer.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <AsgAnalysisInterfaces/IClassificationTool.h>


namespace CP
{

/// \brief an algorithm for decorating classification using a tool based
/// on the \ref CP::IClassificationTool
class AsgClassificationDecorationAlg final : public EL::AnaAlgorithm
{
 public:
  /// \brief the standard constructor
  using EL::AnaAlgorithm::AnaAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;


  /// \brief truth classifier tool handle
private:
  ToolHandle<IClassificationTool> m_tool {this, "tool", "", "classification tool"};

  /// \brief the systematics list we run
private:
  CP::SysListHandle m_systematicsList {this};

  /// \brief particles container handle
private:
  CP::SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
    this, "particles", "", "the container to use"};

  /// \brief the decoration for the truth classification
private:
  CP::SysWriteDecorHandle<int> m_classificationDecorator {
    this, "decoration", "IFFClass_%SYS%", "decoration for per-object IFF truth class"};

};

} // namespace CP

#endif
