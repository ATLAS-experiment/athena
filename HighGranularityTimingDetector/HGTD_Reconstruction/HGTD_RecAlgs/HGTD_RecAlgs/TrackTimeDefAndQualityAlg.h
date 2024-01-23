/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/TrackTimeDefAndQualityAlg.h
 * @author Valentina Raskina <valentina.raskina@cern.ch>
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date December, 2022
 *
 * @brief Gets the track time extension and checks the time compatibility,
 * defines a track time (and resolution) and sets a bitfield to encode the time
 * quality.
 *
 * The summary info will be a 32 bit field, containing the structure explained
 * below. The subparts contain hit patterns, where the layer closest to the IP
 * is the LSB of the subpattern. e.g. 0101 for reco'ed means that hits were
 * found in layer 0 and 2 of HGTD, no hits found in layer 1 and 3.
 *
 * | Bit   | 15        12 | 11            8 | 7        4 |3        0 |
 * | Size  | 4            |          4      |   4        | 4         |
 * | Value |  ITk holes   | after time comp |  expected  |  reco'ed  |
 *
 * | Bit    | 31       28 | 27           24 | 23      20 |19      16 |
 * | Size   | 4           | 4               | 4          | 4         |
 * | Value  | unassigned  |   unasigned     | potential  |   primes  |
 *
 * if need be, could be extended to 64 bits in the future.
 *
 * NOTE:
 * - no hole search implemented in HGTD atm, so "expected" is empty for now
 * - hole search after the last hit in ITK not implemented, only checking if
 *   the last hit is within a certain area. For now, ITk holes encodes if a
 *   last hit was missing close to HGTD:
 *   1 = no last hit on track within accepted volume, 0 = hit found.
 *
 * TODO:
 *  - meanTime should compute a resolution weighted mean. To be implemented
 *    once radiation damage is included.
 *  - change chi2 to chi2/ndof?
 */

#ifndef HGTD_RECALGS_TRACKTIMEDEFANDQUALITYALG_H
#define HGTD_RECALGS_TRACKTIMEDEFANDQUALITYALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "GeneratorObjects/McEventCollection.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
// #include <string>

namespace {
static constexpr unsigned short n_hgtd_layers = 4; // two double sided layers
} // namespace

