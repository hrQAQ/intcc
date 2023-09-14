import os
from typing import NamedTuple, List

class Config(NamedTuple):
    flow_file_name:str
    cc_mode:int

root_data_dir="../../data/"
data_dir_list = os.listdir(root_data_dir)
# {{1, "DCQCN"}, {3, "HPCC"}, {7, "TIMELY"}, {8, "DCTCP"}, {13, "GEAR"}};
cc_mode_list = [1, 3, 7, 8, 13]
configs:List[Config] = []
for data_dir in data_dir_list:
    for cc_mode in cc_mode_list:
        config:Config = Config(data_dir, cc_mode)
        configs.append(config)
for config in configs:
    print("FLOW_FILE_NAME ", config.flow_file_name)
    print("CC_MODE ", config.cc_mode, "\n")