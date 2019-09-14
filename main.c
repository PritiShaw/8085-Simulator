#include "8085.h"

int labelCount = 0;
int lineCount = 1;
int instCount = 0;
Instruction instSet[100];
Label labels[100];

int searchLabel(char * key){
    for(int i = 0 ; i < labelCount; i++)
       if(strcmp(labels[i].name,key)==0){
           return i;
       }
    return -1;
}

int detectLabel(char * line){
    char temp[10];
    temp[0] = '\0';
    int i,j;
    int hasLabel= -1;
    for(i = 0; i<strlen(line) ; i++){
        if(line[i] == ':'){
            if(hasLabel>0){
                printf("ERROR %s\n",line);
                exit(0);
            }
            hasLabel = i;
        }
    }
    for(j = 0,i=0; i < hasLabel;i++)
        if(line[i]!=' ')
            temp[j++] = line[i];
    temp[j] = '\0';
    if(strlen(temp)==0)
        return -2;
    i = searchLabel(temp);
    if(i ==-1){
        Label tempLabel;
        tempLabel.lineNo = lineCount;
        tempLabel.instNo = instCount;
        tempLabel.references = 0;
        strcpy(tempLabel.name,temp);
        labels[labelCount++] = tempLabel;
        return -1;
    }
    return i;
}

intruction_type getMnemonicsType(char * m){
    // UNASSIGNED, DATA_TRANSFER , ARITHMETIC_LOGIC, BRANCH, MACHINE_CONTROL, ERROR
    if( strcmp(m,"MOV")==0 || strcmp(m,"MVI")==0 || strcmp(m,"XCHG")==0 || strcmp(m,"LXI")==0 || 
        strcmp(m,"LDAX")==0 || strcmp(m,"LHLD")==0 || strcmp(m,"LDA")==0 || strcmp(m,"STAX")==0 ||
        strcmp(m,"SHLD")==0 || strcmp(m,"STA")==0
    )
        return DATA_TRANSFER;
    else if( strcmp(m,"ADD")==0 || strcmp(m,"ADC")==0 || strcmp(m,"SUB")==0 || strcmp(m,"SBB")==0 || 
        strcmp(m,"DAD")==0 || strcmp(m,"INR")==0 || strcmp(m,"INX")==0 || strcmp(m,"DCR")==0 ||
        strcmp(m,"DCX")==0 || strcmp(m,"DAA")==0 || strcmp(m,"CMA")==0 || strcmp(m,"STC")==0 ||
        strcmp(m,"CMC")==0 || strcmp(m,"RLC")==0 || strcmp(m,"RRC")==0 || strcmp(m,"RAL")==0 ||
        strcmp(m,"RAR")==0 || strcmp(m,"ANA")==0 || strcmp(m,"XRA")==0 || strcmp(m,"ORA")==0 ||
        strcmp(m,"CMP")==0 || strcmp(m,"ADI")==0 || strcmp(m,"ACI")==0 || strcmp(m,"SUI")==0 ||
        strcmp(m,"SBI")==0 || strcmp(m,"ANI")==0 || strcmp(m,"XRI")==0 || strcmp(m,"ORI")==0 ||
        strcmp(m,"CPI")==0 
    )
        return ARITHMETIC_LOGIC;
    else if( strcmp(m,"JMP")==0 || strcmp(m,"JNZ")==0 || strcmp(m,"JZ")==0 || strcmp(m,"JNC")==0 || 
        strcmp(m,"JC")==0 || strcmp(m,"JPO")==0 || strcmp(m,"JPE")==0 || strcmp(m,"JP")==0 ||
        strcmp(m,"JM")==0 || strcmp(m,"PCHL")==0 || strcmp(m,"CALL")==0 || strcmp(m,"CNZ")==0 ||
        strcmp(m,"CNC")==0 || strcmp(m,"CZ")==0 || strcmp(m,"CC")==0 || strcmp(m,"CPO")==0 || 
        strcmp(m,"CPE")==0 || strcmp(m,"CP")==0 || strcmp(m,"CM")==0 || strcmp(m,"RET")==0 || 
        strcmp(m,"RNZ")==0 || strcmp(m,"RZ")==0 || strcmp(m,"RNC")==0 || strcmp(m,"RC")==0 || 
        strcmp(m,"RPO")==0 || strcmp(m,"RPE")==0 || strcmp(m,"RP")==0 || strcmp(m,"RM")==0 ||
        strcmp(m,"RST")==0
    )
        return BRANCH;
    else if( strcmp(m,"PUSH")==0 || strcmp(m,"POP")==0 || strcmp(m,"XTML")==0 || strcmp(m,"SPHL")==0 || 
        strcmp(m,"OUT")==0 || strcmp(m,"IN")==0 || strcmp(m,"DI")==0 || strcmp(m,"EI")==0 ||
        strcmp(m,"NOP")==0 || strcmp(m,"HLT")==0 || strcmp(m,"RIM")==0 || strcmp(m,"SIM")==0 
    )
        return MACHINE_CONTROL;
    /*else if( strcmp(m,"ORG")==0 || strcmp(m,"END")==0 || strcmp(m,"EQU")==0 || strcmp(m,"SET")==0 || 
        strcmp(m,"DS")==0 || strcmp(m,"DB")==0 || strcmp(m,"DW")==0 || strcmp(m,"MACRO")==0 ||
        strcmp(m,"ENDM")==0 || strcmp(m,"LOCAL")==0 || strcmp(m,"REPT")==0 || strcmp(m,"ZRP")==0 ||
        strcmp(m,"IRPC")==0 || strcmp(m,"EXITM")==0 || strcmp(m,"ASEG")==0 || strcmp(m,"DSEG")==0 || 
        strcmp(m,"CSEG")==0 || strcmp(m,"PUBLIC")==0 || strcmp(m,"EXTRN")==0  || strcmp(m,"NAME")==0 || 
        strcmp(m,"STKLN")==0 || strcmp(m,"STACK")==0 || strcmp(m,"MEMORY")==0 ||strcmp(m,"IF")==0 || 
        strcmp(m,"ELSE")==0 || strcmp(m,"ENDIF")==0
    )
        return ;*/
    else
        return ERROR;
}

