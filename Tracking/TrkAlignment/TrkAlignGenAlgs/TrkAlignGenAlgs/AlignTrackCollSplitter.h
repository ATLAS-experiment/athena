/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENALGS_ALIGNTRACKCOLLSPLITTER_H
#define TRKALIGNGENALGS_ALIGNTRACKCOLLSPLITTER_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>

/**
   @file AlignTrackCollSplitter.h
   @class AlignTrackCollSplitter

   @brief Algorithm to split the "AlignTracks" track collection into
   several smaller track collections corresponding to different types
   of AlignTracks.

   @author Daniel Kollar <daniel.kollar@cern.ch>
*/  

namespace Trk {

  class AlignTrackCollSplitter : public AthAlgorithm {
    
  public: 

    /** constructor */
    AlignTrackCollSplitter(const std::string& name, ISvcLocator* pSvcLocator);

    /** destructor */
    virtual ~AlignTrackCollSplitter();
    
    /** initialize method */
    virtual StatusCode initialize();

    /** loops over tracks in event, and accumulates information necessary for alignmnet */
    virtual StatusCode execute();

    /** stop method */
    virtual StatusCode stop();

    /** finalize method */
    virtual StatusCode finalize();

  private:
    
    StringProperty m_inputCol{this, "InputTrkCol", "AlignTracks",
	"Name of input track collection"};
    StringProperty m_outColPrefix{this, "OutputColPrefix", "AlignTracks",
      "Prefix for names of output track collections"};

    std::vector<std::string> m_outColNames; //!< Names of output track collections

    int m_nevents = 0;    //!< number of processed events
    int m_ntracks = 0;    //!< number of processed tracks

    std::vector<int> m_trackTypeCounter; //!< counter for different types of tracks

    int m_nRetrievalErrors = 0;   //!< number of retrieval errors at the beginning of the job
    IntegerProperty m_maxRetrievalErrors{this, "MaxRetrievalErrors", -1,
      "maximum allowed number of retrieval errors at the beginning of the job (-1 for infinite)"};
  };

} // end namespace

#endif // TRKALIGNGENALGS_ALIGNTRACKCOLLSPLITTER_H
