#include <stdio.h>
#include <stdlib.h>

static char M[100][4];//memory
static char IR[4];//Instruction Register
static int IC;//Instruction Counter Register
static char R[4];//General Purpose Register
static int C;//Toggle
static char buffer[10][4];
static char charP;
static int SI;
static FILE *filepointer;

void clearBuffer();
void mosMasterMode();
void read();
void write();
void terminate();
void load(char* filename);
void mos_or_startExecution();
void clearCpu();
void executeUserProgram();
void programToBuffer();
void bufferToMemory();
int toInt(char c);
void printMemory();
/*
MOS (MASTER MODE)
	SI = 3 (Initialization)
	Case SI of
		1: Read
		2: Write
		3: Terminate
	Endcase
	
READ
	IR [4] <- 0
	Read next (data) card from input file in memory locations IR [3,4] through IR [3,4] +9
	If M [IR [3,4]] = $END, abort (out-of-data)
	EXECUTEUSERPROGRAM
	
WRITE
	IR [4] <- 0
	Write one block (10 words of memory) from memory locations IR [3,4] through IR [3,4] + 9 to
	output file
	EXECUTEUSERPROGRAM
	
TERMINATE
	Write 2 blank lines in output file
	MOS/LOAD
	
LOAD
	m <- 0
	While not e-o-f
			Read next (program or control) card from input file in a buffer
			Control card: 
				$AMJ, end-while
				$DTA, MOS/STARTEXECUTION
				$END, end-while
			Program Card: 
				If m = 100, abort (memory exceeded)
				Store buffer in memory locations m through m + 9
				m <- m + 10
	End-While
STOP

MOS/STARTEXECUTION
	IC <- 00
	EXECUTEUSERPROGRAM
	
EXECUTEUSERPROGRAM (SLAVE MODE)
	Loop
		IR <- M [IC]
		IC <- IC+1
		Examine IR[1,2]
			LR: R <- M [IR[3,4]]
			SR: R -> M [IR[3,4]]
			CR: Compare R and M [IR[3,4]]
			If equal C <- T else C <- F
			BT: If C = T then IC <- IR [3,4]
			GD: SI = 1
			PD: SI = 2
			H:  SI = 3
		End-Examine
	End-Loop
*/

int toInt(char c){
	return c - '0';
}
void printMemory(){
	for(int i =0;i<100;i++){
		printf("%d", i);
		for(int j =0;j<4;j++) printf(" %c ", M[i][j]);
		printf("\n");
	}
	
}
void mosMasterMode(){
	switch(SI){
		case 1:
			read();
			break;
		case 2:
			write();
			break;
		case 3:
			terminate();
			break;
	}
	SI =3;
}


void read(){
	int count = toInt(IR[2])*10+toInt(IR[3]);
	//printf("\nIN read %c\n",charP);
	while(charP!='\n'){
		//printf(" %c ",charP);
		if(count >toInt(IR[2])*10+toInt(IR[3]) + 10) break;
		else{
			for(int i =0;i<4 && charP!='\n';i++){			
				M[count][i] = charP;
				//printf(" %c ",charP);
				charP = fgetc(filepointer);
			}
			count ++;
		}
	}
	charP = fgetc(filepointer);
	//printf("\nout read %c",charP);
}
void write(){
	//printf("\nWriting\n");
	int count = toInt(IR[2])*10+toInt(IR[3]);
	while(count <toInt(IR[2])*10 + toInt(IR[3]) +10 ){
		for(int i =0;i<4;i++){
			if(M[count][i]=='\0'){
      	printf(" ");
      	break;
      }
			printf("%c",M[count][i]);
		}
		count ++;
	}
	printf("\n");
}

void terminate(){
	printf("\n\n");
}

