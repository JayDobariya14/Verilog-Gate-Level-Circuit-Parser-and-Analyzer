#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

using namespace std;

struct Gate
{
    string name;            // Unique identifier for the gate
    string type;            // Type of the gate (e.g., AND, OR, NOT)
    vector<string> inputs;  // Input nets for the gate
    string output;          // Output net of the gate
    int level = -1;             // Level of the gate based on dependencies
};

struct Net
{
    string name;
    string type; // for gate type
    vector<string> drivers; // input coming towards node
    vector<string> loads; // output going through node
    int level;
};

struct NetStats {
    int totalSignals = 0;
    int primaryInputs = 0;
    int primaryOutputs = 0;
    int wireCount = 0;
};


void parseVerilogFile(const string &filename, vector<Gate> &gates, map<string, Net> &nets)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error opening file" << endl;
        return;
    }

    string line;
    while (getline(file, line))
    {
        stringstream iss(line);
        string word;
        iss >> word;

        if (word == "module" || word == "endmodule")
        {
            continue;
        }
        else if (word == "input")
        {
            while (iss >> word)
            {
                // Remove trailing commas and semicolons
                while (!word.empty() && (word.back() == ',' || word.back() == ';'))
                {
                    word.pop_back();
                }

                // Check if the word is not empty before adding to the map
                if (!word.empty())
                {
                    nets[word] = {word, "P", {}, {}, 0};
                }
            }
        }
        else if (word == "output")
        {
            while (iss >> word)
            {
                // Remove trailing commas and semicolons
                while (!word.empty() && (word.back() == ',' || word.back() == ';'))
                {
                    word.pop_back();
                }

                if (!word.empty())
                    nets[word] = {word, "", {}, {}, -1}; // Initialize output nets
            }
        }
        else if (word == "wire")
        {
            while (iss >> word)
            {
                if (word.back() == ',' || word.back() == ';')
                    word.pop_back();
                if (!word.empty())
                    nets[word] = {word, "", {}, {}, -1}; // Initialize wire nets
            }
        }
        else if (word.rfind("oai", 0) == 0 || word.rfind("aoi", 0) == 0 || word == "nand" || word == "or" || word == "not" || word == "buf" || word == "and" || word == "nor" || word == "xor" || word == "xnor")
        {
            Gate gate;
            gate.type = word;
            string out;
            vector<string> inputs;
            iss >> word;
            
            gate.name = word; // Assign the unique name to the gate

            // Read the gate connections
            while (iss >> word)
            {
                // Remove any trailing characters like ',' or ';'
                while (!word.empty() && (word == "(" || (word.back() == ',' || word.back() == ';' || word.back() == ')')))
                {
                    word.pop_back();
                }

                // Identify output and inputs
                if (out.empty())
                {
                    out = word;
                }
                else 
                {
                    inputs.push_back(word);
                }
            }

            // Store the gate in gates vector
            gate.output = out;
            gate.inputs = inputs;

            gates.push_back(gate);

            nets[out].type = gate.type;

            // Update nets       
            for (const auto &input : inputs)
            {
                nets[input].loads.push_back(out);
                nets[out].drivers.push_back(input);
            }

        }
    }

    file.close();
}


void calculateNetLevels(map<string, Net> &nets) {
    queue<string> bfsQueue;

    // Initialize levels for primary inputs and add them to the queue
    for (auto &netPair : nets) {
        Net &net = netPair.second;
        if (net.drivers.empty()) {  
            net.level = 0;
            bfsQueue.push(net.name);  
        } else {
            net.level = -1;
        }
    }

    // BFS to calculate levels for all nets
    while (!bfsQueue.empty()) {
        string currentNetName = bfsQueue.front();
        bfsQueue.pop();
        Net &currentNet = nets[currentNetName];

        // For each load (output node) of the current net
        for (const auto &loadName : currentNet.loads) {
            Net &loadNet = nets[loadName];

            // Check if all drivers of the loadNet have computed levels
            bool allDriversComputed = true;
            int maxDriverLevel = -1;
            for (const auto &driverName : loadNet.drivers) {
                if (nets[driverName].level == -1) {  // Level -1 means not computed
                    allDriversComputed = false;
                    break;
                }
                maxDriverLevel = max(maxDriverLevel, nets[driverName].level);
            }

            // If all drivers have levels, assign level to loadNet and add to the queue
            if (allDriversComputed && loadNet.level == -1) {  
                if (loadNet.type == "aoi21") {
                    // Calculate levels for each driver path specifically for aoi21
                    int level_din1 = nets[loadNet.drivers[0]].level + 3;
                    int level_din2 = nets[loadNet.drivers[1]].level + 3;
                    int level_din3 = nets[loadNet.drivers[2]].level + 2;

                    // Set the gate's output level to the maximum of these levels
                    loadNet.level = max({level_din1, level_din2, level_din3});
                } 
                else if (loadNet.type == "oai321") {
                    // Calculate levels for each part of the oai321 gate
                    int level_or1 = max({nets[loadNet.drivers[0]].level, nets[loadNet.drivers[1]].level, nets[loadNet.drivers[2]].level}) + 1;
                    int level_or2 = max(nets[loadNet.drivers[3]].level, nets[loadNet.drivers[4]].level) + 1;
                    int level_and = max({level_or1, level_or2, nets[loadNet.drivers[5]].level}) + 1;

                    // Final inverter level
                    loadNet.level = level_and + 1;
                }
                else if (loadNet.type == "oai1112") {
                    // Calculate levels for each part of the oai1112 gate
                    int level_or = max(nets[loadNet.drivers[3]].level, nets[loadNet.drivers[4]].level) + 1; // OR gate with 4th and 5th inputs
                    int level_and = max({level_or, nets[loadNet.drivers[0]].level, nets[loadNet.drivers[1]].level, nets[loadNet.drivers[2]].level}) + 1; // AND gate

                    // Final inverter level
                    loadNet.level = level_and + 1;
                }
                else {
                    loadNet.level = maxDriverLevel + 1;  // Add 1 for other gates
                }
                bfsQueue.push(loadNet.name);
            }
        }
    }

    std::cout << "---------------------------------------------" << endl;
    cout << "Net level information: " << endl;
    for (const auto &netPair : nets) {
        cout << "Net: " << netPair.first << ", Level: " << netPair.second.level << endl;
    }
}


