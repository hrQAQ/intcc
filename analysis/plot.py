import os
import matplotlib.pyplot as plt
import re
import getopt
import sys
import logging
from typing import List, Dict, Tuple

from constant import *

logging.basicConfig(level=logging.INFO,
                    filename='plot.log',
                    datefmt='%Y/%m/%d %H:%M:%S',
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
L = logging.getLogger(__name__)

def Usage():
    print("Usage: python plot.py -c case -m method")
    print("  -c case: case name, default all cases")
    print("  -m method: method name, default all methods")
    print("  -h help")

def get_css():
    marker = ''
    line_width = 3
    mark_size = 5

    font = {'family': 'Times new roman', 'weight': 'bold', 'size': '22'}
    colors = ['#606c38', '#f6bd60', '#c32f27','#f5cac3', '#84a59d', '#f28482', '#8e9aaf', '#cbc0d3', '#9b5de5', '#f7ede2']

    # css = {
    #     'flow-0': [colors[0], '-', marker, 'Flow-1', line_width, mark_size],
    #     'flow-1': [colors[2], '-', marker, 'Flow-2', line_width, mark_size],
    #     'flow-2': [colors[3], '-', marker, 'Flow-3', line_width, mark_size],
    #     'flow-3': [colors[4], '-', marker, 'Flow-4', line_width, mark_size],
    #     'flow-4': [colors[5], '-', marker, 'Flow-5', line_width, mark_size]
    # }
    return colors, font

def get_x_and_y(file: str) -> (list, list):
    with open(file, 'r') as f:
        data = f.read().splitlines()
        x, y = [], []
        for line in data:
            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                x.append(float(line.split()[0]))
                y.append(float(line.split()[1]))
    # xwan_new, ywan_new = [], []
    # for i in range(0, len(xwan), wan_batch):
    #     xwan_new.append(xwan[i])
    #     ywan_new.append(sum(ywan[i:i+wan_batch])/wan_batch)
    return (x, y)

def preprocess_data(x: list, y: list) -> (list, list):
    pass

def univer_plot(x_lists: [list], y_lists: [list], labels: [str], drawMap: [bool], xlabel: str, ylabel: str, colors: [str], font: dict, filename: str):
    L.info("Plot " + filename + " Save to " + FIGDIR + filename + ".pdf")
    assert len(x_lists) == len(y_lists) == len(labels) <= len(colors)
    plt.figure(figsize=(6, 4))
    colors, font = get_css()
    # plt.title(filename, font)
    plt.rc("font", **font)
    for i in range(len(x_lists)):
        if drawMap[i] == True:
            plt.plot(x_lists[i], y_lists[i], color=colors[i], linestyle='-', marker='', label=labels[i], linewidth=1)
    plt.xlabel(xlabel, font)
    plt.ylabel(ylabel, font)
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=12)
    plt.savefig(FIGDIR + filename + '.pdf', bbox_inches='tight')
    plt.close()

def diff_metric_plot(case: str, method: str, dataFilenameSuffixs: [str], labels: [str], drawMap: [bool], xlabel: str, ylabel: str, colors: [str], font: dict, filename: str):
    X, Y = [], []
    hasData = True
    assert len(dataFilenameSuffixs) == len(labels) == len(drawMap) <= len(colors)
    for dataFilenameSuffix in dataFilenameSuffixs:
        try:
            x, y = get_x_and_y(DATA_DIR + case + "/" + method + dataFilenameSuffix)
            X.append(x)
            Y.append(y)
        except:
            L.error("No file: " + DATA_DIR + case + "/" + method + dataFilenameSuffix)
            hasData = False
    if hasData:
        univer_plot(X, Y, labels, drawMap, xlabel, ylabel, colors, font, filename)

DirectCases = ["start", "exit", "congestion", "intra", "inter", "context", "lineRateStart", "startRate", "converge", "midRate", "larRate"]

