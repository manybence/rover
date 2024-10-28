#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pigpio.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <signal.h>
//#include <unistd.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <fstream>
//#include <arpa/inet.h>
#include <vector>
//#include <cstring>
#include <chrono>
#include <tuple>
//#include "filt.h"
//#include "bmp_gen.h"
#include "gpio_handler.h"
#include "signal_processing.h"
#include "serial_comm.h"
#include <iomanip>
#include <filesystem>
#include <cstdio>
#include <map>
#include <algorithm>
#include <string>

// Default values
const std::string XPOSMIN_DEF = "0.0";
const std::string XPOSMAX_DEF = "38.0";
const std::string ZPOSMIN_DEF = "0.0";
const std::string ZPOSMAX_DEF = "4.0";
const std::string A_MODE_OFFSETMIN_DEF = "5";
const std::string A_MODE_OFFSETMAX_DEF = "42";
const std::string A_MODE_AUTOGAIN_DEF = "true";
const std::string A_MODE_MANUALGAIN_DEF = "700";
const std::string A_MODE_GAINRATE_DEF = "10";
const std::string A_MODE_FILTERTYPE_DEF = "BPF";
const std::string DOPPLER_OFFSETMIN_DEF = "5";
const std::string DOPPLER_OFFSETMAX_DEF = "42";
const std::string DOPPLER_AUTOGAIN_DEF = "true";
const std::string DOPPLER_MANUALGAIN_DEF = "700";
const std::string DOPPLER_FILTERTYPE_DEF = "BPF";
const std::string DOPPLER_SCANLINES_DEF = "200";
const std::string A_MODE_PORT_DEF = "X2";
const std::string DOPPLER_PORT_DEF = "X2";
const std::string A_MODE_TXPAT_DEF = "10 MHz 8 Pulses";
const std::string DOPPLER_TXPAT_DEF = "4 MHz 6 Pulses";
const std::string DOPPLER_ANGLE_DEF = "0";
const std::string MODE_DEF = "A-MODE";
const std::string XSTEP_DEF = "0.1";
const std::string A_MODE_SCANLINES_DEF = "100";
const std::string NEEDLEPOS_DEF = "0.0";
const std::string COMMENT_DEF = "No comment";

std::string generate_filename(const std::string& directory) {
    int counter = 0;
    std::ostringstream oss;
    do {
        oss.str("");
        oss << directory << "/out" << std::setw(4) << std::setfill('0') << counter << ".txt";
        counter++;
    } while (std::filesystem::exists(oss.str()));
    return oss.str();
}

