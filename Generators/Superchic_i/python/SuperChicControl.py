# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


class SuperChicControl:

    def __init__(self, Config={}, gridpack="auto", makeGridpack=False, strictGridpack=True):
        """
        Config          : dictionary of SuperChicConfig parameters
        gridpack        : "auto" - use the DSID gridpack mc_<E>TeV.<physics_short>.GRID.tar.gz
                                   if found, otherwise run init in this job
                          <path> - use this gridpack
                          False  - ignore gridpacks and run init
        makeGridpack    : run init and pack the result into a gridpack
        strictGridpack  : treat differences in input.DAT (other than iseed and nev)
                          between the gridpack and this job as errors
        """
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
        self.scConfig.prepareGrids(gridpack=self.gridpack, makeGridpackFile=self.makeGridpack, strict=self.strictGridpack)

    def generate(self):
        # Run SuperChic internal generation script
        self.scConfig.superChicExecute()
