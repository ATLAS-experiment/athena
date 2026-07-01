/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PixelClusterOnTrackTool_H
#define PixelClusterOnTrackTool_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/PixelRIO_OnTrackErrorScaling.h"

#include "InDetPrepRawData/PixelGangedClusterAmbiguities.h"
#include "TrkParameters/TrackParameters.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"

#include "PixelConditionsData/PixelOfflineCalibData.h"
#include "PixelConditionsData/PixelDistortionData.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "PixelGeoModel/IIBLParameterSvc.h"

#include "TrkEventUtils/ClusterSplitProbabilityContainer.h"

#include <atomic>
#include <mutex>

namespace InDet {

  /** @brief creates PixelClusterOnTrack objects allowing to
    calibrate cluster position and error using a given track hypothesis. 

    See doxygen of Trk::RIO_OnTrackCreator for details.
    Different strategies to calibrate the cluster error can be chosen
    by job Option. Also the handle to the general hit-error scaling
    is implemented.

    Special strategies for correction can be invoked by calling the
    correct method with an additional argument from the 
    PixelClusterStrategy enumeration

   */

  class NnClusterizationFactory;

  enum PixelClusterStrategy {
    PIXELCLUSTER_DEFAULT=0,
    PIXELCLUSTER_OUTLIER=1,
    PIXELCLUSTER_SHARED =2,
    PIXELCLUSTER_SPLIT  =3
  };

  class PixelClusterOnTrackTool final: 
        public AthAlgTool, virtual public Trk::IRIO_OnTrackCreator
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:

  //! AlgTool constructor 
  PixelClusterOnTrackTool(const std::string&,const std::string&,
                          const IInterface*);
  virtual ~PixelClusterOnTrackTool ();
  //! AlgTool initialisation
  virtual StatusCode initialize() override;
  //! AlgTool termination
  virtual StatusCode finalize  () override;

  /** @brief produces a PixelClusterOnTrack (object factory!).

      Depending on job options it changes the pixel cluster position
      and error according to the parameters (in particular, the angle)
      of the intersecting track.
  */
  virtual InDet::PixelClusterOnTrack* correct(
      const Trk::PrepRawData&, const Trk::TrackParameters&, const EventContext&) const override;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////

protected:
  void correctBow(const Identifier&, Amg::Vector2D& locpos, const double tanphi, const double taneta, const EventContext& ctx) const;

  InDet::PixelClusterOnTrack* correctDefault(const Trk::PrepRawData&,
                                             const Trk::TrackParameters&,
                                             const EventContext& ctx) const;

  InDet::PixelClusterOnTrack* correctNN(const Trk::PrepRawData&,
                                        const Trk::TrackParameters&,
                                        const EventContext& ctx) const;

  bool getErrorsDefaultAmbi( const InDet::PixelCluster*, const Trk::TrackParameters&,
                             Amg::Vector2D&,  Amg::MatrixX&, const EventContext&) const;

  bool getErrorsTIDE_Ambi( const InDet::PixelCluster*, const Trk::TrackParameters&,
                           Amg::Vector2D&,  Amg::MatrixX&, const EventContext&) const;

  InDet::PixelClusterOnTrack* correct(const Trk::PrepRawData&,
                                      const Trk::TrackParameters&,
                                      const InDet::PixelClusterStrategy,
                                      const EventContext&) const;

  const Trk::ClusterSplitProbabilityContainer::ProbabilityInfo &getClusterSplittingProbability(const InDet::PixelCluster*pix, const EventContext& ctx) const {
      if (!pix || m_clusterSplitProbContainer.key().empty())  return Trk::ClusterSplitProbabilityContainer::getNoSplitProbability();

      SG::ReadHandle<Trk::ClusterSplitProbabilityContainer> splitProbContainer(m_clusterSplitProbContainer, ctx);
      if (!splitProbContainer.isValid()) {
         ATH_MSG_FATAL("Failed to get cluster splitting probability container " << m_clusterSplitProbContainer);
      }
      return splitProbContainer->splitProbability(pix);
  }

private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////

