# encoding: utf-8
from typing import List
from enum import Enum, unique

@unique
class NodeType(Enum):
    DC_HOST = 1
    WAN_HOST = 2
    DC_TOR = 3
    DC_AGG = 4
    DC_CORE = 5
    WAN_TOR = 6
    WAN_AGG = 7
    
@unique
class LinkType(Enum):
    """LinkType describe the type of links between two groups of nodes

    Args:
        Enum (FULL_CONNECTION): Fully connected
        Enum (LESS_TO_MORE_PARTITION): partition the dst nodes into several groups, and each group connect to one unique src node, note that the number of dst nodes should be a multiple of the number of src nodes
    """
    FULL_CONNECTION = 1
    LESS_TO_MORE_PARTITION = 2

class Node:
    def __init__(self, label: str, node_id: int, switch_type: NodeType):
        self.label = label
        self.node_id = node_id
        self.switch_type = switch_type
    
    def __str__(self):
        return self.label + " " + str(self.node_id) + " " + str(self.switch_type)
    
class Links:
    """
    Describe links between two groups of nodes
    """
    def __init__(self, src: List[Node], dst: List[Node], rate: str, delay: str, error_rate: str, link_type: LinkType):
        self.src = src
        self.dst = dst
        self.rate = rate
        self.delay = delay
        self.error_rate = error_rate
        self.link_type = link_type
    
    def __str__(self):
        outStream = ""
        if(self.link_type == LinkType.FULL_CONNECTION):
            for src_node in self.src:
                for dst_node in self.dst:
                    outStream += str(src_node.node_id) + " " + str(dst_node.node_id) + " " + str(self.rate) + " " + str(self.delay) + " " + str(self.error_rate) + "\n"
        if(self.link_type == LinkType.LESS_TO_MORE_PARTITION):
            if (__debug__):
                print("src len: " + str(len(self.src)))
                print("dst len: " + str(len(self.dst)))
                print("partition size: " + str(int(len(self.dst) / len(self.src))))
            partition_size = int(len(self.dst) / len(self.src))
            
            for i in range(len(self.src)):
                src_node : Node = self.src[i]
                for j in range(partition_size):
                    dst_node : Node = self.dst[i * partition_size + j]
                    outStream += str(src_node.node_id) + " " + str(dst_node.node_id) + " " + str(self.rate) + " " + str(self.delay) + " " + str(self.error_rate) + "\n"
        return outStream
        
    def link_cnt(self):
        if(self.link_type == LinkType.FULL_CONNECTION):
            return len(self.src) * len(self.dst)
        if(self.link_type == LinkType.LESS_TO_MORE_PARTITION):
            return len(self.dst)
    
