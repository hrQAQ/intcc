#!/bin/bash

######################################
# Name: batch_gen.sh
# Usage:
#   use traffic_gen.py to generate flow file
# Parameters Setup:
OUTPUT_DIR="../simulation/mix/traffic"
Distributions=("FbHdp_distribution.txt" "WebSearch_distribution.txt")
declare -A miniDistribution
miniDistribution=([FbHdp_distribution.txt]="hdp" [WebSearch_distribution.txt]="web")
Topology="mix.txt"
ndc=128
nwan=128
Loads=("0.3")
Bandwidths=("10G" "40G")
Times=("0.1")
# Proportion of DCs and WANs, 1:1, 1:9, 3:7, 7:3, 9:1
ProportionOfDcs=('1' '1' '3' '7' '9')
ProportionOfWans=('1' '9' '7' '3' '1')
# Output file name format:
#   Distribution_topology_load_bandwidth_time_proportionOfDc:proportionOfWan.txt
#   eg: hdp_mix_0.3_10G_0.1_1:1.txt
######################################

echo -e "\033[34mInfo: \033[0mStart generating traffic..."
for distribution in ${Distributions[@]};do
    for load in ${Loads[@]};do
        for bandwidth in ${Bandwidths[@]};do
            for time in ${Times[@]};do
                for((i=0;i<${#ProportionOfDcs[@]};i++));do
                    output_file=$OUTPUT_DIR/${miniDistribution[$distribution]}\_$(basename "$Topology" .txt)\_$load\_$bandwidth\_$time\_${ProportionOfDcs[$i]}:${ProportionOfWans[$i]}.txt
                    echo -e "\033[31mGenerating:\033[0m $(basename $output_file)"
                    {
                        python traffic_gen.py -c $distribution --ndc $ndc --nwan $nwan -l $load -b $bandwidth -t $time --pdc ${ProportionOfDcs[$i]} --pwan ${ProportionOfWans[$i]} -o $output_file
                    }&
                done
            done
        done
    done
done
wait
echo -e "\033[34mInfo: \033[0mDone."
