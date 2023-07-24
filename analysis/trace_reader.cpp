#include <unistd.h>

#include <cstdio>
#include <unordered_map>
#include <unordered_set>

#include "sim-setting.h"
#include "trace-format.h"
#include "trace_filter.hpp"
#include "utils.hpp"

using namespace ns3;
using namespace std;

uint32_t ip_to_node_id(uint32_t ip) { return (ip >> 8) & 0xffff; }

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        printf("Usage: ./trace_reader <trace_file> [filter_expr]\n");
        return 0;
    }
    FILE* file = fopen(argv[1], "r");
    string filename = argv[1];  // ../data/load1:1/GEAR.tr
    // change to load1:1/GEAR
    string MIDDIR = filename.substr(8, filename.find_last_of(".") - 8);
    printf("MIDDIR: %s\n", MIDDIR.c_str());
    TraceFilter f;
    if (argc == 3) {
        f.parse(argv[2]);
        if (f.root == NULL) {
            printf("Invalid filter\n");
            return 0;
        }
    }
    // printf("filter: %s\n", f.str().c_str());

    // first read SimSetting
    SimSetting sim_setting;
    sim_setting.Deserialize(file);
#if 0
	// print sim_setting
	for (auto i : sim_setting.port_speed)
		for (auto j : i.second)
			printf("%u,%u:%lu\n", i.first, j.first, j.second);
