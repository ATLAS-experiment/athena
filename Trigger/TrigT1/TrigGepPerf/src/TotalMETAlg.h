/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_TOTALMETALG_H
#define TRIGGEPPERF_TOTALMETALG_H

/*
  TotalMETAlg:
  -----------
  Athena wiring for the bitwise GEP MET core (Gep::TotalMETMaker). A port of the
  standalone metEmulation.cc, in the same format the JetTaggerLRJ/WTACone uses:

    TrigGepPerf/TotalMETMaker.h  the bitwise core, digitized in and digitized out
    TotalMETAlg                  this class -- the floating point adapter and the
                                 StoreGate wiring around it

  The float adapter lives here rather than in a separate maker (as
  JetTaggerLRJJetMaker is) because this algorithm is the core's only floating point
  client, and MET output is scalars rather than an xAOD object needing conversion back.
  Clients whose input is already digitized, e.g.  GlobalSim,
  should use Gep::TotalMETMaker directly: going through here would re-quantize values
  already on the grid, which can cost an LSB.

  Four MET flavors are computed in one pass; each is outputted as its own
  xAOD::EnergySumRoI, and each can be enabled independently. Only total MET is on by
  default. Total MET always computes the jet and tower terms internally regardless of
  their flags, since it is their weighted sum -- the flags govern what is recorded.

  UNITS: every value written here is in GeV, matching the standalone emulation's metTree
  and NOT the MeV convention of GepMETAlg's EnergySumRoI. The two never mix, since they
  are separate containers reaching the ntuple through separate branches.
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTrigger/EnergySumRoI.h"

#include "TrigGepPerf/TotalMETMaker.h"

#include <string>
#include <vector>

class TotalMETAlg: public ::AthReentrantAlgorithm {
public:
  TotalMETAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override;

private:

  // ---------------- inputs ----------------------------------------
  // Which pileup-suppression variant is used is decided purely by WHICH containers these
  // point at -- there is no SK/EtaSK/NoSK switch, unlike the standalone emulation where
  // that choice selects between trees in one file. The default configuration points both
  // at the EtaSK collections
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "GEP cell towers used for the tower MET term"};

  SG::ReadHandleKey<xAOD::JetContainer> m_gepJetsKey {
    this, "gepJetsKey", "", "GEP jets (WTACone) used for the jet MET term"};

  // ---------------- outputs ---------------------------------------
  // One EnergySumRoI per flavor. energyX / energyY carry METx / METy and energyT carries
  // SumET; the magnitude and azimuth are the "met" and "metPhi" aux decorations,
  // because EnergySumRoI has no field for them and they are NOT recoverable from the
  // components: the magnitude is a LUT root and the azimuth a comparator ladder, neither
  // of which is the sqrt/atan2 of what is stored.
  // Note EnergySumRoI was used by previous GEPMET implementations, so is re-used here. 
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputTotalMETKey {
    this, "outputTotalMETKey", "", "key to write the total MET object"};
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputJetMETKey {
    this, "outputJetMETKey", "", "key to write the jet MET object"};
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputTowerMETKey {
    this, "outputTowerMETKey", "", "key to write the tower MET object"};
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputJwoJMETKey {
    this, "outputJwoJMETKey", "", "key to write the GEP JwoJ MET object"};
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputJwoJHardMETKey {
    this, "outputJwoJHardMETKey", "", "key to write the uncoefficiented GEP JwoJ hard term"};
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputJwoJSoftMETKey {
    this, "outputJwoJSoftMETKey", "", "key to write the uncoefficiented GEP JwoJ soft term"};

  // ---------------- algorithm enables -----------------------------
  Gaudi::Property<bool> m_doTotalMET{this, "DoTotalMET", true,
      "Write the total MET object (tower and jet terms combined with their scale factors)."};
  Gaudi::Property<bool> m_doJetMET{this, "DoJetMET", false,
      "Write the jet MET object. Computed regardless, since total MET needs it."};
  Gaudi::Property<bool> m_doTowerMET{this, "DoTowerMET", false,
      "Write the tower MET object. Computed regardless, since total MET needs it."};
  Gaudi::Property<bool> m_doGEPJwoJMET{this, "DoGEPJwoJMET", false,
      "Run and write the GEP JwoJ MET objects (combined, plus the hard and soft terms)."};

  // ---------------- physics thresholds / flow ---------------------
  Gaudi::Property<float> m_jetEtThresholdGeV{this, "JetEtThresholdGeV", 0.0,
      "Minimum jet E_T (GeV) entering the jet MET sum. Compared in digitized units."};
  Gaudi::Property<float> m_towerEtThresholdGeV{this, "TowerEtThresholdGeV", 0.0,
      "Minimum tower E_T (GeV) entering the tower MET sum. Compared in digitized units."};
  Gaudi::Property<bool> m_doJetTowerOverlapRemoval{this, "DoJetTowerOverlapRemoval", false,
      "Drop towers within the upstream jet cone radius of a jet that passed JetEtThresholdGeV."};
  Gaudi::Property<float> m_towerScaleFactor{this, "TowerScaleFactor", 1.0,
      "Scalar weight on tower MET in the total. Placeholder for an eta-binned calibration."};
  Gaudi::Property<float> m_jetScaleFactor{this, "JetScaleFactor", 1.0,
      "Scalar weight on jet MET in the total. Placeholder for an eta-binned calibration."};

  // ---------------- multiplicities --------------------------------
  Gaudi::Property<unsigned int> m_maxTowersConsidered{this, "MaxTowersConsidered", 4096,
      "Towers loaded per event."};

  // ---------------- digitization: field widths --------------------
  Gaudi::Property<unsigned int> m_etBitLength{this, "EtBitLength", 13, "E_T field width."};
  Gaudi::Property<unsigned int> m_signedEtBitLength{this, "SignedEtBitLength", 13,
      "Signed E_T field width (1 sign bit + magnitude)."};
  Gaudi::Property<unsigned int> m_etaBitLength{this, "EtaBitLength", 7, "Eta field width."};
  Gaudi::Property<unsigned int> m_phiBitLength{this, "PhiBitLength", 6, "Tower phi field width."};
  Gaudi::Property<unsigned int> m_sinBitLength{this, "SinBitLength", 13, "sin/cos LUT field width."};

  // ---------------- digitization: the tower grid ------------------
  // Sized by index COUNTS, never by the field widths above.
  Gaudi::Property<unsigned int> m_etaRange{this, "EtaRange", 98, "Number of eta towers."};
  Gaudi::Property<unsigned int> m_phiRange{this, "PhiRange", 64, "Number of phi towers."};
  Gaudi::Property<float> m_etaMin{this, "EtaMin", -4.85, "Center of the first eta tower."};
  Gaudi::Property<float> m_etaGranularity{this, "EtaGranularity", 0.1, "Eta tower width."};

  // ---------------- digitization: E_T -----------------------------
  Gaudi::Property<float> m_etMin{this, "EtMin", 0.0, "E_T range minimum (GeV)."};
  Gaudi::Property<float> m_etMax{this, "EtMax", 2048.0,
      "E_T range maximum (GeV). With EtBitLength 13 this gives the 0.25 GeV LSB."};
  Gaudi::Property<float> m_inputEtToGeV{this, "InputEtToGeV", 1.0e-3,
      "Input E_T (MeV) -> GeV scale."};

  // ---------------- MET output azimuth ----------------------------
  Gaudi::Property<unsigned int> m_metPhiBitLength{this, "METPhiBitLength", 6,
      "Width of the output phi_miss field. Unlike the tower grid, the output azimuth is "
      "SIZED BY THIS: it comes from arctan(Ey, Ex), so nothing physical caps its "
      "resolution and widening the field resolves more finely."};
  Gaudi::Property<unsigned int> m_metPhiTanScaleBitLength{this, "METPhiTanScaleBitLength", 10,
      "Fixed-point scale of the phi comparator ladder. Must grow with METPhiBitLength: "
      "roughly METPhiBitLength + 4 to stay within 0.501 bins of ideal atan2 binning."};

  // ---------------- MET magnitude LUT -----------------------------
  Gaudi::Property<unsigned int> m_sqrtLutIndexBitLength{this, "SqrtLutIndexBitLength", 9,
      "Normalized square-root LUT index width: the bits taken from just below the "
      "radicand's leading one, which address the ROM."};
  Gaudi::Property<unsigned int> m_sqrtFracBitLength{this, "SqrtFracBitLength", 13,
      "Normalized square-root LUT coefficient fractional bits (Q1.13)."};
  Gaudi::Property<unsigned int> m_sqrtCoeffBitLength{this, "SqrtCoeffBitLength", 14,
      "Normalized square-root LUT coefficient width."};
  Gaudi::Property<unsigned int> m_sqrtRadicandBitLength{this, "SqrtRadicandBitLength", 26,
      "Normalized square-root LUT radicand width."};

  // ---------------- GEP JwoJ --------------------------------------
  Gaudi::Property<float> m_jwojHardEtThresholdGeV{this, "JwoJHardEtThresholdGeV", 7.5,
      "Block E_T (GeV) above which a tower block joins the GEP JwoJ hard term."};
  Gaudi::Property<unsigned int> m_jwojBlockSize{this, "JwoJBlockSize", 1,
      "Side of the square tower block, in towers: 1 (0.1x0.1) or 2 (0.2x0.2). Must tile "
      "both axes of the tower grid exactly."};
  Gaudi::Property<float> m_jwojHardCoeff{this, "JwoJHardCoeff", 1.0,
      "Coefficient on the GEP JwoJ hard term. Separate from JetScaleFactor, so total MET "
      "can run at unit weights while JwoJ uses its own."};
  Gaudi::Property<float> m_jwojSoftCoeff{this, "JwoJSoftCoeff", 0.3,
      "Coefficient on the GEP JwoJ soft term. Separate from TowerScaleFactor."};

  // Configured once in initialize() (builds the LUTs); used read-only in execute().
  Gep::TotalMETMaker m_metMaker;

  StatusCode configureMETMaker();

  // ---- float front end ----
  // Digitize the input collections onto the configured grid. Eta and phi are digitized from floats
  std::vector<Gep::TotalMETMaker::DigiObj>
  digitizeTowers(const xAOD::CaloClusterContainer& clusters) const;

  std::vector<Gep::TotalMETMaker::DigiObj>
  digitizeJets(const xAOD::JetContainer& jets) const;

  // Pack a single term into an EnergySumRoI and record it. The components and SumET go through
  // the TOB encoders first, so what is written is what the firmware would emit
  // (saturated, not wrapped) rather than the full-width accumulator.
  StatusCode recordMET(const SG::WriteHandleKey<xAOD::EnergySumRoI>& key,
                       const EventContext& ctx,
                       const Gep::TotalMETMaker::DigiMETTerm& term) const;
};

#endif // TRIGGEPPERF_TOTALMETALG_H
