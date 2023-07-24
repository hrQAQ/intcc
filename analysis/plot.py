import os
import matplotlib.pyplot as plt
import re

subdir="exit/"
FIGDIR = "/home/huangrui/intcc/hpcc/analysis/Figtures/" + subdir

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

methods = ["DCTCP", "DCQCN", "HPCC", "TIMELY", "GEAR"]
Throuput_DC = "_thoughput_dc.txt"
Throuput_WAN = "_thoughput_wan.txt"
Delay_DC = "_delay_dc.txt"
Delay_WAN = "_delay_wan.txt"

def plot_throuput(xdc, ydc, xwan, ywan, colors, font, method, load_ratio):
    print("[" + method + "] ")
    if (sum(ywan)!=0):
        print("     Minus: " + str(sum(ydc)/sum(ywan) - load_ratio))
    print("     Sum: " + str(sum(ydc)+sum(ywan)))
    plt.plot(xdc, ydc, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(xwan, ywan, color=colors[2], linestyle='-', marker='', label='WAN', linewidth=1)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Throuput (MB)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Throuput(' + method + ').pdf', bbox_inches='tight')
    plt.close()
    
def plot_delay(xdc, ydc, xwan, ywan, colors, font, method, load_ratio):
    print("[" + method + "] ")
    print("sum(ydc): " + str(sum(ydc)) + " sum(ywan): " + str(sum(ywan)))
    plt.plot(xdc, ydc, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(xwan, ywan, color=colors[2], linestyle='-', marker='', label='WAN', linewidth=1)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Delay (us)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Delay(' + method + ').pdf', bbox_inches='tight')
    plt.close()

if "load" in subdir:
    load=subdir
    # regex = re.compile(r'load(\d+):(\d+)')
    # match = regex.match(load)
    # load_ratio = float(match.group(1))/float(match.group(2))
    load_ratio = 5
    print("Throuput DC:Wan with load ratio: " + str(load_ratio))
    for method in methods:
        xdc = []
        ydc = []
        xwan = []
        ywan = []
        if os.path.exists("traceinfo/" + load + method + Throuput_DC):
            with open("traceinfo/" + load + method + Throuput_DC, 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    if(float(line.split()[0]) < 2100000000 and float(line.split()[0]) > 2000000000):
                        xdc.append(float(line.split()[0])/1000)
                        ydc.append(float(line.split()[1])/1024/1024)
        else:
            print("No file: " + "traceinfo/" + load + method + Throuput_DC)
        if os.path.exists("traceinfo/" + load + method + Throuput_WAN):
            print("Open file: " + "traceinfo/" + load + method + Throuput_WAN)
            with open("traceinfo/" + load + method + Throuput_WAN, 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    # 2050000000
                    if(float(line.split()[0]) < 2100000000 and float(line.split()[0]) > 2000000000):
                        xwan.append(float(line.split()[0])/1000)
                        ywan.append(float(line.split()[1])/1024/1024)
        else:
            print("No file: " + "traceinfo/" + load + method + Throuput_WAN)
        plt.figure(figsize=(6, 4))
        colors, font = get_css()
        plt.title(method + ': Throuput of DC and WAN', font)
        plt.rc("font", **font)
        plot_throuput(xdc, ydc, xwan, ywan, colors, font, method, load_ratio)

    print("Delay with load ratio: " + str(load_ratio))
    for method in methods:
        xdc = []
        ydc = []
        xwan = []
        ywan = []
        if os.path.exists("traceinfo/" + load + method + Delay_DC):
            with open("traceinfo/" + load + method + Delay_DC, 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    if(float(line.split()[0]) < 2800000000 and float(line.split()[0]) > 2000000000):
                        xdc.append(float(line.split()[0])/1000)
                        ydc.append(float(line.split()[1])/1000)
        else:
            print("No file: " + "traceinfo/" + load + method + Delay_DC)
        if os.path.exists("traceinfo/" + load + method + Delay_WAN):
            with open("traceinfo/" + load + method + Delay_WAN, 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    if(float(line.split()[0]) < 2800000000 and float(line.split()[0]) > 2000000000):
                        xwan.append(float(line.split()[0])/1000)
                        ywan.append(float(line.split()[1])/1000)
        else:
            print("No file: " + "traceinfo/" + load + method + Delay_WAN)
        plt.figure(figsize=(6, 4))
        colors, font = get_css()
        plt.title(method + ': Delay of DC and WAN', font)
        plt.rc("font", **font)
        plot_delay(xdc, ydc, xwan, ywan, colors, font, method, load_ratio)


def plot_throuput_flow(x1, y1, x2, y2, x3, y3, colors, font, method):
    print("[" + method + "] ")
    plt.plot(x1, y1, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(x2, y2, color=colors[4], linestyle='-', marker='', label='WAN', linewidth=1)
    plt.plot(x3, y3, color=colors[3], linestyle='-', marker='', label='ADD', linewidth=1)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Throuput (MB)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Throuput(' + method + ').pdf', bbox_inches='tight')
    plt.close()
    
def plot_delay_flow(x1, y1, x2, y2, x3, y3, colors, font, method):
    print("[" + method + "] ")
    plt.plot(x1, y1, color=colors[0], linestyle='-', marker='', label='DC', linewidth=1)
    plt.plot(x2, y2, color=colors[4], linestyle='-', marker='', label='WAN', linewidth=1)
    # plt.plot(x3, y3, color=colors[3], linestyle='-', marker='', label='ADD', linewidth=1j)
    plt.xlabel('Time (us)', font)
    plt.ylabel('Delay (us)', font)
    # plt.xticks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='best', fontsize=16)
    plt.savefig(FIGDIR+'Delay(' + method + ').pdf', bbox_inches='tight')
    plt.close()

if "start" in subdir or "exit" in subdir:
    print("Throuput of three flow")
    start_time = 3000000000
    end_time = 4000000000
    for method in methods:
    #     x1 = []
    #     y1 = []
    #     x2 = []
    #     y2 = []
    #     x3 = []
    #     y3 = []
    #     flows = ["_thoughput_flow_1.txt", "_thoughput_flow_2.txt", "_thoughput_flow_3.txt"]
    #     if os.path.exists("traceinfo/" + subdir + method + flows[0]):
    #         with open("traceinfo/" + subdir + method + flows[0], 'r') as f:
    #             data = f.read().splitlines()
    #             for line in data:
    #                 if(float(line.split()[0]) < end_time and float(line.split()[0]) > start_time):
    #                     x1.append(float(line.split()[0])/1000)
    #                     y1.append(float(line.split()[1])/1024/1024)
    #     if os.path.exists("traceinfo/" + subdir + method + flows[1]):
    #         with open("traceinfo/" + subdir + method + flows[1], 'r') as f:
    #             data = f.read().splitlines()
    #             for line in data:
    #                 if(float(line.split()[0]) < end_time and float(line.split()[0]) > start_time):
    #                     x2.append(float(line.split()[0])/1000)
    #                     y2.append(float(line.split()[1])/1024/1024)
    #     if os.path.exists("traceinfo/" + subdir + method + flows[2]):
    #         with open("traceinfo/" + subdir + method + flows[2], 'r') as f:
    #             data = f.read().splitlines()
    #             for line in data:
    #                 # 2050000000
    #                 if(float(line.split()[0]) < end_time and float(line.split()[0]) > start_time):
    #                     x3.append(float(line.split()[0])/1000)
    #                     y3.append(float(line.split()[1])/1024/1024)
    #     plt.figure(figsize=(6, 4))
    #     colors, font = get_css()
    #     plt.title(method + ': Throuput of DC and WAN', font)
    #     plt.rc("font", **font)
    #     plot_throuput_flow(x1, y1, x2, y2, x3, y3, colors, font, method)

    print("Delay of three flow")
    start_time = 2000000000
    end_time = 5000000000
    for method in methods:
        x1 = []
        y1 = []
        x2 = []
        y2 = []
        x3 = []
        y3 = []
        flows = ["_delay_flow_1.txt", "_delay_flow_2.txt", "_delay_flow_3.txt"]
        if os.path.exists("traceinfo/" + subdir + method + flows[0]):
            with open("traceinfo/" + subdir + method + flows[0], 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    if(float(line.split()[0]) < end_time and float(line.split()[0]) > start_time):
                        x1.append(float(line.split()[0])/1000)
                        y1.append(float(line.split()[1])/1000)
        if os.path.exists("traceinfo/" + subdir + method + flows[1]):
            with open("traceinfo/" + subdir + method + flows[1], 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    if(float(line.split()[0]) < end_time and float(line.split()[0]) > start_time):
                        x2.append(float(line.split()[0])/1000)
                        y2.append(float(line.split()[1])/1000)
        if os.path.exists("traceinfo/" + subdir + method + flows[2]):
            with open("traceinfo/" + subdir + method + flows[2], 'r') as f:
                data = f.read().splitlines()
                for line in data:
                    if(float(line.split()[0]) < end_time and float(line.split()[0]) > start_time):
                        x3.append(float(line.split()[0])/1000)
                        y3.append(float(line.split()[1])/1024/1024)
        plt.figure(figsize=(6, 4))
        colors, font = get_css()
        plt.title(method + ': Throuput of DC and WAN', font)
        plt.rc("font", **font)
        plot_delay_flow(x1, y1, x2, y2, x3, y3, colors, font, method)

