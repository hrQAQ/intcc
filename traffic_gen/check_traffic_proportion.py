import os

flow_file = open("tmp_traffic.txt", "r")
flow_cnt = int(flow_file.readline())
cnt_dst_dc = 0
cnt_dst_wan = 0
for i in range(flow_cnt):
    src, dst, _, _, size, time = flow_file.readline().split()
    # print("%s %s %s %s %s %s"%(src, dst, _, _, size, time))
    if(int(src) > 128):
        print("panic at wrong src %d"%i)
    if(int(dst) < 128):
        cnt_dst_dc += 1
    else:
        cnt_dst_wan += 1
print("dc: %d, wan: %d"%(cnt_dst_dc, cnt_dst_wan))
print("dc: %f, wan: %f"%(cnt_dst_dc/flow_cnt, cnt_dst_wan/flow_cnt))
    
    