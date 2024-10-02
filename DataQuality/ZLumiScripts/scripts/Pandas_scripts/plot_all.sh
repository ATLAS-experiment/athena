# do all plot updates, year is hardcoded below
# ./plot_all.sh [quiet] [update]
# e.g. for a quiet, minimal update: ./plot_all.sh 1 1

if [[ $# -lt 1 ]] || [[ $1 -gt 0 ]]; then
    filter="INFO|Info|WARNING|ERROR"
    echo "INFO: Will suppress output of scripts."
else
    filter="."
    echo "INFO: Pass through of all output."
fi

if [[ $# -lt 2 ]] || [[ $2 -gt 0 ]]; then
    update=1
    echo "INFO: Will keep Plots for existing runs and only add new ones"
else
    echo "INFO: Will overwrite all run-dependent Plots on Z counting EOS"
    update=0
fi

echo "INFO: Running now ./plot_runwise.sh 24 $update"

./plot_runwise.sh 24 $update | egrep $filter

echo "INFO: Running now ./plot_yearwise.sh 24 run3"
./plot_yearwise.sh 24 run3 | egrep $filter


echo "INFO: Running now ./make_latexslides"
./make_latexslides.sh | egrep $filter

