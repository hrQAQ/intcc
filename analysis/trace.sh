make

# for subdir in "testACK"; do
# for subdir in "load1:9" "load3:7" "load1:1" "load7:3" "load9:1"; do
# for subdir in "web" "hdp"; do
for subdir in "start" "exit"; do
    for method in "GEAR" "HPCC" "DCTCP" "DCQCN" "TIMELY"; do
        {
            echo $subdir $method
            ./trace_reader ../data/$subdir/$method.tr
        } &
    done
done