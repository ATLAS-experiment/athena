/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/******************************************************

   @class CombinerToolTag
   General tool to combine likelihoods

   (uses Andi Wildauer's CombinerTool)

   Created - 23 April 2007

   @author Giacinto Piacquadio (giacinto.piacquadio AT physik.uni-freiburg.de)
   @author2 Christian Weiser (christian.weiser AT physik.uni-freiburg.de)

   --- ( University of FREIBURG ) ---

   (c) 2007 - ATLAS Detector Software

********************************************************/

#ifndef JETTAGTOOLS_COMBINERTOOLTAG_H
#define JETTAGTOOLS_COMBINERTOOLTAG_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "JetTagTools/ITagTool.h"
#include <vector>

namespace Tracking {
  class VxCandidate;
}

namespace Analysis { 

  class ICombinerTool;

  class CombinerToolTag : public AthAlgTool , virtual public ITagTool
  {
    public:
      CombinerToolTag(const std::string&,const std::string&,const IInterface*);
      
      /**
	 Implementations of the methods defined in the abstract base class
      */
      virtual ~CombinerToolTag() = default;
      virtual StatusCode initialize() override;
      virtual void tagJet(xAOD::Jet& jetToTag);

    private:      

      /** List of the variables to be used in the likelihood */
      std::vector<std::string> m_listTaggers;
      std::string m_combinedTagString;

      ToolHandle<ICombinerTool> m_combinerTool;

  }; // End class
} // End namespace 

#endif
