# SinHelpers
Author Andrea Dell'Acqua (dellacqu@mail.cern.ch)
Converted from packagedoc.h

## Introduction

This package includes several helpers for use during simulation.  Running simulation uses these, whether you realize it or not...

## Class Overview

The helpers in this package are:

 - AthenaHitsCollectionHelper : Helper functions for dealing with collections of hits in sensitive detectors (to be persistified, usually)
 - DetectorGeometryHelper : Helper funtions for dealing with detector geometry
 - MemorySnooper : Set of functions for tracking memory being used during the simulation
 - ProcessSubTypeMap : (Now in Geant4) makes a map of subtypes (e.g. ionization) so that we can use enumerations rather than strings for checking process types.
 - SecondaryTracksHelper : Helper for tracks not from the primary event
 - ServiceAccessor : Helper for getting services
 - StepHelper : Set of functions for dealing with G4Steps
 - TrackVisualizationHelper : Set of functions for dealing with the visualization of tracks
