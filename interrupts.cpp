/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include"interrupts.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

	int sys_time = 0;
	int isr_activity_time = 200;
	int save_restore_context_time = 10;
	int set_step_value = 1;
	/*set_step_value is used for mode switching, memory start calculation,
	getting ISR address, and IRET execution*/
	std::string type;
	unsigned int line_number = 1;
	
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
	if (activity.compare("CPU") == 0) {
		type = "CPU Burst\n";	
		execution += std::to_string(sys_time) + ", " + std::to_string(duration_intr) + ", " + type;
		sys_time += duration_intr;
	} else if (activity.compare("SYSCALL") == 0){
		int device_number = duration_intr - 1;
		if (device_number >= std::min(delays.size(), vectors.size()) || device_number< 0) {
				std::cout << "Line "<< line_number << "\nInvalid device number: " << device_number
				<< "\nDevice number must be between 0 and " << std::min(delays.size(), vectors.size()) << std::endl;
		}
		auto p = intr_boilerplate(sys_time, device_number, save_restore_context_time, vectors); 
		execution += std::get<0>(p);
		sys_time = std::get<1>(p);
		int isr_duration = delays.at(device_number);
		execution += std::to_string(sys_time) + ", " + std::to_string(isr_activity_time) + ", " + "SYSCALL: run the ISR (device driver)\n";
		sys_time += isr_activity_time;
		execution += std::to_string(sys_time) + ", " + std::to_string(isr_activity_time) + ", " + "transfer data from device to memory\n";
		sys_time += isr_activity_time;
		if (3 * isr_activity_time < isr_duration) {
			int remaining_time = (isr_duration - isr_activity_time);
			execution += std::to_string(sys_time) + ", " + std::to_string(remaining_time) + ", " + "check for errors\n";
			sys_time += remaining_time;
		} else {
			execution += std::to_string(sys_time) + ", " + std::to_string(isr_activity_time) + ", " + "check for errors\n";
			sys_time += isr_activity_time;
		}
		execution += std::to_string(sys_time) + ", " + std::to_string(1) + ", " + "IRET\n";
		sys_time++;
	} else if (activity.compare("END_IO") == 0) {
		int device_number = duration_intr - 1;
		if (device_number >= std::min(delays.size(), vectors.size()) || device_number < 0) {
				std::cout << "Line "<< line_number << "\nInvalid device number: " << device_number
				<< "\nDevice number must be between 0 and " << std::min(delays.size(), vectors.size()) << std::endl;
		}
		auto p = intr_boilerplate(sys_time, device_number, save_restore_context_time, vectors); 
		execution += std::get<0>(p);
		sys_time = std::get<1>(p);
		int isr_duration = delays.at(device_number);
		execution += std::to_string(sys_time) + ", " + std::to_string(isr_activity_time) + ", " + "ENDIO: run the ISR (device driver)\n";
		sys_time += isr_activity_time;
		if (isr_activity_time * 2 < isr_duration) {
			int remaining_time = (isr_duration - isr_activity_time);
			execution += std::to_string(sys_time) + ", " + std::to_string(remaining_time) + ", " + "check device status\n";
			sys_time += remaining_time;
		} else {
			execution += std::to_string(sys_time) + ", " + std::to_string(isr_activity_time) + ", " + "check device status\n";
			sys_time += isr_activity_time;
		}
		execution += std::to_string(sys_time) + ", " + std::to_string(set_step_value) + ", " + "IRET\n";
		sys_time++;
	} else {
		std::cout << "Line: " << line_number << "\nInvalid activity: " << activity
		<< "\nActivity must be one of 'CPU', 'SYSCALL', or 'END_IO." << std::endl;
	}
	line_number++;
        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
