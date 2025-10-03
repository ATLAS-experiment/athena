/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_NUM_DECORATION_SELECTION_TOOL_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_NUM_DECORATION_SELECTION_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AthContainers/AuxElement.h>
#include <PATCore/IAsgSelectionTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgMessaging/StatusCode.h>
#include <memory>
#include <typeinfo>

#include <xAODBase/IParticle.h>

namespace CP
{
  /// \brief a templated \ref IAsgSelectionTool that performs basic
  /// cut on numerical decorations (e.g., int, uint8_t)

  template <typename T>
  class AsgNumDecorationSelectionTool
    : public asg::AsgTool, public virtual IAsgSelectionTool
  {
  public:
    #ifndef XAOD_STANDALONE
    AsgNumDecorationSelectionTool(const std::string& type,
                                  const std::string& myname,
                                  const IInterface* parent);
    virtual ~AsgNumDecorationSelectionTool();
    #else
    AsgNumDecorationSelectionTool(const std::string& myname);
    #endif

    virtual StatusCode initialize() override;
    virtual const asg::AcceptInfo& getAcceptInfo() const override;
    virtual asg::AcceptData accept(const xAOD::IParticle *particle) const override;

  private:
    // Helper function to generate type-dependent default name
    static std::string getDefaultDecorationName() {
      std::string typeName = typeid(T).name();
      return "dummy_" + typeName;
    }

    Gaudi::Property<std::string> m_name {this, "decorationName", getDefaultDecorationName(), "name of the decoration on which the cuts are applied"};
    Gaudi::Property<bool> m_doEqual {this, "doEqual", false, "require to equal a value"};
    Gaudi::Property<bool> m_doMin {this, "doMin", false, "require a min value"};
    Gaudi::Property<bool> m_doMax {this, "doMax", false, "require a max value"};
    Gaudi::Property<float> m_equal {this, "equal", 0.0f, "equal value to require"};
    Gaudi::Property<float> m_min {this, "min", 0.0f, "minimum value to require"};
    Gaudi::Property<float> m_max {this, "max", 0.0f, "maximum value to require"};

    int m_equalCutIndex{ -1 };
    int m_minCutIndex{ -1 };
    int m_maxCutIndex{ -1 };

    std::unique_ptr<SG::ConstAccessor<T>> m_accessor;

    asg::AcceptInfo m_accept;
  };

  class AsgNumDecorationSelectionToolInt final
    : public AsgNumDecorationSelectionTool<int>
  {
  public:
    #ifndef XAOD_STANDALONE
    AsgNumDecorationSelectionToolInt(const std::string& type,
                                     const std::string& myname,
                                     const IInterface* parent);
    #else
    AsgNumDecorationSelectionToolInt(const std::string& myname);
    #endif
  };

  class AsgNumDecorationSelectionToolUInt8 final
    : public AsgNumDecorationSelectionTool<uint8_t>
  {
  public:
    #ifndef XAOD_STANDALONE
    AsgNumDecorationSelectionToolUInt8(const std::string& type,
                                       const std::string& myname,
                                       const IInterface* parent);
    #else
    AsgNumDecorationSelectionToolUInt8(const std::string& myname);
    #endif
  };
}

#endif
