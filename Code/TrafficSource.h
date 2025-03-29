/*
	TrafficSource 
*/

component TrafficSource : public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:
  
		// Connections
		outport void out(Packet &packet);
		outport void trace(char* input);

		// Timer
		Timer <trigger_t> inter_packet_timer;
		inport inline void new_packet(trigger_t& t); // action that takes place when timer expires

		TrafficSource () { connect inter_packet_timer.to_component,new_packet; }

	public:
		long seq_number; 
		double bandwidth;     // Source bandwidth
		double packet_generation_rate;
		double L;
		int source_id;
		int access_category;

	private:
		char msg[100];
};

void TrafficSource :: Setup()
{	

};

void TrafficSource :: Start()
{

  // Start with the first packet
	seq_number = 0;
	packet_generation_rate = bandwidth/L;

  // Generate the first packet and set the timer
  inter_packet_timer.Set(Exponential(1/packet_generation_rate));

};

void TrafficSource :: Stop()
{
	// Nothing here!
};

void TrafficSource :: new_packet(trigger_t &)
{
	Packet packet;

	// Packet size (Markovian)
  packet.L = (int) Exponential(L);

  // Mark the timestamp at which the packet is generated
  packet.generated_time = SimTime();

	packet.seq_number = seq_number;
	seq_number++;
	
	packet.source_id = source_id;
	packet.access_category = access_category;
	
  if(LOGS_ENABLED) printf("%f S%d - New packet generated (seq_number=%li) of length %d bits\n",
    SimTime(),source_id, packet.seq_number, packet.L);
	sprintf(msg,"%f S%d - New packet generated (seq_number=%li) of length %d bits",
    SimTime(),source_id, packet.seq_number, packet.L);
	if(TRACES_ENABLED) trace(msg);
	
	out(packet);
	
	// Time until next packet is generated (Markovian)
  inter_packet_timer.Set(SimTime()+Exponential(1/packet_generation_rate));

};
