# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Set VALGRIND_OPTS to default ATLAS options and suppression files:
#
#   source ${Athena_DIR}/bin/valgrind-atlas-opts.sh
#                     or
#   source `which valgrind-atlas-opts.sh`
#

_vg_python_supp=`find_data.py Valkyrie/valgrind-python.supp`
_vg_atlas_supp=`find_data.py Valkyrie/valgrind-atlas.supp`
_vgopts=("--suppressions=${_vg_python_supp}"
         "--suppressions=${_vg_atlas_supp}"
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
unset _vg_atlas_supp
unset _vg_python_supp
