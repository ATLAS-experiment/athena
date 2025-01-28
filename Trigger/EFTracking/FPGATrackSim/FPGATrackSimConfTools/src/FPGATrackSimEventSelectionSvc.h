/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimCONFTOOLS_FPGATrackSimEVENTSELECTIONSVC_H
#define FPGATrackSimCONFTOOLS_FPGATrackSimEVENTSELECTIONSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"

#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"

class FPGATrackSimHit;
class FPGATrackSimTrack;
class FPGATrackSimOfflineTrack;
class FPGATrackSimTruthTrack;
class FPGATrackSimRegionSlices;
class FPGATrackSimEventInputHeader;
class FPGATrackSimLogicalEventInputHeader;


class FPGATrackSimEventSelectionSvc : public extends< AthService, IFPGATrackSimEventSelectionSvc >
{
 public:

  FPGATrackSimEventSelectionSvc(const std::string& name, ISvcLocator* svc);
  virtual ~FPGATrackSimEventSelectionSvc() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  virtual unsigned getRegionID() const override { return m_regionID; }
  virtual SampleType getSampleType() const override { return m_st; }
  virtual bool checkPU() const override { return m_withPU.value(); }
  virtual const FPGATrackSimRegionSlices* getRegions() override;
  virtual int getLRTpdgID() const override { return m_LRT_pdgID; }

  virtual FPGATrackSimTrackPars getMin() const override { return m_min; }
  virtual FPGATrackSimTrackPars getMax() const override { return m_max; }

  virtual bool passCuts(const FPGATrackSimHit&) const override;
  virtual bool passCuts(const FPGATrackSimTrack&) const override;
  virtual bool passCuts(const FPGATrackSimOfflineTrack&) const override;
  virtual bool passCuts(const FPGATrackSimTruthTrack&) const override;

  virtual bool passQOverPt(const FPGATrackSimTrack&) const override;
  virtual bool passEta(const FPGATrackSimTrack&) const override;
  virtual bool passPhi(const FPGATrackSimTrack&) const override;
  virtual bool passD0(const FPGATrackSimTrack&) const override;
  virtual bool passZ0(const FPGATrackSimTrack&) const override;
  virtual bool passQOverPt(const FPGATrackSimOfflineTrack&) const override;
  virtual bool passEta(const FPGATrackSimOfflineTrack&) const override;
  virtual bool passPhi(const FPGATrackSimOfflineTrack&) const override;
  virtual bool passD0(const FPGATrackSimOfflineTrack&) const override;
  virtual bool passZ0(const FPGATrackSimOfflineTrack&) const override;
  
  virtual bool passMatching(FPGATrackSimTrack const &) const override;
  virtual bool passMatching(FPGATrackSimTruthTrack const &) const override;

  virtual bool selectEvent(FPGATrackSimEventInputHeader*) override;
  virtual bool selectEvent(FPGATrackSimLogicalEventInputHeader*) override;

  // Allow code to query whether or not an event was selected by the service.
  // If running over RDO part of this decision is made inside the dataprep algorithm, so it needs a setter.
  virtual bool getSelectedEvent() const override { return m_selectEvent; }
  virtual void setSelectedEvent(bool s) override { m_selectEvent =  s; }

 private:

  // Gaudi parameters:
  Gaudi::Property<unsigned int> m_regionID     { this, "regionID", 0, "current region under processing"};  // Current region of interest
  Gaudi::Property<unsigned int> m_skipRegionCheck     { this, "skipRegionCheck", false, "this should be set to true for DataPrep or in cases where region cuts don't matter"};
  Gaudi::Property<std::string> m_regions_path  { this, "regions", "", "path of the slices file"};         // path to slices file
  Gaudi::Property<std::string> m_sampleType    { this, "sampleType", "singleMuons", "type of sample under processing (skipTruth, singleElectrons, singleMuons, singlePions, or LLPs)"};           // type of sample ("skipTruth", "singleElectrons", "singleMuons", "singlePions")
  Gaudi::Property<bool> m_withPU           { this, "withPU",  false, "flag to say if there is pile-up or not"};              // flag to say if there is pile-up or not
  Gaudi::Property<bool> m_LRT              { this, "doLRT",   false, "Change track selection to LRT quantities; hit selection unchanged"};         // flag to require cancelling of selections on d0 and z0 in the case of large-radius tracking
  Gaudi::Property<float> m_minLRTpT        { this, "minLRTpT", 5., "Minimum pT to use in LRT selection, in GeV"};         // minimum pT, in GeV, to use in LRT selection
  Gaudi::Property<int> m_LRT_pdgID         { this, "lrt_truthMatchPDGID", 0, "If we are running an LLP sample but want only some PDGID of output in the truth selection, set this"};        // If we are running an LLP sample but want only some PDGID of output in the truth selection, set this
  Gaudi::Property<bool> m_allowHighBarcode { this, "allowHighBarcode", false, "Whether or not to allow barcodes over 200000 in truth matching"}; // whether or not to allow barcodes over 200000 in truth matching


  
  SampleType m_st = SampleType::skipTruth;           // internal value for faster comparisons in selectEvent()
  FPGATrackSimRegionSlices* m_regions = nullptr;  // pointer to RegionSlices class

  FPGATrackSimTrackPars m_min;                    // min limits of current region
  FPGATrackSimTrackPars m_max;                    // max limits of current region
  FPGATrackSimTrackPars m_trackmin;               // min limits of tracks to be accepted in region
  FPGATrackSimTrackPars m_trackmax;               // max limits of tracks to be accepted in region

 
  void createRegions();                  // helper function to create RegionSlices object
  bool checkTruthTracks(const std::vector<FPGATrackSimTruthTrack>&) const; // helper function to check the truth tracks for selectEvent()
  bool checkTruthTracksLRT(const std::vector<FPGATrackSimTruthTrack>&) const; // check the truth tracks for selectEvent() with LRT requirements

  // Cache result of selectEvent().
  bool m_selectEvent = false;

};

#endif  // FPGATrackSimCONFTOOLS_FPGATrackSimEVENTSELECTIONSVC_H
