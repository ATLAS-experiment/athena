# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Set VALGRIND_OPTS to default ATLAS options and suppression files:
#
#   source ${Athena_DIR}/bin/valgrind-atlas-opts.sh
#                     or
#   source `which valgrind-atlas-opts.sh`
#

target_var="${AtlasProject}_DIR" #e.g. Athena_DIR
# no common way to expand in bash and zsh
if [ -n "$ZSH_VERSION" ]; then
    project_DIR="${(P)target_var}"  # zsh
else
    project_DIR="${!target_var}"    # bash
fi
_vgopts=("--suppressions=${project_DIR}/data/Valkyrie/valgrind-python.supp"
         "--suppressions=${project_DIR}/data/Valkyrie/valgrind-atlas.supp"
         "--suppressions=${ROOTSYS}/etc/valgrind-root.supp"
         "--suppressions=${ROOTSYS}/etc/valgrind-root-python.supp"
         "--smc-check=all")

# Add the option if not already present
for o in ${_vgopts[@]}; do
    if [[ $VALGRIND_OPTS != *"$o"* ]]; then
        export VALGRIND_OPTS="${VALGRIND_OPTS:+"$VALGRIND_OPTS "}$o"
    fi
done

unset _vgopts
