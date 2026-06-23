/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENTCNV_ActsToTrkConverterTool_H
#define ACTSEVENTCNV_ActsToTrkConverterTool_H

#include <tuple>

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"


#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

#include "TrkParameters/TrackParameters.h" //typedef, cannot fwd declare
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"

#include "TrkPrepRawData/PrepRawData.h"


// PACKAGE
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "ActsCalibBase/SourceLinkType.h"
#include "ActsCalibrators/TrkMeasurementCalibrator.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

#include "Acts/EventData/BoundTrackParameters.hpp"


#include "MuonRecToolInterfaces/IMuonCompetingClustersOnTrackCreator.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace ActsTrk {
class ActsToTrkConverterTool : public extends<AthAlgTool, IActsToTrkConverterTool>
{

public:
  virtual StatusCode initialize() override;

  using base_class::base_class;



  /// Find the ATLAS surface corresponding to the Acts surface 
  /// Only work if the Acts surface has an associated detector element
  /// (Pixel and SCT)
  virtual SurfacePtr_t actsSurfaceToTrkSurface(const EventContext& ctx,
                                               const Acts::Surface &actsSurface) const override;

  /// Find the Acts surface corresponding to the ATLAS surface 
  /// Use a map associating ATLAS ID to Acts surfaces
  /// (Pixel and SCT)
  virtual std::shared_ptr<const Acts::Surface> trkSurfaceToActsSurface(const Trk::Surface &atlasSurface) const override;

  /// Transform an ATLAS track into a vector of SourceLink to be use in the avts tracking
  /// Transform both measurement and outliers.
  virtual std::vector<Acts::SourceLink> trkTrackToSourceLinks(const Trk::Track& track) const override;

  virtual void toSourceLinks(const std::vector<const Trk::MeasurementBase*>& measSet,
                              std::vector<Acts::SourceLink>& links) const override final;

  virtual void toSourceLinks(const std::vector<const Trk::PrepRawData*>& prdSet,
                             std::vector<Acts::SourceLink>& links) const override final;

  virtual std::unique_ptr<Trk::Track> convertFitResult(const EventContext& ctx,
                                                       TrackFitResult_t& fitResult,
                                                       const Trk::TrackInfo::TrackFitter fitAuthor) const override final;
  /// Create Acts TrackParameter from ATLAS one.
  /// Take care of unit conversion between the two.  
  virtual
  const Acts::BoundTrackParameters
  trkTrackParametersToActsParameters(const Trk::TrackParameters &atlasParameter, const Acts::GeometryContext& gctx, Trk::ParticleHypothesis = Trk::pion) const override;

  /// Create ATLAS TrackParameter from Acts one.
  /// Take care of unit conversion between the two.  
  virtual
  std::unique_ptr<Trk::TrackParameters>
  actsTrackParametersToTrkParameters(const EventContext& ctx, const Acts::BoundTrackParameters &actsParameter, const Acts::GeometryContext& gctx) const override;

  /** Convert TrackCollection to Acts track container. 
   * @param tc The track container to fill
  */
  virtual 
  void trkTrackCollectionToActsTrackContainer(ActsTrk::MutableTrackContainer &tc, const TrackCollection& trackColl, const Acts::GeometryContext& gctx) const override;

 virtual std::unique_ptr<TrackCollection> 
      convertActsToTrkContainer(const EventContext& ctx,
                                const ActsTrk::TrackContainer& trackCont) const override final;
private:
  /** @brief Helper function to convert a Acts TrackPoxy (which may be const or not)
   *         into a Trk::Track
   * @param ctx: The EventContext to access the current calibration constants
   * @param track: Reference to the track state proxy for translation
   * @param fitAuthor: Author that is written in the track summary info */
  template <typename Proxy_t>
  std::unique_ptr<Trk::Track> convertActsTrack(const EventContext& ctx,
                                               const Proxy_t& track,
                                               const Trk::TrackInfo::TrackFitter fitAuthor) const;

  /*** @brief Translate the Acts surface bounds to its equivalent in the Trk realm.
   *          @note Not all bounds are implemented
   *  @param bounds: Refrence to the bounds to be translated */
  std::shared_ptr<Trk::SurfaceBounds> translateBounds(const Acts::SurfaceBounds& bounds) const;
  /** @brief Translate a surface that is not associated with any detector element.
   *         Bounds of the surface are also translated
   *  @param surface Reference to the Acts surface for translation */
  SurfacePtr_t translateFreeSurface(const Acts::Surface& surface) const;

