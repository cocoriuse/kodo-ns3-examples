#include <ns3/core-module.h>
#include <ns3/net-device.h>
#include "pep-wifi-net-device.h"
#include <kodo/rlnc/full_vector_codes.h>
#include <ns3/llc-snap-header.h>
#include <ns3/adhoc-wifi-mac.h>
#include <ns3/packet.h>
#include <ns3/llc-snap-header.h>
#include <ns3/ipv4-header.h>
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
  recodedNum=0;
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
  numNodes=3;
  arrayMac [numNodes];
  //int array [400][numNodes];//maximun generation number and num of nodes
  buffer1 = new uint8_t[100];//packetsize defined by default 
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


bool
PepWifiNetDevice::promisc (Ptr<NetDevice> device, Ptr<const Packet> packet1, uint16_t type,
                                    const Address & from, const Address & to, enum NetDevice::PacketType typ)
{


	Ptr<Packet> packet = packet1->Copy ();
	CodeHeader h1;
	packet->RemoveHeader (h1);
	  
  PointerValue ptr;
  GetAttribute ("Mac",ptr);
  Ptr<AdhocWifiMac> m_mac = ptr.Get<AdhocWifiMac> ();
	  

	
      if (typ != NetDevice::PACKET_OTHERHOST || type==2054 || h1.GetCode() == 0 || h1.GetSourceMAC() == m_mac->GetAddress ())
	      return true;	   
	   	
	
     
      NS_LOG_DEBUG ( "received_relay:" << received_relay++);	

      if ( recode == 1)
        {
      NS_LOG_DEBUG ( "hi1:" <<h1.GetGeneration () );	

          Ptr<Packet> pkt = rencoding (packet,(int)h1.GetGeneration (),(Mac48Address)h1.GetSourceMAC());
            NS_LOG_DEBUG ( "hi2:" );	
          pkt->AddHeader (h1);
          srand ( seed );
          seed++;

          if ((rand () % 100 + 1) > relay_activity)
            {
         
              // Send recoded packet
              WifiNetDevice::Send (pkt,to,type );
              sent_code++;
              NS_LOG_DEBUG ( "sent_code:" << sent_code );

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
              NS_LOG_DEBUG ( "sent_code:" << sent_code );
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
PepWifiNetDevice::rencoding (Ptr<Packet> packet,int seq,Mac48Address source)
{//guessing that we have a finite number of generations!!!!!!!!!!!!!!!!!!!
  
   int i=0;  
   bool insertMac=false;
   while (insertMac==false)
   {
     
    if (arrayMac[i]=="00:00:00:00:00:00" ) 
      {
        arrayMac[i]=source;
        NS_LOG_DEBUG ( "reconding"  << arrayMac[i]);
        insertMac=true;
        array[seq][i]=recodedNum++;
        
      }
    else if (arrayMac[i]!=source) 
      i++;
    else
      {
      if (array[seq][i]==0)  
         array[seq][i]=recodedNum++;

      insertMac=true;
      }
   }
  
  // uint8_t *buffer1 = new uint8_t[packet->GetSize ()];
 
 if (forward.find(array[seq][i]) == forward.end()) 
   forward[array[seq][i]]= m_decoder_factory.build((max_symbols), max_size);

   	
  packet->CopyData(buffer1,packet->GetSize());
  
  forward[array[seq][i]]->decode( buffer1 );
  forward[array[seq][i]]->recode( &payload[0]);

  Ptr<Packet> pkt = Create<Packet> (&payload[0],forward[array[seq][i]]->payload_size());

return 	pkt ;
		
}


bool
PepWifiNetDevice::DecodingReceive (Ptr< NetDevice > device, Ptr< const
                                                                 Packet > packet1, uint16_t type, const Address & from)
{
 
  PointerValue ptr;
  GetAttribute ("Mac",ptr);
  Ptr<AdhocWifiMac> m_mac = ptr.Get<AdhocWifiMac> ();
  NS_LOG_DEBUG ( "received from"  << from );
 
  if ( type == 2054){

     m_mac->NotifyRx (packet1);
     m_receiveCallback (this, packet1, type, from);
     NS_LOG_DEBUG ( "received an ARP packet!!!"  << from );
     NS_LOG_DEBUG ( "how am i"<< m_mac->GetAddress ());
     return true;
	}


  NS_LOG_DEBUG ( "Max symbols" << max_symbols );

  Ptr<Packet> packet = packet1->Copy ();


		CodeHeader h1;
		packet->RemoveHeader (h1);
		
		Mac48Address source=h1.GetSourceMAC();
		
		
		  NS_LOG_DEBUG ( "from: " << from);
  		  NS_LOG_DEBUG ( "dource: " << source);

  if (from == source )
    {
			
      NS_LOG_DEBUG ( "from_source:" << from_source );
      from_source++;
    }

  

  if ( m_mac->GetAddress () == source )
    {

      NS_LOG_DEBUG ( "Generation is decoded:" << (int)h1.GetGeneration () );
      decoded_flag[(int)h1.GetGeneration ()] = 1;
	
      return true; 
    }

  if ( code == 1)
    {
      received++;
      NS_LOG_DEBUG ( "received:" << received );
      uint8_t *buffer1 = new uint8_t[packet->GetSize ()];

      if (from != source)
        {
          NS_LOG_DEBUG ( "from_relay:" << from_relay );
          from_relay++;
        }


      
          if (decoding.find((int)h1.GetGeneration()) == decoding.end())
    {
        decoding[h1.GetGeneration()]= m_decoder_factory.build((max_symbols), max_size);

    }

    rlnc_decoder::pointer decoder = decoding[h1.GetGeneration()];
    NS_LOG_DEBUG ( "payload size 3 "  << decoder->payload_size() );
    rank = (int)decoder->rank();
    packet->CopyData(buffer1,packet->GetSize());


    NS_ASSERT(packet->GetSize() == decoder->payload_size());

   
    decoder->decode( buffer1 );


    NS_LOG_DEBUG ( "Generation : " << h1.GetGeneration());

    if ((rank+1)==(int)(decoder->rank()) && from!=source)
    {
        NS_LOG_DEBUG ( "increased:" <<inc ) ;
        inc++;
    }
    if ((rank)==(int)(decoder->rank()) && from!=source)
    {
        NS_LOG_DEBUG ( "not increased:" <<ninc );
        ninc++;
    }
    if (from == source)
    {
        NS_LOG_DEBUG ( "recevied_source:" <<rsource++);

    }
        
    NS_LOG_DEBUG ( "rank after:" << decoder->rank());



      if (decoder->is_complete () && decoded_flag[(int)h1.GetGeneration ()] == 0)
        {
          decoded_flag[(int)h1.GetGeneration ()] = 1;

          NS_LOG_DEBUG ( "time:" << Simulator::Now ().GetSeconds () );

          countcode++;
          NS_LOG_DEBUG ( "decoded packets:" << (countcode * (max_symbols)) );
      

          Ptr<Packet> ACK = Create<Packet> (10);

			h1.DisableCode	();
	  

          ACK->AddHeader (h1);
          WifiNetDevice::Send (ACK,from,100 );

          vector<uint8_t> data_out (decoding[h1.GetGeneration ()]->block_size ());
          kodo::copy_symbols (kodo::storage (data_out), decoding[h1.GetGeneration ()]);

          for (int i = 0; i < (max_symbols); i++)
            {
              uint8_t *buffer1 = new uint8_t[max_size];
              memcpy (buffer1,&data_out[i * max_size],max_size);

              Ptr<Packet> pkt = Create<Packet> (buffer1,max_size);
              m_mac->NotifyRx (pkt);
              m_receiveCallback (this, pkt, type, from);

            }

        }
      else if (decoding[h1.GetGeneration ()]->is_complete () && decoded_flag[(int)h1.GetGeneration ()] == 1)
        {

          Ptr<Packet> ACK = Create<Packet> (10);
          h1.DisableCode	();
          ACK->AddHeader (h1);
          WifiNetDevice::Send (ACK,source,100 );

        }

    }

  return true;
}


bool PepWifiNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{

NS_LOG_DEBUG ( "protocol number" << protocolNumber );
NS_LOG_DEBUG ( "destination" << dest );
  if (code == 1 && protocolNumber !=2054)
    {
      NS_LOG_DEBUG ( "coding is enabled" );
      coding (packet,dest, protocolNumber);
	
    }
  else
    {
      NS_LOG_DEBUG ( "coding is disabled " );
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
      NS_LOG_DEBUG ( "sent:" << sent_packet );
      NS_LOG_DEBUG ( "Interval:" << interval);

      m_coded->k++;
      kodo::set_symbols (kodo::storage (m_coded->m_encoder_data), m_coded->m_encoder);

      vector<uint8_t> payload (m_coded->m_encoder->payload_size ());
      m_coded->m_encoder->encode ( &payload[0] );

      Ptr<Packet> pkt = Create<Packet> (&payload[0], m_coded->m_encoder->payload_size ());

      pkt->AddHeader (m_coded->h1);

      
      NS_LOG_DEBUG ( "generation number: " << m_coded->h1.GetGeneration () );

      WifiNetDevice::Send (pkt,m_coded->realTo,m_coded->protocolNumber );
      
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

  PointerValue ptr;
  GetAttribute ("Mac",ptr);
  Ptr<AdhocWifiMac> m_mac = ptr.Get<AdhocWifiMac> ();

  int k = 0;

  Enqueue1 (packet);

  if ((int)m_queue.size () == max_symbols )
    {
      encoder = m_encoder_factory.build ((max_symbols), packet->GetSize ());
      
      CodeHeader h1;
      h1.SetGeneration (generation);
      h1.SetSourceMAC(m_mac->GetAddress());
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
          NS_LOG_DEBUG ( "data in " << (i * packet->GetSize ()) );
        }

      decoded_flag[(int)h1.GetGeneration ()] = 0;
      Simulator::Schedule ( Seconds (interval),&PepWifiNetDevice::SendCode, this, m_coded);

    }
  return true;


}

}

