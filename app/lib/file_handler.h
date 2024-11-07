#ifndef file_handler_h
#define file_handler_h

#include "defaults.h"
#include "serial_comm.h"

std::string FILES_DIRECTORY = "/home/rapid/projects/rover/log/";

using DataBufferType = std::vector<std::tuple<float, int, long long, int, int, std::vector<int16_t>>>;

std::string logfilename, datfilename, picfilename;

std::string update_filename(std::string filename, int counter) {

    // Extract filename
    std::string baseName = filename.substr(0, filename.find_last_of('.'));  // Extract "log0000" part
    std::string extension = filename.substr(filename.find_last_of('.'));  // Extract ".csv" part

    // Create a new filename by appending a counter value
    std::string newFilename = baseName + "_" + std::to_string(counter) + extension;
    return newFilename;
}

std::string generate_filename(const std::string& directory) {
    int counter = 0;
    std::ostringstream oss;
    do {
        oss.str("");
        oss << directory << "/log" << std::setw(4) << std::setfill('0') << counter << ".txt";
        counter++;
    } while (std::filesystem::exists(oss.str()));
    return oss.str();
}

void SendParametersToStream() {
    std::cout << "Parameters:" << std::endl;
    for (const auto& pair : parameters) {
        const auto& key = pair.first;
        const auto& value = pair.second;
        std::cout << key << ": " << value << std::endl;
    }
};

void GetParameters(int argc, char* argv[]) {
	
    // Predefined key order
    std::vector<std::string> keyOrder = {
        "XPOSMIN", "XPOSMAX", "XSPEED", "XSTEP", "A_MODE_OFFSETMIN", "A_MODE_OFFSETMAX", 
        "A_MODE_AUTOGAIN", "A_MODE_MANUALGAIN", "A_MODE_GAINRATE", "A_MODE_FILTERTYPE", 
        "M_MODE_SCANTIME", "DOPPLER_OFFSETMIN", "DOPPLER_OFFSETMAX", "DOPPLER_AUTOGAIN", 
        "DOPPLER_MANUALGAIN", "DOPPLER_FILTERTYPE", "A_MODE_PORT", "DOPPLER_PORT", 
        "A_MODE_TXPAT", "DOPPLER_TXPAT", "DOPPLER_ANGLE", "MODE", 
        "A_MODE_SCANLINES", "DOPPLER_SCANLINES", "COMMENT", "IS_CONFIGURED"
    };

    // Read variables from command line, in the defined order
    for (size_t i = 0; i < keyOrder.size() && i < static_cast<size_t>(argc) - 1; ++i) {
        const std::string& key = keyOrder[i]; 
        parameters[key] = std::string(argv[i + 1]); 
    }

    // Lookup strings in the table
    if (compareStrings(parameters["MODE"], "A-MODE") || compareStrings(parameters["MODE"], "M-MODE")) {
        printf("A-MODE / M-MODE\n");
        IsDopplerMode = false;
        short int lookupResult = lookupString(parameters["A_MODE_TXPAT"], translationTable);
        txpat = lookupResult | BSCAN; // set lowest bit to signal B scan mode to the fpga
        lines = stringToInt(parameters["A_MODE_SCANLINES"]);
        offsetmin = stringToInt(parameters["A_MODE_OFFSETMIN"]);
        offsetmax = stringToInt(parameters["A_MODE_OFFSETMAX"]);
        filsel = lookupString(parameters["A_MODE_FILTERTYPE"], translationTable);
        manualgain = stringToInt(parameters["A_MODE_MANUALGAIN"]);
        gainrate = stringToInt(parameters["A_MODE_GAINRATE"]);
    };

    if (compareStrings(parameters["MODE"], "DOPPLER")) {
        printf("DOPPLER\n");
        IsDopplerMode = true;
        short int lookupResult = lookupString(parameters["DOPPLER_TXPAT"], translationTable);
        txpat = lookupResult; // |DOPPL, but DOPPLER is 0 so no need to change lowest bit.
        lines = stringToInt(parameters["DOPPLER_SCANLINES"]);
        offsetmin = stringToInt(parameters["DOPPLER_OFFSETMIN"]);
        offsetmax = stringToInt(parameters["DOPPLER_OFFSETMAX"]);
        filsel = lookupString(parameters["DOPPLER_FILTERTYPE"], translationTable);
        manualgain = stringToInt(parameters["DOPPLER_MANUALGAIN"]);
        angle = stringToInt(parameters["DOPPLER_ANGLE"]);
    };

    xposmin   = stringToFloat(parameters["XPOSMIN"]);
    xposmax   = stringToFloat(parameters["XPOSMAX"]);
    xspeed    = stringToFloat(parameters["XSPEED"]);
    xstep       = stringToFloat(parameters["XSTEP"]);
    scanning_time = stringToFloat(parameters["M_MODE_SCANTIME"]);
    configured = stringToBool(parameters["IS_CONFIGURED"]);
};

int saveParameters(int argc, char* argv[]){

    // Read default values 
    parameters = loadDefaultParams(default_param_path);

    // Read incoming parameters
    GetParameters(argc, argv);    

    // Ensure the data directory exists
    std::string directory = FILES_DIRECTORY;
    if (!std::filesystem::exists(directory)) {
        std::cout << "Debug: Creating directory " << directory << "\n";
        std::filesystem::create_directory(directory);
    };

    // Generate a unique filename
    std::string filename = generate_filename(directory);
    logfilename = filename;
    datfilename = filename;
    picfilename = filename;
    replaceAll(datfilename, "\\log", "\\dat");
    replaceAll(datfilename, ".txt", ".csv");
    replaceAll(datfilename, "\\log", "\\pic");
    replaceAll(datfilename, ".txt", ".bmp");


    // Open the file
    std::ofstream outfile(logfilename);
    if (!outfile) {
        std::cout << "Error: Could not open file " << logfilename << " for writing\n";
        return 1;
    };

    // Redirect std::cout to the file
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(outfile.rdbuf());

    // Display parameters to the file
    SendParametersToStream();

    // Restore std::cout
    std::cout.rdbuf(coutbuf);
    
    std::cout << "Parameters saved to " << logfilename << "\n";
    return 0;

}

int save_data(const DataBufferType& dataBuffer, bool moving=false) {
   
    // Open CSV file
    std::ofstream csvFile(datfilename);
    if (!csvFile.is_open()) {
	    std::cout << "Failed to open the CSV file for writing." << std::endl;
	    return 1;
    }
    
    // Write header
    csvFile << "XPOS,STRATEG,TIME,DACVAL,OFFSET";
    
    // Set buffer size
    size_t BUFFER_SIZE = A_MODE_BUFLEN;
    if (IsDopplerMode) BUFFER_SIZE = ARRAY_SIZE;
	    
    // Write header
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
	    csvFile << ",D[" << i << "]";
    }
    csvFile << "\n";
    
    // Log data
    //float ip0 = -10000.0;
    float pos = pos_final;
    for (const auto& entry : dataBuffer) {
	    
	    // Log variables
	    if (moving) {	
		    float tm = std::get<2>(entry) / 1000.0;
		    float ip = interpolatePosition(tm);
		    //if (abs(ip0-ip) < 0.01) break;
		    //ip0 = ip;
		    pos = ip;
	    }
	    csvFile << pos << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
	    
	    // Log data points
	    const auto& rawData = std::get<5>(entry);
	    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
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