if __name__ == "__main__":
    L.info("Start plot.py")
    cases = os.listdir(DATA_DIR)
    methods = ["DCTCP", "DCQCN", "HPCC", "TIMELY", "GEAR"]
    try:
        opts, args = getopt.getopt(sys.argv[1:], "c:m:h", ["case=", "method=", "help"])
    except getopt.GetoptError:
        L.error("getopt error")
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            Usage()
            sys.exit()
        elif opt in ("-c", "--case"):
            cases = [arg]
        elif opt in ("-m", "--method"):
            methods = [arg]
        else:
            L.error("unhandled option")
            sys.exit(2)
    colors, font = get_css()
    L.info("Plot cases: " + str(cases))
    L.info("Plot methods: " + str(methods))
    for case in cases:
        FIGDIR = BASE_FIGDIR + case + "/"
        for method in methods:
            if "hdp" in case or "mix" in case:
                L.info("[Use TraceInfo] case: " + case + " | method: " + method)
                DRAW_START_TIME = 1e9
                DRAW_END_TIME = 4e9 + 1e8
                diff_metric_plot(case, method, ["_thoughput_dc.txt", "_thoughput_wan.txt"], ["DC", "WAN"], [True, True], "Time (ns)", "Throughput (bps)", colors, font, method + "_throughput")
                diff_metric_plot(case, method, ["_delay_dc.txt", "_delay_wan.txt"], ["DC", "WAN"], [True, True], "Time (ns)", "Delay (ns)", colors, font, method + "_delay")
            for directcase in DirectCases:
                if directcase in case:
                    DRAW_START_TIME = 0
                    DRAW_END_TIME = 8e9
                    # -- Use TraceInfo --
                    # L.info("[Use TraceInfo] case: " + case + " | method: " + method)
                    # suffixs_throuput = ["_thoughput_flow_1.txt", "_thoughput_flow_2.txt", "_thoughput_flow_3.txt"]
                    # suffixs_delay = ["_delay_flow_1.txt", "_delay_flow_2.txt", "_delay_flow_3.txt"]
                    # labels = ["Flow-1", "Flow-2", "Flow-3"]
                    # drawMap = [True, True, True]
                    # diff_metric_plot(case, method, suffixs_throuput, labels, drawMap, "Time (ns)", "Throughput (bps)", colors, font, method + "_throughput")
                    # diff_metric_plot(case, method, suffixs_delay, labels, drawMap, "Time (ns)", "Delay (ns)", colors, font, method + "_delay")
                    # -- Use Direct Metric --
                    L.info("[Use Direct Metric] case: " + case + " | method: " + method)
                    suffixs_throuput = []
                    suffixs_delay = []
                    for file in os.listdir(DATA_DIR + case):
                        if file.endswith(".rate") and file.startswith(method):
                            suffixs_throuput.append(file.split("_")[1])
                        elif file.endswith(".rtt") and file.startswith(method):
                            suffixs_delay.append(file.split("_")[1])
                    labels = [ i.split(".")[0] for i in suffixs_throuput ]
                    suffixs_throuput =[ "_" + i for i in suffixs_throuput ]
                    drawMap = [True for i in range(len(suffixs_throuput))]
                    # remove WAN flow plot
                    # for suffix in suffixs_throuput:
                    #     if suffix[1] >= '5':
                    #         drawMap[suffixs_throuput.index(suffix)] = False
                    diff_metric_plot(case, method, suffixs_throuput, labels, drawMap, "Time (ns)", "Sending Rate (bps)", colors, font, method + "_RATE")
                    # remove WAN flow plot
                    labels = [ i.split(".")[0] for i in suffixs_delay ]
                    suffixs_delay =[ "_" + i for i in suffixs_delay ]
                    drawMap = [True for i in range(len(suffixs_delay))]
                    for suffix in suffixs_delay:
                        if suffix[1] >= '5':
                            drawMap[suffixs_delay.index(suffix)] = False
                    diff_metric_plot(case, method, suffixs_delay, labels, drawMap, "Time (ns)", "Delay (ns)", colors, font, method + "_RTT")
    L.info("End plot.py")