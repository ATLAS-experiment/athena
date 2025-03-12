/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 
 -------------------
 ATLAS Collaboration
 ***************************************************************************/

#include "Identifier/Identifier.h"
#include "SiSpacePointFormation/SiElementProperties.h" 
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h" 

namespace InDet{

SiElementProperties::SiElementProperties(const IdentifierHash&			idHash, 
					 const SCT_ID&				idHelper,
					 const InDetDD::SiDetectorElement&	element,
					 float					epsilonWidth,
                     const bool doEndcapEtaNeighbours) : m_neighbours(),m_halfWidth(0)

{
    // construct vector of neighbours of this wafer.
    Identifier compact;
    IdContext idCntxt(idHelper.wafer_context());
    
    int res = idHelper.get_id(idHash,compact, &idCntxt);
    if (res) return;      // cannot convert
       
    //if (idHelper.side(compact)==0) return;
    // if it is the stereo side do nothing
    if (element.isStereo()) return;
    IdentifierHash otherHash;
    IdentifierHash neighbourHash;

    res = idHelper.get_other_side(idHash, otherHash);
    if (res==0) m_neighbours.push_back(otherHash);
    res  = idHelper.get_prev_in_phi(otherHash, neighbourHash);
    if (res==0) m_neighbours.push_back(neighbourHash);
    res = idHelper.get_next_in_phi(otherHash, neighbourHash);
    if (res==0) m_neighbours.push_back(neighbourHash);
 
    // These neighbours are only defined for barrel, and may fail
    // because wafer is at one of the ends
    if (idHelper.is_barrel(compact))
    {
	    res = idHelper.get_prev_in_eta(otherHash, neighbourHash);
        if (res ==0)
            m_neighbours.push_back(neighbourHash);
        res  = idHelper.get_next_in_eta(otherHash, neighbourHash );
        if (res ==0)
            m_neighbours.push_back(neighbourHash);
    }
    else if(doEndcapEtaNeighbours){
        const InDetDD::SiDetectorElement* nextInEta = element.nextInEta();
        if(nextInEta){
            neighbourHash = nextInEta->identifyHash();
            m_neighbours.push_back(neighbourHash);
        }

        int trig_eta = idHelper.eta_module(compact);
        if(trig_eta==9){
            // For ITk strip endcaps we have a change of module granularity between eta_module=9 and 10
            // therefore when we are at eta_module=9, we can have two neighbours at larger radius
            // the second neighbour is put in "prevInEta()" in SCT_DetectorManager (a bit hacky, yes)
            const InDetDD::SiDetectorElement* nextInEta2 = element.prevInEta();
            if(nextInEta2){
                neighbourHash = nextInEta2->identifyHash();
                m_neighbours.push_back(neighbourHash);
            }   
        }
    }

    // Find half width of wafer at centre

    m_halfWidth = (element.design().minWidth()+element.design().maxWidth())*.25 
		  + epsilonWidth;        // add a bit for safety.
}

//-------------------------------------------------------------------------
SiElementProperties::~SiElementProperties()
= default;

//----------------------------------------------------------------------------

}
