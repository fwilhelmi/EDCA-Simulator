/*
	EdcaSim
*/

#include <stdio.h>
#include <iostream>

#include "./COST/cost.h"
#include "TrafficSource.h"
#include "Queue.h"
#include "Channel.h"
#include "Sink.h"
#include "Tools.h"
#include "Logger.h"

using namespace std;

component EdcaSim : public CostSimEng
{
	public:
		void Setup();
		void Start();		
		void Stop();

    void SetupVariablesByReadingConfigFile();
    void displayScenarioConfiguration();
		
	public:

    // Number of SOURCES of each AC
    int num_sources_vo;
    int num_sources_vi;
    int num_sources_be;
    // CW per AC
    int* cw_array;
    // AIFS per AC
    double* aifs_array;
    // Max. TXOP duration per AC
    double* max_txop_array;

		TrafficSource[] source;
		Queue[] queue;
    Channel[] channel;
		Sink sink;
		Logger log;

};

void EdcaSim :: Setup()
{

  // Initialize arrays
  cw_array = new int[NUMBER_OF_QUEUES];
  aifs_array = new double[NUMBER_OF_QUEUES];
  max_txop_array = new double[NUMBER_OF_QUEUES];

  // Get the scenario configuration by reading the config file
  SetupVariablesByReadingConfigFile();

  // Logging!
	log.collectTraces = 1;  // Collect the traces (traces.txt)
	log.collectResults = 0; // Collect the results (results.csv)
	sprintf(log.myLabels,"TS1_DELAY,TS2_DELAY,TS1_THROUGHPUT,TS2_THROUGHPUT"); // Put some labels to your results!
	
  int TOTAL_NUM_SOURCES = num_sources_vo+num_sources_vi+num_sources_be;

  // Initialize the sources of traffic (i.e., applications)
	source.SetSize(TOTAL_NUM_SOURCES);
	// - VO SOURCES: 
  for (int i = 0; i < num_sources_vo; ++i){
    source[i].source_id = i+1;
    source[i].access_category = AC_VO;
    source[i].L = EL_VO;          
    source[i].bandwidth = B_VO;   
  }
  // - VI SOURCES: 
  for (int i = num_sources_vo; i < num_sources_vo+num_sources_vi; ++i){
    source[i].source_id = i+1;
    source[i].access_category = AC_VI;
    source[i].L = EL_VI;         
    source[i].bandwidth = B_VI;     
  }
  // - BE SOURCES: 
  for (int i = num_sources_vo+num_sources_vi; i < num_sources_vo+num_sources_vi+num_sources_be; ++i){
    source[i].source_id = i+1;
    source[i].access_category = AC_BE;
    source[i].L = EL_BE;         
    source[i].bandwidth = B_BE;         
  }

  // Initialize the queues 
  queue.SetSize(NUMBER_OF_QUEUES);
  for (int i = 0; i < NUMBER_OF_QUEUES; ++i){
    // Queue
    queue[i].queue_id = i;
    queue[i].cw = cw_array[i];
    queue[i].aifs = aifs_array[i];
    queue[i].max_txop = max_txop_array[i];
    queue[i].rate = DATA_RATE;
    queue[i].buffer_size = BUFFER_SIZE;
  }

	// - Initialize the channel
  channel.SetSize(1);
  channel[0].num_queues = NUMBER_OF_QUEUES;
  
  // Connection between components

  // - Connect the sources of traffic to the queues
  for (int i = 0; i < TOTAL_NUM_SOURCES; ++i) {
      connect source[i].out,queue[source[i].access_category].in; 
      connect source[i].trace,log.trace; 
  }

  // - Connect the queues to the sink and the channel
  for (int i = 0; i < NUMBER_OF_QUEUES; ++i) {
      //  * Connections (Queue & Sink) 
      connect queue[i].out,sink.in;      
      //  * Connections (Queue & Channel)     
      connect queue[i].modifyChannelStatus,channel[0].startStopTransmission; 
      connect channel[0].channelStatusReport,queue[i].getChannelStatus; 
      //  * Connections (Queue & Traces)
	    connect queue[i].trace,log.trace; 
  }

  // - Other connections
	connect sink.trace,log.trace;   // Connections (Traces)
  connect sink.result,log.result; // Connections (Results)	

  // Display the configuration
  displayScenarioConfiguration();
	
};

void EdcaSim :: Start()
{
	// Nothing here!
}

void EdcaSim :: Stop()
{
	// Nothing here!
}

/**
 * Set up the scenario by reading the config_edca file (MS-DOS type)
 */
