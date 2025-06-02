#include "cache_engine.cpp"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

using std::cerr;
using std::cout;
using std::endl;
using std::FILE;
using std::ifstream;
using std::string;
using std::stringstream;

/*************************** MAIN *****************************/

int main(int argc, char **argv) {

    if (argc < 19) {
        cerr << "Not enough arguments" << endl;
        return 0;
    }

    // Get input arguments

    // File
    // Assuming it is the first argument
    char *fileString = argv[1];
    ifstream file(fileString); // input file stream
    string line;
    if (!file || !file.good()) {
        // File doesn't exist or some other error
        cerr << "File not found" << endl;
        return 0;
    }
    unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
             L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

    for (int i = 2; i < 19; i += 2) {
        string s(argv[i]);
        if (s == "--mem-cyc") {
            MemCyc = atoi(argv[i + 1]);
        } else if (s == "--bsize") {
            BSize = atoi(argv[i + 1]);
        } else if (s == "--l1-size") {
            L1Size = atoi(argv[i + 1]);
        } else if (s == "--l2-size") {
            L2Size = atoi(argv[i + 1]);
        } else if (s == "--l1-cyc") {
            L1Cyc = atoi(argv[i + 1]);
        } else if (s == "--l2-cyc") {
            L2Cyc = atoi(argv[i + 1]);
        } else if (s == "--l1-assoc") {
            L1Assoc = atoi(argv[i + 1]);
        } else if (s == "--l2-assoc") {
            L2Assoc = atoi(argv[i + 1]);
        } else if (s == "--wr-alloc") {
            WrAlloc = atoi(argv[i + 1]);
        } else {
            cerr << "Error in arguments" << endl;
            return 0;
        }
    }
    Cache_Engine myCache = Cache_Engine(MemCyc, BSize, L1Size, L2Size, L1Cyc,
                                        L2Cyc, L1Assoc, L2Assoc, WrAlloc);
    DEBUG_COMMAND(myCache.print_DEBUG());
    while (getline(file, line)) {
        stringstream ss(line);
        string address;
        char operation = 0; // read (R) or write (W)
        if (!(ss >> operation >> address)) {
            // Operation appears in an Invalid format
            cout << "Command Format error" << endl;
            return 0;
        }

        // DEBUG - remove this line
        DEBUG_COMMAND(cout << "operation: " << operation);

        string cutAddress =
            address.substr(2); // Removing the "0x" part of the address

        // DEBUG - remove this line
        DEBUG_COMMAND(cout << ", address (hex)" << cutAddress;)

        unsigned long int num = 0;
        num = strtoul(cutAddress.c_str(), NULL, 16);

        // DEBUG - remove this line
        DEBUG_COMMAND(cout << " (dec) " << num << endl);

        if (operation == 'r') {
            myCache.read_from_mem(num);
        } else {
            myCache.write_to_mem(num);
        }
        DEBUG_COMMAND(myCache.print_DEBUG());
        DEBUG_COMMAND(myCache.printSimInfo());
    }

    double L1MissRate;
    double L2MissRate;
    double avgAccTime;

    myCache.getSimInfo(L1MissRate, L2MissRate, avgAccTime);

    printf("L1miss=%.03f ", L1MissRate);
    printf("L2miss=%.03f ", L2MissRate);
    printf("AccTimeAvg=%.03f\n", avgAccTime);

    return 0;
}
