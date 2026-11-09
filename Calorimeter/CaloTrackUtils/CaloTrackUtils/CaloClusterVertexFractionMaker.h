/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERVERTEXFRACTIONMAKER_H
#define CALOCLUSTERVERTEXFRACTIONMAKER_H

#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include <string>
#include <vector>

class CaloClusterContainer;
namespace Trk {
  class Surface;
}

class CaloClusterVertexFractionMaker: public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:
  CaloClusterVertexFractionMaker(const std::string& type, const std::string& name, const IInterface* parent);
  ~CaloClusterVertexFractionMaker();

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override;

 private:
  static double calculateDPhi(double phi1, double phi2) ;

    const double m_CALO_INNER_R{1450.}; // inner radius of the EM barrel envelope in mm
    const double m_CALO_INNER_Z{3641.}; // millimeter
//     const double INV_CALO_INNER_R; // the inverse for error calculation
    Gaudi::Property<double> m_dRMatchMax{this, "dRMatchMax", 0.1,
      "Maximum dR for the track to be matched to the cluster"};  // maximum value in dR for the track to be matched to the cluster, can be set by jobOptions
    double m_dR2MatchMax{0.}; // will be overwritten in initialize. m_dRMatchMax * m_dRMatchMax, this is used to avoid taking the square root all the time for each track and cluster
    Gaudi::Property<double> m_maxClusterEta{this, "maxClusterEta", 2.5,
      "Maximum |eta| of clusters for which a CVF is calculated"}; // maximum |eta| of clusters for which a CVF is calculated

    // Pre-configured extrapolator
    ToolHandle< Trk::IExtrapolator > m_extrapolator{this, "Extrapolator",
      "Trk::Extrapolator/AtlasExtrapolator", "Pre-configured extrapolator"};

    // name of VxContainer
    Gaudi::Property<std::string> m_vxContainerName{this, "VxContainerName",
      "VxPrimaryCandidate", "Name of VxContainer"};

    std::unique_ptr<Trk::Surface> m_cylinderSurface_atCaloEntrance; // to extrapolate to clusters in barrel
    std::unique_ptr<Trk::Surface> m_discSurface_atCaloEntrance_positiveZ; // to extrapolate to clusters in endcap, this surface has to be created new for each cluster ...
    std::unique_ptr<Trk::Surface> m_discSurface_atCaloEntrance_negativeZ; // to extrapolate to clusters in endcap, this surface has to be created new for each cluster ...
};

#endif // CALOCLUSTERVERTEXFRACTIONMAKER_H