void EdcaSim :: SetupVariablesByReadingConfigFile() {

	const char *config_filename = "config_edca";
	char delim[] = "=";
	char *ptr;
	int ix_param = 0;
	printf("\nReading system configuration file '%s'...\n", config_filename);
	FILE* test_input_config = fopen(config_filename, "r");
	if (!test_input_config){
		printf("Config file '%s' not found!\n", config_filename);
		exit(-1);
	}
	char line_system[100];
	while (fgets(line_system, 100, test_input_config)){
		// Ignore lines with comments
		if(line_system[0] == '#') {
			continue;
		}
		// Separate the value of the parameter from the entire line
		ptr = strtok(line_system, delim);
		ptr = strtok(NULL, delim);
		// Store the parameter as a global variable
		if (ix_param == 0){
			// Number of SOURCES with AC=VO
			num_sources_vo = atoi(ptr);
		} else if (ix_param == 1) {
			// Number of SOURCES with AC=VI
			num_sources_vi = atoi(ptr);
		} else if (ix_param == 2) {
			// Number of SOURCES with AC=BE
			num_sources_be = atoi(ptr);
		} else if (ix_param == 3) {
			// CW for SOURCES with AC=VO
			cw_array[0] = atoi(ptr);
		} else if (ix_param == 4) {
			// CW for SOURCES with AC=VI
			cw_array[1] = atoi(ptr);
		} else if (ix_param == 5) {
			// CW for SOURCES with AC=BE
			cw_array[2] = atoi(ptr);
		} else if (ix_param == 6) {
			// AIFS for SOURCES with AC=VO
			aifs_array[0] = atof(ptr);
		} else if (ix_param == 7) {
			// AIFS for SOURCES with AC=VI
			aifs_array[1] = atof(ptr);
		} else if (ix_param == 8) {
			// AIFS for SOURCES with AC=BE
			aifs_array[2] = atof(ptr);
		} else if (ix_param == 9) {
			// Max. TXOP for SOURCES with AC=VO
			max_txop_array[0] = atof(ptr);
		} else if (ix_param == 10) {
			// Max. TXOP for SOURCES with AC=VI
			max_txop_array[1] = atof(ptr);
		} else if (ix_param == 11) {
			// Max. TXOP for SOURCES with AC=BE
			max_txop_array[2] = atof(ptr);
		}
		++ix_param;
	}
	fclose(test_input_config);

	printf("The simulation scenario was properly set!\n");

}

void EdcaSim :: displayScenarioConfiguration() {

  printf("-------------------------------\n");
  printf("        SCENARIO DETAILS       \n");
  printf("-------------------------------\n");
  printf("AC_VO (%d):\n", AC_VO);
  printf("  - Number of traffic sources: %d\n", num_sources_vo);
  printf("  - Bandwidth per source: %f Mbps\n", B_VO / 1E6);
  printf("  - Expected packet length: %d bits\n", EL_VO);
  printf("  - CW: %d\n", cw_array[0]);
  printf("  - AIFS: %.2f micro-seconds\n", aifs_array[0] * 1E6);
  printf("  - Max. TXOP duration: %.2f ms\n", max_txop_array[0] * 1E3);
  printf("AC_VI (%d):\n", AC_VI);
  printf("  - Number of traffic sources: %d\n", num_sources_vi);
  printf("  - Bandwidth per source: %f Mbps\n", B_VI / 1E6);
  printf("  - Expected packet length: %d bits\n", EL_VI);
  printf("  - CW: %d\n", cw_array[1]);
  printf("  - AIFS: %.2f micro-seconds\n", aifs_array[1] * 1E6);
  printf("  - Max. TXOP duration: %.2f ms\n", max_txop_array[1] * 1E3);
  printf("AC_BE (%d):\n", AC_BE);
  printf("  - Number of traffic sources: %d\n", num_sources_be);
  printf("  - Bandwidth per source: %f Mbps\n", B_BE / 1E6);
  printf("  - Expected packet length: %d bits\n", EL_BE);
  printf("  - CW: %d\n", cw_array[2]);
  printf("  - AIFS: %.2f micro-seconds\n", aifs_array[2] * 1E6);
  printf("  - Max. TXOP duration: %.2f ms\n", max_txop_array[2] * 1E3);
  printf("-------------------------------\n");

}

int main(int argc, char *argv[])
{
  // Get the random seed as an input argument
  long int seed = atof(argv[1]);   
  double sim_time = atof(argv[2]);   

	EdcaSim Simulator;
	Simulator.Seed = seed;
	Simulator.StopTime(sim_time);
	Simulator.Setup();
	Simulator.Run();

	return 0;
};
