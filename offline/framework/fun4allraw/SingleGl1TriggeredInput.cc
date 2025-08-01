#include "SingleGl1TriggeredInput.h"

#include "Fun4AllPrdfInputTriggerManager.h"
#include "InputManagerType.h"

#include <ffarawobjects/Gl1Packetv3.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNode.h>          // for PHNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/Eventiterator.h>
#include <Event/packet.h>  // for Packet

#include <TSystem.h>

#include <cstdint>   // for uint64_t
#include <iostream>  // for operator<<, basic_ostream<...
#include <iterator>  // for reverse_iterator
#include <limits>    // for numeric_limits
#include <memory>
#include <set>
#include <utility>  // for pair

SingleGl1TriggeredInput::SingleGl1TriggeredInput(const std::string &name)
  : SingleTriggeredInput(name)
{
}

void SingleGl1TriggeredInput::FillPool(int index)
{
  if (AllDone())  // no more files and all events read
  {
    return;
  }
  if (!FilesDone())
  {
    m_SkipEvents = 0;
    FillEventVector(index);
  }
  return;
}

void SingleGl1TriggeredInput::Print(const std::string &what) const
{
  std::cout << "what: " << what << std::endl;
}

void SingleGl1TriggeredInput::CreateDSTNodes(Event *evt)
{
  std::string CompositeNodeName = "Packets";
  if (KeepMyPackets())
  {
    CompositeNodeName = "PacketsKeep";
  }
  PHNodeIterator iter(m_topNode);
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode)
  {
    dstNode = new PHCompositeNode("DST");
    m_topNode->addNode(dstNode);
  }
  PHNodeIterator iterDst(dstNode);
  PHCompositeNode *detNode = dynamic_cast<PHCompositeNode *>(iterDst.findFirst("PHCompositeNode", CompositeNodeName));
  if (!detNode)
  {
    detNode = new PHCompositeNode(CompositeNodeName);
    dstNode->addNode(detNode);
  }
  std::vector<Packet *> pktvec = evt->getPacketVector();
  for (auto *piter : pktvec)
  {
    int packet_id = piter->getIdentifier();
    m_PacketSet.insert(packet_id);
    std::string PacketNodeName = std::to_string(packet_id);
    OfflinePacket *gl1hitcont = findNode::getClass<OfflinePacket>(detNode, PacketNodeName);
    if (!gl1hitcont)
    {
      gl1hitcont = new Gl1Packetv3();
      PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(gl1hitcont, PacketNodeName, "PHObject");
      detNode->addNode(newNode);
    }
    delete piter;
  }
}

uint64_t SingleGl1TriggeredInput::GetClock(Event *evt)
{
  Packet *packet = evt->getPacket(14001);
  if (!packet)
  {
    std::cout << Name()
              << " no packet 14001 for event, possible corrupt data event before EOR"
              << std::endl;
    std::cout << Name() << ": ";
    evt->identify();
    return std::numeric_limits<uint64_t>::max();
  }
  uint64_t clock = packet->lValue(0, "BCO");
  m_LastPacketNumber = m_PacketNumber;
  m_PacketNumber = packet->iValue(0);  // just fill this here while we are at it
  if (Verbosity() > 0)
  {
    std::cout << Name() << " Event " << evt->getEvtSequence() << " packet nr: "
              << m_PacketNumber << std::endl;
  }

  if (m_PacketNumber - m_LastPacketNumber == 1 || m_LastPacketNumber == 0)
  {
    if (Verbosity() > 0)
    {
      std::cout << "GL1 is in order" << std::endl;
    }
  }
  else
  {
    std::cout << "GL1 problem, gl1 skipped " << m_PacketNumber - m_LastPacketNumber
              << " Events" << std::endl;
    m_SkipEvents = m_PacketNumber - m_LastPacketNumber;
  }

  delete packet;
  return clock;
}

int SingleGl1TriggeredInput::ReadEvent()
{
  if (m_EventDeque.empty())
  {
    std::cout << Name() << ":all events done" << std::endl;
    AllDone(1);
    return -1;
  }
  Event *evt = m_EventDeque.front();
  m_EventDeque.pop_front();
  RunNumber(evt->getRunNumber());
  int EventSequence = evt->getEvtSequence();
  EventNumber(evt->getEvtSequence());
  //  evt->identify();
  Packet *packet = evt->getPacket(14001);
  if (packet)
  {
    Gl1Packet *gl1packet = findNode::getClass<Gl1Packet>(topNode(), 14001);
    unsigned int packetnumber = packet->iValue(0);
    uint64_t gtm_bco = packet->lValue(0, "BCO");
    //    std::cout << "saving bco 0x" << std::hex << gtm_bco << std::dec << std::endl;
    gl1packet->setBCO(packet->lValue(0, "BCO"));
    gl1packet->setHitFormat(packet->getHitFormat());
    gl1packet->setIdentifier(packet->getIdentifier());
    gl1packet->setEvtSequence(EventSequence);
    gl1packet->setPacketNumber(packetnumber);

    gl1packet->setBunchNumber(packet->lValue(0, "BunchNumber"));
    gl1packet->setTriggerInput(packet->lValue(0, "TriggerInput"));
    gl1packet->setLiveVector(packet->lValue(0, "LiveVector"));
    gl1packet->setScaledVector(packet->lValue(0, "ScaledVector"));
    gl1packet->setGTMBusyVector(packet->lValue(0, "GTMBusyVector"));
    gl1packet->setGTMAllBusyVector(packet->lValue(0, "GTMAllBusyVector"));
    for (int i = 0; i < 64; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        gl1packet->setScaler(i, j, packet->lValue(i, j));
      }
    }
    for (int i = 0; i < 12; i++)
    {
      gl1packet->setGl1pScaler(i, 0, packet->lValue(i, "GL1PRAW"));
      gl1packet->setGl1pScaler(i, 1, packet->lValue(i, "GL1PLIVE"));
      gl1packet->setGl1pScaler(i, 2, packet->lValue(i, "GL1PSCALED"));
    }
    if (Verbosity() > 2)
    {
      std::cout << PHWHERE << " Packet: " << packet->getIdentifier()
                << " evtno: " << EventSequence
                << ", bco: 0x" << std::hex << gtm_bco << std::dec
                << ", bunch no: " << packet->lValue(0, "BunchNumber")
                << std::endl;
      // std::cout << PHWHERE << " RB Packet: " << gl1packet->getIdentifier()
      //           << " evtno: " << gl1packet->getEvtSequence()
      //           << ", bco: 0x" << std::hex << gl1packet->getBCO() << std::dec
      //           << ", bunch no: " << +gl1packet->getBunchNumber()
      //           << std::endl;
    }
    delete packet;
  }
  delete evt;
  return Fun4AllReturnCodes::EVENT_OK;
}