void load(char* filename){
	//printf("%s",filename);
	int loadPointer = 0;
	filepointer = fopen(filename,"r");
	charP = fgetc(filepointer);
	int justc = 1;
	int isExec = 0;
	while(charP != EOF){	
		if((charP)=='$'){
			charP = fgetc(filepointer);
			switch(charP){
				case 'A':
					//printf("clearStuf %c",charP);
					clearCpu();
					while(charP!='\n') charP = fgetc(filepointer); 
					break;
				case 'D':
					//printf("Start already");
					//printf("StartExecution %c",charP);
					
					while(charP!='\n') charP = fgetc(filepointer);
					charP=fgetc(filepointer);
					//printf("\nBefore %c",charP); 
					mos_or_startExecution();
					isExec = 1;
					//printf("\nMemory after execution\n");
					printMemory();
					//printf("\nAfter %c",charP);
					break;
				case 'E':
					//printf("End%c",charP);
					while(charP!='\n' && charP!=EOF){
						charP = fgetc(filepointer); 
					}
					loadPointer = 0; 
					break;
			}
		}
		else{
			//printf("Program card %d",justc++);
			
			while(charP!='\n'){
				if(loadPointer>100) break;	
				programToBuffer();
				bufferToMemory(loadPointer);
				loadPointer = loadPointer+10;
				clearBuffer();				
			}
			//printf("Memory after program card\n");
			//printMemory();
			
			//while(charP!='\n') charP = fgetc(filepointer);
			//printf("Execution done %c",charP);
		}
		if(charP == EOF) break;
		if(isExec == 0) charP = getc(filepointer);
		else isExec =0;
	}
	fclose(filepointer);
}

void clearBuffer(){
	for(int i =0;i<10;i++){
		for(int j =0;j<4;j++) buffer[i][j] = '\0';
	}
}
void mos_or_startExecution(){
	IC = 0;
	executeUserProgram();
}

void executeUserProgram(){
	int isH = 0;
	while(1){
	
		for(int i=0;i<4; i++) IR[i] = M[IC][i];
		IC = IC +1;
		switch(IR[0]){
			case 'L':
				for(int i =0; i<4; i++) R[i] = M[toInt(IR[2])*10 + toInt(IR[3])][i];
				break;
				
			case 'S':
				for(int i =0; i<4; i++) M[toInt(IR[2])*10 + toInt(IR[3])][i] = R[i] ;
				break;
				
			case 'C':
				for(int i =0; i<4;i++) {
					if(R[i] == M[toInt(IR[2])*10+toInt(IR[3])][i]) C = 0;
					else{
						C = 1;
						break;
					}
				}
				break;
				
			case 'B':
				if(C == 1){
					for(int i =0 ; i< 4; i++) IC = toInt(IR[2])*10 + toInt(IR[3]);
					C = 0;	
				}
				break;
				
			case 'G':
				SI = 1;
				mosMasterMode();
				break;
				
			case 'P':
				SI = 2;
				mosMasterMode();
				break;
				
			case 'H':
				SI = 3;
				isH = 1;
				//printf("\nisH %d\n",isH);
				mosMasterMode();
				break;
		}
		if(isH == 1) break;	
	}
}

void clearCpu(){
	for(int i =0;i<4;i++){
		IR[i]= '0';
		R[i] = '0';
	}
	IC =0;
	for(int i=0; i<100;i++) for(int j =0; j<4;j++) M[i][j] = '\0';
}

void programToBuffer(){
	int m=0;
	while(charP!='\n'){
	//printf(" itb %c ",charP);
		if(m >9) break;
		
		if(charP == 'H'){
			buffer[m][0] = 'H';
			charP = fgetc(filepointer);
			m++;
		}	
		else{
			for(int i =0;i<4;i++){			
				buffer[m][i] = charP;
				charP = fgetc(filepointer);
			}
			m++;
		}
	}	
	//charP = fgetc(filepointer);
}

void bufferToMemory(int loadPointer){
	for(int i =0;i<10;i++){
		for(int j=0;j<4;j++){
			
			M[loadPointer][j] = buffer[i][j];
		}
		loadPointer++;
	}
}

int main(){
	load("input_program.txt");
	return 0;
}
