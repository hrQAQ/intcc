#!/bin/bash
Usage() {
    echo "Usage: $0 -c case -m method"
    echo "  -c case: case name, default all cases"
    echo "  -m method: method name, default all methods"
    echo "  -j job: 1 for trace_reader, 2 for cp, 3 for rm, 4 for make case dir, default 1"
    exit 1
}

Start=$(date +%s)
DATA_DIR="/home/huangrui/intcc/hpcc/data"
TRACE_DIR="/home/huangrui/intcc/hpcc/analysis/traceinfo"
FIG_DIR="/home/huangrui/intcc/hpcc/analysis/Figures"


while getopts "c:m:j:h" opt; do
    case $opt in
        c)
            c=$OPTARG
            ;;
        m)
            m=$OPTARG
            ;;
        j)
            j=$OPTARG
            ;;
        h)
            Usage
            ;;
        ?)
            Usage
            ;;
    esac
done

if [[ -z $c ]]; then
    subdirs=$(ls $DATA_DIR)
else
    subdirs=$c
fi

if [[ -z $m ]]; then
    methods="DCQCN HPCC TIMELY DCTCP GEAR"
else
    methods=$m
fi

if [[ $j -eq 4 ]]; then
    for i in $@; do
        if [[ $i == -* ]]; then
            continue
        fi
        echo "mkdir $DATA_DIR/$i $TRACE_DIR/$i $FIG_DIR/$i"
        mkdir -p $DATA_DIR/$i $TRACE_DIR/$i $FIG_DIR/$i
        if [[ $? -ne 0 ]]; then
            echo "mkdir $DATA_DIR/$i $TRACE_DIR/$i $FIG_DIR/$i failed"
        fi
    done
    exit 0
fi

para_type=0
idx=0
for subdir in $subdirs; do
    if [[ $subdir == "exit" || $subdir == "start" || $subdir == "congestion" ]]; then
        para_type=2
    else
        para_type=1
    fi
    for method in $methods; do
        joblist=($(jobs -p))
        while (( ${#joblist[*]} >= 10 )); do
            sleep 1
            joblist=($(jobs -p))
        done
        idx=$(($idx+1))
        {
            if [[ $j -eq 1 ]]; then    
                echo -e "`date "+%Y-%m-%d %H:%M:%S"` \033[32mINFO\033[0m \033[36m#Run\033[0m ${#joblist[*]} | \033[36mJobIDX\033[0m $idx | \033[36mCase\033[0m $subdir | \033[36mMethod\033[0m $method | \033[36mPara\033[0m $para_type"
                ./trace_reader $DATA_DIR/$subdir/$method.tr $para_type
                if [[ $? -ne 0 ]]; then
                    echo -e "`date "+%Y-%m-%d %H:%M:%S"` \033[31mERROR\033[0m \033[36mCase\033[0m $subdir | \033[36mMethod\033[0m $method | \033[36mPara\033[0m $para_type \033[36mFailed\033[0m"
                fi
            fi
            if [[ $j -eq 2 ]]; then
            # file in ls $DATA_DIR/$subdir + $TRACE_DIR/$subdir
                for file in $(ls $DATA_DIR/$subdir); do
                    if [[ $file == *.rate || $file == *.rtt ]]; then
                        if [[ $file == $method* ]]; then
                            echo -e "`date "+%Y-%m-%d %H:%M:%S"` \033[32mINFO\033[0m \033[36mcp\033[0m $DATA_DIR/$subdir/$file \033[36mto\033[0m $TRACE_DIR/$subdir/$file"
                            cp -f $DATA_DIR/$subdir/$file $TRACE_DIR/$subdir/$file
                        fi
                    fi
                done 
            fi
            if [[ $j -eq 3 ]]; then
                for file in $(ls $DATA_DIR/$subdir $TRACE_DIR/$subdir); do
                    if [[ $file == *.rate || $file == *.rtt ]]; then
                        if [[ $file == $method* ]]; then
                            echo -e "`date "+%Y-%m-%d %H:%M:%S"` \033[32mINFO\033[0m \033[36mrm\033[0m $DATA_DIR/$subdir/$file \033[36mto\033[0m $TRACE_DIR/$subdir/$file"
                            rm -f $TRACE_DIR/$subdir/$file $DATA_DIR/$subdir/$file
                        fi
                    fi
                done 
            fi 
        } &
    done
done
wait
echo -e "`date "+%Y-%m-%d %H:%M:%S"` \033[32mINFO\033[0m \033[36mTotal time\033[0m $(($(date +%s)-Start))s" 
