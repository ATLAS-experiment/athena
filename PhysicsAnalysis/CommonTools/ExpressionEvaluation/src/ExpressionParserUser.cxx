/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ExpressionEvaluation/ExpressionParserUser.h"
#include "ExpressionEvaluation/ExpressionParserUser.icc"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"

template class ExpressionParserUserBase<AthAlgTool,1>;
template class ExpressionParserUserBase<AthAnalysisAlgorithm,1>;
template class ExpressionParserUser<AthAlgTool>;
template class ExpressionParserUser<AthAnalysisAlgorithm>;

// multi parser users
template class ExpressionParserUserBase<AthAlgTool,2>;
template class ExpressionParserUserBase<AthAnalysisAlgorithm,2>;
template class ExpressionParserUser<AthAlgTool,2>;
template class ExpressionParserUser<AthAnalysisAlgorithm,2>;