int detectFeedbackLoops(const vector<Gate> &gates, const map<string, Net> &nets)
{
    int feedbackLoopCount = 0;

    for (const auto &gate : gates)
    {
        int outputLevel = nets.at(gate.output).level;

        for (const auto &input : gate.inputs)
        {
            int inputLevel = nets.at(input).level;
            
            if (inputLevel > outputLevel)
            {
                feedbackLoopCount++;
                break; // Count each gate with feedback only once
            }
        }
    }

    std::cout << "---------------------------------------------" << endl;
    if (feedbackLoopCount > 0)
    {
        cout << "Feedback loops detected: " << feedbackLoopCount << endl;
    }
    else
    {
        cout << "No feedback loops detected." << endl;
    }

    return feedbackLoopCount;
}


vector<Gate> levelBasedSort(vector<Gate> &gates, map<string, Net> &nets) 
{
    calculateNetLevels(nets);

    for (auto &gate : gates) 
    {
        int maxInputLevel = -1;
        for (const auto &input : gate.inputs) 
        {
            if (nets.find(input) != nets.end())
            {
                maxInputLevel = max(maxInputLevel, nets[input].level);
            }
        }
        gate.level = maxInputLevel + 1;
    }

    sort(gates.begin(), gates.end(), [](const Gate &a, const Gate &b) 
    {
        return a.level < b.level;
    });

    std::cout << "---------------------------------------------" << endl;
    cout << "Sorted order of gates by assigned level:" << endl;
    for (const auto &gate : gates) 
    {
        cout << gate.name << " (Gate Level: " << gate.level << ")" << endl;
    }

    return gates;
}


void checkAndUpdateNets(std::map<std::string, Net> &nets, int &totalSignals, int &numPrimaryInputs, int &numPrimaryOutputs)
{
    int initialSize = nets.size();
    int removedCount = 0;
    
    for (auto it = nets.begin(); it != nets.end(); )
    {
        auto& net = it->second;
        
        if (net.level == -1)
        {
            // Remove net from the map
            it = nets.erase(it);
            ++removedCount;
        }
        else
        {
            // Update counts based on the type of net
            if (net.drivers.empty() && !net.loads.empty()) // Primary input nets have no drivers
            {
                ++numPrimaryInputs;
            }
            else if (!net.drivers.empty() && net.loads.empty()) // Primary output nets have drivers but no loads
            {
                ++numPrimaryOutputs;
            }
            ++totalSignals;
            ++it;
        }
    }

    std::cout << "---------------------------------------------" << endl;
    std::cout << "Initial number of nets: " << initialSize << std::endl;
    std::cout << "Number of removed nets: " << removedCount << std::endl;
    std::cout << "Total number of signals after cleaning: " << totalSignals << std::endl;
    std::cout << "---------------------------------------------" << endl;
    std::cout << "Number of Primary Inputs: " << numPrimaryInputs << std::endl;
    std::cout << "Number of Primary Outputs: " << numPrimaryOutputs << std::endl;
    std::cout << "---------------------------------------------" << endl;
}


