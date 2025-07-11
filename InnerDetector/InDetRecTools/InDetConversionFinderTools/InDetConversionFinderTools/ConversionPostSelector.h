/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETCONVERSIONFINDERTOOLS_CONVERSIONPOSTSELECTOR_H
#define INDETCONVERSIONFINDERTOOLS_CONVERSIONPOSTSELECTOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkParameters/TrackParameters.h" //typedef
#include "xAODTracking/VertexFwd.h"
#include "GeoPrimitives/GeoPrimitives.h" //Amg::Vector3D typedef
#include "TruthUtils/ParticleConstants.h"
#include <vector>



namespace CLHEP{
  class HepLorentzVector;
}

namespace InDet {

  /**
     @class ConversionPostSelector
     This class selects tracks for conversion finder
     @author Tatjana Lenz , Thomas Koffas
  */

  class ConversionPostSelector : public AthAlgTool {

  public:
    ConversionPostSelector (const std::string& type,const std::string& name, const IInterface* parent);
    virtual ~ConversionPostSelector() = default;

    static const InterfaceID& interfaceID();
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    /** Conversion candidate post-fit selectors. Return true if the argument
     * track fulfills the selection*/
    bool selectConversionCandidate(xAOD::Vertex* myCandidate,
                                   int flag,
                                   std::vector<Amg::Vector3D>& trkL) const;
    bool selectSecVtxCandidate(xAOD::Vertex* myCandidate,
                               int flag,
                               std::vector<Amg::Vector3D>& trkL,
                               int&) const;

    /** Decorate vertices with values used in post selector **/
    static void decorateVertex(xAOD::Vertex& vertex,
                        float inv_mass,
                        float pt1,
                        float pt2,
                        float fR,
                        float deltaPhiVtxTrk) ;

  private:
    /** Properties for track selection:
	all cuts are ANDed */
    DoubleArrayProperty m_maxChi2
      {this, "MaxChi2Vtx", {35., 25., 20.}, "Chi2 cut"};
    DoubleArrayProperty m_invMassCut
      {this, "MaxInvariantMass", {10000., 10000., 10000.}, "Invariant mass cut"};
    DoubleArrayProperty m_fitMomentum
      {this, "MinFitMomentum", {0., 0., 0.},
       "Converted photon reconstructed momentum at vertex cut"};
    DoubleArrayProperty m_minRadius
      {this, "MinRadius", {-10000., -10000., -10000.},
       "Converted photon reconstructed vertex radial position cut"};
    DoubleProperty m_minPt {this, "MinPt", 0.,
      "Pt of the two participating tracks at the vertex"};
    DoubleProperty m_maxdR {this, "MaxdR", -10000.,
      "Distance of first track hit- reconstructed vertex radial position"};
    DoubleProperty m_maxPhiVtxTrk{this, "MaxPhiVtxTrk", 0.2,
      "Maximum difference in phi between reconstructed vertex and track at vertex"};
    BooleanProperty m_decorateVertices{this, "DecorateVertices", true,
      "Decorate vertices with values used for vertex selection"};


    /** Masses and mass ranges for different V0 hypotheses */
    static constexpr double m_massK0 = ParticleConstants::KZeroMassInMeV;
    static constexpr double m_sigmaK0 = 8.5;
    static constexpr double m_massLambda = ParticleConstants::lambdaMassInMeV;
    static constexpr double m_sigmaLambda = 3.5;
    IntegerProperty m_nsig{this, "NSigma", 5};


    /** Compute the four-momentum of a particle according to a mass hypothesis.  */
    CLHEP::HepLorentzVector fourP(const Trk::TrackParameters&, const Trk::TrackParameters&, double, bool) const;
  };

}
#endif // INDETCONVERSIONFINDERTOOLS_CONVERSIONPOSTSELECTOR_H

