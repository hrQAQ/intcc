import os

flow_file = open("./load7:3.txt", "r")
flow_cnt = int(flow_file.readline())
cnt_dst_dc = 0
cnt_dst_wan = 0
cnt_size = 0
cnt_size_dc = 0
cnt_size_wan = 0
for i in range(flow_cnt):
    src, dst, _, _, size, time = flow_file.readline().split()
    # print("%s %s %s %s %s %s"%(src, dst, _, _, size, time))
    cnt_size += int(size)
    if(int(src) > 128):
        print("panic at wrong src %d"%i)
    if(int(dst) < 128):
        cnt_dst_dc += 1
        cnt_size_dc += int(size)
    else:
        cnt_dst_wan += 1
        cnt_size_wan += int(size)
print("dc: %d, wan: %d"%(cnt_dst_dc, cnt_dst_wan))
print("dc: %f, wan: %f"%(cnt_dst_dc/flow_cnt, cnt_dst_wan/flow_cnt))
MBsize = cnt_size/1024/1024
print("size: %d"%MBsize)
print("dc: %f, wan: %f"%(cnt_size_dc/cnt_size, cnt_size_wan/cnt_size))

    