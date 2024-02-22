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
import ROOT

def getJvtEffToolCfg(flags, jetalg):
  """Configure the JVT efficiency tool"""

  acc = ComponentAccumulator()

  configs = {"AntiKt4EMTopo": "JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root",
             "AntiKt4EMPFlow": "JetJvtEfficiency/Moriond2018/JvtSFFile_EMPFlow.root"}

  jvtefftool = CompFactory.CP.JetJvtEfficiency("JVTEff_{0}".format(jetalg))
  jvtefftool.SFFile=configs[jetalg]
  # NNJvt isn't calculated for EMTopo jets (yet) so fallback to Jvt
  if jetalg == "AntiKt4EMTopo":
    jvtefftool.TaggingAlg = ROOT.CP.JvtTagger.Jvt

  acc.setPrivateTools(jvtefftool)
  return acc
