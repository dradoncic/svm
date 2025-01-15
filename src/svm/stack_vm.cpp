#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>

using namespace std;

enum class OpCode {
    PUSH, POP, ADD, SUB, MUL, DIV, MOD,
    LOAD, STORE, JMP, JZ, JNZ, CALL, RET,
    PRINT, HALT,
    DUP,    // Duplicate top stack value
    SWAP,   // Swap top two stack values
    CMP     // Compare top two values and push result (-1, 0, 1)
};

struct Instruction {
    OpCode opcode;
    vector<int> operands;
    
    // Constructor for convenience
    Instruction(OpCode op, vector<int> ops = {}) : opcode(op), operands(ops) {}
};

class Stack {
private:
    vector<int> stack;
    static const size_t MAX_SIZE = 1024;  // Stack size limit

public:
    void push(int value) {
        if (stack.size() >= MAX_SIZE) {
            throw runtime_error("Stack Overflow");
        }
        stack.push_back(value);
    }

    int pop() {
        if (stack.empty()) throw runtime_error("Stack Underflow");
        int value = stack.back();
        stack.pop_back();
        return value;
    }

    int peek() const {
        if (stack.empty()) throw runtime_error("Stack is empty");
        return stack.back();
    }

    void dup() {
        if (stack.empty()) throw runtime_error("Stack Underflow");
        push(peek());
    }

    void swap() {
        if (stack.size() < 2) throw runtime_error("Stack Underflow");
        int a = pop();
        int b = pop();
        push(a);
        push(b);
    }

    bool empty() const {
        return stack.empty();
    }

    size_t size() const {
        return stack.size();
    }
};

class Memory {
private:
    unordered_map<int, int> memory;
    static const int MAX_ADDRESS = 65535;  // Memory address limit

public:
    void store(int address, int value) {
        if (address < 0 || address > MAX_ADDRESS) {
            throw runtime_error("Memory address out of bounds");
        }
        memory[address] = value;
    }

    int load(int address) const {
        if (address < 0 || address > MAX_ADDRESS) {
            throw runtime_error("Memory address out of bounds");
        }
        auto it = memory.find(address);
        if (it == memory.end()) return 0;  // Return 0 for uninitialized memory
        return it->second;
    }
};

class ExecutionEngine {
private:
    vector<Instruction> program;
    Stack stack;
    Memory memory;
    Stack call_stack;  // For implementing CALL/RET
    size_t pc = 0;
    bool running = true;
    static const size_t MAX_CALL_DEPTH = 256;

    void execute(const Instruction& instr) {
        try {
            switch (instr.opcode) {
                case OpCode::PUSH:
                    stack.push(instr.operands[0]);
                    break;
                case OpCode::POP:
                    stack.pop();
                    break;
                case OpCode::ADD: {
                    int b = stack.pop();
                    int a = stack.pop();
                    stack.push(a + b);
                    break;
                }
                case OpCode::SUB: {
                    int b = stack.pop();
                    int a = stack.pop();
                    stack.push(a - b);
                    break;
                }
                case OpCode::MUL: {
                    int b = stack.pop();
                    int a = stack.pop();
                    stack.push(a * b);
                    break;
                }
                case OpCode::DIV: {
                    int b = stack.pop();
                    if (b == 0) throw runtime_error("Division by zero");
                    int a = stack.pop();
                    stack.push(a / b);
                    break;
                }
                case OpCode::MOD: {
                    int b = stack.pop();
                    if (b == 0) throw runtime_error("Modulo by zero");
                    int a = stack.pop();
                    stack.push(a % b);
                    break;
                }
                case OpCode::LOAD:
                    stack.push(memory.load(instr.operands[0]));
                    break;
                case OpCode::STORE:
                    memory.store(instr.operands[0], stack.pop());
                    break;
                /*
                case OpCode::JMP:
                    pc = instr.operands[0] - 1;  // -1 because pc++ happens after execute
                    break;
                case OpCode::JZ: {
                    if (stack.pop() == 0) {
                        pc = instr.operands[0] - 1;
                    }
                    break;
                }
                case OpCode::JNZ: {
                    if (stack.pop() != 0) {
                        pc = instr.operands[0] - 1;
                    }
                    break;
                }
                case OpCode::CALL: {
                    if (call_stack.size() >= MAX_CALL_DEPTH) {
                        throw runtime_error("Call stack overflow");
                    }
                    call_stack.push(pc);
                    pc = instr.operands[0] - 1;
                    break;
                }
                case OpCode::RET: {
                    if (call_stack.empty()) {
                        throw runtime_error("Return without call");
                    }
                    pc = call_stack.peek();
                    call_stack.pop();
                    break;
                }
                */
                case OpCode::DUP:
                    stack.dup();
                    break;
                case OpCode::SWAP:
                    stack.swap();
                    break;
                case OpCode::CMP: {
                    int b = stack.pop();
                    int a = stack.pop();
                    stack.push((a < b) ? -1 : (a > b) ? 1 : 0);
                    break;
                }
                case OpCode::PRINT:
                    cout << stack.pop() << endl;
                    break;
                case OpCode::HALT:
                    running = false;
                    break;
                default:
                    throw runtime_error("Unknown instruction");
            }
        } catch (const runtime_error& e) {
            cerr << "Runtime error at PC=" << pc << ": " << e.what() << endl;
            running = false;
        }
    }

public:
    void load_program(const vector<Instruction>& bytecode) {
        program = bytecode;
        pc = 0;
        running = true;
        while (!call_stack.empty()) call_stack.pop();
    }

    void run() {
        while (running && pc < program.size()) {
            execute(program[pc]);
            pc++;
        }
    }

    // Debug helper
    void dump_stack() const {
        cout << "Stack size: " << stack.size() << endl;
        if (!stack.empty()) {
            cout << "Top of stack: " << stack.peek() << endl;
        }
    }
};

class VirtualMachine {
private:
    ExecutionEngine engine;

public:
    void load_program(const vector<Instruction>& program) {
        engine.load_program(program);
    }

    void run() {
        engine.run();
    }

    void dump_state() {
        engine.dump_stack();
    }
};

// Example Usage
int main() {
    VirtualMachine vm;
    
    // Calculate factorial of 5 using loops
    vector<Instruction> program = {
        {OpCode::PUSH, {5}},
        {OpCode::STORE, {0}},
        {OpCode::PUSH, {10}},
        {OpCode::STORE, {1}},
        {OpCode::LOAD, {0}},
        {OpCode::LOAD, {1}},
        {OpCode::ADD, {}},
        {OpCode::PRINT, {}},
        {OpCode::HALT, {}}
    };

    vm.load_program(program);
    vm.run();

    return 0;
}