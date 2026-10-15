# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


class SuperChicControl:

    def __init__(self, Config={}):
         # Create the `SuperChicConfig` instance used to run SuperChic
        from Superchic_i.SuperChicUtils import SuperChicConfig
        self.scConfig = SuperChicConfig()

         # Parse parameters from `Config` dict to the `SuperChicConfig` instance
        self.scConfig.from_dict(Config)
        print(self.scConfig.toFortran())

        # Create SuperChic run card
        self.scConfig.writeInputDAT()

    def initialize(self):
        # Run SuperChic internal initialization script
        self.scConfig.superChicInitialize()

    def generate(self):
        # Run SuperChic internal generation script
        self.scConfig.superChicExecute()
