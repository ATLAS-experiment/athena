# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

########################################################################
#                                                                      #
# JetJvtEfficiencyToolConfig: A helper module for configuring jet jvt  #
# efficiency configurations. This way the derivation framework can     #
# easily be kept up to date with the latest recommendations.           #
# Author: mswiatlo
#                                                                      #
########################################################################

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
import ROOT

def getJvtEffToolCfg(flags, jetalg):
  """Configure the JVT efficiency tool"""

  acc = ComponentAccumulator()

  run = "Run3" if flags.GeoModel.Run is LHCPeriod.Run3 else "Run2"
  configs = {"AntiKt4EMTopo": "JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root",
             "AntiKt4EMPFlow": f"JetJvtEfficiency/May2024/NNJvtSFFile_{run}_EMPFlow.root"}

  configs["AntiKt4EMPFlowCustomVtx"] = configs["AntiKt4EMPFlow"]

  jvtefftool = CompFactory.CP.JetJvtEfficiency("JVTEff_{0}".format(jetalg))
  jvtefftool.SFFile=configs[jetalg]
  # NNJvt isn't calculated for EMTopo jets (yet) so fallback to Jvt
  if jetalg == "AntiKt4EMTopo":
    jvtefftool.TaggingAlg = ROOT.CP.JvtTagger.Jvt

  acc.setPrivateTools(jvtefftool)
  return acc
