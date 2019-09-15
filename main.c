#include "8085.h"

int labelCount = 0;
int lineCount = 1;
int instCount = 0;
uint8_t ram[0x1000];
uint16_t temp_sp = 0x0000;
Instruction instSet[1000];
Label labels[100];
uint16_t load_address = 0x0000;

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
        tempLabel.linkedCount = 0;
        strcpy(tempLabel.name,temp);
        labels[labelCount++] = tempLabel;
        return -1;
    }
    else if(labels[i].instNo==-1 && labels[i].references>0){
        labels[i].instNo = instCount;
        labels[i].lineNo = lineCount;
        return -1;
    }
    return i;
}

int generateInstruction(){
    char operand[10];
    char mne[10];
    int size,isComma,bit;
    for (int i = 0; i < instCount ; i++) {
        strcpy(operand, instSet[i].operand);
        strcpy(mne, instSet[i].mnemonics);
        bit = -1;
        isComma = -1;
        switch(strlen(operand)){
            case 0:
                size = 1;
                break;
            case 1:
                size = 2;
                break;
            default:
                for(int i = 0; i < strlen(operand); i++){
                    if(operand[i]==',')
                        if(isComma>0){
                            printf("Parse Error\n");
                            exit(0);
                        }
                        else
                            isComma = i;
                }
                if(isComma>-1){
                    if(strlen(operand)==7){
                        bit = 16;
                        size = 3;
                    }
                    else if(strlen(operand)==5){
                        bit = 8;
                        size = 2;
                    }
                    else if(strlen(operand)==3){
                        bit = -1;
                        size = 1;
                    }
                }
                else {
                    if(strlen(operand)==5){
                        bit = 16;
                        size = 3;
                    }
                    else if(strlen(operand)==3){
                        bit = 8;
                        size = 2;
                    }
                    else{
                        size = 3;
                    }
                }
                break;
        }
        uint8_t opCode;
        int labelID,tempInt,tempI;
        char temp[16];
        for(tempI = 0; tempI < labelCount; tempI++){
            if(labels[tempI].instNo == i){
                labels[tempI].declaration = temp_sp+load_address;
                break;
            }
        }
        switch (size * bit * isComma / abs(isComma))
        {
            case -1:
                // MOV A,D
                strcpy(temp,mne);
                strcat(temp,operand);
                opCode = getOpcode(temp,size);
                ram[temp_sp++] = opCode;
                break;
            case 1:
                // NOP
                opCode = getOpcode(mne,size);
                ram[temp_sp++] = opCode;
                break;
            case 2:
                // ADD A
                strcpy(temp,mne);
                strcat(temp,operand);
                opCode = getOpcode(temp,1);
                ram[temp_sp++] = opCode;
                break;
            case 3:
                // JZ LABEL
                opCode = getOpcode(mne,size);
                ram[temp_sp++] = opCode;
                labelID = searchLabel(operand);
                if(labels[labelID].instNo==-1){
                    printf("Error Label %s not defined\n",labels[labelID].name);
                    exit(0);
                }
                labels[labelID].linkedFrom[labels[labelID].linkedCount++] = temp_sp;
                ram[temp_sp++] = labelID;
                ram[temp_sp++] = labelID;
                break;
            case 16:
                // MVI A 00H
                strcpy(temp,mne);
                strcat(temp,operand);
                temp[strlen(temp)-4] = '\0';
                opCode = getOpcode(temp,size);
                ram[temp_sp++] = opCode;
                strcpy(temp,operand);
                for(tempI=0;tempI<strlen(temp);tempI++){
                    if(temp[tempI]==',')
                        break;
                }
                tempI++;
                for(tempInt=0;tempI<strlen(temp);tempInt++,tempI++){
                    temp[tempInt] = temp[tempI];
                }
                temp[tempInt] = '\0';
                opCode = strtoul(temp, NULL, 16);
                ram[temp_sp++] = opCode;
                break;
            case -16:
                // CPI 00H
                opCode = getOpcode(mne,size);
                ram[temp_sp++] = opCode;
                opCode = strtoul(operand, NULL, 16);
                ram[temp_sp++] = opCode;
                break;
            case 48:
                // LXI B 0000H
                strcpy(temp,mne);
                strcat(temp,operand);
                temp[strlen(temp)-6] = '\0';
                opCode = getOpcode(temp,size);
                ram[temp_sp++] = opCode;
                strcpy(temp,operand);
                for(tempI=0;tempI<strlen(temp);tempI++)
                    if(temp[tempI]==',')
                        break;
                
                tempI++;
                for(tempInt=0;tempI<strlen(temp);tempInt++,tempI++)
                    temp[tempInt] = temp[tempI];
                
                temp[tempInt] = '\0';              
                opCode = strtoul(temp, NULL, 16);
                ram[temp_sp++] = opCode;
                temp[2] = 'H';
                opCode = strtoul(temp, NULL, 16);
                ram[temp_sp++] = opCode;
                break;
            case -48:
                // CP 0000H
                opCode = getOpcode(mne,size);
                ram[temp_sp++] = opCode;
                opCode = strtoul(operand, NULL, 16);
                ram[temp_sp++] = opCode;
                strcpy(temp,operand);
                temp[2] = 'H';
                opCode = strtoul(temp, NULL, 16);
                ram[temp_sp++] = opCode;
                break;
        }
    }

    for(int i = 0; i < labelCount; i++){
        for (int labelI = 0; labelI < labels[i].linkedCount; labelI++){
            ram[labels[i].linkedFrom[labelI]] = (uint8_t)labels[i].declaration;
            ram[labels[i].linkedFrom[labelI]+0x01] = (uint8_t)((labels[i].declaration >> 8));
        }
    }
}

