# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from MGCtrl_Py8EG_A14NNPDF23LO_tetrabNJ0_4mu_3pt2_v3 import TetrabSample


# Use a class deriving from the EvgenConfig base class in the control file
# to set required parameters specific to this sample.
class Sample(TetrabSample):
    # A class attribute is excluded from the instance metadata printed by Gen_tf.
    safety = 2.0

    # Anything that we declare here becomes a property also of the base class 
    # so that it can be accessed there with self.X e.g. we can add explicitly
    # mb4 = 17
