/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
@page TrigInDetEvent_page TrigInDetEvent Package

This package contains classes used in the LVL2 ID algorithms

@author D.Emeliyanov AT rlDOTacDOTuk

@section TrigInDetEvent_TrigInDetEventIntro Introduction

The following classes are defined

   - TrigInDetTrackFitPar : represents a set of LVL2 track parameters with associated covariance 
   - TrigInDetTrack       : a LVL2 track containing TrigInDetTrackFitPar(s) and input data - Pixel and SCT 
                            spacepoints and TRT drift circles
   - TrigSiSpacePoint     : a LVL2 spacepoint in Pixel or SCT
   - TrigVertex           : a LVL2 vertex class containing vertex coordinates, covariance matrix, and 
                            some auxilliary information about vertex

This package also contains selection.xml - a file for lcgdict generation
      


*/
