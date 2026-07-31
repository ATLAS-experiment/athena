Feature Extractor (FEX) Unpackers for GlobalSimulation
===

This holds the source code for simulation of the FEX unpackers for the GlobalSimulation

Converter AlgTools
---

These AlgTools read in xAOD FEX RoI containers, convert to a GlobalTOB format, and write the result to StoreGate.
They make no selection on the incoming RoIs, just convert.

RoI AlgTools
---

These AlgTools read in the xAOD FEX RoI containers, make selection cuts on the RoIs, convert to a GlobalFormat and return the selected RoIs.