int main(int argc, char* argv[]) {
    std::cerr << "Debug: Starting main function\n";
    // Parameters with default values
    std::string XPOSMIN = XPOSMIN_DEF;
    std::string XPOSMAX = XPOSMAX_DEF;
    std::string ZPOSMIN = ZPOSMIN_DEF;
    std::string ZPOSMAX = ZPOSMAX_DEF;

    std::string A_MODE_OFFSETMIN = A_MODE_OFFSETMIN_DEF;
    std::string A_MODE_OFFSETMAX = A_MODE_OFFSETMAX_DEF;
    std::string A_MODE_AUTOGAIN = A_MODE_AUTOGAIN_DEF;
    std::string A_MODE_MANUALGAIN = A_MODE_MANUALGAIN_DEF;
    std::string A_MODE_GAINRATE = A_MODE_GAINRATE_DEF;
    std::string A_MODE_FILTERTYPE = A_MODE_FILTERTYPE_DEF;

    std::string DOPPLER_OFFSETMIN = DOPPLER_OFFSETMIN_DEF;
    std::string DOPPLER_OFFSETMAX = DOPPLER_OFFSETMAX_DEF;
    std::string DOPPLER_AUTOGAIN = DOPPLER_AUTOGAIN_DEF;
    std::string DOPPLER_MANUALGAIN = DOPPLER_MANUALGAIN_DEF;
    std::string DOPPLER_FILTERTYPE = DOPPLER_FILTERTYPE_DEF;

    std::string A_MODE_PORT = A_MODE_PORT_DEF;
    std::string DOPPLER_PORT = DOPPLER_PORT_DEF;
    std::string A_MODE_TXPAT = A_MODE_TXPAT_DEF;
    std::string DOPPLER_TXPAT = DOPPLER_TXPAT_DEF;
    std::string DOPPLER_ANGLE = DOPPLER_ANGLE_DEF;
    std::string MODE = MODE_DEF;
    std::string XSTEP = XSTEP_DEF;
    std::string A_MODE_SCANLINES = A_MODE_SCANLINES_DEF;
    std::string DOPPLER_SCANLINES = DOPPLER_SCANLINES_DEF;
    std::string NEEDLEPOS = NEEDLEPOS_DEF;
    std::string COMMENT = COMMENT_DEF;

    // Override defaults with command-line arguments
    if (argc >  1) XPOSMIN              = std::string(argv[ 1]);
    if (argc >  2) XPOSMAX              = std::string(argv[ 2]);
    if (argc >  3) ZPOSMIN              = std::string(argv[ 3]);
    if (argc >  4) ZPOSMAX              = std::string(argv[ 4]);
    if (argc >  5) A_MODE_OFFSETMIN     = std::string(argv[ 5]);
    if (argc >  6) A_MODE_OFFSETMAX     = std::string(argv[ 6]);
    if (argc >  7) A_MODE_AUTOGAIN      = std::string(argv[ 7]);
    if (argc >  8) A_MODE_MANUALGAIN    = std::string(argv[ 8]);
    if (argc >  9) A_MODE_GAINRATE      = std::string(argv[ 9]);
    if (argc > 10) A_MODE_FILTERTYPE    = std::string(argv[10]);
    if (argc > 11) DOPPLER_OFFSETMIN    = std::string(argv[11]);
    if (argc > 12) DOPPLER_OFFSETMAX    = std::string(argv[12]);
    if (argc > 13) DOPPLER_AUTOGAIN     = std::string(argv[13]);
    if (argc > 14) DOPPLER_MANUALGAIN   = std::string(argv[14]);
    if (argc > 15) DOPPLER_FILTERTYPE   = std::string(argv[15]);
    if (argc > 16) A_MODE_PORT          = std::string(argv[16]);
    if (argc > 17) DOPPLER_PORT         = std::string(argv[17]);
    if (argc > 18) A_MODE_TXPAT         = std::string(argv[18]);
    if (argc > 19) DOPPLER_TXPAT        = std::string(argv[19]);
    if (argc > 20) DOPPLER_ANGLE        = std::string(argv[20]);
    if (argc > 21) MODE                 = std::string(argv[21]);
    if (argc > 22) XSTEP                = std::string(argv[22]);
    if (argc > 23) A_MODE_SCANLINES     = std::string(argv[23]);
    if (argc > 24) DOPPLER_SCANLINES    = std::string(argv[24]);
    if (argc > 25) NEEDLEPOS            = std::string(argv[25]);
    if (argc > 26) COMMENT              = std::string(argv[26]);

    // Display parameters
    std::cerr << "Debug: Displaying parameters to stdout\n";
    std::cout << "Parameters:"                                  << std::endl;
    std::cout << "XPOSMIN: "            << XPOSMIN              << std::endl;
    std::cout << "XPOSMAX: "            << XPOSMAX              << std::endl;
    std::cout << "ZPOSMIN: "            << ZPOSMIN              << std::endl;
    std::cout << "ZPOSMAX: "            << ZPOSMAX              << std::endl;
    std::cout << "A_MODE_OFFSETMIN: "   << A_MODE_OFFSETMIN     << std::endl;
    std::cout << "A_MODE_OFFSETMAX: "   << A_MODE_OFFSETMAX     << std::endl;
    std::cout << "A_MODE_AUTOGAIN: "    << A_MODE_AUTOGAIN      << std::endl;
    std::cout << "A_MODE_MANUALGAIN: "  << A_MODE_MANUALGAIN    << std::endl;
    std::cout << "A_MODE_GAINRATE: "    << A_MODE_GAINRATE      << std::endl;
    std::cout << "A_MODE_FILTERTYPE: "  << A_MODE_FILTERTYPE    << std::endl;
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
    std::cout << "XSTEP: "              << XSTEP                << std::endl;
    std::cout << "A-MODE SCANLINES: "   << A_MODE_SCANLINES     << std::endl;
    std::cout << "DOPPLER SCANLINES: "  << DOPPLER_SCANLINES    << std::endl;
    std::cout << "NEEDLEPOS: "          << NEEDLEPOS            << std::endl;
    std::cout << "COMMENT: "            << COMMENT              << std::endl;

    // Ensure the data directory exists
    std::string directory = "data";
    if (!std::filesystem::exists(directory)) {
        std::cerr << "Debug: Creating directory " << directory << "\n";
        std::filesystem::create_directory(directory);
    }

    // Generate a unique filename
    std::cerr << "Debug: Generating filename\n";
    std::string filename = generate_filename(directory);

    // Open the file
    std::cerr << "Debug: Opening file: " << filename << "\n";
    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "Error: Could not open file " << filename << " for writing\n";
        return 1;
    }

    // Redirect std::cout to the file
    std::cerr << "Debug: Redirecting stdout to file\n";
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(outfile.rdbuf());

    // Display parameters to the file
    std::cout << "Parameters:"                                  << std::endl;
    std::cout << "XPOSMIN: "            << XPOSMIN              << std::endl;
    std::cout << "XPOSMAX: "            << XPOSMAX              << std::endl;
    std::cout << "ZPOSMIN: "            << ZPOSMIN              << std::endl;
    std::cout << "ZPOSMAX: "            << ZPOSMAX              << std::endl;
    std::cout << "A_MODE_OFFSETMIN: "   << A_MODE_OFFSETMIN     << std::endl;
    std::cout << "A_MODE_OFFSETMAX: "   << A_MODE_OFFSETMAX     << std::endl;
    std::cout << "A_MODE_AUTOGAIN: "    << A_MODE_AUTOGAIN      << std::endl;
    std::cout << "A_MODE_MANUALGAIN: "  << A_MODE_MANUALGAIN    << std::endl;
    std::cout << "A_MODE_GAINRATE: "    << A_MODE_GAINRATE      << std::endl;
    std::cout << "A_MODE_FILTERTYPE: "  << A_MODE_FILTERTYPE    << std::endl;
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
    std::cout << "XSTEP: "              << XSTEP                << std::endl;
    std::cout << "A-MODE SCANLINES: "   << A_MODE_SCANLINES     << std::endl;
    std::cout << "DOPPLER SCANLINES: "  << DOPPLER_SCANLINES    << std::endl;
    std::cout << "NEEDLEPOS: "          << NEEDLEPOS            << std::endl;
    std::cout << "COMMENT: "            << COMMENT              << std::endl;

    // Restore std::cout
    std::cout.rdbuf(coutbuf);
    std::cerr << "Debug: Restored stdout\n";

    return 0;
}
