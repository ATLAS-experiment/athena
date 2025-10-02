// for text editors: this file is -*- C++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DL2_TOOL_H
#define DL2_TOOL_H

#include "AsgTools/AsgTool.h"
#include "FlavorTagInference/IJetTagConditionalDecorator.h"

namespace FlavorTagDiscriminants {

  class DL2HighLevel;

  struct DL2Properties {
    std::string nnFile;
    std::string flipTagConfig;
    std::map<std::string,std::string> variableRemapping;
    std::string trackLinkType;
    float default_output_value = NAN;
  };

  class DL2Tool : public asg::AsgTool,
                  virtual public IJetTagConditionalDecorator
  {
    ASG_TOOL_CLASS(DL2Tool, IJetTagConditionalDecorator )
  public:
    DL2Tool(const std::string& name);
    ~DL2Tool();

    StatusCode initialize() override;

    // returns 0 for success
    virtual void decorate(const xAOD::IParticle& i_jet) const override;
    virtual void decorateWithDefaults(const xAOD::IParticle& i_jet) const override;

    virtual std::set<std::string> getDecoratorKeys() const override;
    virtual std::set<std::string> getAuxInputKeys() const override;
    virtual std::set<std::string> getConstituentAuxInputKeys() const override;
  private:
    DL2Properties m_props; //!
    std::unique_ptr<DL2HighLevel> m_dl2; //!
  };

}
#endif
