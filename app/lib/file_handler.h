#ifndef file_handler_h
#define file_handler_h

#include "defaults.h"
#include "serial_comm.h"

std::string FILES_DIRECTORY = "/home/rapid/projects/rover/app/data/";

using DataBufferType = std::vector<std::tuple<float, int, long long, int, int, std::vector<int16_t>>>;

std::string logfilename, datfilename, picfilename;

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
    std::cout << "Parameters:"                                  << std::endl;
    std::cout << "XPOSMIN: "            << XPOSMIN              << std::endl;
    std::cout << "XPOSMAX: "            << XPOSMAX              << std::endl;
    std::cout << "XSPEED: "             << XSPEED               << std::endl;
    std::cout << "A_MODE_OFFSETMIN: "   << A_MODE_OFFSETMIN     << std::endl;
    std::cout << "A_MODE_OFFSETMAX: "   << A_MODE_OFFSETMAX     << std::endl;
    std::cout << "A_MODE_AUTOGAIN: "    << A_MODE_AUTOGAIN      << std::endl;
    std::cout << "A_MODE_MANUALGAIN: "  << A_MODE_MANUALGAIN    << std::endl;
    std::cout << "A_MODE_GAINRATE: "    << A_MODE_GAINRATE      << std::endl;
    std::cout << "A_MODE_FILTERTYPE: "  << A_MODE_FILTERTYPE    << std::endl;
    std::cout << "M_MODE_SCANTIME: "    << M_MODE_SCANTIME      << std::endl;
    std::cout << "DOPPLER_OFFSETMIN: "  << DOPPLER_OFFSETMIN    << std::endl;
    std::cout << "DOPPLER_OFFSETMAX: "  << DOPPLER_OFFSETMAX    << std::endl;
    std::cout << "DOPPLER_AUTOGAIN: "   << DOPPLER_AUTOGAIN     << std::endl;
    std::cout << "DOPPLER_MANUALGAIN: " << DOPPLER_MANUALGAIN   << std::endl;
    std::cout << "DOPPLER_FILTERTYPE: " << DOPPLER_FILTERTYPE   << std::endl;
    std::cout << "A-MODE PORT: "        << A_MODE_PORT          << std::endl;
    std::cout << "DOPPLER PORT: "       << DOPPLER_PORT         << std::endl;
    std::cout << "A-MODE TXPAT: "       << A_MODE_TXPAT         << std::endl;
    std::cout << "DOPPLER TXPAT: "      << DOPPLER_TXPAT        << std::endl;
    std::cout << "DOPPLER ANGLE: "      << DOPPLER_ANGLE        << std::endl;
    std::cout << "MODE: "               << MODE                 << std::endl;
    std::cout << "A-MODE SCANLINES: "   << A_MODE_SCANLINES     << std::endl;
    std::cout << "DOPPLER SCANLINES: "  << DOPPLER_SCANLINES    << std::endl;
    std::cout << "COMMENT: "            << COMMENT              << std::endl;
    std::cout << "IS CONFIGURED: "      << IS_CONFIGURED        << std::endl;
};

void GetParameters(int argc, char* argv[]) {
	
    int i = 1;
    
    // Override defaults with command-line arguments
    if (argc > 24) {
        XPOSMIN              = std::string(argv[i++]);
        XPOSMAX              = std::string(argv[i++]);
        XSPEED               = std::string(argv[i++]);
        A_MODE_OFFSETMIN     = std::string(argv[i++]);
        A_MODE_OFFSETMAX     = std::string(argv[i++]);
        A_MODE_AUTOGAIN      = std::string(argv[i++]);
        A_MODE_MANUALGAIN    = std::string(argv[i++]);
        A_MODE_GAINRATE      = std::string(argv[i++]);
        A_MODE_FILTERTYPE    = std::string(argv[i++]);
        M_MODE_SCANTIME      = std::string(argv[i++]);
        DOPPLER_OFFSETMIN    = std::string(argv[i++]);
        DOPPLER_OFFSETMAX    = std::string(argv[i++]);
        DOPPLER_AUTOGAIN     = std::string(argv[i++]);
        DOPPLER_MANUALGAIN   = std::string(argv[i++]);
        DOPPLER_FILTERTYPE   = std::string(argv[i++]);
        A_MODE_PORT          = std::string(argv[i++]);
        DOPPLER_PORT         = std::string(argv[i++]);
        A_MODE_TXPAT         = std::string(argv[i++]);
        DOPPLER_TXPAT        = std::string(argv[i++]);
        DOPPLER_ANGLE        = std::string(argv[i++]);
        MODE                 = std::string(argv[i++]);
        A_MODE_SCANLINES     = std::string(argv[i++]);
        DOPPLER_SCANLINES    = std::string(argv[i++]);
        COMMENT              = std::string(argv[i++]);
        IS_CONFIGURED        = std::string(argv[i++]);
    };

    // Lookup strings in the table
    if (compareStrings(MODE,"A-MODE") || compareStrings(MODE,"M-MODE")) {
        printf("A-MODE / M-MODE\n");
        IsDopplerMode = false;
        short int lookupResult = lookupString(A_MODE_TXPAT, translationTable);
        txpat = lookupResult | BSCAN; // set lowest bit to signal B scan mode to the fpga
        lines = stringToInt(A_MODE_SCANLINES);
        offsetmin = stringToInt(A_MODE_OFFSETMIN);
        offsetmax = stringToInt(A_MODE_OFFSETMAX);
        filsel = lookupString(A_MODE_FILTERTYPE, translationTable);
        manualgain = stringToInt(A_MODE_MANUALGAIN);
        gainrate = stringToInt(A_MODE_GAINRATE);
    };

    if (compareStrings(MODE,"DOPPLER")) {
        printf("DOPPLER\n");
        IsDopplerMode = true;
        short int lookupResult = lookupString(DOPPLER_TXPAT, translationTable);
        txpat = lookupResult; // |DOPPL, but DOPPLER is 0 so no need to change lowest bit.
        lines = stringToInt(DOPPLER_SCANLINES);
        offsetmin = stringToInt(DOPPLER_OFFSETMIN);
        offsetmax = stringToInt(DOPPLER_OFFSETMAX);
        filsel = lookupString(DOPPLER_FILTERTYPE, translationTable);
        manualgain = stringToInt(DOPPLER_MANUALGAIN);
        angle = stringToInt(DOPPLER_ANGLE);
    };

    xposmin   = stringToFloat(XPOSMIN);
    xposmax   = stringToFloat(XPOSMAX);
    xspeed    = stringToFloat(XSPEED);
    scanning_time = stringToFloat(M_MODE_SCANTIME);
    configured = stringToBool(IS_CONFIGURED);
};

int saveParameters(int argc, char* argv[]){
	
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
    float pos = xposmax;
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
