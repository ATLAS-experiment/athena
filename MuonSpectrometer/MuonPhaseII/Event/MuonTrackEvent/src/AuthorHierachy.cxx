/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/AuthorHierachy.h"
#include "GeoModelKernel/throwExcept.h"

namespace MuonR4 {

    bool AuthorHierachy::operator()(const Author a, const Author b) const {
        return  authorRank(a) < authorRank(b);
    }
    unsigned AuthorHierachy::authorRank(const Author author) const {
        switch (author) {
           using enum Author; 
           case MuidCo: return 0;
           case MuGirl: return 1;
           case STACO: return 2;
           case MuTagIMO: return 3;
  
           case CaloScore:
           case CaloTag: return 4;
           case MuidSA: return 5;
           case NumberOfMuonAuthors: break;
           default:
              THROW_EXCEPTION("The author "<<author<<" is not supported");
        }
        return std::numeric_limits<unsigned>::max();
    }
   
}