namespace HGTD {

class TrackTimeDefAndQualityAlg : public AthReentrantAlgorithm {

private:
  // different shift distances for bitfield definition
  const short m_recoed_ptrn_sft = 0;
  //const short m_exp_ptrn_sft = 4;
  const short m_comp_ptrn_sft = 8;
  const short m_holes_ptrn_sft = 12;
  const short m_primes_ptrn_sft = 16;

public:
  TrackTimeDefAndQualityAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrackTimeDefAndQualityAlg() {}
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{
      this, "TrackParticleContainerName", "InDetTrackParticles",
      "Name of the TrackParticle container"};

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerHasExtensionKey{
      this, "HGTD_has_extension", "InDetTrackParticles.HGTD_has_extension",
      "deco with a handle for an extension"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTimeKey{
      this, "HGTD_cluster_time", "InDetTrackParticles.HGTD_cluster_time",
      "deco with a handle for cluster time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>
      m_layerClusterTruthClassKey{
          this, "HGTD_cluster_truth_class",
          "InDetTrackParticles.HGTD_cluster_truth_class",
          "deco with a handle for a truth time"};

  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_time_dec_key{
      this, "time", "InDetTrackParticles.time", "Time assigned to this track"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_time_res_dec_key{
      this, "timeResolution", "InDetTrackParticles.timeResolution",
      "Time resolution assigned to this track"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>
      m_summarypattern_dec_key{this, "HGTD_summaryinfo",
                               "InDetTrackParticles.HGTD_summaryinfo",
                               "Bitfield for working point definition"};

  // TODO: the resolution is fixed to 0.035 ps, should add the resolution
  // calculation after the irradiation
  struct Hit {
    float m_time = 0.;
    float m_resolution = 0.035;
    bool m_isprime = false;
    bool m_isvalid = false;
    short m_layer = -1;
  };

  struct CleaningResult {
    std::array<Hit, n_hgtd_layers> m_hits;
    uint32_t m_field = 0x0;
    float m_time;
    float m_resolution;
  };

  FloatProperty m_chi2_threshold{
      this, "Chi2Threshold", 1.5,
      "Quality cut for decision to keep hits compatible in time"};
  FloatProperty m_deltat_cut{this, "DeltaTCut", 2.0,
                             "Upper limit for a cluster delta t cut"};

  FloatProperty m_default_time{
      this, "DefaultTime", 0.0,
      "Default time used for tracks without HGTD timing info"};
  // time resolution default value, set to +- half a bunch crossing, using flat
  // distribution
  FloatProperty m_default_time_res{
      this, "DefaultTimeRes", 50. / std::sqrt(12.),
      "Default time resolution used for tracks without HGTD timing info"};

  CleaningResult
  runTimeConsistencyCuts(const xAOD::TrackParticle* track_particle) const;

  std::array<Hit, n_hgtd_layers>
  getValidHits(const xAOD::TrackParticle* track_particle) const;

  /**
   * @brief Calculates the chi2 of the hit times given their resolution.
   *
   * @param [in] hits Array of hits.
   *
   * @return Chi2 value.
   */
  float calculateChi2(const std::array<Hit, n_hgtd_layers>& hits) const;

  // float calculateChi2(const std::vector<Hit>& hits) const;

  /**
   * @brief Checks two hits for time compatibility
   *
   * @param [in] hits Array of hits.
   *
   * @return Returns true if the times were in agreement.
   */
  bool passesDeltaT(const std::array<Hit, n_hgtd_layers>& hits) const;

  /**
   * @brief Calculates the arithmetic mean of the valid hit times;
   *
   *
   * @param [in] hits Array of hits.
   *
   * @return Returns true if the times were in agreement.
   */
  float meanTime(const std::array<Hit, n_hgtd_layers>& hits) const;

  /**
   * @brief Calculates the combined resolution.
   */
  float trackTimeResolution(const std::array<Hit, n_hgtd_layers>& hits) const;

  /**
   * @brief Identifies time outliers by finding the layer within which a hit
   * contributes negatively to the overall chi2 value and returns the layer
   * number. Copies the hits, since they have to be invalidated one by one.
   *
   * @param [in] hits Array of hits.
   *
   * @return Layer with outlier time, value between 0 and 3.
   */
  short findLayerWithBadChi2(std::array<Hit, n_hgtd_layers> hits) const;

  /**
   * @brief Given a layer number, the hit sitting on this layer is flagged as
   * invalid. Used for outlier removal.
   *
   * @param [in] hits Array of hits.
   * @param [in] layer The layer that should be masked.
   */
  void setLayerAsInvalid(std::array<Hit, n_hgtd_layers>& hits,
                         short layer) const;

  /**
   * @brief Returns the pattern of valid hits in HGTD as a 4-bit bitfield, where
   * a 1 encodes that a valid hit was found, while a 0 means no valid hit found.
   * The LSB carries the hit information for the layer closest to the IP.
   *
   * @param [in] hits Array of hits.
   *
   * @return Bit pattern encoding valid hits in layers.
   */
  short getValidPattern(const std::array<Hit, n_hgtd_layers>& hits) const;

  /**
   * @brief Checks if the last hit on track was found on a pre-specified set of
   * Pixel and Strip layers close to the HGTD surfaces.
   * FIXME: should not be hardcoded. To be exchanged by a hole search at a later
   * point.
   *
   * @param [in] track_particle Track reconstructed in ITk.
   *
   * @return True if the last hit on track was within the specified volume.
   */
  bool lastHitIsOnLastSurface(const xAOD::TrackParticle& track_particle) const;

  const Trk::TrackParameters* getLastHitOnTrack(const Trk::Track& track) const;
};

} // namespace HGTD

#endif // HGTD_RECALGS_TRACKTIMEDEFANDQUALITYALG_H
