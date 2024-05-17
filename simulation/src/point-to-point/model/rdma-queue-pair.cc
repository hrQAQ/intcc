#include "rdma-queue-pair.h"

#include <ns3/hash.h>
#include <ns3/ipv4-header.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/udp-header.h>
#include <ns3/uinteger.h>

#include "ns3/ppp-header.h"

using namespace std;
namespace ns3 {

/**************************
 * RdmaQueuePair
 *************************/
TypeId RdmaQueuePair::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaQueuePair").SetParent<Object>();
    return tid;
}

RdmaQueuePair::RdmaQueuePair(uint16_t pg, Ipv4Address _sip, Ipv4Address _dip, uint16_t _sport, uint16_t _dport) {
    startTime = Simulator::Now();
    sip = _sip;
    dip = _dip;
    sport = _sport;
    dport = _dport;
    m_size = 0;
    snd_nxt = snd_una = 0;
    m_pg = pg;
    m_ipid = 0;
    m_win = 0;
    m_baseRtt = 0;
    m_max_rate = 0;
    m_var_win = false;
    m_rate = 0;
    m_nextAvail = Time(0);
    m_flowType = false;
    m_stopTime = 10;
    mlx.m_alpha = 1;
    mlx.m_alpha_cnp_arrived = false;
    mlx.m_first_cnp = true;
    mlx.m_decrease_cnp_arrived = false;
    mlx.m_rpTimeStage = 0;
    hp.m_lastUpdateSeq = 0;
    for (uint32_t i = 0; i < sizeof(hp.keep) / sizeof(hp.keep[0]); i++) hp.keep[i] = 0;
    hp.m_incStage = 0;
    hp.m_lastGap = 0;
    hp.u = 0.95;
    for (uint32_t i = 0; i < IntHeader::maxHop; i++) {
        hp.hopState[i].u = 0.95;
        hp.hopState[i].incStage = 0;
    }

    tmly.m_lastUpdateSeq = 0;
    tmly.m_incStage = 0;
    tmly.lastRtt = 0;
    tmly.rttDiff = 0;

    dctcp.m_lastUpdateSeq = 0;
    dctcp.m_caState = 0;
    dctcp.m_highSeq = 0;
    dctcp.m_alpha = 1;
    dctcp.m_ecnCnt = 0;
    dctcp.m_batchSizeOfAlpha = 0;

    gemini.m_lastUpdateSeq = 0;
    gemini.m_caState = 0;
    gemini.m_highSeq = 0;
    gemini.m_alpha = 1.0;
    gemini.m_ecnCnt = 0;
    gemini.m_batchSizeOfAlpha = 0;
    gemini.m_incstage = 0;

    hpccPint.m_lastUpdateSeq = 0;
    hpccPint.m_incStage = 0;

    intcc.m_flag = true;
    intcc.epsilon = 0.01;
    intcc.m_lastUpdateSeq = 0;
    intcc.m_lastUtilization = 0;
    intcc.m_lastRate = 0;
    intcc.m_bketa = 0.0;
    intcc.m_curRate = 0;
    intcc.m_incStage = 0;
    intcc.m_maxU = 0;
    intcc.u = 0.95;
    for (uint32_t i = 0; i < IntHeader::maxHop; i++) {
      intcc.hopState[i].u = 0.95;
      intcc.hopState[i].incStage = 0;
      intcc.hopState[i].m_lastUtilization = 0;
    }
    intcc.m_delta_t = 0;

    annulus.m_ns_rate = 0;
    annulus.m_e2e_rate = 0;
    annulus.m_lastUpdateSeq = 0;
    annulus.m_caState = 0;
    annulus.m_highSeq = 0;
    annulus.m_alpha = 1;
    annulus.m_ecnCnt = 0;
    annulus.m_batchSizeOfAlpha = 0;
}

void RdmaQueuePair::SetLogFile(string metric_mon_file_prefix) {
    std::string rate_log_filename = metric_mon_file_prefix + std::to_string((sip.Get() >> 8) & 0xffff) + "-" + std::to_string((dip.Get() >> 8) & 0xffff) + "-" + std::to_string(sport) + "-" + std::to_string(dport) + ".rate";
    std::string rtt_log_filename = metric_mon_file_prefix + std::to_string((sip.Get() >> 8) & 0xffff) + "-" + std::to_string((dip.Get() >> 8) & 0xffff) + "-" + std::to_string(sport) + "-" + std::to_string(dport) + ".rtt";
    m_rateLog = std::ofstream(rate_log_filename);
    m_rttLog = std::ofstream(rtt_log_filename);
    printf("RdmaQueuePair::SetLogFile %s %s\n", rate_log_filename.c_str(), rtt_log_filename.c_str());
}

