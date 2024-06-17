# G4StepLimitation
Author Andrea Dell'Acqua (dellacqu@mail.cern.ch)
Converted from packagedoc.h

## Introduction

This class provides an interface for the Geant4 physics process called step limitation.  Step limitation limits the maximum length of a step allowed.  It can be applied only in specific volumes in order to further divide the Geant4 step.  It was most recently used in the silicon of the pixel and SCT detectors in order to ensure a more uniform distribution of energy through the silicon as a track traverses the sensitive detector.

## Class Overview

The only class included in this package is G4StepLimitProcess , a UserPhysicsProcess.  It defines the step limitation.
