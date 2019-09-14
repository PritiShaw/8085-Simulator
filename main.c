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

intruction_type getInstructionType(char * m){
    // UNASSIGNED, DATA_TRANSFER , ARITHETIC_LOGIC, BRANCH, MACHINE_CONTROL, ERROR
    if( strcmp(m,"MOV")==0 || strcmp(m,"MVI")==0 || strcmp(m,"XCHG")==0 || strcmp(m,"LXI")==0 || 
        strcmp(m,"LDAX")==0 || strcmp(m,"LHLD")==0 || strcmp(m,"LDA")==0 || strcmp(m,"STAX")==0 ||
        strcmp(m,"SHLD")==0 || strcmp(m,"STA")==0
    )
        return DATA_TRANSFER;
    if()
        return ;
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
        parsed.type = getInstructionType(temp);
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