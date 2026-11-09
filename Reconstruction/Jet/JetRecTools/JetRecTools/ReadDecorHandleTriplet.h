/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

//// Petr Jacka, Czech Technical University in Prague
//// February, 2026

#include "AsgTools/AsgTool.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODPFlow/FlowElementAuxContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/PFO.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include <memory>


template<typename T>
class ReadDecorHandleTriplet
{
  
  std::unique_ptr<SG::ReadDecorHandle<xAOD::IParticleContainer, T>> m_standard;
  std::unique_ptr<SG::ReadDecorHandle<xAOD::IParticleContainer, T>> m_charged;
  std::unique_ptr<SG::ReadDecorHandle<xAOD::IParticleContainer, T>> m_neutral;
  unsigned short m_objectType;

  void checkType() {
    switch (m_objectType) {
      case xAOD::Type::CaloCluster:
      case xAOD::Type::TrackCaloCluster:
      case xAOD::Type::ParticleFlow:
      case xAOD::Type::FlowElement:
        return;
        break;

      default:
        throw std::runtime_error(
          "ReadDecorHandleTriplet: Unsupported input type " +
          std::to_string(static_cast<int>(m_objectType)));
      }

  }


  public:

  void setObjectType(unsigned short type)
  {
    m_objectType = type;
    checkType();
  }

  void setStandard(
    const SG::ReadDecorHandleKey<xAOD::IParticleContainer>& key,
    const EventContext& ctx)
  {
    m_standard =
      std::make_unique<
        SG::ReadDecorHandle<xAOD::IParticleContainer,T>
      >(key, ctx);
  }

  void setCharged(
    const SG::ReadDecorHandleKey<xAOD::IParticleContainer>& key,
    const EventContext& ctx)
  {
    m_charged =
      std::make_unique<
        SG::ReadDecorHandle<xAOD::IParticleContainer,T>
      >(key, ctx);
  }

  void setNeutral(
      const SG::ReadDecorHandleKey<xAOD::IParticleContainer>& key,
      const EventContext& ctx)
  {
    m_neutral =
      std::make_unique<
        SG::ReadDecorHandle<xAOD::IParticleContainer,T>
      >(key, ctx);
  }

  

  T operator()(const xAOD::IParticle& part) const
  {
    switch (m_objectType) {

      case xAOD::Type::CaloCluster:
      case xAOD::Type::TrackCaloCluster:
        return (*m_standard)(part);

      case xAOD::Type::ParticleFlow:
      {
        auto& pfo = static_cast<const xAOD::PFO&>(part);
        return pfo.isCharged()
          ? (*m_charged)(pfo)
          : (*m_neutral)(pfo);
      }

      case xAOD::Type::FlowElement:
      {
        auto& fe = static_cast<const xAOD::FlowElement&>(part);
        return fe.isCharged()
          ? (*m_charged)(fe)
          : (*m_neutral)(fe);
      }
      default:
        throw std::runtime_error(
          "ReadDecorHandleTriplet: Unsupported input type " +
          std::to_string(static_cast<int>(m_objectType)));
    }

  }

  bool isAvailable()
  {
    switch (m_objectType) {

    case xAOD::Type::CaloCluster:
    case xAOD::Type::TrackCaloCluster:
      return m_standard && m_standard->isAvailable();

    case xAOD::Type::ParticleFlow:
    case xAOD::Type::FlowElement:
      return (m_charged && m_charged->isAvailable()) ||
            (m_neutral && m_neutral->isAvailable());

    default:
      return false;
    }
  }
};

template<typename T>
class ReadDecorHandleKeyTriplet
{
  SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_standard;
  SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_charged;
  SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_neutral;

  static bool startsWith(
      const std::string& value,
      const std::string& prefix)
  {
    return value.compare(0, prefix.size(), prefix) == 0;
  }

  public:

  
  ReadDecorHandleKeyTriplet(
    asg::AsgTool* owner,
    const std::string& propertyName,
    const std::string& decorName,
    const std::string& description)
    : m_standard(
          owner,
          propertyName.c_str(),
          decorName.c_str(),
          description.c_str())
    , m_charged(
          owner,
          ("Charged" + propertyName).c_str(),
          decorName.c_str(),
          ("Charged " + description).c_str())
    , m_neutral(
          owner,
          ("Neutral" + propertyName).c_str(),
          decorName.c_str(),
          ("Neutral " + description).c_str())
  {}

  StatusCode assign(
    const std::string& standardCont,
    const std::string& chargedCont,
    const std::string& neutralCont)
  {
    if (!standardCont.empty() && !startsWith(m_standard.key(), standardCont + ".")) {
      ATH_CHECK(m_standard.assign(
          standardCont + "." + m_standard.key()));
    }

    if (!chargedCont.empty() && !startsWith(m_charged.key(), chargedCont + ".")) {
      ATH_CHECK(m_charged.assign(
          chargedCont + "." + m_charged.key()));
    }

    if (!neutralCont.empty() &&!startsWith(m_neutral.key(), neutralCont + ".")) {
      ATH_CHECK(m_neutral.assign(
          neutralCont + "." + m_neutral.key()));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode initialize()
  {
    ATH_CHECK(m_standard.initialize());
    ATH_CHECK(m_charged.initialize());
    ATH_CHECK(m_neutral.initialize());

    return StatusCode::SUCCESS;
  }

  ReadDecorHandleTriplet<T> makeHandles(const unsigned short& inputType, const EventContext& ctx) const
  {
    ReadDecorHandleTriplet<T> handles;
    handles.setObjectType(inputType);

    if (inputType == xAOD::Type::CaloCluster || inputType==xAOD::Type::TrackCaloCluster) {
      handles.setStandard(m_standard, ctx);
    }
    else if (inputType == xAOD::Type::ParticleFlow || inputType==xAOD::Type::FlowElement) {
      handles.setCharged(m_charged, ctx);
      handles.setNeutral(m_neutral, ctx);
    }

    return handles;
  }

};
