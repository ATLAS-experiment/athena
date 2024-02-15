################################################################################
# Package: Digitization
################################################################################

# Declare the package name:
atlas_subdir( Digitization )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py )
atlas_install_runtime( data/*.ascii )