Instruction parseInstruction(char * inst) {
    Instruction parsed = {"","","","","","",0,UNASSIGNED,0};
    strcpy(parsed.instruction,inst);
    switch(strlen(inst)){
        case 0:
            parsed.error = 0;
            break;
        case 1:
            parsed.error = -1;
            break;
        case 2:
            parsed.error = -1;
            break;
        default:
            parsed.error = 2;
            break;
    }
    if(parsed.error == 2){
        int hasLabel = detectLabel(inst);
        int step = 0;
        int i,j;
        char temp[10];
        switch(hasLabel){
            case -1:
                step = 0;
                for(i = 0; i < strlen(inst);i++)
                    if(inst[i] == ':')
                        break;
                i++;
                for(; i < strlen(inst);i++)
                    if(inst[i] != ' ')
                        break;
                for(j = 0; i < strlen(inst); i++,j++){
                    inst[j] = inst[i];
                }
                inst[j] = '\0';
                strcpy(parsed.instruction,inst);
            case -2:
                break;
            default:
                printf("Error: Label \"%s\" defined at Line %d and %d\n",labels[hasLabel].name,labels[hasLabel].lineNo,lineCount);
                exit(0);
        }
        step = 0;
        for(i = 0; i < strlen(inst); i++){
            if(inst[i] == ' ')
                break;
            temp[i] = inst[i];
        }
        temp[i] = '\0';
        parsed.type = getMnemonicsType(temp);
        parsed.error = 1;
    }
    return parsed;
}
Instruction parseLine(char* line){
    char temp[16];
    int length = strlen(line);
    char * p = line;
    //Trim
    while(* p && isspace(* p)) ++p;

    length = 0;
    while(*p != ';' && *p != '\n' && *p != '\0'){
        temp[length++] = toupper(*p);
        p++;
    }
    temp[length] = '\0';
    return parseInstruction(temp);    
}
int main(int argc, char** argv){
    FILE *fptr;
    char line[256];
    if(argc != 2){
        printf("Invalid Command\n");
        exit(0);
    }
    fptr = fopen(argv[1],"r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
    while (fgets(line, sizeof(line), fptr)) {
        Instruction temp = parseLine(line);
        switch(temp.error){
            case -1:
                printf("Error L%2d: %s",lineCount,line);
                break;
            case 0:
                break;
            case 1:
                instSet[instCount++] = temp;
                break;
        }
        lineCount++;
    }
    fclose(fptr);
    for (int i = 0; i < instCount ; i++) {
        // printf("%s\n",instSet[i].instruction);
    }
}