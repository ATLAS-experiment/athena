/*
 *  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include <iostream>

#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include "TrkTrack/MultiComponentStateOnSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"

int main ATLAS_NOT_THREAD_SAFE() {
  std::cout << "Trk::Track test" << std::endl;

  Athena_test::Leakcheck check;

  Trk::FitQuality fq(10, 20);
  Trk::TrackInfo info{};
  std::cout << "--> Creating Track from TSOS" << '\n';

  // Track from TSOS
  auto tsvec1 = std::make_unique<Trk::TrackStates>(
      SG::OWN_ELEMENTS);
  auto* TSOS = new Trk::TrackStateOnSurface(fq, nullptr, nullptr, nullptr);
  tsvec1->push_back(TSOS);
  Trk::Track track1(info, std::move(tsvec1),
                    std::make_unique<Trk::FitQuality>(fq));

  std::cout << track1.isValid() << '\n';
  // Track from MTSOS
  std::cout << " --> Creating Track from MTSOS" << '\n';
  auto tsvec2 =
      std::make_unique<MultiComponentStateOnSurfaceDV>(
          SG::OWN_ELEMENTS);
  auto* MTSOS = new Trk::MultiComponentStateOnSurface(fq, nullptr, nullptr, {},
                                                      nullptr, 0);
  tsvec2->push_back(MTSOS);
  Trk::Track track2(info, std::move(tsvec2),
                    std::make_unique<Trk::FitQuality>(fq));

  std::cout << track2.isValid() << '\n';
  std::cout << "Check DV casting" << '\n';
  std::cout << track2.trackStateOnSurfaces()->at(0)->variety() << '\n';
  const MultiComponentStateOnSurfaceDV* multiStates =
      dynamic_cast<MultiComponentStateOnSurfaceDV*>(
          track2.trackStateOnSurfaces());
  std::cout << (multiStates->at(0) != nullptr) << '\n';

  // Manipulate TSOS
  std::cout << "--> Adding extra TSOS" << '\n';
  auto* states = track1.trackStateOnSurfaces();
  std::cout << "Own Elements : " << (states->ownPolicy() == SG::OWN_ELEMENTS)
            << '\n';
  std::cout << "states before add " << '\n';
  std::cout << track1.trackStateOnSurfaces()->at(0)->fitQualityOnSurface()
            << '\n';
  Trk::FitQuality fq1(15, 20);
  auto* TSOSnew = new Trk::TrackStateOnSurface(fq1, nullptr, nullptr, nullptr);
  states->push_back(TSOSnew);
  std::cout << "States after add " << '\n';
  std::cout << track1.trackStateOnSurfaces()->at(0)->fitQualityOnSurface()
            << '\n';
  std::cout << track1.trackStateOnSurfaces()->at(1)->fitQualityOnSurface()
            << '\n';
  std::cout << "--> Modify existing TSOS" << '\n';
  Trk::FitQuality fq2(25, 20);
  auto* TSOSMod = new Trk::TrackStateOnSurface(fq2, nullptr, nullptr, nullptr);
  track1.trackStateOnSurfaces()->at(0) = TSOSMod;
  std::cout << "States after modify " << '\n';
  std::cout << track1.trackStateOnSurfaces()->at(0)->fitQualityOnSurface()
            << '\n';
  std::cout << track1.trackStateOnSurfaces()->at(1)->fitQualityOnSurface()
            << '\n';
}

