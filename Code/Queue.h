/*
	Queue
*/

#include "FIFO.h"

component Queue : public TypeII
{
	public:

		void Setup();
		void Start();
		void Stop();

    void checkTransmission();

	public:

		// Connections
		inport void in(Packet &packet);
		outport void out(Packet &packet);
		outport void trace(char* input);

    inport void getChannelStatus(int new_channel_status, int transmitter_id);
    outport void modifyChannelStatus(int occupy_or_release, int transmitter_id);

		// Timer for transmissions
		Timer <trigger_t> service_time;
		inport inline void endService(trigger_t& t);

    // Timer for AIFS check
    Timer <trigger_t> aifs_check;
		inport inline void aifsEnd(trigger_t& t);

    // Timer for the backoff
    Timer <trigger_t> backoff_counter;
		inport inline void backoffEnd(trigger_t& t);
    
		Queue () { 
      connect service_time.to_component,endService; 
      connect aifs_check.to_component,aifsEnd; 
      connect backoff_counter.to_component,backoffEnd; 
    }

	public:
  
    // Channel access and transmission parameters
    // - AIFS value (in seconds)
    double aifs;
    // - Max. TXOP duration value (in seconds)
    double max_txop;
    // - CW value (in max. slots)
    int cw;
    // - Current BO value (in seconds)
    double backoff;
    // - Transmission Rate
    double rate;    
    // - Status of the channel (busy:1, idle:0)
    int channel_status;  
    // - To indicate whether a backoff has already started or not
    int backoff_started;

    // Queue parameters
    int queue_id;
    // - FIFO queue object
		FIFO fifo_queue;            
    // - Buffer size
		int buffer_size;   
		// - Blocked packets 
		int blocked_packets;
    // - Arrived packets 
		int arrived_packets;
    // - Status of the queue (prepared:1, idle:0)
    int queue_status;

    // Other simulation parameters

    // - Number of packets aggregated in a transmission
    int num_packets_aggregated;
    int total_num_packets_aggregated;
    // - Expected length of aggregated packets
    int expected_length;
    // - Auxiliary variable to keep track of the access channel time
    double timestamp_access_channel;
    // - Total data transmitted successfully in bits
    int data_transmitted;
    // - Total data generated in bits
    int data_generated;
    // - Backoff values and times computed
    int bo_values;
    int times_bo_computed;
    // - Channel occupancy time
    double total_channel_occupancy_time;
    int times_channel_occupied;
    // - Channel access time
    double timestamp_start_channel_access;
    double total_channel_access_time;
    int times_channel_accessed;

	private:
		char msg[200];

};

void Queue :: Setup()
{
    // Initialize simulation parameters
    blocked_packets = 0;
    arrived_packets = 0;
    data_transmitted = 0;
    data_generated = 0;
    bo_values = 0;
    times_bo_computed = 0;
    total_channel_occupancy_time = 0;
    times_channel_occupied = 0;
    timestamp_start_channel_access = 0;
    total_channel_access_time = 0;
    times_channel_accessed = 0;
    total_num_packets_aggregated = 0;
    backoff_started = 0;
    
};

void Queue :: Start()
{
  // DO NOTHING
};

void Queue :: Stop()
{

  // Display simulation results
	printf("\n###### Results Node N%d ######\n", queue_id);
  printf("      - Traffic generation rate = %f Mbps\n", (double)data_generated*1E-6/SimTime());
  printf("      - Mean Backoff = %f slots\n", (double)bo_values/times_bo_computed);
  printf("      - Mean channel access time = %f ms\n", 1E3*total_channel_access_time/times_channel_accessed);
  printf("      - Mean channel occupancy time = %f ms\n", 1E3*total_channel_occupancy_time/times_channel_occupied);
  printf("      - Mean num. of packets per transmission = %.2f\n", (double)total_num_packets_aggregated/times_channel_accessed);
  printf("      - Blocking Probability = %f\n", (double)blocked_packets/arrived_packets);
	//printf("     - Average number of packets in the queue E[Nq] = %f\n",queue_length/arrived_packets);
  printf("\n");

};