#endif

    TraceFormat tr;
    // topo args
    int num_dc_nodes = 128;
    int num_wan_nodes = 128;
    uint64_t interval = 14 * 1000 * 2;  // 2RTT
    uint64_t start_time = 2000000000;   // 2s 开始传输数据
    uint64_t end_time = 40000000000;    // 4s 结束传输数据
    uint64_t interval_cnt = (end_time - start_time) / interval + 1;
    double EWMA = 0.9;
    // 统计总值、均值
    if (0) {
        // 吞吐量统计
        uint64_t* thoughput_dc_time = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* thoughput_wan_time = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        memset(thoughput_dc_time, 0, sizeof(uint64_t) * interval_cnt);
        memset(thoughput_wan_time, 0, sizeof(uint64_t) * interval_cnt);
        // 时延统计
        uint64_t* delay_dc_time = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* delay_dc_time_cnt = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* delay_wan_time = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* delay_wan_time_cnt = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        memset(delay_dc_time, 0, sizeof(uint64_t) * interval_cnt);
        memset(delay_dc_time_cnt, 0, sizeof(uint64_t) * interval_cnt);
        memset(delay_wan_time, 0, sizeof(uint64_t) * interval_cnt);
        memset(delay_wan_time_cnt, 0, sizeof(uint64_t) * interval_cnt);
        bool has_delay_dc = false;
        bool has_delay_wan = false;
        // 丢包率统计
        uint64_t* loss_dc_time = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* loss_wan_time = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        while (tr.Deserialize(file) > 0) {
            if (!f.test(tr)) continue;
            // print_trace(tr);
            // -------Throughput--------//
            uint16_t nodeID = tr.node;
            uint16_t dstNodeID = ip_to_node_id(tr.dip);
            if (tr.event == Event::Dequ && nodeID < num_dc_nodes && tr.l3Prot == 0x11) {  // UDP
                if (dstNodeID < num_dc_nodes) {
                    // printf("DC dstNodeID: %u\n", dstNodeID);
                    thoughput_dc_time[(tr.time - start_time) / interval] += tr.size;
                } else if (dstNodeID < num_dc_nodes + num_wan_nodes) {
                    // printf("WAN dstNodeID: %u\n", dstNodeID);
                    thoughput_wan_time[(tr.time - start_time) / interval] += tr.size;
                }
            }
            // -------Delay--------//
            if (nodeID < num_dc_nodes && tr.event == Event::Recv && tr.l3Prot == 0xFC) {  // ACK
                uint16_t sourceNodeID = ip_to_node_id(tr.sip);
                if (sourceNodeID < num_dc_nodes) {
                    has_delay_dc = true;
                    uint64_t delay = tr.time - tr.ack.ts;
                    delay_dc_time[(tr.time - start_time) / interval] += delay;
                    delay_dc_time_cnt[(tr.time - start_time) / interval] += 1;
                    printf("[DC] delay %lu\n", delay);
                } else if (sourceNodeID < num_dc_nodes + num_wan_nodes) {
                    has_delay_wan = true;
                    uint64_t delay = tr.time - tr.ack.ts;
                    delay_wan_time[(tr.time - start_time) / interval] += delay;
                    delay_wan_time_cnt[(tr.time - start_time) / interval] += 1;
                    printf("[WAN] delay %lu\n", delay);
                }
            }
            // -------Loss--------//
        }

        // -------Throughput--------//
        uint64_t dc_total_time = 0;
        uint64_t wan_total_time = 0;
        for (uint64_t i = 0; i < interval_cnt; i++) {
            uint64_t cur_time = thoughput_dc_time[i] * interval + start_time;
            if (cur_time > 2100000000) {
                dc_total_time += thoughput_dc_time[i];
                wan_total_time += thoughput_wan_time[i];
            }
        }
        printf("dc_total_time: %lu\n", dc_total_time);
        printf("wan_total_time: %lu\n", wan_total_time);
        if (wan_total_time != 0) printf("[Throuput] dc : wan = %lf\n", (double)dc_total_time / wan_total_time);
        // print thoughput with time to file
        string thoughput_dc_file_name = "traceinfo/" + MIDDIR + "_thoughput_dc.txt";
        string thoughput_wan_file_name = "traceinfo/" + MIDDIR + "_thoughput_wan.txt";
        FILE* thoughput_dc_file = fopen(thoughput_dc_file_name.c_str(), "w");
        FILE* thoughput_wan_file = fopen(thoughput_wan_file_name.c_str(), "w");
        for (uint64_t i = 0; i < interval_cnt; i++) {
            fprintf(thoughput_dc_file, "%lu %lu\n", start_time + i * interval, thoughput_dc_time[i]);
            fprintf(thoughput_wan_file, "%lu %lu\n", start_time + i * interval, thoughput_wan_time[i]);
        }
        // -------Delay--------//
        printf("has_delay_dc: %d\n", has_delay_dc);
        printf("has_delay_wan: %d\n", has_delay_wan);
        uint64_t dc_total_delay = 0;
        uint64_t wan_total_delay = 0;
        uint64_t dc_total_delay_cnt = 0;
        uint64_t wan_total_delay_cnt = 0;
        for (uint64_t i = 0; i < interval_cnt; i++) {
            if (delay_dc_time_cnt[i] != 0) {
                dc_total_delay += delay_dc_time[i];
                dc_total_delay_cnt += delay_dc_time_cnt[i];
                delay_dc_time[i] /= delay_dc_time_cnt[i];
            }
            if (delay_wan_time_cnt[i] != 0) {
                wan_total_delay += delay_wan_time[i];
                wan_total_delay_cnt += delay_wan_time_cnt[i];
                delay_wan_time[i] /= delay_wan_time_cnt[i];
            }
            // printf("delay_wan_time_cnt: %lu\n", delay_wan_time_cnt[i]);
            // printf("delay_dc_time_cnt: %lu\n", delay_dc_time_cnt[i]);
        }
        uint64_t dc_avg_delay, wan_avg_delay;
        if (dc_total_delay_cnt == 0) {
            dc_avg_delay = 0;
        } else {
            dc_avg_delay = dc_total_delay / dc_total_delay_cnt;
        }
        if (wan_total_delay_cnt == 0) {
            wan_avg_delay = 0;
        } else {
            wan_avg_delay = wan_total_delay / wan_total_delay_cnt;
        }
        printf("[AVG Delay] dc %lu : wan %lu\n", dc_avg_delay, wan_avg_delay);
        // print delay with time to file
        string delay_dc_file_name = "traceinfo/" + MIDDIR + "_delay_dc.txt";
        string delay_wan_file_name = "traceinfo/" + MIDDIR + "_delay_wan.txt";
        FILE* delay_dc_file = fopen(delay_dc_file_name.c_str(), "w");
        FILE* delay_wan_file = fopen(delay_wan_file_name.c_str(), "w");
        // EWMA process
        uint64_t prev_dc = 0, prev_wan = 0, x, y;
        for (uint64_t i = 0; i < interval_cnt; i++) {
            if (delay_dc_time[i] != 0) {
                if (prev_dc == 0) {
                    prev_dc = delay_dc_time[i];
                }
                y = EWMA * prev_dc + (1 - EWMA) * delay_dc_time[i];
                fprintf(delay_dc_file, "%lu %lu\n", start_time + i * interval, delay_dc_time[i]);
                prev_dc = y;
            }
            if (delay_wan_time[i] != 0) {
                if (prev_wan == 0) {
                    prev_wan = delay_wan_time[i];
                }
                y = EWMA * prev_wan + (1 - EWMA) * delay_wan_time[i];
                fprintf(delay_wan_file, "%lu %lu\n", start_time + i * interval, delay_wan_time[i]);
                prev_wan = y;
            }
        }
        fclose(thoughput_dc_file);
        fclose(thoughput_wan_file);
    }
    // 逐流统计
    if (1) {
        int flow_1_nodeID = 0, flow_1_dst_nodeID = 3;
        int flow_2_nodeID = 1, flow_2_dst_nodeID = 4;
        int flow_3_nodeID = 2, flow_3_dst_nodeID = 5;
        uint64_t* flow_1_throughput = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* flow_2_throughput = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        uint64_t* flow_3_throughput = (uint64_t*)malloc(sizeof(uint64_t) * interval_cnt);
        map<uint64_t, uint64_t> flow_1_delay, flow_1_seq_send_time;
        map<uint64_t, uint64_t> flow_2_delay, flow_2_seq_send_time;
        map<uint64_t, uint64_t> flow_3_delay, flow_3_seq_send_time;

        while (tr.Deserialize(file) > 0) {
            if (!f.test(tr)) continue;
            // print_trace(tr);
            if (tr.time < start_time) {
                continue;
            }
            if (tr.node == flow_1_nodeID) {
                // Throughput
                if (tr.event == Event::Dequ && tr.l3Prot == 0x11) {
                    flow_1_throughput[(tr.time - start_time) / interval] += tr.size;
                    flow_1_seq_send_time[tr.data.seq] = tr.time;
                } else if (tr.l3Prot == 0xFC) {
                    // uint64_t seq = tr.ack.seq;
                    // uint64_t seq_send_time = flow_1_seq_send_time[seq];
                    // flow_1_delay[tr.time] = tr.time - seq_send_time;
                }
            } else if (tr.node == flow_2_nodeID) {
                // Throughput
                if (tr.event == Event::Dequ && tr.l3Prot == 0x11) {
                    flow_2_throughput[(tr.time - start_time) / interval] += tr.size;
                    flow_2_seq_send_time[tr.data.seq] = tr.time;
                } else if (tr.l3Prot == 0xFC) {
                    // uint64_t seq = tr.ack.seq;
                    // uint64_t seq_send_time = flow_2_seq_send_time[seq];
                    // flow_2_delay[tr.time] = tr.time - seq_send_time;
                }
            } else if (tr.node == flow_3_nodeID) {
                // Throughput
                if (tr.event == Event::Dequ && tr.l3Prot == 0x11) {
                    flow_3_throughput[(tr.time - start_time) / interval] += tr.size;
                    flow_3_seq_send_time[tr.data.seq] = tr.time;
                }
                if (tr.l3Prot == 0xFC) {
                    // uint64_t seq = tr.ack.seq;
                    // uint64_t seq_send_time = flow_3_seq_send_time[seq];
                    // flow_3_delay[tr.time] = tr.time - seq_send_time;
                }
            } else if (tr.node == flow_1_dst_nodeID) {
                if (tr.l3Prot == 0x11) {
                    flow_1_delay[tr.time] = tr.time - flow_1_seq_send_time[tr.data.seq];
                }
            } else if (tr.node == flow_2_dst_nodeID) {
                if (tr.l3Prot == 0x11) {
                    flow_2_delay[tr.time] = tr.time - flow_2_seq_send_time[tr.data.seq];
                }
            } else if (tr.node == flow_3_dst_nodeID) {
                if (tr.l3Prot == 0x11) {
                    flow_3_delay[tr.time] = tr.time - flow_3_seq_send_time[tr.data.seq];
                }
            }
        }
        // Throughput
        string thoughput_flow_1_file_name = "traceinfo/" + MIDDIR + "_thoughput_flow_1.txt";
        string thoughput_flow_2_file_name = "traceinfo/" + MIDDIR + "_thoughput_flow_2.txt";
        string thoughput_flow_3_file_name = "traceinfo/" + MIDDIR + "_thoughput_flow_3.txt";
        FILE* thoughput_flow_1_file = fopen(thoughput_flow_1_file_name.c_str(), "w");
        FILE* thoughput_flow_2_file = fopen(thoughput_flow_2_file_name.c_str(), "w");
        FILE* thoughput_flow_3_file = fopen(thoughput_flow_3_file_name.c_str(), "w");
        for (uint64_t i = 0; i < interval_cnt; i++) {
            fprintf(thoughput_flow_1_file, "%lu %lu\n", start_time + i * interval, flow_1_throughput[i]);
            fprintf(thoughput_flow_2_file, "%lu %lu\n", start_time + i * interval, flow_2_throughput[i]);
            fprintf(thoughput_flow_3_file, "%lu %lu\n", start_time + i * interval, flow_3_throughput[i]);
        }
        fclose(thoughput_flow_1_file);
        fclose(thoughput_flow_2_file);
        fclose(thoughput_flow_3_file);
        // Delay
        string delay_flow_1_file_name = "traceinfo/" + MIDDIR + "_delay_flow_1.txt";
        string delay_flow_2_file_name = "traceinfo/" + MIDDIR + "_delay_flow_2.txt";
        string delay_flow_3_file_name = "traceinfo/" + MIDDIR + "_delay_flow_3.txt";
        FILE* delay_flow_1_file = fopen(delay_flow_1_file_name.c_str(), "w");
        FILE* delay_flow_2_file = fopen(delay_flow_2_file_name.c_str(), "w");
        FILE* delay_flow_3_file = fopen(delay_flow_3_file_name.c_str(), "w");
        for (auto i : flow_1_delay) {
            fprintf(delay_flow_1_file, "%lu %lu\n", i.first, i.second);
        }
        for (auto i : flow_2_delay) {
            fprintf(delay_flow_2_file, "%lu %lu\n", i.first, i.second);
        }
        for (auto i : flow_3_delay) {
            fprintf(delay_flow_3_file, "%lu %lu\n", i.first, i.second);
        }
        fclose(delay_flow_1_file);
        fclose(delay_flow_2_file);
        fclose(delay_flow_3_file);
    }
    fclose(file);
}