void RdmaQueuePair::SetUtarget(double utarget) { 
    hp.u = utarget;
    intcc.u = utarget; 
}

void RdmaQueuePair::SetSize(uint64_t size) { m_size = size; }

void RdmaQueuePair::SetWin(uint64_t win) { m_win = win; }

void RdmaQueuePair::SetBaseRtt(uint64_t baseRtt) { m_baseRtt = baseRtt; }

void RdmaQueuePair::SetVarWin(bool v) { m_var_win = v; }

void RdmaQueuePair::SetAppNotifyCallback(Callback<void> notifyAppFinish) { m_notifyAppFinish = notifyAppFinish; }

uint64_t RdmaQueuePair::GetBytesLeft() { return m_size >= snd_nxt ? m_size - snd_nxt : 0; }

uint32_t RdmaQueuePair::GetHash(void) {
    union {
        struct {
            uint32_t sip, dip;
            uint16_t sport, dport;
        };
        char c[12];
    } buf;
    buf.sip = sip.Get();
    buf.dip = dip.Get();
    buf.sport = sport;
    buf.dport = dport;
    return Hash32(buf.c, 12);
}

void RdmaQueuePair::Acknowledge(uint64_t ack) {
    if (ack > snd_una) {
        snd_una = ack;
    }
}

uint64_t RdmaQueuePair::GetOnTheFly() { return snd_nxt - snd_una; }

bool RdmaQueuePair::IsWinBound() {
    uint64_t w = GetWin();
    return w != 0 && GetOnTheFly() >= w;
}

uint64_t RdmaQueuePair::GetWin() {
    if (m_win == 0) return 0;
    uint64_t w;
    if (m_var_win) {
        w = m_win * m_rate.GetBitRate() / m_max_rate.GetBitRate();
        if (w == 0) w = 1;  // must > 0
    } else {
        w = m_win;
    }
    return w;
}

uint64_t RdmaQueuePair::HpGetCurWin() {
    if (m_win == 0) return 0;
    uint64_t w;
    if (m_var_win) {
        w = m_win * hp.m_curRate.GetBitRate() / m_max_rate.GetBitRate();
        if (w == 0) w = 1;  // must > 0
    } else {
        w = m_win;
    }
    return w;
}

bool RdmaQueuePair::IsFinished() { return snd_una >= m_size; }

/*********************
 * RdmaRxQueuePair
 ********************/
TypeId RdmaRxQueuePair::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaRxQueuePair").SetParent<Object>();
    return tid;
}

RdmaRxQueuePair::RdmaRxQueuePair() {
    sip = dip = sport = dport = 0;
    m_ipid = 0;
    ReceiverNextExpectedSeq = 0;
    m_nackTimer = Time(0);
    m_milestone_rx = 0;
    m_lastNACK = 0;
}

uint32_t RdmaRxQueuePair::GetHash(void) {
    union {
        struct {
            uint32_t sip, dip;
            uint16_t sport, dport;
        };
        char c[12];
    } buf;
    buf.sip = sip;
    buf.dip = dip;
    buf.sport = sport;
    buf.dport = dport;
    return Hash32(buf.c, 12);
}

/*********************
 * RdmaQueuePairGroup
 ********************/
TypeId RdmaQueuePairGroup::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaQueuePairGroup").SetParent<Object>();
    return tid;
}

RdmaQueuePairGroup::RdmaQueuePairGroup(void) {}

uint32_t RdmaQueuePairGroup::GetN(void) { return m_qps.size(); }

Ptr<RdmaQueuePair> RdmaQueuePairGroup::Get(uint32_t idx) { return m_qps[idx]; }

Ptr<RdmaQueuePair> RdmaQueuePairGroup::operator[](uint32_t idx) { return m_qps[idx]; }

void RdmaQueuePairGroup::AddQp(Ptr<RdmaQueuePair> qp) { m_qps.push_back(qp); }

#if 0
void RdmaQueuePairGroup::AddRxQp(Ptr<RdmaRxQueuePair> rxQp){
	m_rxQps.push_back(rxQp);
}
#endif

void RdmaQueuePairGroup::Clear(void) { m_qps.clear(); }

}  // namespace ns3