void Queue :: in(Packet &packet)
{

  if(LOGS_ENABLED) printf("%f N%d - New packet arrived to the queue (source_id=%d, seq_number=%li, length=%d bits). There were %d packets in the buffer.\n",
    SimTime(),queue_id,packet.source_id,packet.seq_number,packet.L,fifo_queue.QueueSize());
	sprintf(msg,"%f N%d - New packet arrived to the queue (source_id=%d, seq_number=%li, length=%d bits). There were %d packets in the buffer.",
    SimTime(),queue_id,packet.source_id,packet.seq_number,packet.L,fifo_queue.QueueSize());
	if(TRACES_ENABLED) trace(msg);

	++arrived_packets;

  data_generated += packet.L;

  // Check if the queue is full before adding the packet
  if(fifo_queue.QueueSize() < BUFFER_SIZE) {
    // Add the packet to the queue
    fifo_queue.PutPacket(packet);
  }
  else {
    ++blocked_packets;
  }

  // Check if a packet transmission can be scheduled
  checkTransmission();

};

void Queue :: checkTransmission() {
  
  if(LOGS_ENABLED) printf("%f N%d - Checking if a transmission can be scheduled (queue size = %d).\n",
    SimTime(),queue_id, fifo_queue.QueueSize());
  sprintf(msg,"%f N%d - Checking if a transmission can be scheduled (queue size = %d).",
    SimTime(),queue_id, fifo_queue.QueueSize());
  if(TRACES_ENABLED) trace(msg);

  // Check if transmissions can be scheduled
  if(fifo_queue.QueueSize() > 0 && queue_status == 0){
    
    // Compute backoff between 0 and CW[access_category]
    if(backoff_started == 0) {   
      int backoff_slots = (rand()%(cw+1));
      backoff = 9E-6*backoff_slots;
      bo_values += backoff_slots;
      ++times_bo_computed;
      backoff_started = 1;
      if(LOGS_ENABLED) printf("%f N%d - New backoff computed = %f s (%d)\n", SimTime(), queue_id, backoff, backoff_slots);
      sprintf(msg,"%f N%d - New backoff computed = %f s (%d)", SimTime(), queue_id, backoff, backoff_slots);
      if(TRACES_ENABLED) trace(msg);
        queue_status = 1;
        timestamp_start_channel_access = SimTime(); // Keep track of the time at which channel access is started to be attempted
     }    

    if (channel_status == CHANNEL_IDLE) {
      if(LOGS_ENABLED) printf("%f N%d - A new transmission can be scheduled. Start AIFS...\n", SimTime(), queue_id);
      sprintf(msg,"%f N%d - A new transmission can be scheduled. Start AIFS...", SimTime(), queue_id);
      if(TRACES_ENABLED) trace(msg);
      aifs_check.Set(SimTime() + aifs);
    }

  } 
  //else if (queue_status == 1 && waiting == 0) {
  //  // Another packet is already being tried to be served
  //  if(LOGS_ENABLED) printf("%f N%d - The scheduled transmission can be resumed. Start AIFS...\n", SimTime(), queue_id);
  //  sprintf(msg,"%f N%d - The scheduled transmission can be resumed. Start AIFS...", SimTime(), queue_id);
  //  if(TRACES_ENABLED) trace(msg);
  //  aifs_check.Set(SimTime() + aifs);
  //}
}

void Queue :: getChannelStatus(int new_channel_status, int transmitter_id){
  
  if(LOGS_ENABLED) printf("%f N%d Channel status changed to %d\n", SimTime(), queue_id, new_channel_status);
  sprintf(msg,"%f N%d Channel status changed to %d", SimTime(), queue_id, new_channel_status);
  if(TRACES_ENABLED) trace(msg);

  // Update the channel status
  channel_status = new_channel_status;

  // Nodes different to the transmitter to take action
  if(channel_status == CHANNEL_BUSY) {
      if (transmitter_id == queue_id) {
        // Do nothing
      } else {
        // Cancel the AIFS and pause the backoff
        if (aifs_check.Active()) {
          aifs_check.Cancel();
          if(LOGS_ENABLED) printf("%f N%d - Cancel AIFS\n", SimTime(), queue_id);
          sprintf(msg,"%f N%d - Cancel AIFS", SimTime(), queue_id);
          if(TRACES_ENABLED) trace(msg);
        }
        if (backoff_counter.Active()) {
          backoff = backoff_counter.GetTime() - SimTime();
          backoff_counter.Cancel();
          if(LOGS_ENABLED) printf("%f N%d - Backoff paused at %f s (%f slots)\n", SimTime(), queue_id, backoff, ceil(backoff*9e-6));
          sprintf(msg,"%f N%d - Backoff paused at %f s (%f slots)", SimTime(), queue_id, backoff, ceil(backoff*9e-6));
          if(TRACES_ENABLED) trace(msg);
        }
        // Set the queue status to 0 to indicate that no packets are attempted to be transmitted right now
        queue_status = 0;
      }
  } else if (channel_status == CHANNEL_IDLE) {
    // Re-run AIFS and later resume backoff
    checkTransmission();
  }

}

