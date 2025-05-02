/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimCONFTOOLS_IFPGATrackSimEVENTSELECTIONSVC_H
#define FPGATrackSimCONFTOOLS_IFPGATrackSimEVENTSELECTIONSVC_H

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IService.h"

class FPGATrackSimHit;
class FPGATrackSimTrack;
class FPGATrackSimOfflineTrack;
class FPGATrackSimTruthTrack;
class FPGATrackSimTrackPars;
class FPGATrackSimRegionSlices;
class FPGATrackSimEventInputHeader;
class FPGATrackSimLogicalEventInputHeader;
enum class SampleType;

class IFPGATrackSimEventSelectionSvc : public virtual IService
{
 public:
  DeclareInterfaceID(IFPGATrackSimEventSelectionSvc, 1, 0);

  virtual FPGATrackSimTrackPars getMin() const = 0;
  virtual FPGATrackSimTrackPars getMax() const = 0;
  virtual unsigned getRegionID() const = 0;
  virtual SampleType getSampleType() const = 0;
  virtual bool checkPU() const = 0;
  virtual int getLRTpdgID() const = 0;
  virtual const FPGATrackSimRegionSlices* getRegions() = 0;

  virtual bool passCuts(const FPGATrackSimHit&) const = 0;
  virtual bool passCuts(const FPGATrackSimTrack&) const = 0;
  virtual bool passCuts(const FPGATrackSimOfflineTrack&) const = 0;
  virtual bool passCuts(const FPGATrackSimTruthTrack&) const = 0;

  virtual bool passMatching(FPGATrackSimTrack const &) const = 0;
  virtual bool passMatching(FPGATrackSimTruthTrack const &) const = 0;

  virtual bool selectEvent(const FPGATrackSimEventInputHeader*) = 0;
  virtual bool selectEvent(FPGATrackSimLogicalEventInputHeader*) = 0;

  virtual bool passQOverPt(const FPGATrackSimTrack&) const = 0;
  virtual bool passEta(const FPGATrackSimTrack&) const = 0;
  virtual bool passPhi(const FPGATrackSimTrack&) const = 0;
  virtual bool passD0(const FPGATrackSimTrack&) const = 0;
  virtual bool passZ0(const FPGATrackSimTrack&) const = 0;

  virtual bool passQOverPt(const FPGATrackSimOfflineTrack&) const = 0;
  virtual bool passEta(const FPGATrackSimOfflineTrack&) const = 0;
  virtual bool passPhi(const FPGATrackSimOfflineTrack&) const = 0;
  virtual bool passD0(const FPGATrackSimOfflineTrack&) const = 0;
  virtual bool passZ0(const FPGATrackSimOfflineTrack&) const = 0;

  virtual bool getSelectedEvent() const = 0;
  virtual void setSelectedEvent(bool s) = 0;

};

#endif  //FPGATrackSimCONFTOOLS_IFPGATrackSimEVENTSELECTIONSVC_H