  SG::ReadCondHandleKey<PixelDistortionData> m_distortionKey
  {this, "PixelDistortionData", "PixelDistortionData", "Output readout distortion data"};

  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool
    {this, "LorentzAngleTool", "SiLorentzAngleTool", "Tool to retrieve Lorentz angle"};

  SG::ReadCondHandleKey<PixelCalib::PixelOfflineCalibData> m_clusterErrorKey
    {this, "PixelOfflineCalibData", "PixelOfflineCalibData", "Output key of pixel cluster"};
  SG::ReadCondHandleKey<RIO_OnTrackErrorScaling> m_pixelErrorScalingKey
    {this,"PixelErrorScalingKey", "/Indet/TrkErrorScalingPixel", "Key for pixel error scaling conditions data."};

  //! toolhandle for central error scaling
  //! flag storing if errors need scaling or should be kept nominal
  BooleanProperty m_disableDistortions{this, "DisableDistortions", false,
    "Disable simulation of module distortions"};
  IntegerProperty m_positionStrategy{this, "PositionStrategy", 1,
    "Which calibration of cluster positions"};
  mutable std::atomic_int m_errorStrategy{2};
  IntegerProperty  m_errorStrategyProperty
    {this, "ErrorStrategy", 2, "Which calibration of cluster position errors"};
  
  
  /** @brief Flag controlling how module distortions are taken into account:
      
  case 0 -----> No distorsions implemented;
  
  case 1 -----> Set curvature (in 1/meter) and twist (in radiant) equal for all modules;
  
  case 2 -----> Read curvatures and twists from textfile containing Survey data;
  
  case 3 -----> Set curvature and twist from Gaussian random generator with mean and RMS coming from Survey data;
  
  case 4 -----> Read curvatures and twists from database (not ready yet);
  */
  //! identifier-helper
  const PixelID* m_pixelid = nullptr;
  
  /** Enable NN based calibration (do only if NN calibration is applied) **/
  BooleanProperty m_applyNNcorrectionProperty{this, "applyNNcorrection", false};
  bool m_applyNNcorrection = false; // Updated depending on other configs
  BooleanProperty m_NNIBLcorrection{this, "NNIBLcorrection", false};
  bool m_IBLAbsent = true;
  
  /** NN clusterizationi factory for NN based positions and errors **/
  ToolHandle<NnClusterizationFactory> m_NnClusterizationFactory
    {this, "NnClusterizationFactory",
     "InDet::NnClusterizationFactory/NnClusterizationFactory"};
  ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc
    {this, "IBLParameterSvc", "IBLParameterSvc"};

  BooleanProperty m_doNotRecalibrateNN{this, "doNotRecalibrateNN", false};
  BooleanProperty m_noNNandBroadErrors{this, "noNNandBroadErrors", false};
  /** Enable different treatment of  cluster errors based on NN information (do only if TIDE ambi is run) **/
  BooleanProperty m_usingTIDE_Ambi{this, "RunningTIDE_Ambi", false};
  SG::ReadHandleKey<InDet::PixelGangedClusterAmbiguities> m_splitClusterMapKey
    {this, "SplitClusterAmbiguityMap", ""};

  SG::ReadHandleKey<Trk::ClusterSplitProbabilityContainer> m_clusterSplitProbContainer
    {this, "ClusterSplitProbabilityName", "",""};

  //moved from static to member variable
  static constexpr int s_nbinphi=9;
  static constexpr int s_nbineta=6;
  double m_calphi[s_nbinphi]{};
  double m_caleta[s_nbineta][3]{};
  double m_calerrphi[s_nbinphi][3]{};
  double m_calerreta[s_nbineta][3]{};
  double m_phix[s_nbinphi+1]{};
  double m_etax[s_nbineta+1]{};
};

} // end of namespace InDet

#endif // PixelClusterOnTrackTool_H
