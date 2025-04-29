mapfile -t array < <(grep -l "clean_mom" run-st2-*.log | sed 's/run-st2-\([0-9]*\)\.log/\1/')
echo "${array[@]}"

empty_files=( $(find . -name "pwg-*-st2-stat.dat" -size 0 -printf "%f\n" | sed -E 's/pwg-(.*)-st2-stat\.dat/\1/') )
echo "${empty_files[@]}"

rm_files=("${array[@]} ${empty_files[@]}")

for i in "${rm_files[@]}"; do
    echo "rm pwg-${i}-st2-stat.dat"  # Replace this with whatever you want to do with each element
    rm pwg-${i}-st2-stat.dat  # Replace this with whatever you want to do with each element
    echo "rm pwgcounters-st2-${i}.dat"
    rm pwgcounters-st2-${i}.dat
    echo "rm pwg-st2-xgrid-*-rh-*-${i}.top"
    rm pwg-st2-xgrid-*-rh-*-${i}.top
    echo "rm pwhg_checklimits-${i}"
    rm pwhg_checklimits-${i}
    echo "rm pwggrid-rm-${i}.dat"
    rm "pwggrid-rm-"${i}.dat
    echo "rm pwggrid-btl-${i}.dat"
    rm pwggrid-btl-${i}.dat
done
