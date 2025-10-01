# do all plot updates
# ./plot_all.sh [quiet] [update] [year]
# e.g. for a quiet, minimal update: ./plot_all.sh 1 1 25

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

if [[ $# -lt 3 ]]; then
    year=25
else
    year=$3
fi
echo "INFO: running for year $year"

echo "INFO: Running now ./plot_runwise.sh $year $update"

./plot_runwise.sh $year $update | egrep $filter

echo "INFO: Running now ./plot_yearwise.sh $year run3"
./plot_yearwise.sh $year 22_23_24 run3 | egrep $filter
#./plot_yearwise.sh $year run3 | egrep $filter


echo "INFO: Running now ./make_latexslides $year"
./make_latexslides.sh $year | egrep $filter

