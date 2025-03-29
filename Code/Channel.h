/*
	Channel
*/

component Channel : public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:

		// Inports and outports
		inport void startStopTransmission(int occupy_or_release, int transmitter_id);
    outport void channelStatusReport(int channel_status, int transmitter_id);

    // Variables
		int num_queues;
    int channel_status;
    int transmitter_id;

};

void Channel :: Setup()
{
  channel_status=0;
};

void Channel :: Start()
{
	// DO NOTHING
};

void Channel :: Stop()
{
  // DO NOTHING
};

void Channel :: startStopTransmission(int occupy_or_release, int transmitter_id)
{
  // Update channel status information
	channel_status = occupy_or_release;
  transmitter_id = transmitter_id;
  // Notify the new status of the channel to everybody
  channelStatusReport(channel_status, transmitter_id);

};
