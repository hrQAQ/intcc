#include "rdma-hw.h"

#include <ns3/ipv4-header.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/udp-header.h>

#include <fstream>
#include <string>

#include "cn-header.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/ppp-header.h"
#include "ns3/uinteger.h"
#include "ppp-header.h"
#include "qbb-header.h"

#define v1 1
namespace ns3 {

TypeId
RdmaHw::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::RdmaHw")
      .SetParent<Object>()
      .AddAttribute("MinRate",
                    "Minimum rate of a throttled flow",
                    DataRateValue(DataRate("100Mb/s")),
                    MakeDataRateAccessor(&RdmaHw::m_minRate),
                    MakeDataRateChecker())
      .AddAttribute("Mtu",
                    "Mtu.",
                    UintegerValue(1000),
                    MakeUintegerAccessor(&RdmaHw::m_mtu),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("CcMode",
                    "which mode of DCQCN is running",
                    UintegerValue(0),
                    MakeUintegerAccessor(&RdmaHw::m_cc_mode),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("NACK Generation Interval",
                    "The NACK Generation interval",
                    DoubleValue(500.0),
                    MakeDoubleAccessor(&RdmaHw::m_nack_interval),
                    MakeDoubleChecker<double>())
      .AddAttribute("L2ChunkSize",
                    "Layer 2 chunk size. Disable chunk mode if equals to 0.",
                    UintegerValue(0),
                    MakeUintegerAccessor(&RdmaHw::m_chunk),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("L2AckInterval",
                    "Layer 2 Ack intervals. Disable ack if equals to 0.",
                    UintegerValue(0),
                    MakeUintegerAccessor(&RdmaHw::m_ack_interval),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("L2BackToZero",
                    "Layer 2 go back to zero transmission.",
                    BooleanValue(false),
                    MakeBooleanAccessor(&RdmaHw::m_backto0),
                    MakeBooleanChecker())
      .AddAttribute("EwmaGain",
                    "Control gain parameter which determines the level "
                    "of rate decrease",
                    DoubleValue(1.0 / 16),
                    MakeDoubleAccessor(&RdmaHw::m_g),
                    MakeDoubleChecker<double>())
      .AddAttribute("RateOnFirstCnp",
                    "the fraction of rate on first CNP",
                    DoubleValue(1.0),
                    MakeDoubleAccessor(&RdmaHw::m_rateOnFirstCNP),
                    MakeDoubleChecker<double>())
      .AddAttribute("ClampTargetRate",
                    "Clamp target rate.",
                    BooleanValue(false),
                    MakeBooleanAccessor(&RdmaHw::m_EcnClampTgtRate),
                    MakeBooleanChecker())
      .AddAttribute("RPTimer",
                    "The rate increase timer at RP in microseconds",
                    DoubleValue(1500.0),
                    MakeDoubleAccessor(&RdmaHw::m_rpgTimeReset),
                    MakeDoubleChecker<double>())
      .AddAttribute("RateDecreaseInterval",
                    "The interval of rate decrease check",
                    DoubleValue(4.0),
                    MakeDoubleAccessor(&RdmaHw::m_rateDecreaseInterval),
                    MakeDoubleChecker<double>())
      .AddAttribute("FastRecoveryTimes",
                    "The rate increase timer at RP",
                    UintegerValue(5),
                    MakeUintegerAccessor(&RdmaHw::m_rpgThreshold),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("AlphaResumInterval",
                    "The interval of resuming alpha",
                    DoubleValue(55.0),
                    MakeDoubleAccessor(&RdmaHw::m_alpha_resume_interval),
                    MakeDoubleChecker<double>())
      .AddAttribute("RateAI",
                    "Rate increment unit in AI period",
                    DataRateValue(DataRate("5Mb/s")),
                    MakeDataRateAccessor(&RdmaHw::m_rai),
                    MakeDataRateChecker())
      .AddAttribute("RateHAI",
                    "Rate increment unit in hyperactive AI period",
                    DataRateValue(DataRate("50Mb/s")),
                    MakeDataRateAccessor(&RdmaHw::m_rhai),
                    MakeDataRateChecker())
      .AddAttribute("VarWin",
                    "Use variable window size or not",
                    BooleanValue(false),
                    MakeBooleanAccessor(&RdmaHw::m_var_win),
                    MakeBooleanChecker())
      .AddAttribute("FastReact",
                    "Fast React to congestion feedback",
                    BooleanValue(true),
                    MakeBooleanAccessor(&RdmaHw::m_fast_react),
                    MakeBooleanChecker())
      .AddAttribute("MiThresh",
                    "Threshold of number of consecutive AI before MI",
                    UintegerValue(5),
                    MakeUintegerAccessor(&RdmaHw::m_miThresh),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("TargetUtil",
                    "The Target Utilization of the bottleneck "
                    "bandwidth, by default 95%",
                    DoubleValue(0.95),
                    MakeDoubleAccessor(&RdmaHw::m_targetUtil),
                    MakeDoubleChecker<double>())
      .AddAttribute("UtilHigh",
                    "The upper bound of Target Utilization of the "
                    "bottleneck bandwidth, by default 98%",
                    DoubleValue(0.98),
                    MakeDoubleAccessor(&RdmaHw::m_utilHigh),
                    MakeDoubleChecker<double>())
      .AddAttribute("RateBound",
                    "Bound packet sending by rate, for test only",
                    BooleanValue(true),
                    MakeBooleanAccessor(&RdmaHw::m_rateBound),
                    MakeBooleanChecker())
      .AddAttribute("MultiRate",
                    "Maintain multiple rates in `",
                    BooleanValue(true),
                    MakeBooleanAccessor(&RdmaHw::m_multipleRate),
                    MakeBooleanChecker())
      .AddAttribute("SampleFeedback",
                    "Whether sample feedback or not",
                    BooleanValue(false),
                    MakeBooleanAccessor(&RdmaHw::m_sampleFeedback),
                    MakeBooleanChecker())
      .AddAttribute("TimelyAlpha",
                    "Alpha of TIMELY",
                    DoubleValue(0.875),
                    MakeDoubleAccessor(&RdmaHw::m_tmly_alpha),
                    MakeDoubleChecker<double>())
      .AddAttribute("TimelyBeta",
                    "Beta of TIMELY",
                    DoubleValue(0.8),
                    MakeDoubleAccessor(&RdmaHw::m_tmly_beta),
                    MakeDoubleChecker<double>())
      .AddAttribute("TimelyTLow",
                    "TLow of TIMELY (ns)",
                    UintegerValue(30000),
                    MakeUintegerAccessor(&RdmaHw::m_tmly_TLow),
                    MakeUintegerChecker<uint64_t>())
      .AddAttribute("TimelyTHigh",
                    "THigh of TIMELY (ns)",
                    UintegerValue(500000),
                    MakeUintegerAccessor(&RdmaHw::m_tmly_THigh),
                    MakeUintegerChecker<uint64_t>())
      .AddAttribute("TimelyMinRtt",
                    "MinRtt of TIMELY (ns)",
                    UintegerValue(20000),
                    MakeUintegerAccessor(&RdmaHw::m_tmly_minRtt),
                    MakeUintegerChecker<uint64_t>())
      .AddAttribute("DctcpRateAI",
                    "DCTCP's Rate increment unit in AI period",
                    DataRateValue(DataRate("1000Mb/s")),
                    MakeDataRateAccessor(&RdmaHw::m_dctcp_rai),
                    MakeDataRateChecker())
      .AddAttribute("PintSmplThresh",
                    "PINT's sampling threshold in rand()%65536",
                    UintegerValue(65536),
                    MakeUintegerAccessor(&RdmaHw::pint_smpl_thresh),
                    MakeUintegerChecker<uint32_t>());
  return tid;
}

RdmaHw::RdmaHw() {}

void
RdmaHw::SetMonFilePrefix(std::string mon_file_prefix)
{
  m_monFilePrefix = mon_file_prefix;
}
void
RdmaHw::SetNode(Ptr<Node> node)
{
  m_node = node;
}
void
RdmaHw::Setup(QpCompleteCallback cb)
{
  for (uint32_t i = 0; i < m_nic.size(); i++) {
    Ptr<QbbNetDevice> dev = m_nic[i].dev;
    if (dev == NULL)
      continue;
    // share data with NIC
    dev->m_rdmaEQ->m_qpGrp = m_nic[i].qpGrp;
    // setup callback
    dev->m_rdmaReceiveCb = MakeCallback(&RdmaHw::Receive, this);
    dev->m_rdmaLinkDownCb = MakeCallback(&RdmaHw::SetLinkDown, this);
    dev->m_rdmaPktSent = MakeCallback(&RdmaHw::PktSent, this);
    // config NIC
    dev->m_rdmaEQ->m_rdmaGetNxtPkt = MakeCallback(&RdmaHw::GetNxtPacket, this);
  }
  // setup qp complete callback
  m_qpCompleteCallback = cb;
}

uint32_t
RdmaHw::GetNicIdxOfQp(Ptr<RdmaQueuePair> qp)
{
  auto& v = m_rtTable[qp->dip.Get()];
  if (v.size() > 0) {
    return v[qp->GetHash() % v.size()];
  } else {
    NS_ASSERT_MSG(false, "We assume at least one NIC is alive");
  }
}
uint64_t
RdmaHw::GetQpKey(uint32_t dip, uint16_t sport, uint16_t pg)
{
  return ((uint64_t)dip << 32) | ((uint64_t)sport << 16) | (uint64_t)pg;
}
Ptr<RdmaQueuePair>
RdmaHw::GetQp(uint32_t dip, uint16_t sport, uint16_t pg)
{
  uint64_t key = GetQpKey(dip, sport, pg);
  auto it = m_qpMap.find(key);
  if (it != m_qpMap.end())
    return it->second;
  return NULL;
}
void
RdmaHw::AddQueuePair(uint64_t size,
                     uint16_t pg,
                     Ipv4Address sip,
                     Ipv4Address dip,
                     uint16_t sport,
                     uint16_t dport,
                     uint32_t win,
                     uint64_t baseRtt,
                     Callback<void> notifyAppFinish)
{
  // create qp
  Ptr<RdmaQueuePair> qp =
    CreateObject<RdmaQueuePair>(pg, sip, dip, sport, dport);
  qp->SetSize(size);
  qp->SetWin(win);
  qp->SetBaseRtt(baseRtt);
  qp->SetVarWin(m_var_win);
  qp->SetAppNotifyCallback(notifyAppFinish);
  qp->SetLogFile(m_monFilePrefix);
  qp->SetUtarget(m_targetUtil);
  // add qp
  uint32_t nic_idx = GetNicIdxOfQp(qp);
  m_nic[nic_idx].qpGrp->AddQp(qp);
  uint64_t key = GetQpKey(dip.Get(), sport, pg);
  m_qpMap[key] = qp;

  // set init variables
  DataRate m_bps = m_nic[nic_idx].dev->GetDataRate();
  qp->m_rate = m_bps;
  // Config 为特定流设置初始速率
  // if (((qp->sip.Get() >> 8) & 0xffff) == 0) {
  //   qp->m_rate = m_bps * 0.95;
  // }
  qp->m_max_rate = m_bps;
  if (m_cc_mode == 1) {
    qp->mlx.m_targetRate = qp->m_rate;
  } else if (m_cc_mode == 3) {
    qp->hp.m_curRate = qp->m_rate;
    if (m_multipleRate) {
      for (uint32_t i = 0; i < IntHeader::maxHop; i++)
        qp->hp.hopState[i].Rc = qp->m_rate;
    }
  } else if (m_cc_mode == 7) {
    qp->tmly.m_curRate = qp->m_rate;
  } else if (m_cc_mode == 10) {
    qp->hpccPint.m_curRate = qp->m_rate;
  } else if (m_cc_mode == 13) {
    qp->intcc.m_curRate = qp->m_rate;
    if (m_multipleRate) {
      for (uint32_t i = 0; i < IntHeader::maxHop; i++)
        qp->intcc.hopState[i].Rc = qp->m_rate;
    }
  }

  // Notify Nic
  m_nic[nic_idx].dev->NewQp(qp);
}

void
RdmaHw::DeleteQueuePair(Ptr<RdmaQueuePair> qp)
{
  // remove qp from the m_qpMap
  uint64_t key = GetQpKey(qp->dip.Get(), qp->sport, qp->m_pg);
  m_qpMap.erase(key);
}

Ptr<RdmaRxQueuePair>
RdmaHw::GetRxQp(uint32_t sip,
                uint32_t dip,
                uint16_t sport,
                uint16_t dport,
                uint16_t pg,
                bool create)
{
  uint64_t key = ((uint64_t)dip << 32) | ((uint64_t)pg << 16) | (uint64_t)dport;
  auto it = m_rxQpMap.find(key);
  if (it != m_rxQpMap.end())
    return it->second;
  if (create) {
    // create new rx qp
    Ptr<RdmaRxQueuePair> q = CreateObject<RdmaRxQueuePair>();
    // init the qp
    q->sip = sip;
    q->dip = dip;
    q->sport = sport;
    q->dport = dport;
    q->m_ecn_source.qIndex = pg;
    // store in map
    m_rxQpMap[key] = q;
    return q;
  }
  return NULL;
}
uint32_t
RdmaHw::GetNicIdxOfRxQp(Ptr<RdmaRxQueuePair> q)
{
  auto& v = m_rtTable[q->dip];
  if (v.size() > 0) {
    return v[q->GetHash() % v.size()];
  } else {
    NS_ASSERT_MSG(false, "We assume at least one NIC is alive");
  }
}
void
RdmaHw::DeleteRxQp(uint32_t dip, uint16_t pg, uint16_t dport)
{
  uint64_t key = ((uint64_t)dip << 32) | ((uint64_t)pg << 16) | (uint64_t)dport;
  m_rxQpMap.erase(key);
}

int
RdmaHw::ReceiveUdp(Ptr<Packet> p, CustomHeader& ch)
{
  uint8_t ecnbits = ch.GetIpv4EcnBits();

  uint32_t payload_size = p->GetSize() - ch.GetSerializedSize();

  // TODO find corresponding rx queue pair
  Ptr<RdmaRxQueuePair> rxQp =
    GetRxQp(ch.dip, ch.sip, ch.udp.dport, ch.udp.sport, ch.udp.pg, true);
  if (ecnbits != 0) {
    rxQp->m_ecn_source.ecnbits |= ecnbits;
    rxQp->m_ecn_source.qfb++;
  }
  rxQp->m_ecn_source.total++;
  rxQp->m_milestone_rx = m_ack_interval;

  int x = ReceiverCheckSeq(ch.udp.seq, rxQp, payload_size);
  if (x == 1 || x == 2) { // generate ACK or NACK
    qbbHeader seqh;
    seqh.SetSeq(rxQp->ReceiverNextExpectedSeq);
    seqh.SetPG(ch.udp.pg);
    seqh.SetSport(ch.udp.dport);
    seqh.SetDport(ch.udp.sport);
    seqh.SetIntHeader(ch.udp.ih);
    if (ecnbits)
      seqh.SetCnp();

    Ptr<Packet> newp =
      Create<Packet>(std::max(60 - 14 - 20 - (int)seqh.GetSerializedSize(), 0));
    newp->AddHeader(seqh);

    Ipv4Header head; // Prepare IPv4 header
    head.SetDestination(Ipv4Address(ch.sip));
    head.SetSource(Ipv4Address(ch.dip));
    head.SetProtocol(x == 1 ? 0xFC : 0xFD); // ack=0xFC nack=0xFD
    head.SetTtl(64);
    head.SetPayloadSize(newp->GetSize());
    head.SetIdentification(rxQp->m_ipid++);

    newp->AddHeader(head);
    AddHeader(newp, 0x800); // Attach PPP header
    // send
    uint32_t nic_idx = GetNicIdxOfRxQp(rxQp);
    m_nic[nic_idx].dev->RdmaEnqueueHighPrioQ(newp);
    m_nic[nic_idx].dev->TriggerTransmit();
  }
  return 0;
}

int
RdmaHw::ReceiveCnp(Ptr<Packet> p, CustomHeader& ch)
{
  // QCN on NIC
  // This is a Congestion signal
  // Then, extract data from the congestion packet.
  // We assume, without verify, the packet is destinated to me
  uint32_t qIndex = ch.cnp.qIndex;
  if (qIndex == 1) { // DCTCP
    std::cout << "TCP--ignore\n";
    return 0;
  }
  uint16_t udpport = ch.cnp.fid; // corresponds to the sport
  uint8_t ecnbits = ch.cnp.ecnBits;
  uint16_t qfb = ch.cnp.qfb;
  uint16_t total = ch.cnp.total;

  uint32_t i;
  // get qp
  Ptr<RdmaQueuePair> qp = GetQp(ch.sip, udpport, qIndex);
  if (qp == NULL)
    std::cout << "ERROR: QCN NIC cannot find the flow\n";
  // get nic
  uint32_t nic_idx = GetNicIdxOfQp(qp);
  Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;

  if (qp->m_rate == 0) // lazy initialization
  {
    qp->m_rate = dev->GetDataRate();
    if (m_cc_mode == 1) {
      qp->mlx.m_targetRate = dev->GetDataRate();
    } else if (m_cc_mode == 3) {
      qp->hp.m_curRate = dev->GetDataRate();
      if (m_multipleRate) {
        for (uint32_t i = 0; i < IntHeader::maxHop; i++)
          qp->hp.hopState[i].Rc = dev->GetDataRate();
      }
    } else if (m_cc_mode == 7) {
      qp->tmly.m_curRate = dev->GetDataRate();
    } else if (m_cc_mode == 10) {
      qp->hpccPint.m_curRate = dev->GetDataRate();
    } else if (m_cc_mode == 13) {
      qp->intcc.m_curRate = dev->GetDataRate();
    }
  }
  return 0;
}

int
RdmaHw::ReceiveAck(Ptr<Packet> p, CustomHeader& ch)
{
  uint16_t qIndex = ch.ack.pg;
  uint16_t port = ch.ack.dport;
  uint32_t seq = ch.ack.seq;
  uint8_t cnp = (ch.ack.flags >> qbbHeader::FLAG_CNP) & 1;
  int i;
  Ptr<RdmaQueuePair> qp = GetQp(ch.sip, port, qIndex);
  if (qp == NULL) {
    std::cout << "ERROR: " << "Time: " << Simulator::Now().GetSeconds() << " "
              << "node:" << m_node->GetId() << ' '
              << (ch.l3Prot == 0xFC ? "ACK" : "NACK")
              << " NIC cannot find the flow\n";
    return 0;
  }

  if (measure_th && Simulator::Now().GetSeconds() > 12) {
    measure_th = false;
    printf("Throuput init: %u %u %u %u %lu %lu\n",
           (qp->sip.Get() >> 8) & 0xffff,
           (qp->dip.Get() >> 8) & 0xffff,
           qp->sport,
           qp->dport,
           qp->snd_una,
           qp->snd_nxt);
  }

  uint32_t nic_idx = GetNicIdxOfQp(qp);
  Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
  if (m_ack_interval == 0)
    std::cout << "ERROR: shouldn't receive ack\n";
  else {
    if (!m_backto0) {
      qp->Acknowledge(seq);
    } else {
      uint32_t goback_seq = seq / m_chunk * m_chunk;
      qp->Acknowledge(goback_seq);
    }
    if (qp->IsFinished()) {
      QpComplete(qp);
    }
    // Config 实验结束，进行吞吐量统计
    if (20 < Simulator::Now().GetSeconds()) {
      QpComplete(qp);
    }
    // Config 自定义流退出时间
    if (0) {
      if (((qp->sip.Get() >> 8) & 0xffff) == 0) {
        if (7 < Simulator::Now().GetSeconds()) {
          printf("Src: %u Throuput: %lu\n",
                 (qp->sip.Get() >> 8) & 0xffff,
                 qp->snd_nxt);
          QpComplete(qp);
        }
      }
      if (((qp->sip.Get() >> 8) & 0xffff) == 1) {
        if (3 < Simulator::Now().GetSeconds()) {
          printf("Src: %u Throuput: %lu\n",
                 (qp->sip.Get() >> 8) & 0xffff,
                 qp->snd_nxt);
          QpComplete(qp);
        }
      }
      if (((qp->sip.Get() >> 8) & 0xffff) == 2) {
        if (5 < Simulator::Now().GetSeconds()) {
          printf("Src: %u Throuput: %lu\n",
                 (qp->sip.Get() >> 8) & 0xffff,
                 qp->snd_nxt);
          QpComplete(qp);
        }
      }
      if (((qp->sip.Get() >> 8) & 0xffff) == 3) {
        if (4 < Simulator::Now().GetSeconds()) {
          printf("Src: %u Throuput: %lu\n",
                 (qp->sip.Get() >> 8) & 0xffff,
                 qp->snd_nxt);
          QpComplete(qp);
        }
      }
    }
  }
  if (ch.l3Prot == 0xFD) // NACK
    RecoverQueue(qp);

  if (m_cc_mode == 1) {
    uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.ts;
    qp->m_rttLog << Simulator::Now().GetTimeStep() << " " << rtt << std::endl;
    qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                  << qp->m_rate.GetBitRate() << std::endl;
  }
  // handle cnp
  if (cnp) {
    if (m_cc_mode == 1) { // mlx version
      cnp_received_mlx(qp);
    }
  }

  if (m_cc_mode == 3) {
    uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.hpcc.ts;
    qp->m_rttLog << Simulator::Now().GetTimeStep() << " " << rtt << std::endl;
    IsDCFlow(qp, ch);
    HandleAckHp(qp, p, ch);
  } else if (m_cc_mode == 7) {
    uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.ts;
    qp->m_rttLog << Simulator::Now().GetTimeStep() << " " << rtt << std::endl;
    HandleAckTimely(qp, p, ch);
  } else if (m_cc_mode == 8) {
    uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.ts;
    qp->m_rttLog << Simulator::Now().GetTimeStep() << " " << rtt << std::endl;
    HandleAckDctcp(qp, p, ch);
  } else if (m_cc_mode == 10) {
    HandleAckHpPint(qp, p, ch);
  } else if (m_cc_mode == 13) {
    uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.gear.ts;
    qp->m_rttLog << Simulator::Now().GetTimeStep() << " " << rtt << std::endl;
    HandleAckINTCC(qp, p, ch);
  }
  // ACK may advance the on-the-fly window, allowing more packets to send
  dev->TriggerTransmit();
  return 0;
}

int
RdmaHw::Receive(Ptr<Packet> p, CustomHeader& ch)
{
  if (ch.l3Prot == 0x11) { // UDP
    ReceiveUdp(p, ch);
  } else if (ch.l3Prot == 0xFF) { // CNP cnp是一种控制包，用于通知发送端降低速率
    ReceiveCnp(p, ch);
  } else if (ch.l3Prot == 0xFD) { // NACK 用于通知发送端重传
    ReceiveAck(p, ch);
  } else if (ch.l3Prot == 0xFC) { // ACK 用于通知发送端接收成功
    ReceiveAck(p, ch);
  }
  return 0;
}

int
RdmaHw::ReceiverCheckSeq(uint32_t seq, Ptr<RdmaRxQueuePair> q, uint32_t size)
{
  uint32_t expected = q->ReceiverNextExpectedSeq;
  if (seq == expected) {
    q->ReceiverNextExpectedSeq = expected + size;
    if (q->ReceiverNextExpectedSeq >= q->m_milestone_rx) {
      q->m_milestone_rx += m_ack_interval;
      return 1; // Generate ACK
    } else if (q->ReceiverNextExpectedSeq % m_chunk == 0) {
      return 1;
    } else {
      return 5;
    }
  } else if (seq > expected) {
    // Generate NACK
    if (Simulator::Now() >= q->m_nackTimer || q->m_lastNACK != expected) {
      q->m_nackTimer = Simulator::Now() + MicroSeconds(m_nack_interval);
      q->m_lastNACK = expected;
      if (m_backto0) {
        q->ReceiverNextExpectedSeq =
          q->ReceiverNextExpectedSeq / m_chunk * m_chunk;
      }
      return 2;
    } else
      return 4;
  } else {
    // Duplicate.
    return 3;
  }
}
void
RdmaHw::AddHeader(Ptr<Packet> p, uint16_t protocolNumber)
{
  PppHeader ppp;
  ppp.SetProtocol(EtherToPpp(protocolNumber));
  p->AddHeader(ppp);
}
uint16_t
RdmaHw::EtherToPpp(uint16_t proto)
{
  switch (proto) {
    case 0x0800:
      return 0x0021; // IPv4
    case 0x86DD:
      return 0x0057; // IPv6
    default:
      NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
  }
  return 0;
}

void
RdmaHw::RecoverQueue(Ptr<RdmaQueuePair> qp)
{
  qp->snd_nxt = qp->snd_una;
}

void
RdmaHw::QpComplete(Ptr<RdmaQueuePair> qp)
{
  NS_ASSERT(!m_qpCompleteCallback.IsNull());
  printf("Throuput end: %u %u %u %u %lu %lu\n",
         (qp->sip.Get() >> 8) & 0xffff,
         (qp->dip.Get() >> 8) & 0xffff,
         qp->sport,
         qp->dport,
         qp->snd_una,
         qp->snd_nxt);
  if (m_cc_mode == 1) {
    Simulator::Cancel(qp->mlx.m_eventUpdateAlpha);
    Simulator::Cancel(qp->mlx.m_eventDecreaseRate);
    Simulator::Cancel(qp->mlx.m_rpTimer);
  }

  // This callback will log info
  // It may also delete the rxQp on the receiver
  m_qpCompleteCallback(qp);

  qp->m_notifyAppFinish();

  // delete the qp
  DeleteQueuePair(qp);
}

void
RdmaHw::SetLinkDown(Ptr<QbbNetDevice> dev)
{
  printf("RdmaHw: node:%u a link down\n", m_node->GetId());
}

void
RdmaHw::AddTableEntry(Ipv4Address& dstAddr, uint32_t intf_idx)
{
  uint32_t dip = dstAddr.Get();
  m_rtTable[dip].push_back(intf_idx);
}

void
RdmaHw::ClearTable()
{
  m_rtTable.clear();
}

void
RdmaHw::RedistributeQp()
{
  // clear old qpGrp
  for (uint32_t i = 0; i < m_nic.size(); i++) {
    if (m_nic[i].dev == NULL)
      continue;
    m_nic[i].qpGrp->Clear();
  }

  // redistribute qp
  for (auto& it : m_qpMap) {
    Ptr<RdmaQueuePair> qp = it.second;
    uint32_t nic_idx = GetNicIdxOfQp(qp);
    m_nic[nic_idx].qpGrp->AddQp(qp);
    // Notify Nic
    m_nic[nic_idx].dev->ReassignedQp(qp);
  }
}

Ptr<Packet>
RdmaHw::GetNxtPacket(Ptr<RdmaQueuePair> qp)
{
  uint64_t payload_size = qp->GetBytesLeft();
  // printf("GetNxtPacket: %u %u %u %Lu %u\n",
  //        (qp->sip.Get() >> 8) & 0xffff,
  //        (qp->dip.Get() >> 8) & 0xffff,
  //        qp->snd_nxt,
  //        payload_size,
  //        m_mtu);
  if (m_mtu < payload_size)
    payload_size = m_mtu;
  Ptr<Packet> p = Create<Packet>(payload_size);
  // add SeqTsHeader
  SeqTsHeader seqTs;
  seqTs.SetSeq(qp->snd_nxt);
  seqTs.SetPG(qp->m_pg);
  p->AddHeader(seqTs);
  // add udp header
  UdpHeader udpHeader;
  udpHeader.SetDestinationPort(qp->dport);
  udpHeader.SetSourcePort(qp->sport);
  p->AddHeader(udpHeader);
  // add ipv4 header
  Ipv4Header ipHeader;
  ipHeader.SetSource(qp->sip);
  ipHeader.SetDestination(qp->dip);
  ipHeader.SetProtocol(0x11);
  ipHeader.SetPayloadSize(p->GetSize());
  ipHeader.SetTtl(64);
  ipHeader.SetTos(0);
  ipHeader.SetIdentification(qp->m_ipid);
  p->AddHeader(ipHeader);
  // add ppp header
  PppHeader ppp;
  ppp.SetProtocol(
    0x0021); // EtherToPpp(0x800), see point-to-point-net-device.cc
  p->AddHeader(ppp);

  // update state
  qp->snd_nxt += payload_size;
  qp->m_ipid++;

  // return
  return p;
}

void
RdmaHw::PktSent(Ptr<RdmaQueuePair> qp, Ptr<Packet> pkt, Time interframeGap)
{
  qp->lastPktSize = pkt->GetSize();
  UpdateNextAvail(qp, interframeGap, pkt->GetSize());
}

void
RdmaHw::UpdateNextAvail(Ptr<RdmaQueuePair> qp,
                        Time interframeGap,
                        uint32_t pkt_size)
{
  Time sendingTime;
  if (m_rateBound)
    sendingTime = interframeGap + Seconds(qp->m_rate.CalculateTxTime(pkt_size));
  else
    sendingTime =
      interframeGap + Seconds(qp->m_max_rate.CalculateTxTime(pkt_size));
  qp->m_nextAvail = Simulator::Now() + sendingTime;
}

void
RdmaHw::ChangeRate(Ptr<RdmaQueuePair> qp, DataRate new_rate)
{
#if 1
  Time sendingTime = Seconds(qp->m_rate.CalculateTxTime(qp->lastPktSize));
  Time new_sendintTime = Seconds(new_rate.CalculateTxTime(qp->lastPktSize));
  qp->m_nextAvail = qp->m_nextAvail + new_sendintTime - sendingTime;
  // update nic's next avail event
  uint32_t nic_idx = GetNicIdxOfQp(qp);
  m_nic[nic_idx].dev->UpdateNextAvail(qp->m_nextAvail);
#endif

  // change to new rate
  qp->m_rate = new_rate;
}

#define PRINT_LOG 1
/******************************
 * Mellanox's version of DCQCN
 *****************************/
void
RdmaHw::UpdateAlphaMlx(Ptr<RdmaQueuePair> q)
{
#if PRINT_LOG
  std::cout << Simulator::Now() << " alpha update:" << m_node->GetId() << ' '
            << q->mlx.m_alpha << ' ' << (int)q->mlx.m_alpha_cnp_arrived << '\n';
  printf("%lu alpha update: %08x %08x %u %u %.6lf->",
         Simulator::Now().GetTimeStep(),
         q->sip.Get(),
         q->dip.Get(),
         q->sport,
         q->dport,
         q->mlx.m_alpha);
#endif
  if (q->mlx.m_alpha_cnp_arrived) {
    q->mlx.m_alpha = (1 - m_g) * q->mlx.m_alpha + m_g; // binary feedback
  } else {
    q->mlx.m_alpha = (1 - m_g) * q->mlx.m_alpha; // binary feedback
  }
#if PRINT_LOG
  // printf("%.6lf\n", q->mlx.m_alpha);
#endif
  q->mlx.m_alpha_cnp_arrived = false; // clear the CNP_arrived bit
  ScheduleUpdateAlphaMlx(q);
}
void
RdmaHw::ScheduleUpdateAlphaMlx(Ptr<RdmaQueuePair> q)
{
  q->mlx.m_eventUpdateAlpha = Simulator::Schedule(
    MicroSeconds(m_alpha_resume_interval), &RdmaHw::UpdateAlphaMlx, this, q);
}

void
RdmaHw::cnp_received_mlx(Ptr<RdmaQueuePair> q)
{
  q->mlx.m_alpha_cnp_arrived = true;    // set CNP_arrived bit for alpha update
  q->mlx.m_decrease_cnp_arrived = true; // set CNP_arrived bit for rate decrease
  if (q->mlx.m_first_cnp) {
    // init alpha
    q->mlx.m_alpha = 1;
    q->mlx.m_alpha_cnp_arrived = false;
    // schedule alpha update
    ScheduleUpdateAlphaMlx(q);
    // schedule rate decrease
    ScheduleDecreaseRateMlx(q, 1); // add 1 ns to make sure rate
                                   // decrease is after alpha update
    // set rate on first CNP
    q->mlx.m_targetRate = q->m_rate = m_rateOnFirstCNP * q->m_rate;
    q->mlx.m_first_cnp = false;
  }
}

void
RdmaHw::CheckRateDecreaseMlx(Ptr<RdmaQueuePair> q)
{
  ScheduleDecreaseRateMlx(q, 0);
  if (q->mlx.m_decrease_cnp_arrived) {
#if PRINT_LOG
    printf("%lu rate dec: %08x %08x %u %u (%0.3lf %.3lf)->",
           Simulator::Now().GetTimeStep(),
           q->sip.Get(),
           q->dip.Get(),
           q->sport,
           q->dport,
           q->mlx.m_targetRate.GetBitRate() * 1e-9,
           q->m_rate.GetBitRate() * 1e-9);
#endif
    bool clamp = true;
    if (!m_EcnClampTgtRate) {
      if (q->mlx.m_rpTimeStage == 0)
        clamp = false;
    }
    if (clamp)
      q->mlx.m_targetRate = q->m_rate;
    q->m_rate = std::max(m_minRate, q->m_rate * (1 - q->mlx.m_alpha / 2));
    // reset rate increase related things
    q->mlx.m_rpTimeStage = 0;
    q->mlx.m_decrease_cnp_arrived = false;
    Simulator::Cancel(q->mlx.m_rpTimer);
    q->mlx.m_rpTimer = Simulator::Schedule(
      MicroSeconds(m_rpgTimeReset), &RdmaHw::RateIncEventTimerMlx, this, q);
#if PRINT_LOG
    printf("(%.3lf %.3lf)\n",
           q->mlx.m_targetRate.GetBitRate() * 1e-9,
           q->m_rate.GetBitRate() * 1e-9);
#endif
  }
}
void
RdmaHw::ScheduleDecreaseRateMlx(Ptr<RdmaQueuePair> q, uint32_t delta)
{
  q->mlx.m_eventDecreaseRate = Simulator::Schedule(
    MicroSeconds(m_rateDecreaseInterval) + NanoSeconds(delta),
    &RdmaHw::CheckRateDecreaseMlx,
    this,
    q);
}

void
RdmaHw::RateIncEventTimerMlx(Ptr<RdmaQueuePair> q)
{
  q->mlx.m_rpTimer = Simulator::Schedule(
    MicroSeconds(m_rpgTimeReset), &RdmaHw::RateIncEventTimerMlx, this, q);
  RateIncEventMlx(q);
  q->mlx.m_rpTimeStage++;
}
void
RdmaHw::RateIncEventMlx(Ptr<RdmaQueuePair> q)
{
  // check which increase phase: fast recovery, active increase, hyper
  // increase
  if (q->mlx.m_rpTimeStage < m_rpgThreshold) { // fast recovery
    FastRecoveryMlx(q);
  } else if (q->mlx.m_rpTimeStage == m_rpgThreshold) { // active increase
    ActiveIncreaseMlx(q);
  } else { // hyper increase
    HyperIncreaseMlx(q);
  }
}

void
RdmaHw::FastRecoveryMlx(Ptr<RdmaQueuePair> q)
{
#if PRINT_LOG
  printf("%lu fast recovery: %08x %08x %u %u (%0.3lf %.3lf)->",
         Simulator::Now().GetTimeStep(),
         q->sip.Get(),
         q->dip.Get(),
         q->sport,
         q->dport,
         q->mlx.m_targetRate.GetBitRate() * 1e-9,
         q->m_rate.GetBitRate() * 1e-9);
#endif
  q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
  printf("(%.3lf %.3lf)\n",
         q->mlx.m_targetRate.GetBitRate() * 1e-9,
         q->m_rate.GetBitRate() * 1e-9);
#endif
}
void
RdmaHw::ActiveIncreaseMlx(Ptr<RdmaQueuePair> q)
{
#if PRINT_LOG
  printf("%lu active inc: %08x %08x %u %u (%0.3lf %.3lf)->",
         Simulator::Now().GetTimeStep(),
         q->sip.Get(),
         q->dip.Get(),
         q->sport,
         q->dport,
         q->mlx.m_targetRate.GetBitRate() * 1e-9,
         q->m_rate.GetBitRate() * 1e-9);
#endif
  // get NIC
  uint32_t nic_idx = GetNicIdxOfQp(q);
  Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
  // increate rate
  q->mlx.m_targetRate += m_rai;
  if (q->mlx.m_targetRate > dev->GetDataRate())
    q->mlx.m_targetRate = dev->GetDataRate();
  q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
  printf("(%.3lf %.3lf)\n",
         q->mlx.m_targetRate.GetBitRate() * 1e-9,
         q->m_rate.GetBitRate() * 1e-9);
#endif
}
void
RdmaHw::HyperIncreaseMlx(Ptr<RdmaQueuePair> q)
{
#if PRINT_LOG
  printf("%lu hyper inc: %08x %08x %u %u (%0.3lf %.3lf)->",
         Simulator::Now().GetTimeStep(),
         q->sip.Get(),
         q->dip.Get(),
         q->sport,
         q->dport,
         q->mlx.m_targetRate.GetBitRate() * 1e-9,
         q->m_rate.GetBitRate() * 1e-9);
#endif
  // get NIC
  uint32_t nic_idx = GetNicIdxOfQp(q);
  Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
  // increate rate
  q->mlx.m_targetRate += m_rhai;
  if (q->mlx.m_targetRate > dev->GetDataRate())
    q->mlx.m_targetRate = dev->GetDataRate();
  q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
  printf("(%.3lf %.3lf)\n",
         q->mlx.m_targetRate.GetBitRate() * 1e-9,
         q->m_rate.GetBitRate() * 1e-9);
#endif
}

/***********************
 * High Precision CC
 ***********************/
void
RdmaHw::HandleAckHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  uint32_t ack_seq = ch.ack.seq;
  // update rate
  if (ack_seq >
      qp->hp.m_lastUpdateSeq) { // if full RTT feedback is ready, do full update
    UpdateRateHp(qp, p, ch, false);
  } else { // do fast react
    FastReactHp(qp, p, ch);
  }
}

void
RdmaHw::UpdateRateHp(Ptr<RdmaQueuePair> qp,
                     Ptr<Packet> p,
                     CustomHeader& ch,
                     bool fast_react)
{
  uint32_t next_seq = qp->snd_nxt;
  IntHeader& ih = ch.ack.ih;
  if (qp->hp.m_lastUpdateSeq == 0) { // first RTT
    qp->hp.m_lastUpdateSeq = next_seq;
    // store INT
    NS_ASSERT(ih.hpcc.nhop <= IntHeader::maxHop);
    for (uint32_t i = 0; i < ih.hpcc.nhop; i++)
      qp->hp.hop[i] = ih.hpcc.hop[i];
#if PRINT_LOG
    printf("%lu %s %d %d %u %u [%u,%u,%u] baseRTT:%lu\n",
           Simulator::Now().GetTimeStep(),
           fast_react ? "fast" : "update",
           (qp->sip.Get() >> 8) & 0xffff,
           (qp->dip.Get() >> 8) & 0xffff,
           qp->sport,
           qp->dport,
           qp->hp.m_lastUpdateSeq,
           ch.ack.seq,
           next_seq,
           qp->m_baseRtt);
    for (uint32_t i = 0; i < ih.hpcc.nhop; i++)
      printf(" %u %lu %lu",
             ih.hpcc.hop[i].GetQlen(),
             ih.hpcc.hop[i].GetBytes(),
             ih.hpcc.hop[i].GetTime());
    printf("\n");
#endif
  } else {
    // check packet INT
    if (ih.hpcc.nhop <= IntHeader::maxHop) {
      double max_c = 0;
#if PRINT_LOG
      printf("%lu %s %d %d %u %u [%u,%u,%u]",
             Simulator::Now().GetTimeStep(),
             fast_react ? "fast" : "update",
             (qp->sip.Get() >> 8) & 0xffff,
             (qp->dip.Get() >> 8) & 0xffff,
             qp->sport,
             qp->dport,
             qp->hp.m_lastUpdateSeq,
             ch.ack.seq,
             next_seq);
      printf(" rtt=%lu ", Simulator::Now().GetTimeStep() - ch.ack.ih.hpcc.ts);
#endif
      double U = 0;
      uint64_t dt = 0;
      bool updated[IntHeader::maxHop] = { false }, updated_any = false;
      NS_ASSERT(ih.hpcc.nhop <= IntHeader::maxHop);
      for (uint32_t i = 0; i < ih.hpcc.nhop; i++) {
        if (m_sampleFeedback) {
          if (ih.hpcc.hop[i].GetQlen() == 0 && fast_react)
            continue;
        }
        updated[i] = updated_any = true;
#if PRINT_LOG
        printf(" %u(%u) %lu(%lu) %lu(%lu)",
               ih.hpcc.hop[i].GetQlen(),
               qp->hp.hop[i].GetQlen(),
               ih.hpcc.hop[i].GetBytes(),
               qp->hp.hop[i].GetBytes(),
               ih.hpcc.hop[i].GetTime(),
               qp->hp.hop[i].GetTime());
#endif
        uint64_t tau = ih.hpcc.hop[i].GetTimeDelta(qp->hp.hop[i]);
        double duration = tau * 1e-9;
        double txRate =
          (ih.hpcc.hop[i].GetBytesDelta(qp->hp.hop[i])) * 8 / duration;
        double u =
          txRate / ih.hpcc.hop[i].GetLineRate() +
          (double)std::min(ih.hpcc.hop[i].GetQlen(), qp->hp.hop[i].GetQlen()) *
            qp->m_max_rate.GetBitRate() / ih.hpcc.hop[i].GetLineRate() /
            qp->m_win;
#if PRINT_LOG
        printf(" %.3lf %.3lf", txRate * 1e-9, u);
#endif
        if (!m_multipleRate) {
          // for aggregate (single R)
          if (u > U) {
            U = u;
            dt = tau;
          }
        } else {
          // for per hop (per hop R)
          if (tau > qp->m_baseRtt)
            tau = qp->m_baseRtt;
          qp->hp.hopState[i].u =
            (qp->hp.hopState[i].u * (qp->m_baseRtt - tau) + u * tau) /
            double(qp->m_baseRtt);
        }
        qp->hp.hop[i] = ih.hpcc.hop[i];
      }

      DataRate new_rate;
      int32_t new_incStage;
      DataRate new_rate_per_hop[IntHeader::maxHop];
      int32_t new_incStage_per_hop[IntHeader::maxHop];
      if (!m_multipleRate) {
        // for aggregate (single R)
        if (updated_any) {
          if (dt > qp->m_baseRtt)
            dt = qp->m_baseRtt;
          // qp->hp.u = U; // TEST
          qp->hp.u =
            (qp->hp.u * (qp->m_baseRtt - dt) + U * dt) / double(qp->m_baseRtt);
          max_c = qp->hp.u / m_targetUtil;

          if (max_c >= 1 || qp->hp.m_incStage >= m_miThresh) {
            new_rate = qp->hp.m_curRate / max_c + m_rai;
            new_incStage = 0;
          } else {
            new_rate = qp->hp.m_curRate + m_rai;
            new_incStage = qp->hp.m_incStage + 1;
          }
          if (new_rate < m_minRate)
            new_rate = m_minRate;
          if (new_rate > qp->m_max_rate)
            new_rate = qp->m_max_rate;
#if PRINT_LOG
          printf(" u=%.6lf U=%.3lf dt=%lu "
                 "max_c=%.3lf",
                 qp->hp.u,
                 U,
                 dt,
                 max_c);
#endif
#if PRINT_LOG
          printf(" rate: (%.3lf) %.3lf -> %.3lf\n",
                 qp->hp.m_curRate.GetBitRate() * 1e-9,
                 qp->m_rate.GetBitRate() * 1e-9,
                 new_rate.GetBitRate() * 1e-9);
#endif
        }
      } else {
        // for per hop (per hop R)
        new_rate = qp->m_max_rate;
        for (uint32_t i = 0; i < ih.hpcc.nhop; i++) {
          if (updated[i]) {
            double c = qp->hp.hopState[i].u / m_targetUtil;
            if (c >= 1 || qp->hp.hopState[i].incStage >= m_miThresh) {
              new_rate_per_hop[i] = qp->hp.hopState[i].Rc / c + m_rai;
              new_incStage_per_hop[i] = 0;
            } else {
              new_rate_per_hop[i] = qp->hp.hopState[i].Rc + m_rai;
              new_incStage_per_hop[i] = qp->hp.hopState[i].incStage + 1;
            }
            // bound rate
            if (new_rate_per_hop[i] < m_minRate)
              new_rate_per_hop[i] = m_minRate;
            if (new_rate_per_hop[i] > qp->m_max_rate)
              new_rate_per_hop[i] = qp->m_max_rate;
            // find min new_rate
            if (new_rate_per_hop[i] < new_rate)
              new_rate = new_rate_per_hop[i];
#if PRINT_LOG
            printf(" [%u]u=%.6lf c=%.3lf", i, qp->hp.hopState[i].u, c);
            printf(" rate: (%.3lf) %.3lf -> %.3lf",
                   qp->hp.hopState[i].Rc.GetBitRate() * 1e-9,
                   qp->m_rate.GetBitRate() * 1e-9,
                   new_rate.GetBitRate() * 1e-9);
#endif
          } else {
            if (qp->hp.hopState[i].Rc < new_rate)
              new_rate = qp->hp.hopState[i].Rc;
          }
        }
#if PRINT_LOG
        printf("\n");
#endif
      }
      if (updated_any) {
        ChangeRate(qp, new_rate);
        qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                      << new_rate.GetBitRate() << std::endl;
      }
      if (!fast_react) {
        if (updated_any) {
          qp->hp.m_curRate = new_rate;
          qp->hp.m_incStage = new_incStage;
        }
        if (m_multipleRate) {
          // for per hop (per hop R)
          for (uint32_t i = 0; i < ih.hpcc.nhop; i++) {
            if (updated[i]) {
              qp->hp.hopState[i].Rc = new_rate_per_hop[i];
              qp->hp.hopState[i].incStage = new_incStage_per_hop[i];
            }
          }
        }
      }
    }
    if (!fast_react) {
      if (next_seq > qp->hp.m_lastUpdateSeq)
        qp->hp.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
    }
  }
}

void
RdmaHw::FastReactHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  if (m_fast_react)
    UpdateRateHp(qp, p, ch, true);
}

/***********************
 * INT CC
 ************************/
bool
RdmaHw::IsDCFlow(Ptr<RdmaQueuePair> qp, CustomHeader& ch)
{
  // Config
  int dip = (qp->dip.Get() >> 8) & 0xffff;
  if (dip >= 15 && dip <= 19) {
    qp->m_flowType = false;
    return false;
  } else if (dip >= 10 && dip <= 14) {
    qp->m_flowType = true;
    return true;
  }
}
void
RdmaHw::HandleAckINTCC(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  uint32_t seq = ch.ack.seq;
  bool fast_react = true;
  if (seq > qp->intcc.m_lastUpdateSeq) {
    fast_react = false;
  }
  if (IsDCFlow(qp, ch)) {
    UpdateRateIntra(qp, p, ch, fast_react);
  } else {
    UpdateRateCross(qp, p, ch, fast_react);
    // TODO: W = min(W, W_wan)
  }
}

void
RdmaHw::ComptUtilization(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  double U = 0;
  double delta_t = 0.0;
  // first RTT
  if (qp->intcc.m_lastUpdateSeq == 0) {
    qp->intcc.m_lastUpdateSeq = qp->snd_nxt;
    IntHeader& ih = ch.ack.ih;
    for (uint32_t i = 0; i < ih.gear.nhop; i++) {
      // header中的int数据存放到hop中, hop[maxHop], maxHop = 5,
      // 在intHeader中
      qp->intcc.hop[i] = ih.gear.hop[i];
    }
  } else {
    IntHeader& ih = ch.ack.ih;
    // check最大链路利用率
    for (uint32_t i = 0; i < ih.gear.nhop; i++) {
      // 计算utilization
      uint64_t tau = ih.gear.hop[i].GetTimeDelta(qp->intcc.hop[i]);
      double duration = tau * 1e-9;
      double txRate =
        (ih.gear.hop[i].GetBytesDelta(qp->intcc.hop[i])) * 8 / duration;
      /**
       * 1. 取了当前队列长度和记录中队列长度中较小的那个
       * 2. (qlen/B*T)用(qlen/最大inflghts*速率比例)替代
       */
      double u =
        txRate / ih.gear.hop[i].GetLineRate() +
        (double)std::min(ih.gear.hop[i].GetQlen(), qp->intcc.hop[i].GetQlen()) *
          qp->m_max_rate.GetBitRate() / ih.gear.hop[i].GetLineRate() /
          qp->m_win;
#if PRINT_LOG
      // printf ih.hop[i].GetLineRate()
      printf(" %ld", ih.gear.hop[i].GetLineRate());
      printf(" %.3lf %.3lf", txRate * 1e-9, u);
#endif
      // 取最大的链路利用率
      if (u > U) {
        U = u;
        delta_t = tau;
      }
      qp->intcc.hop[i] = ih.gear.hop[i];
    }
  }
  qp->intcc.m_maxU = U;
  qp->intcc.m_delta_t = delta_t;
}

void
RdmaHw::UpdateRateIntra(Ptr<RdmaQueuePair> qp,
                        Ptr<Packet> p,
                        CustomHeader& ch,
                        bool fast_react)
{
  uint32_t next_seq = qp->snd_nxt;
  IntHeader& ih = ch.ack.ih;
#if PRINT_LOG
  // time RATE stage flowtype src dst sport dport [seqs]
  // iter{[i] hop[i].qlen(past) hop[i].bytes(past) hop[i].time(past)}
  // iter{[i] hop[i].txrate hop[i].u}
  printf("%lu %.5lf %s %s %u %u %u %u [%u,%u,%u] incstage=%d",
         Simulator::Now().GetTimeStep(),
         qp->m_rate.GetBitRate() * 1e-9,
         fast_react ? "fast" : "update",
         qp->m_flowType ? "DC" : "WAN",
         (qp->sip.Get() >> 8) & 0xffff,
         (qp->dip.Get() >> 8) & 0xffff,
         qp->sport,
         qp->dport,
         qp->intcc.m_lastUpdateSeq,
         ch.ack.seq,
         next_seq,
         qp->intcc.m_incStage);
  for (uint32_t i = 0; i < ih.gear.nhop; i++)
    printf(" [%u] %u(%u) %lu(%lu) %lu(%lu)",
           i,
           ih.gear.hop[i].GetQlen(),
           qp->intcc.hop[i].GetQlen(),
           ih.gear.hop[i].GetBytes(),
           qp->intcc.hop[i].GetBytes(),
           ih.gear.hop[i].GetTime(),
           qp->intcc.hop[i].GetTime());
#endif
  if (qp->intcc.m_lastUpdateSeq == 0) { // First RTT
    qp->intcc.m_lastUpdateSeq = next_seq;
    NS_ASSERT(ih.gear.nhop <= IntHeader::maxHop);
    for (uint32_t i = 0; i < ih.gear.nhop; i++)
      qp->intcc.hop[i] = ih.gear.hop[i];
    qp->intcc.m_curRate = qp->m_rate;
    qp->intcc.m_lastRate = qp->m_rate;
  } else {
    if (ih.gear.nhop <= IntHeader::maxHop) {
      double max_c = 0.0;
      double U = 0.0;
      uint64_t dt = 0;
      NS_ASSERT(ih.gear.nhop <= IntHeader::maxHop);
      for (uint32_t i = 0; i < ih.gear.nhop; i++) {
        uint64_t tau = ih.gear.hop[i].GetTimeDelta(qp->intcc.hop[i]);
        double duration = tau * 1e-9;
        double txRate =
          (ih.gear.hop[i].GetBytesDelta(qp->intcc.hop[i])) * 8 / duration;
        double u = txRate / ih.gear.hop[i].GetLineRate() +
                   (double)std::min(ih.gear.hop[i].GetQlen(),
                                    qp->intcc.hop[i].GetQlen()) *
                     qp->m_max_rate.GetBitRate() /
                     ih.gear.hop[i].GetLineRate() / qp->m_win;
#if PRINT_LOG
        printf(" [%u] %.5lf %.5lf", i, txRate * 1e-9, u);
#endif
        if (u > U) {
          U = u;
          dt = tau;
        }
        qp->intcc.hop[i] = ih.gear.hop[i];
      }

      DataRate new_rate;
      int32_t new_incStage;
      if (dt > qp->m_baseRtt) {
        qp->intcc.u = U;
      } else {
        qp->intcc.u =
          (qp->intcc.u * (qp->m_baseRtt - dt) + U * dt) / double(qp->m_baseRtt);
      }
      max_c = qp->intcc.u / m_targetUtil;
      if (max_c >= m_targetUtil || qp->intcc.m_incStage >= m_maxStage) {
        if (qp->intcc.m_flag) {
          new_rate = qp->intcc.m_curRate / max_c + m_rai;
          // qp->intcc.m_lastUtilization = qp->intcc.u;
          // qp->intcc.m_lastRate = qp->intcc.m_curRate;
#if PRINT_LOG
          printf(" F[%s]: U: %.6lf m_rate: %.5lf dt: %lu u: %.6lf",
                 (max_c >= m_targetUtil) ? "MD" : "MI",
                 U,
                 qp->intcc.m_lastRate.GetBitRate() * 1e-9,
                 dt,
                 qp->intcc.u);
#endif
        } else {
          double u1 = qp->intcc.m_lastUtilization;
          // // 计算广域网速率并且更新
          DataRate B = qp->intcc.hop[0].GetLineRate();
          DataRate ra = qp->intcc.m_lastRate;
          double u2 = u1 + (m_targetUtil / u1 - 1.0) * (ra / B);
          double u3 = qp->intcc.u;
          if (u3 > m_targetUtil - 0.01 && u3 < m_targetUtil + 0.001) {
            new_rate = qp->intcc.m_curRate / max_c + m_rai;
            // qp->intcc.m_lastUtilization = qp->intcc.u;
            // qp->intcc.m_lastRate = qp->intcc.m_curRate;
#if PRINT_LOG
            printf(" F[%s]: U: %.6lf m_rate: %.5lf dt: %lu u: %.6lf",
                   (max_c >= m_targetUtil) ? "MD" : "MI",
                   U,
                   qp->intcc.m_lastRate.GetBitRate() * 1e-9,
                   dt,
                   qp->intcc.u);
            printf(" maxc=%.3lf ra=%.5lf u1=%.6lf u2=%.6lf u3=%.6lf "
                   "curRate=%.5lf new_rate=%.5lf ",
                   max_c,
                   ra.GetBitRate() * 1e-9,
                   u1,
                   u2,
                   u3,
                   qp->intcc.m_curRate.GetBitRate() * 1e-9,
                   new_rate.GetBitRate() * 1e-9);
#endif
          } else {
            DataRate rb = abs(((u3 - u1) * B.GetBitRate() -
                               (m_targetUtil - u1) / u1 * ra.GetBitRate()) *
                              (u3 / (m_targetUtil - u3)));
            DataRate rc;
            DataRate rWAN =
              u1 * (B.GetBitRate()) * (m_targetUtil - u3) / (m_targetUtil - u1);
            if (u1 * B < ra + rb)
              rc = 0;
            else
              rc = u1 * B.GetBitRate() - ra.GetBitRate() - rb.GetBitRate();
            if (u3 < m_targetUtil + 2 * 10e-3 &&
                u3 > m_targetUtil - 2 * 10e-3) {
              new_rate = qp->intcc.m_curRate + m_rai;
              // new_rate = qp->intcc.m_curRate *
              //              std::abs((m_targetUtil * m_targetUtil -
              //                        2 * m_targetUtil * u1 + u1 * u3) /
              //                       (m_targetUtil * (u3 - u1))) +
              //            m_rai;
            } else {
#if PRINT_LOG
              printf(" [Com]");
#endif
              new_rate =
                (m_targetUtil / u3) * qp->intcc.m_curRate.GetBitRate() +
                (m_targetUtil / u3 - 1) * rc.GetBitRate() *
                  qp->intcc.m_curRate.GetBitRate() /
                  (ra.GetBitRate() + rb.GetBitRate()) +
                m_rai;
              // new_rate = qp->intcc.m_curRate *
              //              std::abs((m_targetUtil * m_targetUtil -
              //                        2 * m_targetUtil * u1 + u1 * u3) /
              //                       (m_targetUtil * (u3 - u1))) +
              //            m_rai;
#if PRINT_LOG
              printf(" !F[Ramp] k=%lf maxc=%.3lf ra=%.5lf rb=%.5lf rc=%lf "
                     "rwan=%.6lf "
                     "U=%.6lf "
                     "u1=%.6lf "
                     "u2=%.6lf u3=%.6lf curRate=%.5lf new_rate=%.5lf ",
                     std::abs((m_targetUtil * m_targetUtil -
                               2 * m_targetUtil * u1 + u1 * u3) /
                              (m_targetUtil * (u3 - u1))),
                     max_c,
                     ra.GetBitRate() * 1e-9,
                     rb.GetBitRate() * 1e-9,
                     rc.GetBitRate() * 1e-9,
                     rWAN.GetBitRate() * 1e-9,
                     U,
                     u1,
                     u2,
                     u3,
                     qp->intcc.m_curRate.GetBitRate() * 1e-9,
                     new_rate.GetBitRate() * 1e-9);
#endif
            }
          }
        }
        if (!fast_react) {
          qp->intcc.m_flag = !qp->intcc.m_flag;
          new_incStage = 0;
        }
      } else {
        new_rate = qp->intcc.m_curRate + m_rai;
        new_incStage = qp->intcc.m_incStage + 1;
      }
      if (new_rate < m_minRate)
        new_rate = m_minRate;
      if (new_rate > qp->m_max_rate)
        new_rate = qp->m_max_rate;
#if PRINT_LOG
      printf(" (%.3lf) %.3lf %s %.3lf",
             qp->intcc.m_curRate.GetBitRate() * 1e-9,
             qp->m_rate.GetBitRate() * 1e-9,
             (qp->intcc.m_curRate < new_rate) ? "↑" : "↓",
             new_rate.GetBitRate() * 1e-9);
#endif
      ChangeRate(qp, new_rate);
      qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                    << new_rate.GetBitRate() << std::endl;
      if (!fast_react) {
        qp->intcc.m_lastUtilization = qp->intcc.u;
        qp->intcc.m_lastRate = qp->intcc.m_curRate;
        qp->intcc.m_curRate = new_rate;
        qp->intcc.m_incStage = new_incStage;
      }
    }
    if (!fast_react) {
      if (next_seq > qp->intcc.m_lastUpdateSeq)
        qp->intcc.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
    }
  }
#if PRINT_LOG
  printf("\n");
#endif
}

#ifdef v1
void
RdmaHw::UpdateRateCross(Ptr<RdmaQueuePair> qp,
                        Ptr<Packet> p,
                        CustomHeader& ch,
                        bool fast_react)
{
  qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                << qp->m_rate.GetBitRate() << std::endl;
  uint32_t next_seq = qp->snd_nxt;
  IntHeader& ih = ch.ack.ih;
  uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.gear.ts;
  uint64_t fairnessArg = std::min(rtt / 32976, uint64_t(2.0));
#if PRINT_LOG
  printf("%lu %s %s %u %u %u %u [%u,%u,%u]",
         Simulator::Now().GetTimeStep(),
         fast_react ? "fast" : "update",
         qp->m_flowType ? "DC" : "WAN",
         (qp->sip.Get() >> 8) & 0xffff,
         (qp->dip.Get() >> 8) & 0xffff,
         qp->sport,
         qp->dport,
         qp->intcc.m_lastUpdateSeq,
         ch.ack.seq,
         next_seq);
#endif
  if (qp->intcc.m_lastUpdateSeq == 0) {
    qp->intcc.m_lastUpdateSeq = next_seq;
    NS_ASSERT(ih.gear.nhop <= IntHeader::maxHop);
    for (uint32_t i = 0; i < ih.gear.nhop; i++)
      qp->intcc.hop[i] = ih.gear.hop[i];
    qp->intcc.m_lastUtilization = 0.0;
    qp->intcc.m_curRate = qp->m_rate;
    qp->intcc.m_bketa = 0.0;
  } else {
    NS_ASSERT(ih.gear.nhop <= IntHeader::maxHop);
    double max_c = 0.0;
    double U = 0.0;
    uint64_t dt = 0;
    for (uint32_t i = 0; i < ih.gear.nhop; i++) {
      uint64_t tau = ih.gear.hop[i].GetTimeDelta(qp->intcc.hop[i]);
      double duration = tau * 1e-9;
      double txRate =
        (ih.gear.hop[i].GetBytesDelta(qp->intcc.hop[i])) * 8 / duration;
      double u =
        txRate / ih.gear.hop[i].GetLineRate() +
        (double)std::min(ih.gear.hop[i].GetQlen(), qp->intcc.hop[i].GetQlen()) *
          qp->m_max_rate.GetBitRate() / ih.gear.hop[i].GetLineRate() /
          qp->m_win;
#if PRINT_LOG
      printf(" [%u] %.5lf %.5lf", i, txRate * 1e-9, u);
#endif
      if (u > U) {
        U = u;
        dt = tau;
      }
      qp->intcc.hop[i] = ih.gear.hop[i];
    }
    DataRate B = qp->intcc.hop[0].GetLineRate();
    qp->intcc.u =
      (qp->intcc.u * (qp->m_baseRtt - dt) + U * dt) / double(qp->m_baseRtt);
    string stage = "";
    if (!fast_react) {
      DataRate new_rate;
      double para = 0.0;
      if (qp->intcc.u < m_targetUtil + 0.01 && qp->intcc.u >= m_targetUtil) {
        stage = " [Single]";
        para = m_targetUtil / qp->intcc.u;
        qp->intcc.m_bketa =
          (para - 1) * qp->intcc.m_curRate.GetBitRate() / B.GetBitRate();
        new_rate = qp->m_rate * para + fairnessArg * m_rai;
      } else {
        stage = " [Multi]";
        para = (4 * m_targetUtil / qp->intcc.u - 3);
        if (qp->intcc.u > m_targetUtil - 0.03 &&
            qp->intcc.u < m_targetUtil - 0.02) {
          para = (200 * m_targetUtil / qp->intcc.u - 199);
        }
        if (qp->intcc.u > m_targetUtil - 0.03 &&
            qp->intcc.u < m_targetUtil - 0.02) {
          para = (180 * m_targetUtil / qp->intcc.u - 179);
        }
        if (qp->intcc.u > m_targetUtil - 0.02 &&
            qp->intcc.u < m_targetUtil - 0.01) {
          para = (160 * m_targetUtil / qp->intcc.u - 159);
        }
        if (qp->intcc.u > m_targetUtil - 0.01 && qp->intcc.u < m_targetUtil) {
          para = (140 * m_targetUtil / qp->intcc.u - 139);
        }
        // if (para > 1.25)
        //   para = 1.25;
        // if (para < 0.75)
        //   para = 0.75;
        qp->intcc.m_bketa =
          (para - 1) * qp->intcc.m_curRate.GetBitRate() / B.GetBitRate();
        new_rate = qp->m_rate * para + fairnessArg * m_rai;
      }
#if PRINT_LOG
      printf("%s %.5lf %.5lf %.5lf (%.3lf) "
             "%.3lf %s %.3lf",
             stage.c_str(),
             qp->intcc.u,
             para,
             qp->intcc.m_bketa,
             qp->intcc.m_curRate.GetBitRate() * 1e-9,
             qp->m_rate.GetBitRate() * 1e-9,
             (qp->intcc.m_curRate < new_rate) ? "↑" : "↓",
             new_rate.GetBitRate() * 1e-9);
#endif
      if (new_rate < m_minRate)
        new_rate = m_minRate;
      if (new_rate > qp->m_max_rate)
        new_rate = qp->m_max_rate;
      ChangeRate(qp, new_rate);
      // 参数更新
      qp->intcc.m_curRate = qp->m_rate;
      if (next_seq > qp->intcc.m_lastUpdateSeq)
        qp->intcc.m_lastUpdateSeq = next_seq + rand() % 2 * m_mtu;
    }
  }
#if PRINT_LOG
  printf("\n");
#endif
}
#endif

#ifdef v2

// ---------------- 量化utilization ----------------
// A.第一个RTT的ack到达时，初始化参数： ok
//    1.m_CurRate = m_rate    ok
//    2.m_compensation=0    ok
//    3.m_probeTimer=simTime()+DC RTT   ok
// B.计时器 m_probeTimer 到时后， 即一个 DC RTT 结束时，更新参数：  ok
//    1.按照 DC 的算法更新    ok
//      1.1 更新 curRate 假速率, 包括AI和MD阶段   ok
//      1.2 更新incstage    ok
//    2.计算累计的补偿值 m_compensation   ok
//    3.更新计时器 m_probeTimer   ok
// C.当1个 WANRTT 结束时  ok
//    1.根据 m_compensation 更新 m_rate[计算补偿值]  ok
//    2.重置 m_compensation = 0   ok
//    3.重置 m_probeTimer = simTime()+DC RTT    ok
//    4.重置 m_CurRate = m_rate   ok
// ---------------- 量化utilization ----------------
void
RdmaHw::UpdateRateCross(Ptr<RdmaQueuePair> qp,
                        Ptr<Packet> p,
                        CustomHeader& ch,
                        bool fast_react)
{
  qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                << qp->m_rate.GetBitRate() << std::endl;
  uint32_t next_seq = qp->snd_nxt;
  IntHeader& ih = ch.ack.ih;
  uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.gear.ts;
  uint64_t fairnessArg = std::min(rtt / 32976, uint64_t(1.0));
#if PRINT_LOG
  // time stage flowtype src dst sport dport [seqs]
  // iter{[i] hop[i].qlen(past) hop[i].bytes(past) hop[i].time(past)}
  // iter{[i] hop[i].txrate hop[i].u}
  printf("%lu %s %s %u %u %u %u [%u,%u,%u]",
         Simulator::Now().GetTimeStep(),
         fast_react ? "fast" : "update",
         qp->m_flowType ? "DC" : "WAN",
         (qp->sip.Get() >> 8) & 0xffff,
         (qp->dip.Get() >> 8) & 0xffff,
         qp->sport,
         qp->dport,
         qp->intcc.m_lastUpdateSeq,
         ch.ack.seq,
         next_seq);
  // for (uint32_t i = 0; i < ih.gear.nhop; i++)
  //   printf(" [%u] %u(%u) %lu(%lu) %lu(%lu)",
  //          i,
  //          ih.gear.hop[i].GetQlen(),
  //          qp->intcc.hop[i].GetQlen(),
  //          ih.gear.hop[i].GetBytes(),
  //          qp->intcc.hop[i].GetBytes(),
  //          ih.gear.hop[i].GetTime(),
  //          qp->intcc.hop[i].GetTime());
#endif
  uint64_t curtime = Simulator::Now().GetTimeStep();
  if (qp->intcc.m_lastUpdateSeq == 0) {
    qp->intcc.m_lastUpdateSeq = next_seq;
    NS_ASSERT(ih.gear.nhop <= IntHeader::maxHop);
    for (uint32_t i = 0; i < ih.gear.nhop; i++)
      qp->intcc.hop[i] = ih.gear.hop[i];
    qp->intcc.m_lastUtilization = 0.0;
    qp->intcc.m_curRate = qp->m_rate;
    qp->intcc.m_compensation = 0.0;
    qp->intcc.m_probeTimer = curtime + 32976;
    qp->intcc.m_bketa = 0.0;
    qp->intcc.m_maxU = 0.0;
  } else {
    NS_ASSERT(ih.gear.nhop <= IntHeader::maxHop);
    double max_c = 0.0;
    double U = 0.0;
    uint64_t dt = 0;
    for (uint32_t i = 0; i < ih.gear.nhop; i++) {
      uint64_t tau = ih.gear.hop[i].GetTimeDelta(qp->intcc.hop[i]);
      double duration = tau * 1e-9;
      double txRate =
        (ih.gear.hop[i].GetBytesDelta(qp->intcc.hop[i])) * 8 / duration;
      double u =
        txRate / ih.gear.hop[i].GetLineRate() +
        (double)std::min(ih.gear.hop[i].GetQlen(), qp->intcc.hop[i].GetQlen()) *
          qp->m_max_rate.GetBitRate() / ih.gear.hop[i].GetLineRate() /
          qp->m_win;
#if PRINT_LOG
      printf(" [%u] %.5lf %.5lf", i, txRate * 1e-9, u);
#endif
      if (u > U) {
        U = u;
        dt = tau;
      }
      qp->intcc.hop[i] = ih.gear.hop[i];
    }
    DataRate B = qp->intcc.hop[0].GetLineRate();
    // qp->intcc.u =
    //   (qp->intcc.u * (qp->m_baseRtt - dt) + U * dt) / double(qp->m_baseRtt);
    if (dt < 32976) {
      qp->intcc.u = (qp->intcc.u * (32976 - dt) + U * dt) / double(32976.0);
    } else {
      qp->intcc.u = U;
    }
    max_c = qp->intcc.u / m_targetUtil;
    if (curtime >= qp->intcc.m_probeTimer && qp->intcc.m_maxU < 230.0) {
      if (max_c >= m_targetUtil || qp->intcc.m_incStage >= m_maxStage) {
#if PRINT_LOG
        printf(" MIMD");
        printf(" u=%.5lf max_c=%.5lf m_curRate=%.5lf",
               qp->intcc.u,
               max_c,
               qp->intcc.m_curRate.GetBitRate() * 1e-9);
#endif
        qp->intcc.m_curRate = qp->intcc.m_curRate / max_c + m_rai;
        qp->intcc.m_incStage = 0;
      } else {
        qp->intcc.m_curRate = qp->intcc.m_curRate + m_rai;
        qp->intcc.m_incStage = qp->intcc.m_incStage + 1;
      }
      qp->intcc.m_compensation =
        ((double(qp->m_rate.GetBitRate() * 1e-9) -
          double(qp->intcc.m_curRate.GetBitRate() * 1e-9)) *
         double(curtime - qp->intcc.m_probeTimer + 32976) * 1e-9) +
        qp->intcc.m_compensation;
      qp->intcc.m_maxU = qp->intcc.m_maxU + 1.0;
#if PRINT_LOG
      printf(" incstage:%u m_rate:%lf m_curRate:%lf time:%lu probeTimer:%lu "
             "curRate:%.5lf compen:%.5lf up:%.5lf down: %lu",
             qp->intcc.m_incStage,
             (qp->m_rate.GetBitRate() * 1e-9),
             (qp->intcc.m_curRate.GetBitRate() * 1e-9),
             curtime,
             qp->intcc.m_probeTimer,
             qp->intcc.m_curRate.GetBitRate() * 1e-9,
             qp->intcc.m_compensation,
             (double(qp->m_rate.GetBitRate() * 1e-9) -
              double(qp->intcc.m_curRate.GetBitRate() * 1e-9)),
             (curtime - qp->intcc.m_probeTimer + 32976));
#endif
      qp->intcc.m_probeTimer = curtime + 32976;
    }
    if (!fast_react) {
      DataRate new_rate;
      double para = 0.0;
      if (abs(qp->intcc.m_compensation) < 0.01) {
        printf(" one");
        para =
          2 * (qp->intcc.m_compensation / double(qp->m_baseRtt * 1e-9) * 1e9);
      } else {
        printf(" two");
        para =
          2 * (qp->intcc.m_compensation / double(qp->m_baseRtt * 1e-9) * 1e9);
      }
// 量化计算
#if PRINT_LOG
      printf(" [change] %.5lf %.5lf", qp->intcc.m_compensation, para);
#endif
      new_rate = qp->m_rate.GetBitRate() - para;

      // if (qp->intcc.u < m_targetUtil + 0.001 && qp->intcc.u >= m_targetUtil)
      // {
      //   printf(" [Single]");
      //   para = m_targetUtil / qp->intcc.u;
      //   // qp->intcc.m_bketa =
      //   //   (para - 1) * qp->intcc.m_curRate.GetBitRate() / B.GetBitRate();
      //   new_rate = qp->m_rate * para + fairnessArg * m_rai;
      // } else {
      //   printf(" [Multi]");
      //   para = (3 * m_targetUtil / qp->intcc.u - 2);
      //   if (para>1.25) para=1.25;
      //   if (para<0.75) para=0.75;
      //   // qp->intcc.m_bketa =
      //   //   (para - 1) * qp->intcc.m_curRate.GetBitRate() / B.GetBitRate();
      //   new_rate = qp->m_rate * para + fairnessArg * m_rai;
      // }
#if PRINT_LOG
      printf(" %.5lf %.5lf (%.3lf) "
             "%.3lf %s %.3lf",
             qp->intcc.u,
             para,
             qp->intcc.m_curRate.GetBitRate() * 1e-9,
             qp->m_rate.GetBitRate() * 1e-9,
             (qp->m_rate < new_rate) ? "↑" : "↓",
             new_rate.GetBitRate() * 1e-9);
#endif
      if (new_rate < m_minRate)
        new_rate = m_minRate;
      if (new_rate > qp->m_max_rate)
        new_rate = qp->m_max_rate;
      ChangeRate(qp, new_rate);
      // 参数更新
      qp->intcc.m_curRate = qp->m_rate;
      qp->intcc.m_compensation = 0.0;
      qp->intcc.m_maxU = 0.0;
      qp->intcc.m_probeTimer = curtime + 32976;
      if (next_seq > qp->intcc.m_lastUpdateSeq)
        qp->intcc.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
    }
  }
#if PRINT_LOG
  printf("\n");
#endif
}
#endif

/**********************
 * TIMELY
 *********************/
void
RdmaHw::HandleAckTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                << qp->m_rate.GetBitRate() << std::endl;
  uint32_t ack_seq = ch.ack.seq;
  // update rate
  if (ack_seq >
      qp->tmly
        .m_lastUpdateSeq) { // if full RTT feedback is ready, do full update
    UpdateRateTimely(qp, p, ch, false);
  } else { // do fast react
    FastReactTimely(qp, p, ch);
  }
}
void
RdmaHw::UpdateRateTimely(Ptr<RdmaQueuePair> qp,
                         Ptr<Packet> p,
                         CustomHeader& ch,
                         bool us)
{
  uint32_t next_seq = qp->snd_nxt;
  uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.ts;
  double scale = qp->m_baseRtt / 32000;
  if (scale > 400)
    scale = 400;
  double inc_scale = IsDCFlow(qp, ch) ? scale : scale / 10;
  double dynamic_beta = scale * m_tmly_beta;
  m_tmly_minRtt = qp->m_baseRtt;
  m_tmly_TLow = qp->m_baseRtt * 1.05;
  m_tmly_THigh = qp->m_baseRtt * 2.5;
  bool print = !us;
  if (qp->tmly.m_lastUpdateSeq != 0) { // not first RTT
    int64_t new_rtt_diff = (int64_t)rtt - (int64_t)qp->tmly.lastRtt;
    double rtt_diff =
      (1 - m_tmly_alpha) * qp->tmly.rttDiff + m_tmly_alpha * new_rtt_diff;
    double gradient = rtt_diff / m_tmly_minRtt;
    bool inc = false;
    double c = 0;
#if PRINT_LOG
    if (print)
      printf("%lu src:%u dst:%u rtt:%lu rttDiff:%.0lf gradient:%.3lf "
             "rate:%.3lf",
             Simulator::Now().GetTimeStep(),
             m_node->GetId(),
             (qp->dip.Get() >> 8) & 0xffff,
             rtt,
             rtt_diff,
             gradient,
             qp->tmly.m_curRate.GetBitRate() * 1e-9);
#endif
    // if (rtt < m_tmly_TLow) {
    //   inc = true;
    // } else if (rtt > m_tmly_THigh) {
    //   c = 1 - m_tmly_beta * (1 - (double)m_tmly_THigh / rtt);
    //   inc = false;
    // }
    // else
    if (gradient <= 0) {
      inc = true;
    } else {
      c = 1 - dynamic_beta * gradient;
      if (c < 0)
        c = 0;
      inc = false;
    }
    if (inc) {
      if (qp->tmly.m_incStage < 5) {
        qp->m_rate = qp->tmly.m_curRate + inc_scale * m_rai;
      } else {
        qp->m_rate = qp->tmly.m_curRate + inc_scale * m_rhai;
      }
      if (qp->m_rate > qp->m_max_rate)
        qp->m_rate = qp->m_max_rate;
      if (!us) {
        qp->tmly.m_curRate = qp->m_rate;
        qp->tmly.m_incStage++;
        qp->tmly.rttDiff = rtt_diff;
      }
    } else {
      qp->m_rate = std::max(m_minRate, qp->tmly.m_curRate * c);
      if (!us) {
        qp->tmly.m_curRate = qp->m_rate;
        qp->tmly.m_incStage = 0;
        qp->tmly.rttDiff = rtt_diff;
      }
    }
#if PRINT_LOG
    if (print) {
      printf(" %c %.3lf\n", inc ? '^' : 'v', qp->m_rate.GetBitRate() * 1e-9);
    }
#endif
  }
  if (!us && next_seq > qp->tmly.m_lastUpdateSeq) {
    qp->tmly.m_lastUpdateSeq = next_seq;
    // update
    qp->tmly.lastRtt = rtt;
  }
}
void
RdmaHw::FastReactTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
}

/**********************
 * DCTCP
 *********************/
void
RdmaHw::HandleAckDctcp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  qp->m_rateLog << Simulator::Now().GetTimeStep() << " "
                << qp->m_rate.GetBitRate() << std::endl;
  uint32_t ack_seq = ch.ack.seq;
  uint8_t cnp = (ch.ack.flags >> qbbHeader::FLAG_CNP) & 1;
  bool new_batch = false;
  // update alpha
  qp->dctcp.m_ecnCnt += (cnp > 0);
  if (ack_seq > qp->dctcp.m_lastUpdateSeq) { // if full RTT feedback is
                                             // ready, do alpha update
#if PRINT_LOG
    printf("%lu %s %u %u %u %u [%u,%u,%lu] %.3lf->",
           Simulator::Now().GetTimeStep(),
           "alpha",
           (qp->sip.Get() >> 8) & 0xffff,
           (qp->dip.Get() >> 8) & 0xffff,
           qp->sport,
           qp->dport,
           qp->dctcp.m_lastUpdateSeq,
           ch.ack.seq,
           qp->snd_nxt,
           qp->dctcp.m_alpha);
#endif
    new_batch = true;
    if (qp->dctcp.m_lastUpdateSeq == 0) { // first RTT
      qp->dctcp.m_lastUpdateSeq = qp->snd_nxt;
      qp->dctcp.m_batchSizeOfAlpha = qp->snd_nxt / m_mtu + 1;
    } else {
      double frac = std::min(
        1.0, double(qp->dctcp.m_ecnCnt) / qp->dctcp.m_batchSizeOfAlpha);
      qp->dctcp.m_alpha = (1 - m_g) * qp->dctcp.m_alpha + m_g * frac;
      qp->dctcp.m_lastUpdateSeq = qp->snd_nxt;
      qp->dctcp.m_ecnCnt = 0;
      qp->dctcp.m_batchSizeOfAlpha = (qp->snd_nxt - ack_seq) / m_mtu + 1;
#if PRINT_LOG
      printf("%.3lf F:%.3lf", qp->dctcp.m_alpha, frac);
#endif
    }
#if PRINT_LOG
    printf("\n");
#endif
  }

  // check cwr exit
  if (qp->dctcp.m_caState == 1) {
    if (ack_seq > qp->dctcp.m_highSeq)
      qp->dctcp.m_caState = 0;
  }

  // check if need to reduce rate: ECN and not in CWR
  if (cnp && qp->dctcp.m_caState == 0) {
#if PRINT_LOG
    printf("%lu %s %u %u %u %u %.3lf->",
           Simulator::Now().GetTimeStep(),
           "rate",
           (qp->sip.Get() >> 8) & 0xffff,
           (qp->dip.Get() >> 8) & 0xffff,
           qp->sport,
           qp->dport,
           qp->m_rate.GetBitRate() * 1e-9);
#endif
    qp->m_rate = std::max(m_minRate, qp->m_rate * (1 - qp->dctcp.m_alpha / 2));
#if PRINT_LOG
    printf("%.3lf\n", qp->m_rate.GetBitRate() * 1e-9);
#endif
    qp->dctcp.m_caState = 1;
    qp->dctcp.m_highSeq = qp->snd_nxt;
  }

  // additive inc
  if (qp->dctcp.m_caState == 0 && new_batch) {
    qp->m_rate = std::min(qp->m_max_rate, qp->m_rate + m_dctcp_rai);
  }
}

/*********************
 * HPCC-PINT
 ********************/
void
RdmaHw::SetPintSmplThresh(double p)
{
  pint_smpl_thresh = (uint32_t)(65536 * p);
}
void
RdmaHw::HandleAckHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader& ch)
{
  uint32_t ack_seq = ch.ack.seq;
  if (rand() % 65536 >= pint_smpl_thresh)
    return;
  // update rate
  if (ack_seq >
      qp->hpccPint
        .m_lastUpdateSeq) { // if full RTT feedback is ready, do full update
    UpdateRateHpPint(qp, p, ch, false);
  } else { // do fast react
    UpdateRateHpPint(qp, p, ch, true);
  }
}

