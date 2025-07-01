/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AsgAnalysisAlgorithms/AsgNumDecorationSelectionTool.h"

namespace CP
{

  #ifndef XAOD_STANDALONE
  template <typename T>
  AsgNumDecorationSelectionTool<T>::AsgNumDecorationSelectionTool(
    const std::string& type,
    const std::string& myname,
    const IInterface* parent)
    : asg::AsgTool(std::string(asg::ptrToString(parent))+"#"+type+"/"+myname)
  {
  }
  template <typename T>
  AsgNumDecorationSelectionTool<T>::~AsgNumDecorationSelectionTool()
  {
  }
  #else
  template <typename T>
  AsgNumDecorationSelectionTool<T>::AsgNumDecorationSelectionTool(const std::string& myname)
    : asg::AsgTool(myname)
  {
  }
  #endif

  template <typename T>
  StatusCode AsgNumDecorationSelectionTool<T>::initialize()
  {
    if (m_doEqual) {
      if (m_doMin || m_doMax) {
        ATH_MSG_WARNING("Equal cut overrides min/max on \"" << m_name << "\"; disabling min/max.");
        m_doMin = false;
        m_doMax = false;
      }
      ATH_MSG_DEBUG("Applying " << m_name << " == " << m_equal << " selection");
      m_equalCutIndex = m_accept.addCut("equal", "Exact equality cut");
    }

    if (m_doMin) {
      ATH_MSG_DEBUG("Applying " << m_name << " >= " << m_min << " selection");
      m_minCutIndex = m_accept.addCut("min", "Minimum cut");
    }

    if (m_doMax) {
      ATH_MSG_DEBUG("Applying " << m_name << " < " << m_max << " selection");
      m_maxCutIndex = m_accept.addCut("max", "Maximum cut");
    }

    // Construct the decoration accessor for the desired type
    m_accessor = std::make_unique<SG::AuxElement::ConstAccessor<T>>(m_name);

    return StatusCode::SUCCESS;
  }

  template <typename T>
  const asg::AcceptInfo& AsgNumDecorationSelectionTool<T>::getAcceptInfo() const
  {
    return m_accept;
  }

  template <typename T>
  asg::AcceptData AsgNumDecorationSelectionTool<T>::accept(const xAOD::IParticle *particle) const
  {
    asg::AcceptData accept(&m_accept);

    const SG::AuxElement* aux = dynamic_cast<const SG::AuxElement*>(particle);
    if (!aux) {
      ATH_MSG_ERROR("Particle is not derived from AuxElement, cannot read decoration. Cut considered as failed.");
      return accept; // reject all cuts by default
    }

    if (!m_accessor->isAvailable(*aux)) {
      ATH_MSG_WARNING("Decoration \"" << m_name << "\" not available; setting all cuts as passed.");
      if (m_equalCutIndex >= 0) accept.setCutResult(m_equalCutIndex, true);
      if (m_minCutIndex >= 0)   accept.setCutResult(m_minCutIndex, true);
      if (m_maxCutIndex >= 0)   accept.setCutResult(m_maxCutIndex, true);
      return accept;
    }

    const T value = (*m_accessor)(*aux);

    if (m_equalCutIndex >= 0) {
      accept.setCutResult(m_equalCutIndex, static_cast<float>(value) == m_equal);
    }

    if (m_minCutIndex >= 0) {
      accept.setCutResult(m_minCutIndex, static_cast<float>(value) >= m_min);
    }

    if (m_maxCutIndex >= 0) {
      accept.setCutResult(m_maxCutIndex, static_cast<float>(value) < m_max);
    }

    return accept;
  }

  #ifndef XAOD_STANDALONE
  AsgNumDecorationSelectionToolInt::AsgNumDecorationSelectionToolInt(
    const std::string& type,
    const std::string& myname,
    const IInterface* parent)
    : AsgNumDecorationSelectionTool<int>(type, myname, parent)
  {
  }
  AsgNumDecorationSelectionToolUInt8::AsgNumDecorationSelectionToolUInt8(
    const std::string& type,
    const std::string& myname,
    const IInterface* parent)
    : AsgNumDecorationSelectionTool<uint8_t>(type, myname, parent)
  {
  }
  #else
  AsgNumDecorationSelectionToolInt::AsgNumDecorationSelectionToolInt(const std::string& myname)
    : AsgNumDecorationSelectionTool<int>(myname)
  {
  }
  AsgNumDecorationSelectionToolUInt8::AsgNumDecorationSelectionToolUInt8(const std::string& myname)
    : AsgNumDecorationSelectionTool<uint8_t>(myname)
  {
  }
  #endif

  // Explicit template instantiations
  template class AsgNumDecorationSelectionTool<int>;
  template class AsgNumDecorationSelectionTool<uint8_t>;

}
