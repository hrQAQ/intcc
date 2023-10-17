#include "int-header.h"

namespace ns3 {

const uint64_t IntHop::lineRateValues[8] = {25000000000lu, 50000000000lu, 100000000000lu, 200000000000lu, 400000000000lu, 10000000000lu, 0, 0};
uint32_t IntHop::multi = 1;

IntHeader::Mode IntHeader::mode = NONE;
int IntHeader::pint_bytes = 2;

IntHeader::IntHeader() : ts(0) {
	hpcc.nhop = 0;
	gear.nhop = 0;
	hpcc.ts = 0;
	gear.ts = 0;
	for (uint32_t i = 0; i < maxHop; i++) {
		hpcc.hop[i] = {0};
		gear.hop[i] = {0};
	}	
}

uint32_t IntHeader::GetStaticSize(){
	if (mode == NORMAL){
		return sizeof(hpcc.hop) + sizeof(hpcc.nhop) + sizeof(hpcc.ts);
	}else if (mode == TS){
		return sizeof(ts);
	}else if (mode == PINT){
		return sizeof(pint);
	}else if (mode == GEAR){
		return sizeof(gear.hop) + sizeof(gear.nhop) + sizeof(gear.ts);
	}else if (mode == NONE){
		return sizeof(ts);
	} else {
		return 0;
	}
}

void IntHeader::PushHop(uint64_t time, uint64_t bytes, uint32_t qlen, uint64_t rate){
	// only do this in INT mode
	if (mode == NORMAL){
		// print ih.time bytes qlen DEBUG
		// printf("PushHop: time=%lu bytess=%lu qlen=%u rate=%lu\n",
		// 		time, bytes, qlen, rate);
		uint32_t idx = hpcc.nhop % maxHop;
		hpcc.hop[idx].Set(time, bytes, qlen, rate);
        hpcc.nhop++;
	} else if (mode == GEAR){
		uint32_t idx = gear.nhop % maxHop;
		gear.hop[idx].Set(time, bytes, qlen, rate);
		gear.nhop++;
	}
}

void IntHeader::Serialize (Buffer::Iterator start) const{
	Buffer::Iterator i = start;
	if (mode == NORMAL){
		for (uint32_t j = 0; j < maxHop; j++){
			i.WriteU32(hpcc.hop[j].buf[0]);
			i.WriteU32(hpcc.hop[j].buf[1]);
		}
		i.WriteU16(hpcc.nhop);
		i.WriteU64(hpcc.ts);
	}else if (mode == TS){
		i.WriteU64(ts);
	}else if (mode == PINT){
		if (pint_bytes == 1)
			i.WriteU8(pint.power_lo8);
		else if (pint_bytes == 2)
			i.WriteU16(pint.power);
	}else if (mode == GEAR){
		for (uint32_t j = 0; j < maxHop; j++){
			i.WriteU32(gear.hop[j].buf[0]);
			i.WriteU32(gear.hop[j].buf[1]);
		}
		i.WriteU16(gear.nhop);
		i.WriteU64(gear.ts);
	}else if (mode == NONE){
		i.WriteU64(ts);
	}
}

uint32_t IntHeader::Deserialize (Buffer::Iterator start){
	Buffer::Iterator i = start;
	if (mode == NORMAL){
		for (uint32_t j = 0; j < maxHop; j++){
			hpcc.hop[j].buf[0] = i.ReadU32();
			hpcc.hop[j].buf[1] = i.ReadU32();
		}
		hpcc.nhop = i.ReadU16();
		hpcc.ts = i.ReadU64();
	}else if (mode == TS){
		ts = i.ReadU64();
	}else if (mode == PINT){
		if (pint_bytes == 1)
			pint.power_lo8 = i.ReadU8();
		else if (pint_bytes == 2)
			pint.power = i.ReadU16();
	}else if (mode == GEAR) {
		for (uint32_t j = 0; j < maxHop; j++){
			gear.hop[j].buf[0] = i.ReadU32();
			gear.hop[j].buf[1] = i.ReadU32();
		}
		gear.nhop = i.ReadU16();
		gear.ts = i.ReadU64();
	}else if (mode == NONE) {
		ts = i.ReadU64();
	}
	return GetStaticSize();
}

uint64_t IntHeader::GetTs(void){
	if (mode == TS)
		return ts;
	if (mode == NORMAL)
		return hpcc.ts;
	if (mode == GEAR)
		return gear.ts;
	if (mode == NONE)
		return ts;
	return 0;
}

uint16_t IntHeader::GetPower(void){
	if (mode == PINT)
		return pint_bytes == 1 ? pint.power_lo8 : pint.power;
	return 0;
}
void IntHeader::SetPower(uint16_t power){
	if (mode == PINT){
		if (pint_bytes == 1)
			pint.power_lo8 = power;
		else
			pint.power = power;
	}
}

}