NetStats checkAndUpdateNets(std::map<std::string, Net> &nets)
{
    NetStats stats;
    int initialSize = nets.size();
    int removedCount = 0;
    
    for (auto it = nets.begin(); it != nets.end(); )
    {
        auto& net = it->second;
        
        if (net.level == -1)
        {
            it = nets.erase(it);
            ++removedCount;
        }
        else
        {
            if (net.drivers.empty() && !net.loads.empty())  // Primary input
            {
                ++stats.primaryInputs;
            }
            else if (!net.drivers.empty() && net.loads.empty())  // Primary output
            {
                ++stats.primaryOutputs;
            }
            else if (!net.drivers.empty() && !net.loads.empty())  // Wire
            {
                ++stats.wireCount;
            }

            ++stats.totalSignals;
            ++it;
        }
    }

    std::cout << "---------------------------------------------" << endl;
    std::cout << "Initial number of nets: " << initialSize << std::endl;
    std::cout << "Number of removed nets: " << removedCount << std::endl;
    std::cout << "Total number of signals after cleaning: " << stats.totalSignals << std::endl;
    std::cout << "---------------------------------------------" << endl;
    std::cout << "Number of Primary Inputs: " << stats.primaryInputs << std::endl;
    std::cout << "Number of Primary Outputs: " << stats.primaryOutputs << std::endl;
    std::cout << "Wire Count: " << stats.wireCount << std::endl;
    std::cout << "---------------------------------------------" << endl;

    return stats;
}


set<string> getUniqueGateTypes(const vector<Gate> &gates)
{
    set<string> uniqueGateTypes;
    for (const auto &gate : gates)
    {
        uniqueGateTypes.insert(gate.type);
    }
    return uniqueGateTypes;
}


bool writeResultsToFile(const string &outputFilePath, const set<string> &uniqueGateTypes, const map<string, Net> &nets, const NetStats &stats)
{
    ofstream outputFile(outputFilePath);
    if (!outputFile.is_open())
    {
        cerr << "Error opening output file: " << outputFilePath << endl;
        return false;
    }
    
    // Write nets information to file
    outputFile << "Net,Type,Level,fan-out,fan-in" << endl;
    outputFile << endl;
    for (const auto &net : nets)
    {
        outputFile << net.first << "," 
               << net.second.type << ","
               << net.second.level << ","
               << net.second.loads.size() << "," 
               << net.second.drivers.size() << endl;
    }

    outputFile.close();
    cout << "Results have been written to " << outputFilePath << endl;
    return true;
}


void printGates(const vector<Gate>& gates)
{
    ofstream outputFile("./output/gates_info.txt");

    if (!outputFile.is_open()) {
        cerr << "Failed to open the file." << endl;
        return;
    }

    // outputFile << "Gates Information:" << endl;
    for (const auto& gate : gates)
    {
        outputFile << "Gate Name: " << gate.name << endl;  
        outputFile << "Gate Type: " << gate.type << endl;
        outputFile << "Level: " << gate.level << endl;
        outputFile << "Output: " << gate.output << endl;
        outputFile << "Inputs: ";
        for (const auto& input : gate.inputs)
        {
            outputFile << input << " ";
        }
        outputFile << endl;
        outputFile << endl;
        // outputFile << "-------------------------" << endl;
    }

    outputFile.close();
}


void printNets(const map<string, Net>& nets)
{
    ofstream outputFile("./output/nets_info.txt");

    if (!outputFile.is_open()) {
        cerr << "Failed to open the file." << endl;
        return;
    }

    for (const auto& [netName, net] : nets)
    {
        outputFile << "Net Name: " << net.name << endl;
        outputFile << "Type: " << net.type << endl;

        outputFile << "Number of Drivers: " << net.drivers.size() << endl;
        outputFile << "Drivers: ";
        for (const auto& driver : net.drivers)
        {
            outputFile << driver << " ";
        }
        outputFile << endl;

        outputFile << "Number of Loads: " << net.loads.size() << endl;
        outputFile << "Loads: ";
        for (const auto& load : net.loads)
        {
            outputFile << load << " ";
        }
        outputFile << endl;

        outputFile << "Level: " << net.level << endl;
        outputFile << endl;
    }

    outputFile.close();
}



int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <verilog_filename>" << endl;
        return 1;
    }

    vector<Gate> gates;
    map<string, Net> nets;

    string verilogFilePath = argv[1];

    parseVerilogFile(verilogFilePath, gates, nets);

    vector<Gate> sortedGates = levelBasedSort(gates, nets); 

    detectFeedbackLoops(sortedGates, nets);

    printGates(sortedGates);
    printNets(nets);

    set<string> uniqueGateTypes = getUniqueGateTypes(gates);
    NetStats stats = checkAndUpdateNets(nets);

    std::string outputPath = "./output";
    if (!CreateDirectory(outputPath.c_str(), NULL))
    {
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
            std::cout << "Output directory already exists." << std::endl;
        }
        else
        {
            std::cerr << "Failed to create output directory." << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "Output directory created successfully." << std::endl;
    }

    std::string outputFilePath = outputPath + "\\results.txt";

    if (!writeResultsToFile(outputFilePath, uniqueGateTypes, nets, stats))
    {
        cerr << "Failed to write results to file." << endl;
        return 1;
    }

    return 0;
}

