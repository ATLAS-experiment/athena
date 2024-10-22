/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNTOOLS_INDETALIGNFILLTRACK_IH
#define INDETALIGNTOOLS_INDETALIGNFILLTRACK_IH
// IInDetAlignFillTrack.h
// is in InDetAlignFillTrack.h
// Carlos Escobar, started 27/12/2007

#include "GaudiKernel/IAlgTool.h"

class IInDetAlignFillTrack: virtual public IAlgTool {
 public:
  DeclareInterfaceID( IInDetAlignFillTrack, 1, 0 );

  virtual StatusCode FillTrack() = 0;

  virtual int GetTrks() const = 0;
  virtual int GetTrkHits() const = 0;
  virtual int GetTrkPixHits() const = 0;
  virtual int GetTrkSCTHits() const = 0;
  virtual int GetTrkTRTHits() const = 0;
  
};

#endif // INDETALIGNTOOLS_INDETALIGNFILLTRACK_IH
