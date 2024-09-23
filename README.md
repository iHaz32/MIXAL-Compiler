# MIXAL-COMPILER Project

## Description
The **MIXAL-COMPILER Project** is a simple compiler that processes a custom language using Flex (for lexical analysis), Bison (for syntax analysis), and generates MIX assembly code. This project includes a symbol table, syntax tree generation, and code generation in MIXAL (MIX Assembly Language). The output from this project includes a syntax tree, a symbol table, and corresponding MIXAL code for further execution in an emulator.

### Key Components:
- **Lexical Analysis**: Handled by Flex (`lexc.l`).
- **Syntax Analysis**: Handled by Bison (`sydc.y`). Including Abstract Syntax Tree and Symbol List.
- **Code Generation**: Output is in MIX assembly (`mixal/mix_codegen.c`).
- **Emulator**: The project recommends the [MixEmul Emulator](https://github.com/rbergen/MixEmul) to run the generated MIX code.

## Installation
To install and run this project locally, you need the following dependencies:
- **Flex**: A tool for generating scanners (tokenizers).
- **Bison**: A parser generator that is part of the GNU project.
- **GCC**: The GNU Compiler Collection for compiling C programs.

### Steps to Install:
1. Clone the repository:
    ```bash
    git clone https://github.com/yourusername/MyParser.git
    ```
2. Navigate to the project directory:
    ```bash
    cd MyParser
    ```

3. Ensure you have the required tools (GCC, Flex and Bison) installed.

## How to Run

Follow these steps to build and run the parser. If you want to create your own testing files, make sure you have them ready in the `testings/` directory.

1. Open the terminal and execute the following commands on Windows:

    ```bash
    flex lexc.l
    ```

    ```bash
    bison -d sydc.y
    ```

    ```bash
    gcc -o myparser sydc.tab.c lex.yy.c symbol_table/symbol_table.c syntax_tree/syntax_tree.c mixal/mix_codegen.c zyywrap.c
    ```

2. Run the parser on a test file (replace `TEST_FILE_NAME` with the actual file name):

    ```bash
    type testings/TEST_FILE_NAME.txt | ./myparser
    ```

3. After these commands, two files will be generated:
    - **log.txt**: Contains the abstract syntax tree, symbol list, logging, and assignments.
    - **mixal.mix**: Contains the generated MIXAL code for the chosen test file.

4. To run the MIXAL code:
    - Open your MIX emulator.
    - Load the `mixal.mix` file.
    - Press the "Run" button in the emulator.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments
- Special thanks to **Donald Knuth** for creating the MIX system and assembly language.
- Thanks to the open-source community for tools like **Flex**, **Bison**, and the **MixEmul** emulator.