  /** @brief Abrivate the state mask for the TSOS */
  using  TrkTSOSMask = std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>;
  /** @brief Append the translated TSOS at the beginning of the states container corresponding
   *         to the parsed measurement.
   * @param ctx: EventContext to access the calibration conditions data needed for 
   *             the Trk::RIO_OnTrack conversion and to access the PrepRawData
   * @param meas: Pointer to the uncalibrated measurement for conversion
   * @param typePattern: Bit mask stating whether the measurement is on track 
   *                     or an outlier
   * @param quality: Chi2 and nDOF contribution from the measurement
   * @param trkPars: The track parameters on the surface
   * @param states: Output container into which the new TSOS is pushed */
  void appendMeasTSOS(const EventContext& ctx,
                      const xAOD::UncalibratedMeasurement* meas,
                      const TrkTSOSMask typePattern,
                      Trk::FitQualityOnSurface&& quality,
                      std::unique_ptr<Trk::TrackParameters> trkPars,
                      Trk::TrackStates& states) const;
   /** @brief Searches a Prd object from a collection according to the 
    *         measurement's Identifier and the container's hash
    *  @param ctx: EventContext to retrieve the container from StoreGate
    *  @param key: ReadHandleKey specifying which container is to be fetched
    *  @param prdId: Identifier of the measurement to retrieve
    *  @param hash: The associated IdentifierHash to look-up the collection */
  template <typename PrdType_t>
  const Trk::PrepRawData* fetchPrd(const EventContext& ctx,
                                   const SG::ReadHandleKey<PrdType_t>& key,
                                   const Identifier& prdId,
                                   const IdentifierHash& hash) const;


  bool actsTrackParameterPositionCheck(
     const Acts::BoundTrackParameters& actsParameter,
     const Trk::TrackParameters& tsos, const Acts::GeometryContext& gctx) const;

  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  
  /** @brief Tools needed to create Trk::Tracks from the ACts fit result */
  ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool {this, "SummaryTool", "", "ToolHandle for track summary tool"};
  ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator {this, "RotCreatorTool", ""};

  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry{};
  std::unordered_map<Identifier, std::shared_ptr<const Acts::Surface>> m_actsSurfaceMap{};

  Gaudi::Property<bool> m_visualDebugOutput{
     this, "VisualDebugOutput", false,
     "Print additional output for debug plots"};


  Gaudi::Property<bool> m_extractMuonSurfaces{this, "ExtractMuonSurfaces", false,
     "If True, use the MuonDetectorManager to extract the Muon surfaces"};
  /** @brief Flag to convert the hole states */
  Gaudi::Property<bool> m_convertHoles{this, "convertHoles", true };
  /** @brief Flag to convert the outlier states */
  Gaudi::Property<bool> m_convertOutliers{this, "convertOutliers", true};
  /** @brief Flag to convert the material states (non sensitive) Acts -> Trk conversion */
  Gaudi::Property<bool> m_convertMaterial{this, "convertMaterial", false};

  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  
  SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_keyMdt{this, "MdtKey", "MDT_DriftCircles"};
  SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_keyRpc{this, "RpcKey", "RPC_Measurements"};
  SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_keyTgc{this, "TgcKey", "TGC_MeasurementsAllBCs"};
  SG::ReadHandleKey<Muon::MMPrepDataContainer> m_keyMm{this, "MmKey", "MM_Measurements"};
  SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_keyStgc{this, "sTgcKey", "STGC_Measurements"};


  ToolHandle<Muon::IMuonCompetingClustersOnTrackCreator> m_compRotCreator{this, "CompetingRotCreator", ""};  //<! competing clusters rio ontrack creator

  /** @brief Detector manager to fetch the legacy Trk surfaces */
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muonMgrKey{this, "MuonManagerKey", "MuonDetectorManager"};

  detail::TrkMeasurementCalibrator m_measCalib{};
  detail::TrkPrepRawDataCalibrator m_prdCalib{};

  Trk::TrackInfo::TrackFitter m_fitAuthor{Trk::TrackInfo::TrackFitter::GlobalChi2Fitter};

};

}; // namespace ActsTrk

#endif
