# G4CosmicFitter
Author Jamie Boyd (jboyd@mail.cern.ch)
Converted from packagedoc.h

## Introduction

This package applies a filter on volumes and hits within those volumes for use during cosmic ray simulation.  It can be applied through the cosmic simulation job transforms.  It is used, for example, to only save cosmic rays passing through a certain subdetector.

## Class Overview

The package has three classes:

 - G4CosmicFilter : This algorithm checks for a cosmic ray having passed through the selected volume
 - G4CosmicAndFilter and G4CosmicOrFilter : These algorithms allow users to "and" or "or" two volumes in order to get a more precise selection criterion.  This is used for the pixel detector (and'ed with the TRT barrel in order to ensure the track passes through the sensitive volumes rather than the services).
