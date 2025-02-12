# Verilog Gate-Level Circuit Parser and Analyzer

## Overview
This project is a C++ program that parses Verilog files to extract gate and net (wire) information, processes the circuit structure, and performs various analyses. It helps in understanding the logical structure of a digital circuit by computing net levels, detecting feedback loops, and sorting gates based on dependency levels.

## Features
- **Verilog Parsing**: Extracts gates, nets, inputs, and outputs.
- **Netlist Construction**: Represents the circuit as a graph of connections.
- **Level Calculation**: Determines the logical depth of each net.
- **Feedback Loop Detection**: Identifies cyclic dependencies in the circuit.
- **Gate Sorting**: Sorts gates based on their dependency levels.
- **Statistics Extraction**: Computes primary inputs, primary outputs, and wire counts.
- **File Output**: Saves the parsed and processed data into structured output files.

## Installation
### Prerequisites
- C++ Compiler (G++/MSVC/Clang)
- Windows (for Windows-specific dependencies like `windows.h`)
- Git (for version control)

### Steps
1. Clone this repository:
   ```sh
   git clone https://github.com/yourusername/verilog-parser.git
   cd verilog-parser
   ```
2. Compile the program:
   ```sh
   g++ -o verilog_parser main.cpp
   ```
3. Run the program with a Verilog file:
   ```sh
   ./verilog_parser example.v
   ```

## Usage
```sh
./verilog_parser <verilog_filename>
```
Example:
```sh
./verilog_parser circuit.v
```

After execution, output files containing parsed gate and net information will be stored in the `output/` directory.

## Logic and Processing

![Flowchart of Parsing Process](images/Diagram.png)

### 1. **Parsing the Verilog File**
- Reads a Verilog file line by line.
- Identifies keywords like `input`, `output`, `wire`, and gate types (`AND`, `OR`, `XOR`, etc.).
- Constructs a data structure mapping nets and gates.

### 2. **Building the Netlist**
- Uses a `map<string, Net>` structure to store net dependencies.
- Stores driver and load connections for each net.

### 3. **Level Calculation**
- Assigns levels to nets based on dependencies using a **Breadth-First Search (BFS)** approach.
- Special handling for gates like `AOI`, `OAI`, and `NAND` with multiple inputs.

### 4. **Feedback Loop Detection**
- Checks if any net's input level is greater than its output level, indicating cyclic dependencies.

### 5. **Sorting Gates**
- Sorts the gates based on their assigned levels using a **custom comparator**.

### 6. **Statistics Calculation**
- Computes counts of **primary inputs**, **primary outputs**, and **wire connections**.

## Example Input (Verilog File)
```
module circuit (input a, b, c, output x);
    wire w1, w2;
    and g1 (w1, a, b);
    or g2 (w2, w1, c);
    not g3 (x, w2);
endmodule
```

## Example Output
**Output (Terminal Log):**
```
---------------------------------------------
Sorted order of gates by assigned level:
g1 (Gate Level: 1)
g2 (Gate Level: 2)
g3 (Gate Level: 3)
---------------------------------------------
No feedback loops detected.
```

**Generated Files (`output/` directory):**
- `gates_info.txt`: Details of each gate (name, type, level, inputs, output).
- `nets_info.txt`: Information about each net (drivers, loads, levels).
- `results.txt`: Overall statistics and netlist details.

## Contributing
Feel free to submit pull requests or report issues.

## License
This project is licensed under the MIT License.

## Contact
For queries, contact [your email] or raise an issue on GitHub.

