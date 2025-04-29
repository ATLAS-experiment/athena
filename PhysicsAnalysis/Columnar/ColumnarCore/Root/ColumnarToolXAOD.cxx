/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarCore/ColumnarTool.h>

//
// method implementations
//

namespace columnar
{
  ColumnarTool<ColumnarModeXAOD> ::
  ColumnarTool ()
  {
  }

  ColumnarTool<ColumnarModeXAOD> ::
  ColumnarTool (ColumnarTool<ColumnarModeXAOD>* /*val_parent*/)
  {}

  ColumnarTool<ColumnarModeXAOD> ::
  ~ColumnarTool ()
  {
  }

  StatusCode ColumnarTool<ColumnarModeXAOD> ::
  initializeColumns ()
  {
    return StatusCode::SUCCESS;
  }

  void ColumnarTool<ColumnarModeXAOD> ::
  callEvents (ObjectRange<ContainerId::eventContext,ColumnarModeXAOD> /*events*/) const
  {
    throw std::runtime_error ("tool didn't implement callEvents");
  }
}
