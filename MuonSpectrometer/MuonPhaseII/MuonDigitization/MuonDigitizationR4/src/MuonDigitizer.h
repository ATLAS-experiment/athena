/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONDIGITIZATIONR4_MUONDIGITIZER_H
#define MUONDIGITIZATIONR4_MUONDIGITIZER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "PileUpTools/IPileUpTool.h"

class MuonDigitizer : public AthAlgorithm {
public:
    /** Constructor with parameters */
    using AthAlgorithm::AthAlgorithm;
    /** Destructor */
    virtual ~MuonDigitizer() = default;

    /** Basic algorithm methods */
    virtual StatusCode initialize() override final;
    virtual StatusCode execute() override final;
    virtual bool isClonable() const override final { return true; }

private:
    ToolHandle<IPileUpTool> m_digTool{this, "DigitizationTool", ""};
};
#endif  