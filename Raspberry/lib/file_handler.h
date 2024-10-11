#ifndef file_handler_h
#define file_handler_h

#include "defaults.h"
#include "serial_comm.h"

using DataBufferType = std::vector<std::tuple<float, int, long long, int, int, std::vector<int16_t>>>;

int save_data_moving(const DataBufferType& dataBuffer) {
   
    // Open CSV file
	std::ofstream csvFile(datfilename);
	if (!csvFile.is_open()) {
		std::cerr << "Failed to open the CSV file for writing." << std::endl;
		return 1;
	}
	
	// Write header
	csvFile << "XPOS,STRATEG,TIME,DACVAL,OFFSET";
	
	// Doppler mode
	if (IsDopplerMode) {
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			csvFile << ",D[" << i << "]";
		}
		csvFile << "\n";
		
		float ip0 = -10000.0;
		for (const auto& entry : dataBuffer) {
			float tm = std::get<2>(entry) / 1000.0;
			float ip = interpolatePosition(tm);
			if (abs(ip0-ip) < 0.01) break;
			csvFile << ip << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
			const auto& rawData = std::get<5>(entry);
			for (size_t i = 0; i < ARRAY_SIZE; ++i) {
				csvFile << "," << rawData[i];
			}
			csvFile << "\n";
			ip0 = ip;
		}
	} 
	
	// A-mode
	else {
		for (size_t i = 0; i < A_MODE_BUFLEN; ++i) {
			csvFile << ",D[" << i << "]";
		}
		csvFile << "\n";
		float ip0;
		ip0 = -10000.0;
		for (const auto& entry : dataBuffer) {
			float tm = std::get<2>(entry) / 1000.0;
			float ip = interpolatePosition(tm);
			if (abs(ip0-ip) < 0.01) break;
			csvFile << ip << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
			const auto& rawData = std::get<5>(entry);
			for (size_t i = 0; i < A_MODE_BUFLEN; ++i) {
				csvFile << "," << rawData[i];
			}
			csvFile << "\n";
			ip0 = ip;
		}
	}

	// Close CSV file
	csvFile.close();
	std::cout << "Data saved to: " << datfilename << std::endl;

	return 0;
}

int save_data_standing(const DataBufferType& dataBuffer) {
	
	// Open CSV file
	std::ofstream csvFile(datfilename);
	if (!csvFile.is_open()) {
		std::cerr << "Failed to open the CSV file for writing." << std::endl;
		return 1;
	}
	
	// Write header
	csvFile << "XPOS,STRATEG,TIME,DACVAL,OFFSET";
	for (size_t i = 0; i < ARRAY_SIZE; ++i) {
		csvFile << ",D[" << i << "]";
	}
	csvFile << "\n";
	
	// Log data
	for (const auto& entry : dataBuffer) {
		csvFile << 0 << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
		const auto& rawData = std::get<5>(entry);
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			csvFile << "," << rawData[i];
		}
		csvFile << "\n";
	}
	
	// Close CSV file
	csvFile.close();
	std::cout << "Data saved to: " << datfilename << std::endl;

	return 0;
}










#endif
