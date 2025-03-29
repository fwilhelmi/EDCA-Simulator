/*
	Tools
*/

// Global variables

#define LOGS_ENABLED 0
#define TRACES_ENABLED 0

// - DATA RATE PER DEVICE
#define DATA_RATE 86E6

// - Number of queues
#define NUMBER_OF_QUEUES 3 // VO, VI & BE

// - Access categories
#define AC_VO 0
#define AC_VI 1
#define AC_BE 2

// - Size of the buffer
#define BUFFER_SIZE 100

// - Bandwidth (i.e., rate at which the App generates data) for each type of application
#define B_VO 1E6
#define B_VI 10E6
#define B_BE 5E6

// - Expected packet length (in bits) for each type of application
#define EL_VO 250
#define EL_VI 1500
#define EL_BE 12000

// - States of the channel
#define CHANNEL_BUSY 1
#define CHANNEL_IDLE 0

// Packet struct
struct Packet
{
	long seq_number;          // Packet ID
	int L;                    // Packet length
  double generated_time;    // Time at which the packet is generated
  double access_time;       // Time at which the packet starts to be transmitted
	double sent_time;         // Time at which the packet is sent
	int source_id;            // Low latency (0) or Data (1)
	int access_category;      // Low latency (0) or Data (1)
};
