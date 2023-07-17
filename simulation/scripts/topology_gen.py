import os

def mix_topology_gen(config_file_name, num_host_per_rack, num_rack, num_agg, num_core, num_wan_switch, num_wan_host, bandwidth_dchost_to_rack, bandwidth_rack_to_agg, bandwidth_agg_to_core, bandwidth_core_to_wanswitch, bandwidth_wanswitch_to_wanhost, delay_dc, delay_wan, error_rate_dc, error_rate_wan, pod):
    """
    Generate topology for FatTree-WAN mixed network, and write to a config file.
    [this topology is a fat tree with WAN switches and hosts connected to the core switches]
    :param config_file_name: name of the config file
    :param num_host_per_rack: number of hosts per rack
    :param num_rack: number of racks in fat tree (num of ToR switches)
    :param num_agg: number of aggregation switches
    :param num_core: number of core switches
    :param num_wan_switch: number of WAN switches
    :param num_wan_host: number of WAN hosts
    :param bandwidth_dchost_to_rack: bandwidth from dc hosts to racks
    :param bandwidth_rack_to_agg: bandwidth from racks to aggregation switches
    :param bandwidth_agg_to_core: bandwidth from aggregation switches to core switches
    :param bandwidth_core_to_wanswitch: bandwidth from core switches to WAN switches
    :param bandwidth_wanswitch_to_wanhost: bandwidth from WAN switches to WAN hosts
    :param delay_dc: delay of dc links
    :param delay_wan: delay of WAN links
    :param error_rate_dc: error rate of dc hosts
    :param error_rate_wan: error rate of WAN hosts
    :param pod: number of pods in fat tree
    :return: None
    
    Note that:
    1) the node is numbered from 0 to N-1, where N is the total number of nodes.
    2) 我们先对dc\wan host进行编号，然后对switch进行编号
    3) config file format:
        First line: total node #, switch node #, link #
        Second line: switch node IDs...
        src0 dst0 rate delay error_rate
        src1 dst1 rate delay error_rate
    """
    # open config file
    config_file = open(config_file_name, 'w')
    cnt_link = 0;
    idx = 0;
    # dc host idx from 0 to num_rack * num_host_per_rack - 1
    dc_host_idx = [i for i in range(num_rack * num_host_per_rack)]
    idx += num_rack * num_host_per_rack
    # wan host idx from num_rack * num_host_per_rack to num_rack * num_host_per_rack + num_wan_host - 1
    wan_host_idx = [i for i in range(idx, idx + num_wan_host)]
    idx += num_wan_host
    dc_ToR_idx = [i for i in range(idx, idx + num_rack)]
    idx += num_rack
    dc_Agg_idx = [i for i in range(idx, idx + num_agg)]
    idx += num_agg
    dc_Core_idx = [i for i in range(idx, idx + num_core)]
    idx += num_core
    wan_switch_idx = [i for i in range(idx, idx + num_wan_switch)]
    idx += num_wan_switch
    
    # 1.write dc host to rack links to config file
    for i in range(num_rack):
        for j in range(num_host_per_rack):
            config_file.write(str(dc_host_idx[i * num_host_per_rack + j]) + " " + str(dc_ToR_idx[i]) + " " + bandwidth_dchost_to_rack + " " + delay_dc + " " + error_rate_dc + "\n")
            cnt_link+=1
    # 2.write rack to agg links to config file
    rack_per_pod = int(num_rack / pod)
    agg_per_pod = int(num_agg / pod)
    for i in range(pod): 
        for j in range(rack_per_pod):
            for k in range(agg_per_pod):
                config_file.write(str(dc_ToR_idx[i * rack_per_pod + j]) + " " + str(dc_Agg_idx[i * agg_per_pod + k]) + " " + bandwidth_rack_to_agg + " " + delay_dc + " " + error_rate_dc + "\n")
                cnt_link+=1
    # 3.write agg to core links to config file
    partition_size = int(num_core/agg_per_pod)
    for i in range(pod):
        for j in range(agg_per_pod):
            for k in range(partition_size):
                config_file.write(str(dc_Agg_idx[i * agg_per_pod + j]) + " " + str(dc_Core_idx[j * partition_size + k]) + " " + bandwidth_agg_to_core + " " + delay_dc + " " + error_rate_dc + "\n")
                cnt_link+=1
    # 4.write core to wan switch links to config file
    for i in range(num_core):
        for j in range(num_wan_switch):
            config_file.write(str(dc_Core_idx[i]) + " " + str(wan_switch_idx[j]) + " " + bandwidth_core_to_wanswitch + " " + delay_wan + " " + error_rate_wan + "\n")
            cnt_link+=1
    # 5.write wan switch to wan host links to config file
    for i in range(num_wan_switch):
        for j in range(num_wan_host):
            config_file.write(str(wan_switch_idx[i]) + " " + str(wan_host_idx[j]) + " " + bandwidth_wanswitch_to_wanhost + " " + delay_wan + " " + error_rate_wan + "\n")
            cnt_link+=1
    # 6.write total node #, switch node #, link # to config file
    # 以append的方式写入文件最前方，不能覆盖最前面的内容
    config_file = open(config_file_name, 'r+')
    content = config_file.read()
    config_file.seek(0, 0)
    config_file.write(str(idx) + " " + str(idx - num_wan_host - num_host_per_rack * num_rack) + " " + str(cnt_link) + "\n")
    # 7.write switch node IDs to config file
    for i in dc_ToR_idx:
        config_file.write(str(i) + " ")
    for i in dc_Agg_idx:
        config_file.write(str(i) + " ")
    for i in dc_Core_idx:
        config_file.write(str(i) + " ")
    for i in wan_switch_idx:
        config_file.write(str(i) + " ")
    config_file.write("\n")
    config_file.write(content)
    config_file.close()
    return

if __name__== "__main__" :

    mix_topology_gen(
        config_file_name="mix.txt", 
        num_host_per_rack=8, 
        num_rack=16, 
        num_agg=16,
        num_core=8,
        num_wan_switch=4,
        num_wan_host=4,
        bandwidth_dchost_to_rack="10Gbps",
        bandwidth_rack_to_agg="10Gbps",
        bandwidth_agg_to_core="10Gbps",
        bandwidth_core_to_wanswitch="10Gbps",
        bandwidth_wanswitch_to_wanhost="10Gbps",
        delay_dc="1000ns",
        delay_wan="10ms",
        error_rate_dc="0.000000",
        error_rate_wan="0.000000",
        pod=4)
    
    # Hpcc FatTree topology [generate for validation this script]
    # mix_topology_gen(
    #     config_file_name="mix.txt", 
    #     num_host_per_rack=16, 
    #     num_rack=20, 
    #     num_agg=20,
    #     num_core=16,
    #     num_wan_switch=4,
    #     num_wan_host=4,
    #     bandwidth_dchost_to_rack="10Gbps",
    #     bandwidth_rack_to_agg="10Gbps",
    #     bandwidth_agg_to_core="10Gbps",
    #     bandwidth_core_to_wanswitch="10Gbps",
    #     bandwidth_wanswitch_to_wanhost="10Gbps",
    #     delay_dc="1000ns",
    #     delay_wan="10ms",
    #     error_rate_dc="0.000000",
    #     error_rate_wan="0.000000",
    #     pod=4)
        
        
        
        
    
    