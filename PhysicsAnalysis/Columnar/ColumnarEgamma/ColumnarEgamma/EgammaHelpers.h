/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EGAMMA_EGAMMA_HELPERS_H
#define COLUMNAR_EGAMMA_EGAMMA_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarEgamma/EgammaDef.h>

namespace columnar
{
  namespace EgammaHelpers
  {
    /// @file accessor for variables that have calculations in @ref xAOD::Egamma
    ///
    /// Essentially this just copies out the relevant parts of the xAOD
    /// class and makes them look like stand-alone accessors.  The name
    /// of each class is derived from the member function in the xAOD
    /// class.



    template<ContainerId CI = ContainerId::egamma,typename CM=ColumnarModeDefault>
    class EnergyAccessor final
    {
      ColumnAccessor<CI,float,CM> m_ptAcc;
      ColumnAccessor<CI,float,CM> m_etaAcc;

    public:
      
      EnergyAccessor (ColumnarTool<CM>& columnarTool) : m_ptAcc (columnarTool, "pt"), m_etaAcc (columnarTool, "eta") {}

      float operator () (ObjectId<CI,CM> object) const
      {
        return m_ptAcc(object) * std::cosh(m_etaAcc(object));
      }
    };
  }
}

#endif
