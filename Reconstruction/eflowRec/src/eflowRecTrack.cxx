/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * eflowRecTrack.cxx
 *
 *  Created on: 30.09.2013
 *      Author: tlodd
 */

#include "eflowRecTrack.h"
#include "eflowDepthCalculator.h"
#include "eflowTrackExtrapolatorBaseAlgTool.h"

#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/StatusCode.h"


eflowRecTrack::eflowRecTrack(
    const EventContext& ctx,
    const ElementLink<xAOD::TrackParticleContainer>& trackElemLink,
    const ToolHandle<eflowTrackExtrapolatorBaseAlgTool>& theTrackExtrapolatorTool) :
    m_trackId(-1), m_trackElemLink(trackElemLink), m_track(*trackElemLink), m_type(5),
    m_pull15(0.0),
    m_layerHED(-1),
    m_eExpect(1.0),
    m_varEExpect(0.0), 
    m_isInDenseEnvironment(false),
    m_isSubtracted(false),
    m_isRecovered(false),
    m_hasBin(true),
    m_trackCaloPoints(theTrackExtrapolatorTool->execute(ctx, m_track))
{
}

eflowRecTrack::eflowRecTrack(const eflowRecTrack& eflowRecTrack)
  : m_trackId (eflowRecTrack.m_trackId),
    m_trackElemLink (eflowRecTrack.m_trackElemLink),
    m_track (*m_trackElemLink),
    m_type (eflowRecTrack.m_type),
    m_pull15 (eflowRecTrack.m_pull15),
    m_layerHED (eflowRecTrack.m_layerHED),
    m_eExpect (eflowRecTrack.m_eExpect),
    m_varEExpect (eflowRecTrack.m_varEExpect),
    m_isInDenseEnvironment (eflowRecTrack.m_isInDenseEnvironment),
    m_isSubtracted (eflowRecTrack.m_isSubtracted),
    m_isRecovered (eflowRecTrack.m_isRecovered),
    m_hasBin (eflowRecTrack.m_hasBin),
    m_trackCaloPoints (std::make_unique<eflowTrackCaloPoints>(*eflowRecTrack.m_trackCaloPoints))
{
}

eflowRecTrack& eflowRecTrack::operator = (const eflowRecTrack& originalEflowRecTrack){
  if (this == &originalEflowRecTrack) return *this;
  //if not assigning to self, then we copy the data to the new object
  else{
    m_trackId = originalEflowRecTrack.m_trackId;
    m_trackElemLink = originalEflowRecTrack.m_trackElemLink;
    m_track = *m_trackElemLink;
    m_type = originalEflowRecTrack.m_type;
    m_pull15 = originalEflowRecTrack.m_pull15;
    m_eExpect = originalEflowRecTrack.m_eExpect;
    m_varEExpect = originalEflowRecTrack.m_varEExpect;
    m_isInDenseEnvironment = originalEflowRecTrack.m_isInDenseEnvironment;
    m_isSubtracted = originalEflowRecTrack.m_isSubtracted;
    m_isRecovered = originalEflowRecTrack.m_isRecovered;
    m_hasBin = originalEflowRecTrack.m_hasBin;
    m_trackCaloPoints = std::make_unique<eflowTrackCaloPoints>(*originalEflowRecTrack.m_trackCaloPoints);
    m_layerHED = originalEflowRecTrack.m_layerHED;
    return *this;
  }//if not assigning to self, then we have copied the data to the new object
}

eflowRecTrack::~eflowRecTrack() = default;

void eflowRecTrack::setCaloDepthArray(const double* depthArray) {
  m_caloDepthArray.assign(depthArray, depthArray + eflowDepthCalculator::NDepth() + 1);
}

const std::vector<eflowTrackClusterLink*>* eflowRecTrack::getAlternativeClusterMatches(std::string_view key) const  { 

  auto thisIterator = m_alternativeClusterMatches.find(key);
  if (thisIterator !=  m_alternativeClusterMatches.end()) return  &(thisIterator->second);
  return nullptr;

}

void eflowRecTrack::insertTruthEnergyPair (const CaloCell* cell, double truthEnergy){
  if (m_cellTruthEnergyStore.count(cell->ID()) == 0) m_cellTruthEnergyStore[cell->ID()] = truthEnergy;
  else m_cellTruthEnergyStore[cell->ID()] += truthEnergy;
}

double eflowRecTrack::getCellTruthEnergy (const CaloCell* cell) const{
  if (m_cellTruthEnergyStore.count(cell->ID()) == 0) return 0.0;
  else return m_cellTruthEnergyStore.at(cell->ID());
}

void eflowRecTrack::setSubtracted() {
  if (isSubtracted()){
    REPORT_MESSAGE_WITH_CONTEXT(MSG::WARNING, "eflowRecTrack")
      << "Invoke setSubtracted() on track that is subtracted already!" << endmsg;
    return;
  }//if track was already subtracted then print a warning to the user about that and return
  m_isSubtracted = true;
}
