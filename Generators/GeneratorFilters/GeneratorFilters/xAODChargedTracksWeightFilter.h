/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODCHARGEDTRACKSWEIGHTFILTER_H
#define GENERATORFILTERS_XAODCHARGEDTRACKSWEIGHTFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

namespace CLHEP {
  class HepRandomEngine;
}

/// Filter events based on presence of charged tracks
class xAODChargedTracksWeightFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;
  virtual StatusCode filterFinalize() override final;

  struct Spline {
     /// Linear spline representation of a function used to calculate weights

     struct Point {
        /// Single point and slope to next point
        Point(double x_, double y_, double slope_) :
           x(x_), y(y_), slope(slope_) {};

        double x;
        double y;
        double slope;
     };

     /// n-1 spline points filled at initialization
     std::vector<Point> points;

     StatusCode initialize( std::vector<double> & x, std::vector<double> & y);

     /// get minimum over certain range (requires initialization to extrapolate)
     double get_minimum(double min, double max) const;

     /// get value for certain x
     double value(int nch) const;
  };

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  /// get weight to filter and weight events
  double get_nch_weight(int nch) const;

  /// read the event weight
  StatusCode event_weight(double & event_weight) const;

  /// modify the event weight by weight
  void weight_event(double weight);

  /// Rndm generator service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  /// Name of the truth jet container
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  // Minimum pT for a track to count
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 50.0};

  // Maximum |pseudorapidity| for a track to count
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 2.5};

  // Minimum number of tracks
  Gaudi::Property<int> m_nchmin{this, "NchMin", 0};

  // Maximum number of tracks
  Gaudi::Property<int> m_nchmax{this, "NchMax", 20};

  Spline m_spline;

  /// Spline points
  Gaudi::Property<std::vector<double>> m_weight_fun_x{this, "SplineX", {}};
  Gaudi::Property<std::vector<double>> m_weight_fun_y{this, "SplineY", {}};

  double m_min_weight{};

  /// Number of events passing selection (weighted, orig weight)
  double m_nevents_selected{};
  /// Number of events passing weight-filtering (weighted, origin weight)
  double m_nevents_accepted{};
};

#endif
