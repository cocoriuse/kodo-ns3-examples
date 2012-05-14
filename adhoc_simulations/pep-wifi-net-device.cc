#include <ns3/core-module.h>
#include <ns3/net-device.h>
#include "pep-wifi-net-device.h"
#include <kodo/rlnc/full_vector_codes.h>
#include <ns3/llc-snap-header.h>
#include <ns3/adhoc-wifi-mac.h>
#include <ns3/packet.h>
#include <ns3/llc-snap-header.h>

NS_LOG_COMPONENT_DEFINE ("PepWifiNetDevice");
namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (PepWifiNetDevice);


PepWifiNetDevice::PepWifiNetDevice ()
  : m_configComplete (false),
    recode (1),
    max_symbols (30),
    max_size (128),
    m_encoder_factory (max_symbols, max_size),
    m_decoder_factory (max_symbols, max_size)
{
  NS_LOG_FUNCTION_NOARGS ();
  code = 1;
  sent_packet = 0;
  interval = 0.5;
  generation = 1;
  received = 0;
  countcode = 0;
  from_source = 0;
  from_relay = 0;
  rank = 0;
  inc = 0;
  ninc = 0;
  rsource = 0;
  r = 0;
  temp = 0;
  sent_code = 0;
  encoder = m_encoder_factory.build ((max_symbols), max_size);
  payload.resize (encoder->payload_size ());
  relay_activity = 100;
  seed = 100;
 received_relay=0;
}


TypeId
PepWifiNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PepWifiNetDevice")
    .SetParent<WifiNetDevice> ()
    .AddConstructor<PepWifiNetDevice> ()
    .AddAttribute ("SymbolsNum",
                   "The number of Symbols in each generation",
                   UintegerValue (30),
                   MakeUintegerAccessor (&PepWifiNetDevice::max_symbols),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EnableCode",
                   "Enable coding",
                   UintegerValue (1),
                   MakeUintegerAccessor (&PepWifiNetDevice::code),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EnableRecode",
                   "Enable Recoding",
                   UintegerValue (1),
                   MakeUintegerAccessor (&PepWifiNetDevice::recode),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RelayActivity",
                   "relay activity",
                   UintegerValue (100),
                   MakeUintegerAccessor (&PepWifiNetDevice::relay_activity),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}


PepWifiNetDevice::~PepWifiNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}


void PepWifiNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb){
	
  m_PromiscReceiveCallback=cb;

  if (code == 1)
    {
      WifiNetDevice::SetPromiscReceiveCallback (ns3::MakeCallback (&PepWifiNetDevice::promisc, this));
    }
  else
    {
      WifiNetDevice::SetPromiscReceiveCallback (m_PromiscReceiveCallback);
    }

}

bool
PepWifiNetDevice::promisc (Ptr<NetDevice> device, Ptr<const Packet> packet1, uint16_t type,
                                    const Address & from, const Address & to, enum NetDevice::PacketType typ)
{

  Mac48Address des = Mac48Address ("00:00:00:00:00:01");
  Mac48Address source = Mac48Address ("00:00:00:00:00:02");
	
      if (typ != NetDevice::PACKET_OTHERHOST || type==2054  )
	      return true;	   
	
      Ptr<Packet> packet = packet1->Copy ();
      CodeHeader h1;
      packet->RemoveHeader (h1);
      if(from==des){
      	 std::cout << "Heollo:" << h1.GetCode()<< endl;	
			return true;
			}
       	
		
      std::cout << "received_relay:" << received_relay++<< endl;	

      if ( recode == 1)
        {

          Ptr<Packet> pkt = rencoding ( packet,(int)h1.GetGeneration ());
          pkt->AddHeader (h1);
          srand ( seed );
          seed++;

          if ((rand () % 100 + 1) > relay_activity)
            {
         
              // Send recoded packet
              WifiNetDevice::Send (pkt,to,type );
              sent_code++;
              cout << "sent_code:" << sent_code << endl;

            }
        }
      else
        {
          // Just forwarding
          packet->AddHeader (h1);
          srand ( (int)h1.GetGeneration () );
	  seed++;

          if ((rand () % 100 + 1) > relay_activity)
            {
              sent_code++;
              cout << "sent_code:" << sent_code << endl;
              WifiNetDevice::Send (packet,to,type );
            }
        
     
      }
   return true;  
}


void PepWifiNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback receiveCallback)
{


  m_receiveCallback = receiveCallback;

  if (code == 1)
    {
      WifiNetDevice::SetReceiveCallback (ns3::MakeCallback (&PepWifiNetDevice::DecodingReceive, this));
    }
  else
    {
      WifiNetDevice::SetReceiveCallback (m_receiveCallback);
    }
}





Ptr<Packet>
PepWifiNetDevice::rencoding (Ptr<Packet> packet,int seq)
{

  uint8_t *buffer1 = new uint8_t[packet->GetSize ()];
 
 if (forward.find(seq) == forward.end()) 
    forward[seq]= m_decoder_factory.build((max_symbols), max_size);
	
  packet->CopyData(buffer1,packet->GetSize());
  
  forward[seq]->decode( buffer1 );
  forward[seq]->recode( &payload[0]);

  Ptr<Packet> pkt = Create<Packet> (&payload[0],forward[seq]->payload_size());

return 	pkt ;
		


}


bool
PepWifiNetDevice::DecodingReceive (Ptr< NetDevice > device, Ptr< const
                                                                 Packet > packet1, uint16_t type, const Address & from)
{
 
  PointerValue ptr;
  GetAttribute ("Mac",ptr);
  Ptr<AdhocWifiMac> m_mac = ptr.Get<AdhocWifiMac> ();
  cout << "khobiii!!!"  << from << endl;
  if ( type == 2054){

     m_mac->NotifyRx (packet1);
     m_receiveCallback (this, packet1, type, from);
     cout << "received an ARP packet!!!"  << from << endl;
     cout << "how am i"<< m_mac->GetAddress ()<<endl;
     return true;
	}


  cout << "Max symbols" << max_symbols << endl;

  Ptr<Packet> packet = packet1->Copy ();
  Mac48Address des = Mac48Address ("00:00:00:00:00:01");
  Mac48Address source = Mac48Address ("00:00:00:00:00:02");

 

  if (from == source && m_mac->GetAddress () == des)
    {
      cout << "from_source:" << from_source << endl;
      from_source++;
    }

  

  if ( m_mac->GetAddress () == source && from == des)
    {
      CodeHeader h1;
      packet->RemoveHeader (h1);
      cout << "Generation is decoded:" << (int)h1.GetGeneration () << endl;
      decoded_flag[(int)h1.GetGeneration ()] = 1;
	
      return true; 
    }

  if (m_mac->GetAddress () == des && code == 1)
    {
      received++;
      cout << "received:" << received << endl;
      uint8_t *buffer1 = new uint8_t[packet->GetSize ()];

      CodeHeader h1;
      packet->RemoveHeader (h1);


      if (from != source)
        {
          cout << "from_relay:" << from_relay << endl;
          from_relay++;
        }


      //inja eshkal dare
          if (decoding.find((int)h1.GetGeneration()) == decoding.end())
    {
        decoding[h1.GetGeneration()]= m_decoder_factory.build((max_symbols), max_size);

    }

    rlnc_decoder::pointer decoder = decoding[h1.GetGeneration()];
    cout << "payload size 3 "  << decoder->payload_size() << endl;
    rank = (int)decoder->rank();
    packet->CopyData(buffer1,packet->GetSize());


    NS_ASSERT(packet->GetSize() == decoder->payload_size());

   
    decoder->decode( buffer1 );


    cout << "Generation : " << h1.GetGeneration()<< endl;

    if ((rank+1)==(int)(decoder->rank()) && from!=source)
    {
        cout << "increased:" <<inc << endl ;
        inc++;
    }
    if ((rank)==(int)(decoder->rank()) && from!=source)
    {
        cout << "not increased:" <<ninc << endl;
        ninc++;
    }
    if (from == source)
    {
        cout << "recevied_source:" <<rsource++<< endl;

    }
        
    cout << "rank after:" << decoder->rank()<< endl;



      if (decoder->is_complete () && decoded_flag[(int)h1.GetGeneration ()] == 0)
        {
          decoded_flag[(int)h1.GetGeneration ()] = 1;

          cout << "time:" << Simulator::Now ().GetSeconds () << endl;

          countcode++;
          cout << "decoded packets:" << (countcode * (max_symbols)) << endl;
      

          Ptr<Packet> ACK = Create<Packet> (10);
             CodeHeader h2;
			h2.DisableCode	();
	      std::cout << "slooo:" << h2.GetCode()<< endl;	
	      std::cout << "slooo:" << from<< endl;	

          ACK->AddHeader (h2);
          WifiNetDevice::Send (ACK,from,100 );

          std::vector<uint8_t> data_out (decoding[h1.GetGeneration ()]->block_size ());
          kodo::copy_symbols (kodo::storage (data_out), decoding[h1.GetGeneration ()]);

          for (int i = 0; i < (max_symbols); i++)
            {
              uint8_t *buffer1 = new uint8_t[max_size];
              memcpy (buffer1,&data_out[i * max_size],max_size);

              Ptr<Packet> pkt = Create<Packet> (buffer1,max_size);
              m_mac->NotifyRx (pkt);
              m_receiveCallback (this, pkt, type, Mac48Address ("00:00:00:00:00:01"));

            }

        }
      else if (decoding[h1.GetGeneration ()]->is_complete () && decoded_flag[(int)h1.GetGeneration ()] == 1)
        {

          Ptr<Packet> ACK = Create<Packet> (10);
          ACK->AddHeader (h1);
          WifiNetDevice::Send (ACK,Mac48Address ("00:00:00:00:00:02"),100 );

        }

    }

  return true;
}