Instruction parseInstruction(char * inst) {
    Instruction parsed = {"","","","","",0,UNASSIGNED,0};
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
        for(i = 0; i < (int)strlen(inst); i++){
            if(inst[i] == ' ')
                break;
            temp[i] = inst[i];
        }
        temp[i] = '\0';
        strcpy(parsed.mnemonics,temp);
        parsed.type = getMnemonicsType(temp);
        step = i;
        for(j=0; i < (int)strlen(inst); i++){
            if(inst[i] == ' ')
                continue;
            else if( inst[i] == '\0')
                break;
            temp[j++] = inst[i];
        }     
        temp[j] = '\0';
        strcpy(parsed.operand,temp);
        if(parsed.type==JUMP){
            i = step;
            if(strcmp(parsed.mnemonics,"PCHL")!=0){
                for(; i < (int)strlen(inst); i++){
                    if(inst[i] != ' ')
                        break;
                }        
                for(j=0; i < (int)strlen(inst); i++){
                    if(inst[i] == ' ' || inst[i] == '\0')
                        break;
                    temp[j++] = inst[i];
                }                        
                temp[j] = '\0';
                int labelPresent = searchLabel(temp);
                if(labelPresent>-1){
                    strcpy(parsed.toLabel,labels[labelPresent].name);
                    labels[labelPresent].references++;
                }
                else{
                    //Forward Reference
                    //Make empty Label
                    Label tempLabel;
                    tempLabel.references = 1;
                    tempLabel.instNo = -1;
                    tempLabel.linkedCount = 0;
                    strcpy(tempLabel.name,temp);
                    labels[labelCount++] = tempLabel;
                }
            }
        }
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

void showOpcode(){
    printf("\n Memory\t| Opcode\n------------------\n");
    for (uint16_t i = 0; i <temp_sp;i++)
        printf("  %04x\t|   %02x\n",i+load_address,ram[i]);
}

void showMnemonics(){
    for (int i = 0; i < instCount ; i++) {
        printf("%s\n",instSet[i].instruction);
    }
}

int main(int argc, char** argv){
    if(argc != 2){
        printf("Invalid Command\n");
        exit(0);
    }
    char tempStr[16] = "0";
    State8085 * simulator = Init8085();
    // printf("Enter load memory address:\t");
    // scanf("%hx",&load_address);
    FILE *fptr;
    char line[256];
    fptr = fopen(argv[1],"r");
    if (fptr == NULL)
    {
        printf("Error: Cannot open file \n");
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
    generateInstruction();    
    LoadProgram(simulator,ram,temp_sp,load_address);
    int option,option2;
    uint8_t temp8;
    uint16_t temp16;
    uint32_t temp32;

    while(1){
        printf("Menu\n\t1. Execute program\n\t2. Get memory location\n\t3. Set memory location\n");
        printf("\t4. Check General Register\n\t5. Set General Register\n");
        printf("\t6. Check Flag Register\n\t7. Set Flag Register\n");
        printf("\t8. Show Opcode\n\t9. Show Mnemonics\n\t10. Dump Memory\n");
        printf("\t0. Exit\n\nEnter your choice:\t");
        scanf("%d",&option);
        switch (option)
        {
        case 1:
            ExecuteProgram(simulator,load_address);
            break;        
        case 2:
            printf("Enter memory location:\t");
            scanf("%x",&temp32);
            getMemory(simulator,temp32);
            break;
        case 3:
            printf("Enter memory location:\t");
            scanf("%x",&temp32);
            printf("Enter 8 Bit number:\t");
            scanf("%hhx",&temp8);
            setMemory(simulator,temp32,temp8);
            break;
        case 4:
            showRegisters(simulator);
            break;
        case 5:
            printf("Please select a General Register:\n");
            printf("\t%d. %s\n",1,"A");
            printf("\t%d. %s\n",2,"B");
            printf("\t%d. %s\n",3,"C");
            printf("\t%d. %s\n",4,"D");
            printf("\t%d. %s\n",5,"E");
            printf("\t%d. %s\n",6,"H");
            printf("\t%d. %s\n",7,"L");
            printf("\t%d. %s\n",8,"Stack Pointer");
            printf("\t%d. %s\n",9,"Program Counter");
            scanf("%d",&option2);
            updateRegister(simulator,option2-1);
            break;
        case 6:
            showFlagRegisters(simulator);
            break;
        case 7:
            printf("Please select a Flag Register:\n");
            printf("\t%d. %s  Flag\n",1,"Carry");
            printf("\t%d. %s  Flag\n",2,"Parity");
            printf("\t%d. %s  Flag\n",3,"Auxiliary Carry");
            printf("\t%d. %s  Flag\n",4,"Zero");
            printf("\t%d. %s  Flag\n",5,"Sign");
            scanf("%d",&option2);
            updateFlag(simulator,option2-1);
            break;
        case 8:
            showOpcode();
            break;
        case 9:
            showMnemonics();
            break;
        case 10:
            dumpMemory(simulator);
            break;
        case 0:
            dumpMemory(simulator);
            exit(0);
        }
    }    
}