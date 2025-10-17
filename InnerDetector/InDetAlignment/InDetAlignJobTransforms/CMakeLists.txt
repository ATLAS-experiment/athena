################################################################################
# Package: InDetAlignJobTransforms
################################################################################

# Declare the package name:
atlas_subdir( InDetAlignJobTransforms )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( scripts/IDAlign_tf.py )