void Queue :: aifsEnd(trigger_t &) 
{
  if(LOGS_ENABLED) printf("%f N%d - AIFS ended. Resuming the backof from %f (%d slots)\n", SimTime(), queue_id, backoff, int(backoff*9e-6));
  sprintf(msg,"%f N%d - AIFS ended. Resuming the backof from %f (%d slots)", SimTime(), queue_id, backoff, int(backoff*9e-6));
  if(TRACES_ENABLED) trace(msg);
  // Resume the backoff
  backoff_counter.Set(SimTime() + backoff);
}

void Queue :: backoffEnd(trigger_t &) 
{

  if(LOGS_ENABLED) printf("%f N%d - Backoff ended.\n", SimTime(), queue_id);
  sprintf(msg,"%f N%d - Backoff ended.", SimTime(), queue_id);
  if(TRACES_ENABLED) trace(msg);
    
  // Occupy the channel (set its status as busy)
  modifyChannelStatus(CHANNEL_BUSY, queue_id);

  // Check how many packets can be transmitted from the queue
  num_packets_aggregated = 0;  
  for (int i = fifo_queue.QueueSize(); i > 0; --i) {  
    // Calculate the total transmission length by aggregating packets
    expected_length = 0; 
    for (int j = 0; j < i; ++j) {
      Packet packet = fifo_queue.GetPacketAt(j);
      expected_length += packet.L;
    }
    // Check total transmission duration accordingly
    if (expected_length/rate < max_txop) {
      num_packets_aggregated = i;
      break;
    }
  }
  
  // Keep track of statistics related to channel access
  timestamp_access_channel = SimTime();
  total_channel_access_time = timestamp_access_channel - timestamp_start_channel_access;
  total_num_packets_aggregated += num_packets_aggregated;
  ++times_channel_accessed;

  // Schedule the end of the transmission
  if(LOGS_ENABLED) printf("%f N%d - Transmitting %d packets, to be finished at %f\n", 
    SimTime(), queue_id, num_packets_aggregated, SimTime()+(expected_length/rate));
  sprintf(msg,"%f N%d - Transmitting %d packets, to be finished at %f", 
    SimTime(), queue_id, num_packets_aggregated, SimTime()+(expected_length/rate));
  if(TRACES_ENABLED) trace(msg);
  service_time.Set(SimTime()+(expected_length/rate));

}

void Queue :: endService(trigger_t &)
{

  if(LOGS_ENABLED) printf("%f N%d - Transmission finished. List of transmitted packets:\n", SimTime(), queue_id);
  sprintf(msg,"%f N%d - Transmission finished. List of transmitted packets:", SimTime(), queue_id);
  if(TRACES_ENABLED) trace(msg);
  
  for (int i = 0; i < num_packets_aggregated; ++i) {

    // Service time ends, we remove the packet from the queue, and send it to the sink module
    Packet packet = fifo_queue.GetFirstPacket();

    if(LOGS_ENABLED) printf("%f     * seq_number = %li (from source %d), length = %d bits\n", 
      SimTime(), packet.seq_number, packet.source_id, packet.L);
    sprintf(msg,"%f     * seq_number = %li (from source %d), length = %d bits", 
      SimTime(), packet.seq_number, packet.source_id, packet.L);
    if(TRACES_ENABLED) trace(msg);

    // Delete the packet from the queue
    fifo_queue.DelFirstPacket();

    // Mark the timestamp at which the packet starts to be transmitted
    packet.access_time = timestamp_access_channel;

    // Mark the timestamp at which the packet is sent
    packet.sent_time = SimTime();

    // Increase the amount of data transmitted for computing statistics
    data_transmitted += packet.L;

    // Increase the time the channel has been occupied
    total_channel_occupancy_time += SimTime() - timestamp_access_channel;
    ++times_channel_occupied;

    // Send the packet to the sink
    out(packet);
  }

  if(LOGS_ENABLED) printf("%f N%d - Updated queue size = %d\n", SimTime(),queue_id, fifo_queue.QueueSize());
  sprintf(msg,"%f N%d - Updated queue size = %d", SimTime(),queue_id, fifo_queue.QueueSize());
  if(TRACES_ENABLED) trace(msg);

  // Reset the backoff started variable
  backoff_started = 0;

  // Reset the status of the queue to "inactive"
  queue_status = 0;

  // Release the channel (set its status as idle)
  modifyChannelStatus(CHANNEL_IDLE, queue_id);
	
};
