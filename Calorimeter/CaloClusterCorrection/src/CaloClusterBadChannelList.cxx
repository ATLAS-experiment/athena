/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     CaloClusterBadChannelList.cxx

AUTHORS:  G.Unal
CREATED:  May 2008

PURPOSE: Fill bad channels in cluster

********************************************************************/
// include header files
#include "CaloClusterBadChannelList.h"

#include "AthenaKernel/errorcheck.h"
#include "CaloEvent/CaloClusterBadChannelData.h"

using xAOD::CaloCluster;
CaloClusterBadChannelList::CaloClusterBadChannelList(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent)
  : CaloClusterCorrection(type, name, parent)
{
}

StatusCode CaloClusterBadChannelList::initialize()
{
  CHECK( m_badChannelTool.retrieve() );
  // Make sure the variable used list is declared to the auxiliary
  // variable registry.  Otherwise, if there are no clusters in the first event,
  // then we can get warnings from AuxSelection.
  SG::Accessor<xAOD::CaloClusterBadChannelList> accBCL("BadChannelList");
  return CaloClusterCorrection::initialize();
}


void CaloClusterBadChannelList::makeCorrection (const Context& myctx,
                                                CaloCluster* cluster) const
{
  xAOD::CaloClusterBadChannelList badChanList;

  CaloCluster::cell_iterator cellIter    = cluster->cell_begin();
  CaloCluster::cell_iterator cellIterEnd = cluster->cell_end();
  for( ;cellIter!=cellIterEnd;cellIter++) {
      const CaloCell* cell = (*cellIter);
      const Identifier id = cell->ID();
      CaloBadChannel status = m_badChannelTool->caloStatus(myctx.ctx(),id);
      bool isBad = cell->badcell();
      if (status.dead() || status.noisy() || isBad )   {
         const float eta = cell->eta();
         const float phi = cell->phi();
         const CaloSampling::CaloSample layer = cell->caloDDE()->getSampling();
         // in case cell is bad, add explicitly the dead bit to the status
         CaloBadChannel::BitWord myword = status.packedData();
         if (isBad && !status.dead()) {
            CaloBadChannel::setBit(CaloBadChannel::deadBit,myword,true);
         }
         ATH_MSG_DEBUG(" bad channel found eta,phi,layer,status "
                       << eta << " " << phi << " " << layer << " " << myword);
         badChanList.emplace_back(eta, phi, layer, myword);
      }
  }  // end loop over cells
  cluster->setBadChannelList(badChanList);
}

