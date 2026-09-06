################################################################################
# Package: L0MuonValidation
################################################################################

# Declare the package name:
atlas_subdir( L0MuonValidation )

# Install python modules:
atlas_install_python_modules( python/*.py )

atlas_add_test( FullChainTrigger
                SCRIPT python -m L0MuonValidation.FullChainTrigger
                       --threads 1 --nEvents 1
                POST_EXEC_SCRIPT noerror.sh )
