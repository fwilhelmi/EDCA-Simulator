/*
	Sink
*/

component Sink : public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:
		// Connections
		inport void in(Packet &packet);
		outport void trace(char* input);
		outport void result(char* input);

	public:

    // Final computed statistics
    
    // - Average end-to-end delay (seconds)
    double *aggregate_e2e_delay;
    // - Aggregate queuing delay (seconds)
    double *aggregate_queuing_delay;
    // - Aggregate transmission delay (seconds)
    double *aggregate_transmission_delay;
    // - Total packets received
		int *received_packets;
    // - Aggregate packet length (total bits received)
		int *aggregate_L; 


	private:
		char msg[100];
};

void Sink :: Setup()
{

};

void Sink :: Start()
{

  
  // Initialize arrays
  aggregate_e2e_delay = new double[NUMBER_OF_QUEUES];
  aggregate_queuing_delay = new double[NUMBER_OF_QUEUES];
  aggregate_transmission_delay = new double[NUMBER_OF_QUEUES];
  received_packets = new int[NUMBER_OF_QUEUES];
  aggregate_L = new int[NUMBER_OF_QUEUES];
  for (int i = 0; i < NUMBER_OF_QUEUES; ++i) {
    aggregate_e2e_delay[i] = 0;
    aggregate_queuing_delay[i] = 0;
    aggregate_transmission_delay[i] = 0;
    received_packets[i] = 0;
    aggregate_L[i] = 0;
  }

};

void Sink :: Stop()
{

	printf("###### SINK: Results per Access Category ######\n");
  for(int i = 0; i < NUMBER_OF_QUEUES; ++i) {
    printf(" * AC%d:\n", i);
    printf("     - Number of packets received = %d\n", received_packets[i]);
    printf("     - Total bits received = %d\n", aggregate_L[i]);
    printf("     - Throughput = %.4f Mbps\n", 1E-6*aggregate_L[i]/SimTime());
    printf("     - Average end-to-end delay = %.2f ms\n", 1E3*aggregate_e2e_delay[i]/received_packets[i]);
    printf("     - Average queuing delay = %.2f ms\n", 1E3*aggregate_queuing_delay[i]/received_packets[i]);
    printf("     - Average transmission delay = %.4f ms\n", 1E3*aggregate_transmission_delay[i]/received_packets[i]);
  }
  printf("\n");

};

void Sink :: in(Packet &packet)
{

	sprintf(msg, "%f - Sink: Packet %li from source %d (AC = %d) received!",
    SimTime(), packet.seq_number, packet.source_id, packet.access_category);
	if(TRACES_ENABLED) trace(msg);

	aggregate_e2e_delay[packet.access_category] += SimTime() - packet.generated_time;
  aggregate_queuing_delay[packet.access_category] += packet.access_time - packet.generated_time;
  aggregate_transmission_delay[packet.access_category] += SimTime() - packet.access_time;
  aggregate_L[packet.access_category] += packet.L;
  ++received_packets[packet.access_category];

};
