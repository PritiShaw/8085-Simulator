# 8085-Simulator

JU BCSEIII Systems Lab Assignment 3

### Made By following students of A3

| Roll No | Name|
| -------- | -------- |
| 001710501067| Dibyajyoti Dhar |
| 001710501070| Akash Ghosh |
| 001710501073| Rohit Rajat Chattopadhyay |
| 001710501076| Priti Shaw |
| 001810501009| Soumika Mukherjee |


### Table of Contents
1. [Assignment Details](#1-assignment-details)
2. [Simulator Manual](#2-simulator-manual)  
    a. [Features](#2a-features)  
    b. [Commands List](#2b-commands-list)  
3. [Flowchart](#3-flowchart)
4. [Structure](#4-structure)
5. [Important Functions](#5-important-functions)
6. [Bug Reporting](#6-bug-reporting)

---
### 1. Assignment Details

**a.** *Design a 8085/8086 simulator/assembler which supports macroprocessor using C language following the principle of Two pass Assembler.*  
**b.** *Show how the generated object code is linked over different functions and show the functionality of loader over that with specific memory location.*

---
### 2. Simulator Manual
### 2.a Features
1. **4MB Memory**  
    The Simulator's memory location ranges from `000` to `FFF`, thus it has **4096** memory locations to work with.
3. **Supports Macroprocessing**  
    You can use macro to define a block of code, this will make it easy to maintain your code and your code file will be shorter.  
    You can pass as many space seperated parameters as you want.  

    Example:
    ```
    ; Defination
    INCADD MACRO A B
        INC A
        ADD B
    INCADD ENDM
    
    ; Usage
    NOP
    INCADD H M
    
    ; Result
    NOP
    INC H
    ADD M
    ```
    
    **Please Note**: This step does not perform type checking.
    
3. **Retains Memory State**  
    Before exit, the state of Memory is dumped in a file which is read when the program is started again. This allows the Simulator to retain its previous state.

4. **General Purpose Registers and Flag Register**  
    There are 7 8-bit registers namely `A`,`B`,`C`,`D`,`E`,`H` and `L` . These can be paired up as `BE`,`DE` and `HL` to make 16-bit register pairs.   
    Program Counter(`PC`) and Stack Pointer(`SP`) Registers are 16-bit registers, Processor Status Word(`PSW`) is 8-bit Register
    
    

### 2.b Commands List
The simulator supports all commonly used Mnenomics used in 8085 microprocessor. It also supports **Macroprocessor operations** using keyword `macro`. All numeric inputs are treated as Hexadecimal.

**Unsupported Mnenomics:** `RIM`,`SIM`,`IN`,`OUT` and `RST` interrupts work like `HLT`.

**Supported Mnenomics**  

|Mnemonic |Opcode|Description|Notes|
|---------|--|--------------------------|-------------|
|ACI n    |CE|Add with Carry Immediate  |A=A+n+CY     |
|ADC r    |8F|Add with Carry            |A=A+r+CY(21X)|
|ADC M    |8E|Add with Carry to Memory  |A=A+[HL]+CY  |
|ADD r    |87|Add                       |A=A+r   (20X)|
|ADD M    |86|Add to Memory             |A=A+[HL]     |
|ADI n    |C6|Add Immediate             |A=A+n        |
|ANA r    |A7|AND Accumulator           |A=A&r   (24X)|
|ANA M    |A6|AND Accumulator and Memory|A=A&[HL]     |
|ANI n    |E6|AND Immediate             |A=A&n        |
|CALL a   |CD|Call unconditional        |-[SP]=PC,PC=a|
|CC a     |DC|Call on Carry             |If CY=1(18~s)|
|CM a     |FC|Call on Minus             |If S=1 (18~s)|
|CMA      |2F|Complement Accumulator    |A=~A         |
|CMC      |3F|Complement Carry          |CY=~CY       |
|CMP r    |BF|Compare                   |A-r     (27X)|
|CMP M    |BF|Compare with Memory       |A-[HL]       |
|CNC a    |D4|Call on No Carry          |If CY=0(18~s)|
|CNZ a    |C4|Call on No Zero           |If Z=0 (18~s)|
|CP a     |F4|Call on Plus              |If S=0 (18~s)|
|CPE a    |EC|Call on Parity Even       |If P=1 (18~s)|
|CPI n    |FE|Compare Immediate         |A-n          |
|CPO a    |E4|Call on Parity Odd        |If P=0 (18~s)|
|CZ a     |CC|Call on Zero              |If Z=1 (18~s)|
|DAA      |27|Decimal Adjust Accumulator|A=BCD format |
|DAD B    |09|Double Add BC to HL       |HL=HL+BC     |
|DAD D    |19|Double Add DE to HL       |HL=HL+DE     |
|DAD H    |29|Double Add HL to HL       |HL=HL+HL     |
|DAD SP   |39|Double Add SP to HL       |HL=HL+SP     |
|DCR r    |3D|Decrement                 |r=r-1   (0X5)|
|DCR M    |35|Decrement Memory          |[HL]=[HL]-1  |
|DCX B    |0B|Decrement BC              |BC=BC-1      |
|DCX D    |1B|Decrement DE              |DE=DE-1      |
|DCX H    |2B|Decrement HL              |HL=HL-1      |
|DCX SP   |3B|Decrement Stack Pointer   |SP=SP-1      |
|HLT      |76|Halt                      |Stop execution |
|INR r    |3C|Increment                 |r=r+1   (0X4)|
|INR M    |3C|Increment Memory          |[HL]=[HL]+1  |
|INX B    |03|Increment BC              |BC=BC+1      |
|INX D    |13|Increment DE              |DE=DE+1      |
|INX H    |23|Increment HL              |HL=HL+1      |
|INX SP   |33|Increment Stack Pointer   |SP=SP+1      |
|JMP a    |C3|Jump unconditional        |PC=a         |
|JC a     |DA|Jump on Carry             |If CY=1(10~s)|
|JM a     |FA|Jump on Minus             |If S=1 (10~s)|
|JNC a    |D2|Jump on No Carry          |If CY=0(10~s)|
|JNZ a    |C2|Jump on No Zero           |If Z=0 (10~s)|
|JP a     |F2|Jump on Plus              |If S=0 (10~s)|
|JPE a    |EA|Jump on Parity Even       |If P=1 (10~s)|
|JPO a    |E2|Jump on Parity Odd        |If P=0 (10~s)|
|JZ a     |CA|Jump on Zero              |If Z=1 (10~s)|
|LDA a    |3A|Load Accumulator direct   |A=[a]        |
|LDAX B   |0A|Load Accumulator indirect |A=[BC]       |
|LDAX D   |1A|Load Accumulator indirect |A=[DE]       |
|LHLD a   |2A|Load HL Direct            |HL=[a]       |
|LXI B,nn |01|Load Immediate BC         |BC=nn        |
|LXI D,nn |11|Load Immediate DE         |DE=nn        |
|LXI H,nn |21|Load Immediate HL         |HL=nn        |
|LXI SP,nn|31|Load Immediate Stack Ptr  |SP=nn        |
|MOV r1,r2|7F|Move register to register |r1=r2   (1XX)|
|MOV M,r  |77|Move register to Memory   |[HL]=r  (16X)|
|MOV r,M  |7E|Move Memory to register   |r=[HL]  (1X6)|
|MVI r,n  |3E|Move Immediate            |r=n     (0X6)|
|MVI M,n  |36|Move Immediate to Memory  |[HL]=n       |
|NOP      |00|No Operation              |             |
|ORA r    |B7|Inclusive OR Accumulator  |A=Avr   (26X)|
|ORA M    |B6|Inclusive OR Accumulator  |A=Av[HL]     |
|ORI n    |F6|Inclusive OR Immediate    |A=Avn        |
|PCHL     |E9|Jump HL indirect          |PC=[HL]      |
|POP B    |C1|Pop BC                    |BC=[SP]+     |
|POP D    |D1|Pop DE                    |DE=[SP]+     |
|POP H    |E1|Pop HL                    |HL=[SP]+     |
|POP PSW  |F1|Pop Processor Status Word |{PSW,A}=[SP]+|
|PUSH B   |C5|Push BC                   |-[SP]=BC     |
|PUSH D   |D5|Push DE                   |-[SP]=DE     |
|PUSH H   |E5|Push HL                   |-[SP]=HL     |
|PUSH PSW |F5|Push Processor Status Word|-[SP]={PSW,A}|
|RAL      |17|Rotate Accumulator Left   |A={CY,A}<-   |
|RAR      |1F|Rotate Accumulator Righ   |A=->{CY,A}   |
|RET      |C9|Return                    |PC=[SP]+     |
|RC       |D8|Return on Carry           |If CY=1(12~s)|
|RM       |F8|Return on Minus           |If S=1 (12~s)|
|RNC      |D0|Return on No Carry        |If CY=0(12~s)|
|RNZ      |C0|Return on No Zero         |If Z=0 (12~s)|
|RP       |F0|Return on Plus            |If S=0 (12~s)|
|RPE      |E8|Return on Parity Even     |If P=1 (12~s)|
|RPO      |E0|Return on Parity Odd      |If P=0 (12~s)|
|RZ       |C8|Return on Zero            |If Z=1 (12~s)|
|RLC      |07|Rotate Left Circular      |A=A<-        |
|RRC      |0F|Rotate Right Circular     |A=->A        |
|RST z    |C7|Restart              (3X7)|-[SP]=PC,PC=z|
|SBB r    |9F|Subtract with Borrow      |A=A-r-CY     |
|SBB M    |9E|Subtract with Borrow      |A=A-[HL]-CY  |
|SBI n    |DE|Subtract with Borrow Immed|A=A-n-CY     |
|SHLD a   |22|Store HL Direct           |[a]=HL       |
|SPHL     |F9|Move HL to SP             |SP=HL        |
|STA a    |32|Store Accumulator         |[a]=A        |
|STAX B   |02|Store Accumulator indirect|[BC]=A       |
|STAX D   |12|Store Accumulator indirect|[DE]=A       |
|STC      |37|Set Carry                 |CY=1         |
|SUB r    |97|Subtract                  |A=A-r   (22X)|
|SUB M    |96|Subtract Memory           |A=A-[HL]     |
|SUI n    |D6|Subtract Immediate        |A=A-n        |
|XCHG     |EB|Exchange HL with DE       |HL<->DE      |
|XRA r    |AF|Exclusive OR Accumulator  |A=Axr   (25X)|
|XRA M    |AE|Exclusive OR Accumulator  |A=Ax[HL]     |
|XRI n    |EE|Exclusive OR Immediate    |A=Axn        |
|XTHL     |E3|Exchange stack Top with HL|[SP]<->HL    |

---
### 3. Flowchart

---
### 4. Structure
1. **State8085**
```
{
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint16_t sp;
	uint16_t pc;
	struct Flags cc;
	uint8_t int_enable;
	uint8_t *memory;
}
```
2. **Flags**
```
{
	uint8_t z : 1;
	uint8_t s : 1;
	uint8_t p : 1;
	uint8_t cy : 1;
	uint8_t ac : 1;
	uint8_t pad : 3; // 3 empty
}
```
3. **Instruction**
```
{
    char instruction[16];
    char mnemonics[5];
    char operand[5];
    char toLabel[10];
    char address[5];
    int size;
    intruction_type type;
    int error;
}
```
5. **Label**
```
{
	int lineNo;
	int instNo;
	uint16_t declaration;
	char name[10];
	int linkedFrom[50];
	int linkedCount;
	int references;
}
```
7. **Macro**
```
	int parametersCount;
	char name[15];
	char template[10][16];	
	int lineCount;	
```

---
### 5. Important Functions

**`Init8085`** : Allocates memory for the processor and prepares the simulator for execution by reading memory state data from dumped file if present.  

**`getOpcode`** : Searches in the mnemonics files and returns the hexadecimal opcode for the provided mnemonic.  

**`getMemory`** : Returns the contents of the Memory location as provided by the user.  

**`setMemory`** : Allows the user to set the content by providing the memory location.  

**`showRegisters`** : Allows the user to check all the *General Purpose Registers*.  

**`updateRegister`** : Allows the user to set values of *General Purpose Register*.  

**`showFlagRegisters`** : Allows the user to check all the *Flag bits*.  

**`updateFlag`** : Allows the user to set or unset the *Flag bits*.  

**`macroProcessor`** : This parses the Assembly code to find the `macro` and stores them, it also checks for duplicate macro labels.  

**`generateInstruction`** : This reads mnemonics from the Assembly code and detects labels, size of the instructions, it also checks for syntax errors.  

**`LoadProgram`** : Converts the mnemonics to opcode and stores in memory at the location as provided by the user.  

**`ExecuteProgram`** : Sequential execution of the program starting from the `load_address` till halt command is received.  

**`captureSnapshot`** : Stores the state (snapshot) of the simulator in a readable file for user reference. The file contains details of Registers, Flag Register, Mnenomics, Opcode and the Memory dump.  

**`dumpMemory`** : This function dumps the simulator memory in a file which can is read by `Init8085` function thus maintaining the state even after the program is closed.  

---
### 6. Bug Reporting
Found a Bug? Report at [Github](https://github.com/pritishaw/8085-simulator/issues)
