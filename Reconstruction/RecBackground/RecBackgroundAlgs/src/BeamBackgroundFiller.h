/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RECBACKGROUNDALGS_BEAMBACKGROUNDFILLER
#define RECBACKGROUNDALGS_BEAMBACKGROUNDFILLER

#include <string>
#include <vector>

#include "AthLinks/ElementLinkVector.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonSegment/MuonSegment.h"
#include "RecBackgroundEvent/BeamBackgroundData.h"
#include "TrkSegment/Segment.h"
#include "TrkSegment/SegmentCollection.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODJet/JetContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

/**
 * @brief Implementation of the Beam Background Identification Method
 *
 * This implementation defines the selection criteria for identifying
 * beam background muons, and looks for them based on several methods. The
 * result are stored in BeamBackgroundData.
 *
 * @author David Salek <David.Salek@cern.ch>
 *
 * $Revision: 693115 $
 * $Date: 2015-09-04 09:22:39 +0200 (Fri, 04 Sep 2015) $
 */
class BeamBackgroundFiller : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~BeamBackgroundFiller() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  struct Cache {
    int m_numSegment{0};  // number of segments with the direction parallel to
                           // the beam pipe
    int m_numSegmentEarly{0};     // number of early segments
    int m_numSegmentACNoTime{0};  // number of matched pairs of segments on
                                   // side A and side C
    int m_numSegmentAC{0};        // number of matched pairs of segments with
                                   // corresponding time difference
    int m_numSegmentMDT{0};   // number of matched NSW segments to a EM MDT segment

    int m_numMatched{0};      // number of clusters matched with the segments
    int m_numNoTimeLoose{0};  // number of clusters identified by the "No-Time
                               // Method"
    int m_numNoTimeMedium{0};
    int m_numNoTimeTight{0};
    int m_numOneSidedLoose{0};  // number of clusters identified by the
                                 // "One-Sided Method"
    int m_numOneSidedMedium{0};
    int m_numOneSidedTight{0};
    int m_numTwoSidedNoTime{0};  // number of clusters identified by the
                                  // "Two-Sided No-Time Method"
    int m_numTwoSided{0};  // number of clusters identified by the "Two-Sided
                            // Method"
    int m_numClusterShape{0};  // number of clusters identified by the
                                // "Cluster-Shape Method"
    int m_numJet{0};           // number of fake jets
    int m_direction{0};  // direction of beam halo from the "Two-Sided Method"
                          // (positive for A->C, negative for C->A)
    // link to the calorimeter cluster
    ElementLinkVector<xAOD::CaloClusterContainer> m_indexClus{};
    // line to the muon segment
    ElementLinkVector<Trk::SegmentCollection> m_indexSeg{};
    // matching matrix (stores results for each muon segment and calorimeter
    // cluster)
    std::vector<std::vector<int> > m_matchMatrix{};
    // summary of the results for each muon segment
    std::vector<int> m_resultSeg{};
    // summary of the results for each cluster segment
    std::vector<int> m_resultClus{};
    // shape of the cluster
    std::vector<float> m_drdzClus{};

