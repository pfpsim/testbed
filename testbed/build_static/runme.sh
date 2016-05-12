rm *.csv
vlvl="INVALID"
echo $vlvl
if [ ! $# -eq 0 ]
then
    echo "No args passed"
    echo $#
    vlvl=$1
else
    vlvl='minimal'
fi
echo $vlvl
./testbed-sim -c Configs/ -Xp4 simple_router.json -Xtpop testbedRoutingTable.txt -v "$vlvl"
