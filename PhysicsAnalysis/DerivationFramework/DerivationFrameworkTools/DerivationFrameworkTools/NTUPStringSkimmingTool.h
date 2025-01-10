/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NTUPStringSkimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_NTUPSTRINGSKIMMINGTOOL_H
#define DERIVATIONFRAMEWORK_NTUPSTRINGSKIMMINGTOOL_H

#include <memory>
#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

namespace ExpressionParsing {
  class ExpressionParser;
}

namespace DerivationFramework {

  class NTUPStringSkimmingTool : public AthAlgTool, public ISkimmingTool {
    public: 
      NTUPStringSkimmingTool(const std::string& t, const std::string& n, const IInterface* p);

      virtual StatusCode initialize() override;
      virtual bool eventPassesFilter() const override;

    private:
      std::string m_expression;
      std::unique_ptr<ExpressionParsing::ExpressionParser> m_parser;
  }; 
}

#endif // DERIVATIONFRAMEWORK_NTUPSTRINGSKIMMINGTOOL_H
