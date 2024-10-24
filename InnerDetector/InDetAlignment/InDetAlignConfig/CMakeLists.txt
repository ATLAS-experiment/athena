################################################################################
# Package: InDetAlignConfig
################################################################################

# Declare the package name:
atlas_subdir( InDetAlignConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( scripts/runIDAlign.py )
