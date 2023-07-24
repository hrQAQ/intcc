import os
import matplotlib.pyplot as plt
import collections

SUBDIR = "FCT/"
FIGDIR = "/home/huangrui/intcc/hpcc/analysis/Figtures/" + SUBDIR
DATADIR = "/home/huangrui/intcc/hpcc/analysis/FCTResults/"
FILE_LIST = ["FCT-load1:1.txt", "FCT-load1:9.txt", "FCT-load3:7.txt","FCT-load5:1.txt", "FCT-load7:3.txt", "FCT-load9:1.txt"]
METHODS = ["GEAR", "DCQCN", "HPCC", "TIMELY", "DCTCP"]
FCT_PERCENTILE = ["50", "95", "99"]

def get_css():
    line_width = 3
    mark_size = 5

    font = {'family': 'Times new roman', 'weight': 'bold', 'size': '24'}
    colors = ['#D76364', '#F1D77E', '#B1CE46', '#9394E7', '#9DC3E7']
    linestyles = ['-', '--', '-.', ':',(0, (3, 1, 1, 1))]
    markers = ['o', 's', 'v', '^', 'D', 'p', 'h', '>', '<', '8', '*', 'H', 'd', 'P', 'X', '1', '2', '3', '4', 'x', '|', '_', '+', '.']
    # css = {
    #     'flow-0': [colors[0], '-', marker, 'Flow-1', line_width, mark_size],
    #     'flow-1': [colors[2], '-', marker, 'Flow-2', line_width, mark_size],
    #     'flow-2': [colors[3], '-', marker, 'Flow-3', line_width, mark_size],
    #     'flow-3': [colors[4], '-', marker, 'Flow-4', line_width, mark_size],
    #     'flow-4': [colors[5], '-', marker, 'Flow-5', line_width, mark_size]
    # }
    return colors, font, linestyles, markers
    
line = collections.namedtuple('line', ['x', 'size', 'DCTCP_50', 'DCTCP_95', 'DCTCP_99', 'DCQCN_50', 'DCQCN_95', 'DCQCN_99', 'HPCC_50', 'HPCC_95', 'HPCC_99', 'TIMELY_50', 'TIMELY_95', 'TIMELY_99', 'GEAR_50', 'GEAR_95', 'GEAR_99'])

def size2str(size):
    if size < 1000:
        return str(size)
    elif size < 1000000:
        return float(size/1000).__round__(1).__str__() + "K"
    elif size < 1000000000:
        return str(int(size/1000000)) + "M"
    else:
        return str(int(size/1000000000)) + "G"

# 设置是否绘制尾延时，False为不绘制
DrawTail99 = False
DrawTail95 = True
Draw50 = False
# 设置要画的方法
DrawM = ["DCTCP", "DCQCN", "HPCC", "TIMELY", "GEAR"]
# DrawM = ["HPCC", "GEAR"]
drawmethods =""
for method in DrawM:
    drawmethods += "-" + method 
if DrawTail99:
    drawmethods += "-99"
if DrawTail95:
    drawmethods += "-95"
if Draw50:
    drawmethods += "-50"
    
colors, font , linestyles, markers = get_css()

for file in FILE_LIST:
    x = []
    size = []
    y = [[] for _ in range(15)]
    with open(DATADIR+file, 'r') as f:
        data = f.read().splitlines()
        for l in data:
            line_data = line(*l.split())
            x.append(float(line_data.x))
            size.append(size2str(int(line_data.size)))
            for method in METHODS:
                for percentile in FCT_PERCENTILE:
                    y[METHODS.index(method)*3+FCT_PERCENTILE.index(percentile)].append(float(line_data.__getattribute__(method+"_"+percentile)))
    # 预处理，按GEAR的FCT为标准进行归一化
    for method in METHODS:
        if method != "GEAR":
            for percentile in FCT_PERCENTILE:
                y[METHODS.index(method)*3+FCT_PERCENTILE.index(percentile)] = [y[METHODS.index(method)*3+FCT_PERCENTILE.index(percentile)][i]/y[METHODS.index("GEAR")*3+FCT_PERCENTILE.index(percentile)][i] for i in range(len(y[METHODS.index(method)*3+FCT_PERCENTILE.index(percentile)]))]
    for percentile in FCT_PERCENTILE:
        y[METHODS.index("GEAR")*3+FCT_PERCENTILE.index(percentile)] = [1 for i in range(len(y[METHODS.index("GEAR")*3+FCT_PERCENTILE.index(percentile)]))]
    # 扁平化图
    plt.figure(figsize=(16, 5))
    # plt.title('30% AVG Load FCT', font)
    plt.rc("font", **font)
    # 删去最后一个点
    x = x[:-1]
    size = size[:-1]
    for i in range(15):
        y[i] = y[i][:-1]
    for method in DrawM:
        linewidth = 2
        if(method == "GEAR"):
            linewidth = 4
        if(Draw50):
            y50 = y[METHODS.index(method)*3]
            plt.plot(x, y50, color=colors[METHODS.index(method)], linestyle=linestyles[METHODS.index(method)], marker=markers[METHODS.index(method)*3], label=method+"-50", linewidth=linewidth)
        if(DrawTail95):
            y95 = y[METHODS.index(method)*3+1]
            plt.plot(x, y95, color=colors[METHODS.index(method)], linestyle=linestyles[METHODS.index(method)], marker=markers[METHODS.index(method)*3+1], label=method+"-95", linewidth=linewidth)
        if(DrawTail99):
            y99 = y[METHODS.index(method)*3+2]
            # 点划线 linestyle='-.'
            plt.plot(x, y99, color=colors[METHODS.index(method)], linestyle=linestyles[METHODS.index(method)], marker=markers[METHODS.index(method)*3+2], label=method+"-99", linewidth=linewidth)
    plt.xlabel('Flow Size (Bytes)', font)
    plt.ylabel('FCT Slowdown', font)
    plt.xticks(x, size, rotation=45, fontsize=24)
    plt.grid(linestyle='--', linewidth=0.5)
    plt.legend(loc='upper left', fontsize=22, ncol=3)
    plt.savefig(FIGDIR+file.split('.')[0] + drawmethods +".pdf", bbox_inches='tight')
    plt.close()