def mix_topology_gen(config_file_name: str):
    """
    [Using Nodes-Links data structure]
    generate non-congestion topology for FatTree-WAN mixed network, which has a same fat tree topology with WAN switches and hosts connected to the core switches.
    This topology satisfies the following conditions:
        1. agg = tor = rack
        2. agg and tor are fully connected
        3. agg and core connect by less to more partition mode
        4. tor and host connect by one to all mode
    Note that:
        1. Non-congestion topology should be calulated by the user.
        2. In this topology, we mirror the fat tree topology to generate the WAN topology.(In order to avoid oversubscription)
    """
    # config args
    # [DC env]: 8 core switches, 4 pods, 4 aggregation switches per pod, 4 ToR switches per pod, 8 hosts per ToR switch
    # [WAN env]: 4 WAN pods, 4 WAN aggregation switches per pod, 4 WAN ToR switches per pod, 8 WAN hosts per ToR switch
    core_num = 8
    pods = 4
    host_per_rack = 8
    agg_per_pod = 4
    tor_per_pod = agg_per_pod
    rack_per_pod = tor_per_pod
    base_idx = 0
    # ------------------- Generate Nodes -------------------
    DC_Hosts : List[Node] = []
    for i in range(pods):
        for j in range(rack_per_pod):
            for k in range(host_per_rack):
                label = "DC_Host_" + str(i) + "_" + str(j) + "_" + str(k)
                DC_Hosts.append(Node(label, base_idx, NodeType.DC_HOST))
                base_idx += 1
    WAN_Hosts : List[Node] = []
    for i in range(pods):
        for j in range(rack_per_pod):
            for k in range(host_per_rack):
                label = "WAN_Host_" + str(i) + "_" + str(j) + "_" + str(k)
                WAN_Hosts.append(Node(label, base_idx, NodeType.WAN_HOST))
                base_idx += 1
    DC_ToR : List[Node] = []
    for i in range(pods):
        for j in range(tor_per_pod):
            label = "DC_ToR_" + str(i) + "_" + str(j)
            DC_ToR.append(Node(label, base_idx, NodeType.DC_TOR))
            base_idx += 1
    WAN_ToR : List[Node] = []
    for i in range(pods):
        for j in range(tor_per_pod):
            label = "WAN_ToR_" + str(i) + "_" + str(j)
            WAN_ToR.append(Node(label, base_idx, NodeType.WAN_TOR))
            base_idx += 1
    DC_Agg : List[Node] = []
    for i in range(pods):
        for j in range(agg_per_pod):
            label = "DC_Agg_" + str(i) + "_" + str(j)
            DC_Agg.append(Node(label, base_idx, NodeType.DC_AGG))
            base_idx += 1
    WAN_Agg : List[Node] = []
    for i in range(pods):
        for j in range(agg_per_pod):
            label = "WAN_Agg_" + str(i) + "_" + str(j)
            WAN_Agg.append(Node(label, base_idx, NodeType.WAN_AGG))
            base_idx += 1
    DC_Core : List[Node] = []     
    for i in range(core_num):
        label = "DC_Core_" + str(i)
        DC_Core.append(Node(label, base_idx, NodeType.DC_CORE))
        base_idx += 1
    all_dc_switches : List[Node] = DC_Core + DC_Agg + DC_ToR
    all_wan_switches : List[Node] = WAN_ToR + WAN_Agg 
    # ------------------- Generate Links -------------------
    # Rack to ToR
    DCHost_To_DCToR : List[Links] = []     
    for i in range(pods):
        for j in range(tor_per_pod):
            DCHost_To_DCToR.append(Links(DC_Hosts[(i*tor_per_pod+j)*host_per_rack:(i*tor_per_pod+j+1)*host_per_rack], [DC_ToR[i*tor_per_pod+j]], "10Gbps", "1000ns", "0.000000", LinkType.FULL_CONNECTION))
    WANHost_To_WANToR : List[Links] = []
    for i in range(pods):
        for j in range(tor_per_pod):
            WANHost_To_WANToR.append(Links(WAN_Hosts[(i*tor_per_pod+j)*host_per_rack:(i*tor_per_pod+j+1)*host_per_rack], [WAN_ToR[i*tor_per_pod+j]], "10Gbps", "1000ns", "0.000000", LinkType.FULL_CONNECTION))
    DCToR_To_DCAgg : List[Links] = []
    for i in range(pods):
        DCToR_To_DCAgg.append(Links(DC_ToR[i*tor_per_pod : (i+1)*tor_per_pod], DC_Agg[i*agg_per_pod : (i+1)*agg_per_pod], "20Gbps", "1000ns", "0.000000", LinkType.FULL_CONNECTION))
    WANToR_To_WANAgg : List[Links] = []
    for i in range(pods):
        WANToR_To_WANAgg.append(Links(WAN_ToR[i*tor_per_pod : (i+1)*tor_per_pod], WAN_Agg[i*agg_per_pod : (i+1)*agg_per_pod], "20Gbps", "1000ns", "0.000000", LinkType.FULL_CONNECTION))
    DCAgg_To_DCCore : List[Links] = []
    for i in range(pods):
        DCAgg_To_DCCore.append(Links(DC_Agg[i*agg_per_pod : (i+1)*agg_per_pod], DC_Core, "40Gbps", "1000ns", "0.000000", LinkType.LESS_TO_MORE_PARTITION))
    WANAgg_To_DCCore : List[Links] = []
    for i in range(pods):
        WANAgg_To_DCCore.append(Links(WAN_Agg[i*agg_per_pod : (i+1)*agg_per_pod], DC_Core,  "40Gbps", "1000ns", "0.000000", LinkType.LESS_TO_MORE_PARTITION))
    all_links : List[Links] = DCHost_To_DCToR + WANHost_To_WANToR + DCToR_To_DCAgg + WANToR_To_WANAgg + DCAgg_To_DCCore + WANAgg_To_DCCore      
    # ------------------- Generate Config File -------------------
    with open(config_file_name, 'w') as f:
        nodes_num = len(DC_Hosts) + len(WAN_Hosts) + len(all_dc_switches) + len(all_wan_switches)
        switch_num = len(all_dc_switches) + len(all_wan_switches)
        link_num = 0
        for link in all_links:
            link_num += link.link_cnt()
        f.write(str(nodes_num) + " " + str(switch_num) + " " + str(link_num) + "\n")
        for switch in all_dc_switches:
            f.write(str(switch.node_id) + " ")
        f.write("\n")
        for switch in all_wan_switches:
                f.write(str(switch.node_id) + " ")
        f.write("\n")
        for link in all_links:
            f.write(str(link))
    

if __name__== "__main__" :
    mix_topology_gen(config_file_name="mix.txt")


