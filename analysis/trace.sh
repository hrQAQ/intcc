make
# for load in "load1:9" "load3:7" "load1:1" "load7:3" "load9:1"; do
#     echo $load
#     ./trace_reader ../data/$load/GEAR.tr
#     ./trace_reader ../data/$load/HPCC.tr
#     ./trace_reader ../data/$load/DCTCP.tr
#     ./trace_reader ../data/$load/DCQCN.tr
#     ./trace_reader ../data/$load/TIMELY.tr
# done

# for cdf in "web" "hdp"; do
#     echo $cdf
#     ./trace_reader ../data/$cdf/GEAR.tr
#     ./trace_reader ../data/$cdf/HPCC.tr
#     ./trace_reader ../data/$cdf/DCTCP.tr
#     ./trace_reader ../data/$cdf/DCQCN.tr
#     ./trace_reader ../data/$cdf/TIMELY.tr
# done

for type in "start" "exit"; do
    echo $type
    ./trace_reader ../data/$type/GEAR.tr
    ./trace_reader ../data/$type/HPCC.tr
    ./trace_reader ../data/$type/DCTCP.tr
    ./trace_reader ../data/$type/DCQCN.tr
    ./trace_reader ../data/$type/TIMELY.tr
done