/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PrimaryVertexSelector, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEVENTUTILS_PRIMARYVERTEXSELECTOR_H
#define TRKEVENTUTILS_PRIMARYVERTEXSELECTOR_H

//Trk
#include "VxVertex/VxContainer.h"
#include "VxVertex/VxCandidate.h"

/** Simple Global Function to wrap around the VxCandidate container 
    @author Andreas.Salzburger -at- cern.ch 
*/

inline const Trk::VxCandidate* PrimaryVertexSelector(const VxContainer& vxContainer){
	// the return vertex
	const Trk::VxCandidate* primaryVxCandidate = 0;
	// simple loop through and get the primary vertex
        for (const Trk::VxCandidate* vtx : vxContainer) {
       // the first and only primary vertex candidate is picked
	   if ( vtx->vertexType() ==  Trk::PriVtx){
			primaryVxCandidate = vtx;
			break;
		}
	}
	// return what you have - users need to check
	return primaryVxCandidate;
}

#endif