bool PepWifiNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{

cout << "protocol number" << protocolNumber << endl;
cout << "destination" << dest << endl;
  if (code == 1 && protocolNumber !=2054)
    {
      cout << "coding is enabled" << flush;
      coding (packet,dest, protocolNumber);
	
    }
  else
    {
      cout << "coding is disabled " << endl;
      WifiNetDevice::Send (packet,dest,protocolNumber);
    }

  return true;
}

void
PepWifiNetDevice::SendCode (Ptr <coded> m_coded)
{

  if (decoded_flag[(int)m_coded->h1.GetGeneration ()] == 0)
    {

      sent_packet++;
      std::cout << "sent:" << sent_packet << std::endl;
      std::cout << "Interval:" << interval << std::endl;

      m_coded->k++;
      kodo::set_symbols (kodo::storage (m_coded->m_encoder_data), m_coded->m_encoder);

      std::vector<uint8_t> payload (m_coded->m_encoder->payload_size ());
      m_coded->m_encoder->encode ( &payload[0] );

      Ptr<Packet> pkt = Create<Packet> (&payload[0], m_coded->m_encoder->payload_size ());

      pkt->AddHeader (m_coded->h1);

      //PointerValue ptr;
      cout << "generation number: " << m_coded->h1.GetGeneration () << endl;

      WifiNetDevice::Send (pkt,m_coded->realTo,m_coded->protocolNumber );

      //m_coded.t2=m_coded.t2+interval;
      Simulator::Schedule ( Seconds (interval), &PepWifiNetDevice::SendCode, this,m_coded);

    }
  else
    {
      return;
    }

}

void
PepWifiNetDevice::Enqueue1 (Ptr<Packet> packet)
{

  m_queue.push_back ((packet));
}

bool
PepWifiNetDevice::coding (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{

  NS_ASSERT (Mac48Address::IsMatchingType (dest));
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);

  int k = 0;

  Enqueue1 (packet);

  if ((int)m_queue.size () == max_symbols )
    {
      encoder = m_encoder_factory.build ((max_symbols), packet->GetSize ());
      //encoder->systematic_off();


      CodeHeader h1;
      h1.SetGeneration (generation);
      generation++;

      Ptr<coded> m_coded = Create<coded> ();
      m_coded->t2 = interval;
      m_coded->k = k;
      m_coded->m_encoder = encoder;
      m_coded->protocolNumber = protocolNumber;
      m_coded->h1 = h1;
      m_coded->realTo = realTo;
      m_coded->m_encoder_data.resize (encoder->block_size ());

      uint8_t *buffer1 = new uint8_t[packet->GetSize ()];

      for (int i = 0; i < (max_symbols); i++)
        {
          Item p = m_queue.front ();
          m_queue.pop_front ();

          p.m_packet->CopyData (buffer1,p.m_packet->GetSize ());
          memcpy (&m_coded->m_encoder_data[(i * packet->GetSize ())],buffer1,p.m_packet->GetSize ());
          cout << "data in " << (i * packet->GetSize ()) << endl;
        }

      decoded_flag[(int)h1.GetGeneration ()] = 0;
      Simulator::Schedule ( Seconds (interval),&PepWifiNetDevice::SendCode, this, m_coded);

    }
  return true;


}

}

