/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_INPUTVARIABLE_H
#define JETTOOLHELPERS_INPUTVARIABLE_H


#include "JetToolHelpers/JetContext.h" //used here
#include "xAODJet/Jet.h" //used here
#include "AthContainers/AuxElement.h" //member variable
#include "JetAnalysisInterfaces/IInputVariable.h"//inheritance

#include <memory> //std::unique_ptr
#include <functional> //std::function
#include <string>
#include <stdexcept> //std::runtime_error


namespace JetHelper {

    /// Class InputVariable
    /// This is design to read any kind of xAOD::Jet or JetContext variable e.g pt, eta, phi, m ..etc
    /// The user need to initialize the class with the name of the variable and the scale

class InputVariable : public IInputVariable
{

    public:
        
        /// This function specialize the variable to the one choose by the user
        static std::unique_ptr<InputVariable> createVariable(
            const std::string& name, 
            const std::string& type, 
            const bool isJetVar
        );

        /// Constructors
        InputVariable(const std::string& name): m_name{name}, m_scale{1.} {}
        InputVariable(
            const std::string& name, 
            std::function<float(const xAOD::Jet& jet, const JetContext& jc)> func
        );

        /// Default destructor
        virtual ~InputVariable() = default;

        /// return the value of the variable choose by the user
        [[nodiscard]] float getValue(const xAOD::Jet& jet, const JetContext& jc) const override {
            return m_scale * getValue_prot(jet, jc);
        }

        /// This function return the name of the variable
        const std::string& getName() const { return m_name;   }
        /// This function return the scale of the variable
        float getScale() const { return m_scale;  }
        /// This function set the scale of the variable
        void setScale(const float scale) { m_scale = scale; }
        /// This function set the scale to GeV, assuming MeV by default
        void setGeV() { m_scale = 1.e-3; }
        /// This function set the scale to MeV, assuming MeV by default (it does nothing)
        void setMeV() { m_scale = 1.;    }

        InputVariable(const InputVariable&) = delete;

    protected:
        const std::string m_name;
        float m_scale;

        std::function<float(const xAOD::Jet& jet, const JetContext& jc)> m_customFunction;

        [[nodiscard]] virtual float getValue_prot(const xAOD::Jet& jet, const JetContext& jc) const {
            return (m_customFunction(jet, jc));
        }  
};

/// Template to read JetContext
template <typename T> class InputVariableJetContext : public InputVariable {
    public:
        InputVariableJetContext(const std::string& name) : InputVariable(name) {}
        // should be T
        virtual float getValue(const xAOD::Jet&, const JetContext& event) const { 
            if (event.isAvailable(m_name)) {
                return event.getValue<T>(m_name);
            } else {
                throw std::runtime_error("Value " + m_name + " is not available");
            }
        }
};
/// Template to read xAOD::Jet variables                  
template <typename T> class InputVariableAttribute : public InputVariable {
    public:
        InputVariableAttribute(const std::string& name) : InputVariable(name), m_acc{name} {}
        virtual float getValue(const xAOD::Jet& jet, const JetContext&) const { 
            if (m_acc.isAvailable(jet)) {
                return m_acc(jet)*m_scale;
            } else {
                throw std::runtime_error("Value " + m_name + " is not available");
            } 
        }
    private:
        SG::AuxElement::ConstAccessor<T> m_acc;
};
} // namespace JetHelper

#endif

