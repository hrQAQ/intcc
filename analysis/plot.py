import os
import matplotlib.pyplot as plt
import re
import getopt
import sys
import logging

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
    colors = ['#D76364', '#EF7A6D', '#F1D77E', '#B1CE46', '#9394E7', '#9DC3E7']

    # css = {
    #     'flow-0': [colors[0], '-', marker, 'Flow-1', line_width, mark_size],
    #     'flow-1': [colors[2], '-', marker, 'Flow-2', line_width, mark_size],
    #     'flow-2': [colors[3], '-', marker, 'Flow-3', line_width, mark_size],
    #     'flow-3': [colors[4], '-', marker, 'Flow-4', line_width, mark_size],
    #     'flow-4': [colors[5], '-', marker, 'Flow-5', line_width, mark_size]
    # }
    return colors, font

def plot_throuput(xdc, ydc, xwan, ywan, colors, font, method):
    L.info("Plot throuput of DC and WAN for " + method + " Save to " + FIGDIR + "Throuput_" + method + ".pdf")
    plt.plot(xdc, ydc, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(xwan, ywan, color=colors[2], linestyle='-', marker='', label='WAN', linewidth=1)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Throuput (MB)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Throuput_' + method + '.pdf', bbox_inches='tight')
    plt.close()
    
def plot_delay(xdc, ydc, xwan, ywan, colors, font, method):
    L.info("Plot delay of DC and WAN for " + method + " Save to " + FIGDIR + "Delay_" + method + ".pdf")
    plt.plot(xdc, ydc, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(xwan, ywan, color=colors[2], linestyle='-', marker='', label='WAN', linewidth=1)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Delay (us)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Delay_' + method + '.pdf', bbox_inches='tight')
    plt.close()

def plot_throuput_flow(x1, y1, x2, y2, x3, y3, colors, font, method):
    L.info("Plot throuput of three flow for " + method + " Save to " + FIGDIR + "Throuput_" + method + ".pdf")
    plt.plot(x1, y1, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(x2, y2, color=colors[4], linestyle='-', marker='', label='WAN', linewidth=1)
    plt.plot(x3, y3, color=colors[3], linestyle='-', marker='', label='ADD', linewidth=1)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Throuput (MB)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Throuput_' + method + '.pdf', bbox_inches='tight')
    plt.close()
    
def plot_delay_flow(x1, y1, x2, y2, x3, y3, colors, font, method):
    L.info("Plot delay of three flow for " + method + " Save to " + FIGDIR + "Delay_" + method + ".pdf")
    plt.plot(x1, y1, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(x2, y2, color=colors[4], linestyle='-', marker='', label='WAN', linewidth=1)
    # plt.plot(x3, y3, color=colors[3], linestyle='-', marker='', label='ADD', linewidth=1j)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Delay (us)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Delay_' + method + '.pdf', bbox_inches='tight')
    plt.close()

if __name__ == "__main__":
    L.info("Start plot.py")
    c, m = None, None
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
            c = arg
        elif opt in ("-m", "--method"):
            m = arg
        else:
            L.error("unhandled option")
            sys.exit(2)
    if c:
        cases = [c]
    else:
        cases = os.listdir(DATA_DIR)
    if m:
        methods = [m]
    else:
        methods = ["DCTCP", "DCQCN", "HPCC", "TIMELY", "GEAR"]

    for case in cases:
        FIGDIR = BASE_FIGDIR + case + "/"
        L.info("Plot case: " + case)
        if "hdp" in case or "mix" in case:
            for method in methods:
                xdc = []
                ydc = []
                xwan = []
                ywan = []
                try:
                    with open(DATA_DIR + case + "/"  + method + Throuput_DC, 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                xdc.append(float(line.split()[0])/1000)
                                ydc.append(float(line.split()[1])/1024/1024)
                except:
                    L.error("No file: " + DATA_DIR + case + "/" + method + Throuput_DC)
                    continue
                try:
                    with open(DATA_DIR + case + "/" + method + Throuput_WAN, 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            # 2050000000
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                xwan.append(float(line.split()[0])/1000)
                                ywan.append(float(line.split()[1])/1024/1024)
                except:
                    L.error("No file: " + DATA_DIR + case + "/" + method + Throuput_WAN)
                    continue
                # each 10 items in WAN aggregate to one item, the x-index is the first item's x-index
                xwan_new = []
                ywan_new = []
                for i in range(0, len(xwan), wan_batch):
                    xwan_new.append(xwan[i])
                    ywan_new.append(sum(ywan[i:i+wan_batch])/wan_batch)
                plt.figure(figsize=(6, 4))
                colors, font = get_css()
                plt.title(method + ': Throuput of DC and WAN', font)
                plt.rc("font", **font)
                plot_throuput(xdc, ydc, xwan_new, ywan_new, colors, font, method)

            for method in methods:
                xdc = []
                ydc = []
                xwan = []
                ywan = []
                try:
                    with open(DATA_DIR + case+ "/"  + method + Delay_DC, 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                xdc.append(float(line.split()[0])/1000)
                                ydc.append(float(line.split()[1])/1000)
                except:
                    L.error("No file: " + DATA_DIR + case+ "/"  + method + Delay_DC)
                    continue
                try:
                    with open(DATA_DIR + case+ "/"  + method + Delay_WAN, 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                xwan.append(float(line.split()[0])/1000)
                                ywan.append(float(line.split()[1])/1000)
                except:
                    L.error("No file: " + DATA_DIR + case+ "/"  + method + Delay_WAN)
                    continue
                # each 10 items in WAN aggregate to one item, the x-index is the first item's x-index
                xwan_new = []
                ywan_new = []
                for i in range(0, len(xwan), wan_batch):
                    xwan_new.append(xwan[i])
                    ywan_new.append(sum(ywan[i:i+wan_batch])/wan_batch)
                plt.figure(figsize=(6, 4))
                colors, font = get_css()
                plt.title(method + ': Delay of DC and WAN', font)
                plt.rc("font", **font)
                plot_delay(xdc, ydc, xwan_new, ywan_new, colors, font, method)
        
        if "start" in case or "exit" in case or "congestion" in case:
            for method in methods:
                x1 = []
                y1 = []
                x2 = []
                y2 = []
                x3 = []
                y3 = []
                flows = ["_thoughput_flow_1.txt", "_thoughput_flow_2.txt", "_thoughput_flow_3.txt"]
                if os.path.exists(DATA_DIR + case + "/" + method + flows[0]):
                    with open(DATA_DIR + case + "/" + method + flows[0], 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                x1.append(float(line.split()[0])/1000)
                                y1.append(float(line.split()[1])/1024/1024)
                if os.path.exists(DATA_DIR + case + "/" + method + flows[1]):
                    with open(DATA_DIR + case + "/" + method + flows[1], 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                x2.append(float(line.split()[0])/1000)
                                y2.append(float(line.split()[1])/1024/1024)
                # each 10 items in WAN aggregate to one item, the x-index is the first item's x-index
                x2_new = []
                y2_new = []
                for i in range(0, len(x2), wan_batch):
                    x2_new.append(x2[i])
                    y2_new.append(sum(y2[i:i+wan_batch])/wan_batch)
                if os.path.exists(DATA_DIR + case + "/" + method + flows[2]):
                    with open(DATA_DIR + case + "/" + method + flows[2], 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            # 2050000000
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                x3.append(float(line.split()[0])/1000)
                                y3.append(float(line.split()[1])/1024/1024)
                x3_new = []
                y3_new = []
                for i in range(0, len(x3), wan_batch):
                    x3_new.append(x3[i])
                    y3_new.append(sum(y3[i:i+wan_batch])/wan_batch)
                plt.figure(figsize=(6, 4))
                colors, font = get_css()
                plt.title(method + ': Throuput of DC and WAN', font)
                plt.rc("font", **font)
                plot_throuput_flow(x1, y1, x2_new, y2_new, x3_new, y3_new, colors, font, method)
            for method in methods:
                x1 = []
                y1 = []
                x2 = []
                y2 = []
                x3 = []
                y3 = []
                flows = ["_delay_flow_1.txt", "_delay_flow_2.txt", "_delay_flow_3.txt"]
                if os.path.exists(DATA_DIR + case+ "/"  + method + flows[0]):
                    with open(DATA_DIR + case+ "/"  + method + flows[0], 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                x1.append(float(line.split()[0])/1000)
                                y1.append(float(line.split()[1])/1000)
                if os.path.exists(DATA_DIR + case+ "/"  + method + flows[1]):
                    with open(DATA_DIR + case+ "/"  + method + flows[1], 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                x2.append(float(line.split()[0])/1000)
                                y2.append(float(line.split()[1])/1000)
                x2_new = []
                y2_new = []
                for i in range(0, len(x2), wan_batch):
                    x2_new.append(x2[i])
                    y2_new.append(sum(y2[i:i+wan_batch])/wan_batch)
                if os.path.exists(DATA_DIR + case + "/" + method + flows[2]):
                    with open(DATA_DIR + case+ "/"  + method + flows[2], 'r') as f:
                        data = f.read().splitlines()
                        for line in data:
                            if(float(line.split()[0]) < DRAW_END_TIME and float(line.split()[0]) > DRAW_START_TIME):
                                x3.append(float(line.split()[0])/1000)
                                y3.append(float(line.split()[1])/1024/1024)
                x3_new = []
                y3_new = []
                for i in range(0, len(x3), wan_batch):
                    x3_new.append(x3[i])
                    y3_new.append(sum(y3[i:i+wan_batch])/wan_batch)
                plt.figure(figsize=(6, 4))
                colors, font = get_css()
                plt.title(method + ': Throuput of DC and WAN', font)
                plt.rc("font", **font)
                plot_delay_flow(x1, y1, x2_new, y2_new, x3_new, y3_new, colors, font, method)

