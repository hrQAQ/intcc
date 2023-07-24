#!/bin/zsh
conda activate py27
if [ "$1" == "debug" ]; then
    echo "debug"
    CXXFLAGS="-Wno-error -std=c++11" CC='gcc-5' CXX='g++-5' ./waf configure --build-profile=debug
elif [ "$1" == "opt" ]; then
    echo "optimized"
    CXXFLAGS="-Wno-error -std=c++11" CC='gcc-5' CXX='g++-5' ./waf configure --build-profile=optimized
fi

# cd traffic_gen
# python traffic_gen.py -c WebSearch_distribution.txt --ndc 128 --nwan 4 -l 0.3 -b 10G -t 0.1 --pdc 7 --pwan 3 -o load7:3.txt
# python traffic_gen.py -c FbHdp_distribution.txt --ndc 128 --nwan 4 -l 0.3 -b 10G -t 0.1 --pdc 5 --pwan 1 -o hdp_load5:1.txt
python traffic_gen.py -c FbHdp_distribution.txt --ndc 128 --nwan 128 -l 0.3 -b 10G -t 0.1 --pdc 5 --pwan 1 -o hdp.txt

python traffic_gen.py -c WebSearch_distribution.txt --ndc 128 --nwan 4 -l 0.1 -b 10G -t 0.1 --pdc 5 --pwan 1 -o load5:1.txt