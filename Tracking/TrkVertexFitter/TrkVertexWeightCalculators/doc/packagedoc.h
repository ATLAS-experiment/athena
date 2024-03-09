/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**

@page TrkVertexWeightCalculators_page TrkVertexWeightCalculators Package

This package provides tools to set a weight to reconstructed primary vertices, in order to select the one
coming from the hard-scatter interaction.

- SumPtVertexWeightCalculator: return sumPT / sumPT2 of the tracks associated to the vertex.
- TrueVertexDistanceWeightCalculator: return 1 / deltaZ, where deltaZ is the distance between the reconstructed vertex and the true vertex.
- BDTVertexWeightCalculator: BDT-based, developed by Davide Riva during his bachelor thesis. @author Ruggero Turra <ruggero.turra@cern.ch>

Most of the configuration is in Tracking/TrkConfig/python/TrkVertexWeightCalculatorsConfig.py
**/