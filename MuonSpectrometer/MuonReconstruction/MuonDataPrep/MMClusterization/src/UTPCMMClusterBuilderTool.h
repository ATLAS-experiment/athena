/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef UTPCMMClusterBuilderTool_h
#define UTPCMMClusterBuilderTool_h

#include <tuple>

#include "AthenaBaseComps/AthAlgTool.h"
#include "MMClusterization/IMMClusterBuilderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "TF1.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TGraphErrors.h"
#include "TH2D.h"
#include "TLinearFitter.h"
#include "TMatrixDSym.h"

//
// Simple clusterization tool for MicroMegas
//
namespace Muon {

    class UTPCMMClusterBuilderTool : virtual public IMMClusterBuilderTool, public AthAlgTool {
    public:
        /** Default constructor */
        UTPCMMClusterBuilderTool(const std::string&, const std::string&, const IInterface*);

        /** Default destructor */
        virtual ~UTPCMMClusterBuilderTool() = default;

        /** standard initialize method */
        virtual StatusCode initialize() override;

        virtual StatusCode getClusters(const EventContext& ctx,
                                       std::vector<Muon::MMPrepData>&& stripsVect,
                                       std::vector<std::unique_ptr<Muon::MMPrepData>>& clustersVect) const override;

        virtual RIO_Author getCalibratedClusterPosition(const EventContext& ctx, 
                                                        const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                        const Amg::Vector3D& directionEstimate, 
                                                        Amg::Vector2D& clusterLocalPosition,
                                                        Amg::MatrixX& covMatrix) const override;

    private:
        using LaySortedPrds = std::array<std::vector<Muon::MMPrepData>, 8>;
        /// Muon Detector Descriptor
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        
        Gaudi::Property<bool> m_writeStripProperties{this, "writeStripProperties", true };

        // params for the hough trafo
        Gaudi::Property<double> m_alphaMin{this, "HoughAlphaMin", -90. }; 
        Gaudi::Property<double> m_alphaMax{this, "HoughAlphaMax", 0. }; 
        Gaudi::Property<double> m_alphaResolution{this, "HoughAlphaResolution", 1.}; 
        Gaudi::Property<double> m_selectionCut{this, "HoughSelectionCut", 1. * Gaudi::Units::mm};
        Gaudi::Property<double> m_dMin{this, "HoughDMin", 0.}; 
        Gaudi::Property<double> m_dMax {this, "HoughDMax", 0.};
        Gaudi::Property<double> m_dResolution{this, "HoughDResolution", 0.125}; 
        Gaudi::Property<double> m_driftRange{this, "HoughExpectedDriftRange", 12.*Gaudi::Units::m};
        Gaudi::Property<unsigned> m_houghMinCounts{this, "HoughMinCounts", 3};

        /// charge ratio cut to supress cross talk
        Gaudi::Property<double> m_outerChargeRatioCut{this,"outerChargeRatioCut", 0.};
        // max number of strips cut by cross talk cut
        Gaudi::Property<int> m_maxStripsCut{this, "maxStripRemove", 4};

        Gaudi::Property<bool> m_digiHasNegativeAngles{this, "digiHasNegativeAngle", true};

        StatusCode runHoughTrafo(const std::vector<Muon::MMPrepData>& mmPrd, std::vector<double>& xpos, std::vector<int>& flag,
                                 std::vector<int>& idx_selected) const;
        StatusCode fillHoughTrafo(const std::vector<Muon::MMPrepData>& mmPrd, std::vector<double>& xpos, std::vector<int>& flag,
                                  std::unique_ptr<TH2D>& h_hough) const;
        StatusCode houghInitCummulator(std::unique_ptr<TH2D>& cummulator, double xmax, double xmin) const;

        StatusCode findAlphaMax(std::unique_ptr<TH2D>& h_hough, std::vector<std::tuple<double, double>>& maxPos) const;
        StatusCode selectTrack(const std::vector<Muon::MMPrepData>& mmPrd, std::vector<double>& xpos, std::vector<int>& flag,
                               std::vector<std::tuple<double, double>>& tracks, std::vector<int>& idxGoodStrips) const;

        StatusCode transformParameters(double alpha, double d, double dRMS, double& slope, double& intercept, double& interceptRMS) const;
        StatusCode applyCrossTalkCut(std::vector<int>& idxSelected, const std::vector<MMPrepData>& MMPrdsOfLayer, std::vector<int>& flag,
                                     int& nStripsCut) const;
        StatusCode finalFit(const std::vector<Identifier>& ids, const std::vector<float>& stripsPos, const std::vector<float>& driftDists,
                            const std::vector<AmgVector(2)>& driftDistErrors, double& x0, double& sigmaX0, double& fitAngle,
                            double& chiSqProb) const;
    };

}  //  namespace Muon
#endif
