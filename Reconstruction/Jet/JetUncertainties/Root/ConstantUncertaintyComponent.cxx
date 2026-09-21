/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetUncertainties/ConstantUncertaintyComponent.h"
#include "JetUncertainties/Helpers.h"

#include <cmath>

namespace jet
{

//////////////////////////////////////////////////
//                                              //
//  Constructor/destructor/initialization       //
//                                              //
//////////////////////////////////////////////////

ConstantUncertaintyComponent::ConstantUncertaintyComponent(const std::string& name)
    : UncertaintyComponent(ComponentHelper(name))
    , m_name("")
    , m_value(0)
    , m_histSpecified(false)
{
    JESUNC_NO_DEFAULT_CONSTRUCTOR;
}

ConstantUncertaintyComponent::ConstantUncertaintyComponent(const ComponentHelper& component)
    : UncertaintyComponent(component,0)
    , m_name(component.name)
    , m_value(component.constantValue)
    , m_histSpecified(!component.uncNames.empty())
{
    ATH_MSG_DEBUG(Form("Creating ConstantUncertaintyComponent named %s",m_name.Data()));
}

ConstantUncertaintyComponent::ConstantUncertaintyComponent(const ConstantUncertaintyComponent& toCopy)
    : UncertaintyComponent(toCopy)
    , m_name(toCopy.m_name)
    , m_value(toCopy.m_value)
    , m_histSpecified(toCopy.m_histSpecified)
{
    ATH_MSG_DEBUG(Form("Creating copy of ConstantUncertaintyComponent named %s",m_name.Data()));
}

ConstantUncertaintyComponent* ConstantUncertaintyComponent::clone() const
{
    return new ConstantUncertaintyComponent(*this);
}

StatusCode ConstantUncertaintyComponent::initialize(TFile* /*histFile*/)
{
    // Prevent double-initialization
    if (m_isInit)
    {
        ATH_MSG_ERROR("Component is already initialized: " << getName().Data());
        return StatusCode::FAILURE;
    }

    // Fail loudly rather than silently ignoring a histogram the config asked for
    if (m_histSpecified)
    {
        ATH_MSG_ERROR("Constant component specifies a Hists key, which it cannot use: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_validHistName != "")
    {
        ATH_MSG_ERROR("Constant component specifies a VHist key, which it cannot use: " << getName().Data());
        return StatusCode::FAILURE;
    }

    if (!std::isfinite(m_value))
    {
        ATH_MSG_ERROR("Constant component has a non-finite Value: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_value < 0)
    {
        ATH_MSG_ERROR("Constant component has a negative Value, use a positive fraction and let the "
                      "systematic sign come from the requested variation: " << getName().Data());
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully initialized ConstantUncertaintyComponent named " << getName().Data()
                  << " with a flat fractional uncertainty of " << m_value);

    m_isInit = true;
    return StatusCode::SUCCESS;
}


//////////////////////////////////////////////////
//                                              //
//  Methods to test for special cases           //
//                                              //
//////////////////////////////////////////////////

bool ConstantUncertaintyComponent::isAlwaysZero() const
{
    if (!m_isInit)
    {
        ATH_MSG_ERROR("Cannot call method before initialization, component: " << getName().Data());
        return false;
    }
    return std::fabs(m_value) < 1.e-8;
}


//////////////////////////////////////////////////
//                                              //
//  Uncertainty/validity retrieval methods      //
//                                              //
//////////////////////////////////////////////////

bool ConstantUncertaintyComponent::getValidityImpl(const xAOD::Jet&, const xAOD::EventInfo&) const
{
    // A flat uncertainty has no parametrisation, so it has no validity range either.
    return true;
}

double ConstantUncertaintyComponent::getUncertaintyImpl(const xAOD::Jet&, const xAOD::EventInfo&) const
{
    return m_value;
}

} // end jet namespace
