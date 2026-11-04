# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


from RootUtils.LoadEnumHeader import loadEnumHeader
loadEnumHeader('ElectronPhotonSelectorTools/egammaPIDdefs.h')

# make the egammaPID namespace visible
from ROOT import egammaPID  # noqa: F401

# pull in the dictionary with EgammaPIDdefs
from ROOT.egammaPID import egammaIDQuality  # noqa: F401
