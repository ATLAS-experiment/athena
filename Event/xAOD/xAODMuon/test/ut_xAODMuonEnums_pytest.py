#! /usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from xAODMuon.xAODMuonEnums import xAODMuonEnums

print("MuidCo : ", xAODMuonEnums.Author.MuidCo)
print("Combined : ", xAODMuonEnums.MuonType.Combined)
print("t0 : ", xAODMuonEnums.ParamDef.t0)
print("Primary : ", xAODMuonEnums.TrackParticleType.Primary)
print("MOP : ", xAODMuonEnums.EnergyLossType.MOP)
print("Tight : ", xAODMuonEnums.Quality.Tight)

