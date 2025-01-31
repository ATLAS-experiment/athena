# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 gluon-fusion di-Higgs production with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["Higgs", "SMHiggs"]
evgenConfig.contact = ["james.robinson@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg ggF_HH_SMEFT process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_ggF_HH_SMEFT_Common.py")

# --------------------------------------------------------------
# Modify couplings
# --------------------------------------------------------------
## parameter usesmeft:
## 0: use HEFT parametrization and ignore CHbox, CH, CuH, CHG (no truncation)
## 1: use SMEFT (Warsaw) parametrization and ignore chhh, ct, ctt, cggh, cgghh (with truncation)
## 2: use HEFT parametrization and ignore CHbox, CH, CuH, CHG (with truncation!, testing purpose/SILH-Lag. calculation)
### HEFT parametrization (no truncation):
#PowhegConfig.usesmeft = 0 # [default: 1] Choose EFT parametrization
#PowhegConfig.chhh = 1.0   # [default: 1.0 (SM)] Trilinear Higgs self-coupling
#PowhegConfig.ct = 1.0     # [default: 1.0 (SM)] Top-Higgs Yukawa coupling
#PowhegConfig.ctt = 0.     # [default: 0. (SM)] Two-top-two-Higgs (tthh) coupling
#PowhegConfig.cggh = 0.    # [default: 0. (SM)] Effective gluon-gluon-Higgs coupling
#PowhegConfig.cgghh = 0.   # [default: 0. (SM)] Effective two-gluon-two-Higgses coupling
### SMEFT (Warsaw) parametrization (with truncation):
#PowhegConfig.usesmeft = 1 # [default: 1] Choose EFT parametrization
#PowhegConfig.Lambda = 1.0 # [default: 1.0] EFT counting mass Scale (in TeV)
#PowhegConfig.CHbox = 0.   # [default: 0.] Kinetic term of SU(2)_L singlet (with d'Alembert operator)
#PowhegConfig.CHD = 0.     # [default: 0.] second Kinetic term
#PowhegConfig.CH = 0.      # [default: 0.] Additional term to Higgs potential
#PowhegConfig.CuH = 0.     # [default: 0.] Modified Yukawa term
#PowhegConfig.CHG = 0.     # [default: 0.] Higgs-Glue-Glue operator
### HEFT parametrization (with truncation!, testing purpose/SILH-Lag. calculation):
#PowhegConfig.usesmeft = 2 # [default: 1] Choose EFT parametrization
#PowhegConfig.chhh = 1.0   # [default: 1.0 (SM)] Trilinear Higgs self-coupling
#PowhegConfig.ct = 1.0     # [default: 1.0 (SM)] Top-Higgs Yukawa coupling
#PowhegConfig.ctt = 0.     # [default: 0. (SM)] Two-top-two-Higgs (tthh) coupling
#PowhegConfig.cggh = 0.    # [default: 0. (SM)] Effective gluon-gluon-Higgs coupling
#PowhegConfig.cgghh = 0.   # [default: 0. (SM)] Effective two-gluon-two-Higgses coupling

# --------------------------------------------------------------
# Modify parameters related to truncation
# --------------------------------------------------------------
### Truncation options (2,3 only available for leading operators defined above)
## parameter SMEFTtruncation:
## 3: cross section based on |A_SM+A_dim6+A_dbldim6|^2
## 2: cross section based on |A_SM+A_dim6|^2+2*Re(A_SM x conj(A_dbldim6))
## 1: cross section based on |A_SM+A_dim6|^2
## 0: cross section based on |A_SM|^2+2*Re(A_SM*conj(A_dim6))
#PowhegConfig.SMEFTtruncation = 1 # [default: 1]

### Subleading operators (only available for truncation options 0,1)
## parameter includesubleading:
## 0: exclude subleading operators
## 1: include subleading operators applying loop power counting for C_tG (i.e. C_tG only enters linearly)
## 2: include subleading operators without loop power counting for C_tG (only valid in bornonly mode)
## 3: for testing purposes only (check O_tG implementation against GoSam, print PS point and ME for 4-top)
#PowhegConfig.includesubleading  0 # [default: 0] include subleading operators
#PowhegConfig.CtG = 0.       # [default: 0.] Wilson coefficient of chromomagnetic operator
#PowhegConfig.CQt = 0.       # [default: 0.] Wilson coefficient of 4-top operator C_Qt^(1)
#PowhegConfig.CQt8 = 0.      # [default: 0.] Wilson coefficient of 4-top operator C_Qt^(8)
#PowhegConfig.CQQtt = 0.     # [default: 0.] sum of Wilson coefficients of 4-top operators C_QQ^(1)+C_tt
#PowhegConfig.CQQ8 = 0.      # [default: 0.] Wilson coefficient of 4-top operator C_QQ^(8)
#PowhegConfig.GAMMA5BMHV = 0 # [default: 0] Choose gamma5 scheme: 0 NDR (default), 1 BMHV

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
