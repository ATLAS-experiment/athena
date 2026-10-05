/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
#ifndef VR_JET_OVERLAP_DECORATOR_H
#define VR_JET_OVERLAP_DECORATOR_H

#include "xAODJet/JetContainer.h"

enum class VRJetParameters {EDM,RHO30MIN02MAX4};

struct VRJetOverlapConfig {
  VRJetOverlapConfig(VRJetParameters = VRJetParameters::RHO30MIN02MAX4);
  std::string relative_name;
  std::string absolute_name;
  VRJetParameters jet_parameters_source;
};

class VRJetOverlapDecorator
{
public:
  VRJetOverlapDecorator(const VRJetOverlapConfig& = VRJetOverlapConfig());
  void decorate(const xAOD::JetContainer& jets) const;
private:
  VRJetParameters m_param_source;
  SG::Decorator<float> m_rel_decorator;
  SG::Decorator<float> m_abs_decorator;
  SG::ConstAccessor<float> m_min_radius;
  SG::ConstAccessor<float> m_max_radius;
  SG::ConstAccessor<float> m_mass_scale;
};

#endif
