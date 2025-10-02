// Dear emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENABASECOMPS_FILTEREDALGORTITHM_H
#define ATHENABASECOMPS_FILTEREDALGORTITHM_H 1

// STL include files
#include <vector>
#include <string>

// Required for inheritance
#include "AthenaBaseComps/AthAlgorithm.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IDecisionSvc.h"

/** @class FilteredAlgorithm
  * @brief algorithm that marks for write data objects in SG
  * 
  * @author srinir@bnl.gov
  * $Id: FilteredAlgorithm.h,v 1.3 2009-01-26 12:48:55 binet Exp $
  */
class FilteredAlgorithm : public AthAlgorithm
{

protected:

  /// Vector of names of Algorithms that this stream accepts
  StringArrayProperty      m_acceptNames{this, "AcceptAlgs", {},
    "Filters which if any are passed enable output"};
  /// Vector of names of Algorithms that this stream requires
  StringArrayProperty      m_requireNames{this, "RequireAlgs", {},
    "Filters which must all be passed to enable output"};
  /// Vector of names of Algorithms that this stream is vetoed by
  StringArrayProperty      m_vetoNames{this, "VetoAlgs", {},
    "Filters which if any are passed disable output"};

  ServiceHandle<IDecisionSvc> m_decSvc{this, "decSvc", "DecisionSvc/DecisionSvc",
    "Handle to DecisionSvc"};

protected:
  /// Standard algorithm Constructor
  FilteredAlgorithm(const std::string& name, ISvcLocator* pSvcLocator); 
  /// Standard Destructor
  virtual ~FilteredAlgorithm();

public:
  /// \name implement IAlgorithm
  //@{
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual StatusCode execute();
  //@}

  /// Test whether this event should be output
  bool isEventAccepted( ) const;


  FilteredAlgorithm(); //> not implemented
  FilteredAlgorithm (const FilteredAlgorithm&); //> not implemented
  FilteredAlgorithm& operator= (const FilteredAlgorithm&); //> not implemented
};

#endif // !ATHENABASECOMPS_FILTEREDALGORITHM_H
