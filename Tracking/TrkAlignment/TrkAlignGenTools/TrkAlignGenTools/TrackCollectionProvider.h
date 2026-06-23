/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_TRACKCOLLECTIONPROVIDER_H
#define TRKALIGNGENTOOLS_TRACKCOLLECTIONPROVIDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrkAlignInterfaces/ITrackCollectionProvider.h"

/**
   @file TrackCollectionProvider.h
   @class TrackCollectionProvider
   
   @brief Tool used to get track collection from StoreGate

   @author Robert Harrington <roberth@bu.edu>
   @date 7/29/2010
*/

namespace Trk {

  class TrackCollectionProvider : virtual public ITrackCollectionProvider, public AthAlgTool {

  public:
    TrackCollectionProvider(const std::string & type, const std::string & name, const IInterface * parent);
    virtual ~TrackCollectionProvider();
    
    StatusCode initialize();
    StatusCode finalize();
    
    StatusCode trackCollection(const TrackCollection*& tracks);

  private:
    Gaudi::Property<std::string> m_inputCol{this, "InputTrkCol", "MooreTracks",
	"Name of input track collection"};
    
    int m_nRetrievalErrors = 0;    //!< number of retrieval errors at the beginning of the job
    Gaudi::Property<int> m_maxRetrievalErrors{this, "MaxRetrievalErrors", 10,
      "maximum allowed number of retrieval errors at the beginning of the job (-1 for infinite)"};
    
  }; // end class

} // end namespace

#endif // TRKALIGNGENTOOLS_TRACKCOLLECTIONPROVIDER_H
