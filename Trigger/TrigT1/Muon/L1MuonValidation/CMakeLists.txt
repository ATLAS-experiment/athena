################################################################################
# Package: L1MuonValidation
################################################################################

# Declare the package name:
atlas_subdir( L1MuonValidation )

# Install python modules:
atlas_install_python_modules( python/*.py )

atlas_add_test( FullChainTrigger
                SCRIPT python -m L1MuonValidation.FullChainTrigger
                       --threads 1 --nEvents 1
                POST_EXEC_SCRIPT noerror.sh )
