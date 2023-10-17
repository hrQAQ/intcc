#ifndef RDMA_QUEUE_PAIR_H
#define RDMA_QUEUE_PAIR_H

#include <ns3/custom-header.h>
#include <ns3/data-rate.h>
#include <ns3/event-id.h>
#include <ns3/int-header.h>
#include <ns3/ipv4-address.h>
#include <ns3/object.h>
#include <ns3/packet.h>

#include <fstream>
#include <string>
#include <vector>

using namespace std;
namespace ns3 {

class RdmaQueuePair : public Object {
   public:
    Time startTime;
    Ipv4Address sip, dip;
    uint16_t sport, dport;
    uint64_t m_size;
    uint64_t snd_nxt, snd_una;  // next seq to send, the highest unacked seq
    uint16_t m_pg;
    uint16_t m_ipid;
    uint32_t m_win;       // bound of on-the-fly packets
    uint64_t m_baseRtt;   // base RTT of this qp
    DataRate m_max_rate;  // max rate
    bool m_var_win;       // variable window size
    Time m_nextAvail;     //< Soonest time of next send
    uint32_t wp;          // current window of packets
    uint32_t lastPktSize;
    Callback<void> m_notifyAppFinish;
    std::ofstream m_rateLog, m_rttLog;
    bool m_flowType;  // true for DC, false for WAN
    double m_stopTime;
    /******************************
     * runtime states
     *****************************/
    DataRate m_rate;  //< Current rate
    struct {
        DataRate m_targetRate;  //< Target rate
        EventId m_eventUpdateAlpha;
        double m_alpha;
        bool m_alpha_cnp_arrived;  // indicate if CNP arrived in the last slot
        bool m_first_cnp;          // indicate if the current CNP is the first CNP
        EventId m_eventDecreaseRate;
        bool m_decrease_cnp_arrived;  // indicate if CNP arrived in the last slot
        uint32_t m_rpTimeStage;
        EventId m_rpTimer;
    } mlx;
    // hpcc状态
    struct {
        uint32_t m_lastUpdateSeq;
        DataRate m_curRate;
        IntHop hop[IntHeader::maxHop];
        uint32_t keep[IntHeader::maxHop];
        uint32_t m_incStage;
        double m_lastGap;
        double u;
        struct {
            double u;
            DataRate Rc;
            uint32_t incStage;
        } hopState[IntHeader::maxHop];
    } hp;
    // timely状态
    struct {
        uint32_t m_lastUpdateSeq;
        DataRate m_curRate;
        uint32_t m_incStage;
        uint64_t lastRtt;
        double rttDiff;
    } tmly;
    // dctcp状态
    struct {
        uint32_t m_lastUpdateSeq;
        uint32_t m_caState;
        uint32_t m_highSeq;  // when to exit cwr
        double m_alpha;
        uint32_t m_ecnCnt;
        uint32_t m_batchSizeOfAlpha;
    } dctcp;
    // hpccPint状态
    struct {
        uint32_t m_lastUpdateSeq;
        DataRate m_curRate;
        uint32_t m_incStage;
    } hpccPint;
    // INTCC状态
    struct {
        // 用于标识DC流处于第一还是第二阶段
        bool m_flag = true;
        // 用于标识WAN流的利用率是否在目标范围内
        double epsilon = 0.01;
        // 上次更新时的包序号
        uint32_t m_lastUpdateSeq;
        // 上次调整的链路利用率
        double m_lastUtilization;
        DataRate m_lastRate;
        // 当前速率
        DataRate m_curRate;
        // 用来存储每一跳的信息
        IntHop hop[IntHeader::maxHop];
        // 加性增加的次数
        uint32_t m_incStage = 0;
        // 统计值，为了减少代码重写量
        double m_bketa;
        double m_maxU;
        double u;
        struct
        {
          double u;
          DataRate Rc;
          uint32_t incStage;
          double m_lastUtilization;
        } hopState[IntHeader::maxHop];
        double m_delta_t;
    } intcc;

    /***********
     * methods
     **********/
    static TypeId GetTypeId(void);
    RdmaQueuePair(uint16_t pg, Ipv4Address _sip, Ipv4Address _dip, uint16_t _sport, uint16_t _dport);
    void SetLogFile(string metric_mon_file_prefix);
    void SetUtarget(double utarget);
    void SetSize(uint64_t size);
    void SetWin(uint32_t win);
    void SetBaseRtt(uint64_t baseRtt);
    void SetVarWin(bool v);
    void SetAppNotifyCallback(Callback<void> notifyAppFinish);

    uint64_t GetBytesLeft();
    uint32_t GetHash(void);
    void Acknowledge(uint64_t ack);
    uint64_t GetOnTheFly();
    bool IsWinBound();
    uint64_t GetWin();  // window size calculated from m_rate
    bool IsFinished();
    uint64_t HpGetCurWin();  // window size calculated from hp.m_curRate, used by HPCC
};

class RdmaRxQueuePair : public Object {  // Rx side queue pair
   public:
    struct ECNAccount {
        uint16_t qIndex;
        uint8_t ecnbits;
        uint16_t qfb;
        uint16_t total;

        ECNAccount() { memset(this, 0, sizeof(ECNAccount)); }
    };
    ECNAccount m_ecn_source;
    uint32_t sip, dip;
    uint16_t sport, dport;
    uint16_t m_ipid;
    uint32_t ReceiverNextExpectedSeq;
    Time m_nackTimer;
    int32_t m_milestone_rx;
    uint32_t m_lastNACK;
    EventId QcnTimerEvent;  // if destroy this rxQp, remember to cancel this timer

    static TypeId GetTypeId(void);
    RdmaRxQueuePair();
    uint32_t GetHash(void);
};

class RdmaQueuePairGroup : public Object {
   public:
    std::vector<Ptr<RdmaQueuePair>> m_qps;
    // std::vector<Ptr<RdmaRxQueuePair> > m_rxQps;

    static TypeId GetTypeId(void);
    RdmaQueuePairGroup(void);
    uint32_t GetN(void);
    Ptr<RdmaQueuePair> Get(uint32_t idx);
    Ptr<RdmaQueuePair> operator[](uint32_t idx);
    void AddQp(Ptr<RdmaQueuePair> qp);
    // void AddRxQp(Ptr<RdmaRxQueuePair> rxQp);
    void Clear(void);
};

}  // namespace ns3

#endif /* RDMA_QUEUE_PAIR_H */
