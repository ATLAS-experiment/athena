################################################################################
# Package: Scripts
################################################################################

# Declare the package name:
atlas_subdir( Scripts )

# Install files from the package:
atlas_install_scripts( share/get_joboptions share/get_files
   share/checkDbgSymbols.sh share/git-package-pseudomerge.py )
