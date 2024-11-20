PATH=/cvmfs/sft.cern.ch/lcg/external/texlive/2023/bin/x86_64-linux:$PATH

if [[ $# -lt 1 ]]; then
    echo "ERROR: Need to give year to process: 22, 23, 24, 25"
    exit 1
fi
if [[ $1 -ge 22 && $1 -le 25 ]]; then
    year=$1
    echo "INFO: Preparing LaTeX beamer summary slides for year 20${year}"
else
    echo "ERROR: Bad year $1 given to process: use 22, 23, 24, 25"
    exit 1
fi

plotdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/"
inputdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/"

echo "INFO: Prepare LaTeX source code"

runlist=`\ls ${plotdir}/data${year}_13p6TeV | grep ^[0-9] | tr [\\\n] [,] | sed s/,$//`
cat slide_template.tex |\
    sed "s/YEAR/${year}/" |\
    sed "s/RUNLIST/${runlist}/" |\
    cat > plotdump.tex

split -n l/2 ${inputdir}/data${year}_13p6TeV/skipruns

echo "INFO: Compile run 1 (this may take a while)"
pdflatex -halt-on-error plotdump
if (($?)); then
    echo "ERROR: compile failed, exit"
    exit 1
fi
echo "INFO: Compile run 2 (this may take a while)"
pdflatex plotdump
if (($?)); then
    echo "ERROR: compile failed, exit"
    exit 1
fi
# compress
newfilename=Zcounting_year20${year}_`date "+%F"`.pdf
echo "INFO: Compress slide PDF file to $newfilename"
gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.5 -dNOPAUSE -dQUIET -dBATCH -dPrinted=false -sOutputFile=${newfilename} plotdump.pdf
echo "INFO: $newfilename ready (hopefully)"

\rm plotdump.* xaa xab