void
RdmaHw::UpdateRateHpPint(Ptr<RdmaQueuePair> qp,
                         Ptr<Packet> p,
                         CustomHeader& ch,
                         bool fast_react)
{
  uint32_t next_seq = qp->snd_nxt;
  if (qp->hpccPint.m_lastUpdateSeq == 0) { // first RTT
    qp->hpccPint.m_lastUpdateSeq = next_seq;
  } else {
    // check packet INT
    IntHeader& ih = ch.ack.ih;
    double U = Pint::decode_u(ih.GetPower());

    DataRate new_rate;
    int32_t new_incStage;
    double max_c = U / m_targetUtil;

    if (max_c >= 1 || qp->hpccPint.m_incStage >= m_miThresh) {
      new_rate = qp->hpccPint.m_curRate / max_c + m_rai;
      new_incStage = 0;
    } else {
      new_rate = qp->hpccPint.m_curRate + m_rai;
      new_incStage = qp->hpccPint.m_incStage + 1;
    }
    if (new_rate < m_minRate)
      new_rate = m_minRate;
    if (new_rate > qp->m_max_rate)
      new_rate = qp->m_max_rate;
    ChangeRate(qp, new_rate);
    if (!fast_react) {
      qp->hpccPint.m_curRate = new_rate;
      qp->hpccPint.m_incStage = new_incStage;
    }
    if (!fast_react) {
      if (next_seq > qp->hpccPint.m_lastUpdateSeq)
        qp->hpccPint.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
    }
  }
}

} // namespace ns3
