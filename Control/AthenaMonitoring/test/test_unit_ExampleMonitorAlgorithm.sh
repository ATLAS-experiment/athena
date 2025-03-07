#!/bin/bash

echo $PWD
python -m AthenaMonitoring.ExampleMonitorAlgorithm
# Grep to avoid RooFit lines
hist_file_dump.py -r name --no_onfile --tree_entries --hash ExampleMonitorOutput.root | grep '^T' | tee hist-content
get_files -symlink test_unit_ExampleMonitorAlgorithm.ref
diff hist-content test_unit_ExampleMonitorAlgorithm.ref
exit $?
