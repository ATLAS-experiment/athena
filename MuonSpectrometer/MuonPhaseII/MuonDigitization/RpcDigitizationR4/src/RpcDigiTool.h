/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef RPC_DIGITIZATIONR4_RPCDIGITOOL_H
#define RPC_DIGITIZATIONR4_RPCDIGITOOL_H

#include "Acts/Utilities/detail/Polynomials.hpp"
#include "CLHEP/Random/RandExponential.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MuonCondData/DigitEffiData.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonDigitizationR4/MuonDigitizationTool.h"

namespace MuonR4 {
class RpcDigiTool final : public MuonDigitizationTool {
public:
  using MuonDigitizationTool::MuonDigitizationTool;

  StatusCode initialize() override final;
  StatusCode finalize() override final;

protected:
  StatusCode
  digitize(const EventContext &ctx, const TimedHits &hitsToDigit,
           xAOD::MuonSimHitContainer *sdoContainer) const override final;

private:
  using EdgeSide = MuonGMR4::RpcReadoutElement::EdgeSide;

  /**  @brief Digitize the sim hit as Rpc strip 1D hit.
   *   @param simHit: Reference to the sim hit to digitize
   *   @param measuresPhi: Process an eta or a phi strip
   *   @param effiMap: Pointer to an efficiency look-up table
   *   @param outContainer: DigitCollection to push the new digit into
   *   @param rndEngine:  Random engine used for smearing
   *   @param deadTimes: Reference to the last digitized times in order to apply
   * the dead time model */
  bool digitizeHit(const TimedHit &simHit, const bool measuresPhi,
                   const Muon::DigitEffiData *effiMap,
                   RpcDigitCollection &outContainer,
                   CLHEP::HepRandomEngine *rndEngine,
                   DeadTimeMap &deadTimes) const;

  /**  @brief Digitize the sim hit as Rpc strip 2D hit.
   *   @param simHit: Reference to the sim hit to digitize
   *   @param effiMap: Pointer to an efficiency look-up table
   *   @param outContainer: DigitCollection to push the new digit into
   *   @param rndEngine:  Random engine used for smearing
   *   @param deadTimes: Reference to the last digitized times in order to apply
   * the dead time model */
  bool digitizeHitBI(const TimedHit &simHit, const Muon::DigitEffiData *effiMap,
                     RpcDigitCollection &outContainer,
                     CLHEP::HepRandomEngine *rndEngine,
                     DeadTimeMap &deadTimes) const;

  using DigiCache = OutDigitCache_t<RpcDigitCollection>;
  SG::WriteHandleKey<RpcDigitContainer> m_writeKey{this, "OutputObjectName",
                                                   "RPC_DIGITS"};

  SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{
      this, "EffiDataKey", "RpcDigitEff",
      "Efficiency constants of the individual Rpc gasGaps"};

  mutable std::array<std::atomic<unsigned>, 2> m_allHits ATLAS_THREAD_SAFE{};
  mutable std::array<std::atomic<unsigned>, 2> m_acceptedHits
      ATLAS_THREAD_SAFE{};

  Gaudi::Property<double> m_propagationVelocity{
      this, "propSpeed", 0.5 * Gaudi::Units::c_light,
      "Propagation speed of the signal inside the strip"}; // in mm/ns

  Gaudi::Property<double> m_stripTimeResolution{
      this, "timeResolution", 0.6 * Gaudi::Units::nanosecond,
      "Estimated time resolution of the strip readout"};

  Gaudi::Property<double> m_deadTime{this, "deadTime",
                                     100. * Gaudi::Units::nanosecond};

  Gaudi::Property<bool> m_digitizeMuonOnly{
      this, "ProcessTrueMuonsOnly", false,
      "If set to true hit with pdgId != 13 are skipped"};

  // Methods for new BIRPC digitization
  //
  /** @brief Returns cluster strip molteplicity.
   *  @param id: Reference to the chamber identifier
   *  @param rndmEngin: Random engine used for probabilities */
  int determineClusterSize(const Identifier &id,
                             CLHEP::HepRandomEngine *rndmEngine, bool isBIRPC) const;

  /** @brief Returns the charge (fC) after the amplification in gas
   *  @param simHit: Reference to the sim hit to digitize
   *  @param rndmEngine: A random engine*/
  double calculateChargeOnStrip(const TimedHit &simHit,
                                CLHEP::HepRandomEngine *rndmEngine,
                                const double gasGapSize) const;

  /** @brief Returns a vector with chages (fC) divided on strips
   *  @param totalCharge: total charge from a single spike (fC)
   *  @param n_strips: number of strips to divide the charge over
   *  @param rndmEngine: A random engine*/
  std::vector<double>
  divideChargeOnStrips(double totalCharge, int n_strips,
                       CLHEP::HepRandomEngine *rndmEngine) const;

  /** @brief Returns Time Over Threshold (ns) for a signal on a strip
   *  @param aCharge: electric charge deposited on the strip (fC) */
  double getTOT(const double aCharge) const; 

  /** @brief Returns Time Of Arrival (ns) for a signal on a strip
   *  @param aCharge: electric charge deposited on the strip (fC)
   *  @param aDistance: distance from hit to strip edge (m) */
  double getTOA(const double aCharge, const double aDistance) const;
};

} // namespace MuonR4

#endif
