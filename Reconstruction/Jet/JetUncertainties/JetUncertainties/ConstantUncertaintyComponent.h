/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETUNCERTAINTIES_CONSTANTUNCERTAINTYCOMPONENT_H
#define JETUNCERTAINTIES_CONSTANTUNCERTAINTYCOMPONENT_H

/*
  ConstantUncertaintyComponent

  A flat fractional uncertainty, taken straight from the configuration file
  rather than from a histogram:

      JESComponent.N.Param: Constant
      JESComponent.N.Value: 0.01

  Every other component in the package loads a histogram and fails if it is
  absent, so a genuinely flat uncertainty previously had to be faked with a
  histogram filled with a constant. This class covers that case directly, which
  keeps the value visible in the config instead of buried in a ROOT file.
*/

#include "JetUncertainties/UncertaintyComponent.h"

namespace jet
{

class ConstantUncertaintyComponent : public UncertaintyComponent
{
    public:
        // Constructor/destructor/initialization
        ConstantUncertaintyComponent(const ComponentHelper& component);
        ConstantUncertaintyComponent(const ConstantUncertaintyComponent& toCopy);
        virtual ConstantUncertaintyComponent* clone() const;
        virtual ~ConstantUncertaintyComponent() {}

        /// Deliberately does not touch the histogram file: there is no histogram.
        virtual StatusCode initialize(TFile* histFile);

        /// The base class reports the histogram name, which this component does not have.
        virtual TString getName() const { return m_name; }

        virtual bool isAlwaysZero() const;

    protected:

        // Uncertainty/validity retrieval helper methods
        virtual bool   getValidityImpl(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo)    const;
        virtual double getUncertaintyImpl(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const;

    private:
        ConstantUncertaintyComponent(const std::string& name = "");

        const TString m_name;
        const double  m_value{};
        /// Set if the config also asked for a histogram, which is a configuration error.
        const bool    m_histSpecified{};

};

} // end jet namespace

#endif