    ElementLinkVector<xAOD::JetContainer> m_indexJet{};  // link to the jet
    std::vector<int> m_resultJet{};  // summary of the results for each jet
  };

  // Function matching calorimeter clusters with muon segments
  StatusCode FillMatchMatrix(const EventContext& ctx, Cache& cache) const;
  // Beam background identification methods
  void SegmentMethod(Cache& cache) const;
  void NSWMDTMatching(const EventContext& ctx, Cache& cache) const;
  void OneSidedMethod(Cache& cache) const;
  void TwoSidedMethod(Cache& cache) const;
  void ClusterShapeMethod(Cache& cache) const;

  // Function calculating time for mboy CSC segments
  // (needed until release 17.1.0, does not work on AOD)
  double GetSegmentTime(const Muon::MuonSegment* pMuonSegment) const;

  // Function to identify fake jets
  void FindFakeJets(const EventContext& ctx, Cache& cache) const;
  // Function to store the results in BeamBackgroundData
  void FillBeamBackgroundData(
      SG::WriteHandle<BeamBackgroundData>& beamBackgroundDataWriteHandle,
      Cache& cache) const;

  /** ReadHandleKey for Trk::SegmentCollection from NSW */
  SG::ReadHandleKey<Trk::SegmentCollection> m_nswSegmentContainerReadHandleKey{
      this, "nswSegmentContainerKey", "NCB_TrackMuonSegments",
      "ReadHandleKey for Trk::SegmentCollection from NSW"};

  /** ReadHandleKey for Trk::SegmentCollection from CSC */
  SG::ReadHandleKey<Trk::SegmentCollection> m_cscSegmentContainerReadHandleKey{
      this, "cscSegmentContainerKey", "NCB_TrackMuonSegments",
      "ReadHandleKey for Trk::SegmentCollection from CSC"};

  /** ReadHandleKey for Trk::SegmentCollection from MDT */
  SG::ReadHandleKey<Trk::SegmentCollection> m_mdtSegmentContainerReadHandleKey{
      this, "mdtSegmentContainerKey", "TrackMuonSegments",
      "ReadHandleKey for Trk::SegmentCollection from MDT"};

  /** ReadHandleKey for CaloClusterContainer */
  SG::ReadHandleKey<xAOD::CaloClusterContainer>
      m_caloClusterContainerReadHandleKey{
          this, "caloClusterContainerKey", "CaloCalTopoClusters",
          "ReadHandleKey for CaloClusterContainer"};

  /** ReadHandleKey for JetContainer */
  SG::ReadHandleKey<xAOD::JetContainer> m_jetContainerReadHandleKey{
      this, "jetContainerKey", "AntiKt4EMTopoJets",
      "ReadHandleKey for JetContainer"};

  /* WriteHandleKey for BeamBackgroundData */
  SG::WriteHandleKey<BeamBackgroundData> m_beamBackgroundDataWriteHandleKey{
      this, "BeamBackgroundKey", "BeamBackgroundData",
      "WriteHandleKey for BeamBackgroundData"};



  Gaudi::Property<bool> m_isRun3{this, "isRun3", true};

  // cuts used in the Beam Background Identification Method
  Gaudi::Property<double> m_cutThetaNsw{this, "cutThetaNsw", 8. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutThetaCsc{this, "cutThetaCsc", 5. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutThetaMdtI{this, "cutThetaMdtI", 10. * Gaudi::Units::deg};

  Gaudi::Property<double> m_cutPhiSegNsw{this, "cutPhiSegNsw", 10. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutPhiSeg{this, "cutPhiSeg", 4. * Gaudi::Units::deg};

  Gaudi::Property<double> m_cutPhiSegMdt{this, "cutPhiSegMdt", 8. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutRadSegMdt{this, "cutRadSegMdt", -700. * Gaudi::Units::mm};

  Gaudi::Property<double> m_cutPhiNsw{this, "cutPhiNsw", 4. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutPhiCsc{this, "cutPhiCsc", 4. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutPhiMdtI{this, "cutPhiMdtI", 4. * Gaudi::Units::deg};
  Gaudi::Property<double> m_cutRadiusNsw{this, "cutRadiusNsw", 400. * Gaudi::Units::mm};
  Gaudi::Property<double> m_cutRadiusCsc{this, "cutRadiusCsc", 300. * Gaudi::Units::mm};
  Gaudi::Property<double> m_cutRadiusMdtI{this, "cutRadiusMdtI", 800. * Gaudi::Units::mm};
  Gaudi::Property<double> m_cutEnergy{this, "cutEnergy", 10. *Gaudi::Units::GeV};
  // CSC :  881 < R < 2081
  // LAr barrel :  1500 < R < 1970
  // TileCal :  2280 < R < 4250
  Gaudi::Property<double> m_cutRadiusLow{this, "cutRadiusLow", 881. * Gaudi::Units::mm};
  Gaudi::Property<double> m_cutRadiusHigh{this, "cutRadiusHigh", 4250. * Gaudi::Units::mm};

  Gaudi::Property<double> m_cutMuonTime{this, "cutMuonTime", 25.};
  Gaudi::Property<double> m_cutClusTime{this, "cutClusTime", 2.5};

  Gaudi::Property<double> m_cutTimeDiffAC{this, "cutTimeDiffAC", 25.};

  Gaudi::Property<double> m_cutDrdz{this, "cutDrdz", .15};

  ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{
      this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
      "Handle to the service providing the IMuonEDMHelperSvc interface"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
};

#endif
