/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
#ifndef FPGATrackSimHoughFUNCTIONS_H
#define FPGATrackSimHoughFUNCTIONS_H

#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/Monitored.h"

enum class ORAlgo {Normal, InvertGrouping};

long getVolumeID(const FPGATrackSimHit & hit);
long getCoarseID(const FPGATrackSimHit & hit);
long getFineID(const FPGATrackSimHit & hit);
bool isFineIDInStrip(long ID);
bool isFineIDInPixel(long ID);

void getMissingInfo(const FPGATrackSimRoad & road, int & nMissing, bool & missPixel, bool & missStrip, layer_bitmask_t & missing_mask, layer_bitmask_t & norecovery_mask, const ServiceHandle<IFPGATrackSimMappingSvc> & FPGATrackSimMapping, const TrackCorrType idealCoordFitType);
void makeTrackCandidates(const FPGATrackSimRoad & road, const FPGATrackSimTrack & temp, std::vector<FPGATrackSimTrack>& track_cands, const ServiceHandle<IFPGATrackSimMappingSvc> & FPGATrackSimMapping);
StatusCode runOverlapRemoval(std::vector<FPGATrackSimTrack>& tracks, const float minChi2, const int NumOfHitPerGrouping, ORAlgo orAlgo, ToolHandle<GenericMonitoringTool> & monTool, bool compareAllHits = false);
void findMinChi2MaxHit(const std::vector<int>& duplicates, std::vector<FPGATrackSimTrack>& RMtracks, std::vector<int> flags_OR, const float minChi2);
int findNonOverlapHits(const FPGATrackSimTrack& Track1, const FPGATrackSimTrack& Track2);
int findNCommonHits(const FPGATrackSimTrack& Track1, const FPGATrackSimTrack& Track2);
int findNCommonHitsGlobal(const FPGATrackSimTrack& Track1, const FPGATrackSimTrack& Track2);
void roadsToTrack(std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads, std::vector<FPGATrackSimTrack>& track_cands, const FPGATrackSimPlaneMap *pmap);


#endif // FPGATrackSimHoughFUNCTIONS_H
