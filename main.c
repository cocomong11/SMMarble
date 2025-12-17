//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"
#define EXPERIMENT 3

//board configuration parameters
static int smm_board_nr;
static int smm_food_nr;
static int smm_festival_nr;
static int smm_player_nr;


typedef struct{
	char name[MAX_CHARNAME];
	int pos;
	int credit;
	int energy;
	int flag_graduated;
	int experimenting;
	int experiment_threshold;
} smm_player_t;

smm_player_t *smm_players;

//function prototypes
void generatePlayers(int n, int initEnergy); //generate a new player
void printPlayerStatus(void); //print all player status at the beginning of each turn
void* findGrade(int player, char*lectureName);
int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print grade history of the player
float calcAverageGrade(int player); //calculate average grade of the player
int takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)

void* findGrade(int player, char *lectureName) //find the grade from the player's grade history
{
	int size=smmdb_len(LISTNO_OFFSET_GRADE+player);
	int i;
	for (i=0;i<size;i++){
		void *ptr=smmdb_getData(LISTNO_OFFSET_GRADE+player,i);
		if(strcmp(smmObj_getObjectName(ptr),lectureName)==0)
		{
			return ptr;
		}
	}
	return NULL;
}
// 졸업한 플레이어가 있는지 확인 
int isGraduated(void)
{
	int i;
	
	for(i=0;i<smm_player_nr;i++)
	{
		if(smm_players[i].flag_graduated==1)
		return 1;
	}
	return 0;
	
}

void goForward(int player, int step)
{ //make player go "step" steps on the board (check if player is graduated)
    int i;
    void *ptr;
    
    ptr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
    printf("start from %i(%s) (%i)\n", smm_players[player].pos,
                                     smmObj_getObjectName(ptr), step);  
    for (i=0; i<step; i++)
    {
        smm_players[player].pos = (smm_players[player].pos + 1) % smm_board_nr;
        //이동한 칸의 정보를 가져옴 
        void* nextPtr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
        // 집을 지나가거나 도착했을 때 에너지 충전 
        if (smmObj_getObjectType(nextPtr) == SMMNODE_TYPE_HOME) 
        {
            int recharge = smmObj_getObjectEnergy(nextPtr);
            smm_players[player].energy += recharge;
            printf(" -> Energy recharged! (+%i) -> Current Energy: %i\n", 
                   recharge, smm_players[player].energy);
        }
        printf("  => moved to %i(%s)\n", smm_players[player].pos,
                                     smmObj_getObjectName(nextPtr));
    }
}
// 매 턴 시작 시 모든 플레이어의 상태 출력
void printPlayerStatus(void)
{
     int i;
     for (i=0;i<smm_player_nr;i++)
     {
     	void *ptr=smmdb_getData(LISTNO_NODE,smm_players[i].pos);
     	char statusMsg[50] = "";
        if (smm_players[i].experimenting) {
            sprintf(statusMsg, " [Experimenting]");
        }
         printf("%s - position:%i(%s), credit:%i, energy:%i\n",
                    smm_players[i].name, smm_players[i].pos, smmObj_getObjectName(ptr), smm_players[i].credit, smm_players[i].energy);
     }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
    int i;
    smm_players=malloc(n*sizeof(smm_player_t));
     
    for (i=0;i<n;i++){
        smm_players[i].pos = 0;
        smm_players[i].credit = 0;
        smm_players[i].energy = initEnergy;
        smm_players[i].flag_graduated = 0;
        smm_players[i].experimenting = 0;
        smm_players[i].experiment_threshold = 0;

         printf("Input %i-th player name:", i);
         scanf("%s", &smm_players[i].name[0]);
         fflush(stdin); 
     }
}
// 플레이어의 성적표 출력 
void printGrades(int player)
{
    int i;
    void *ptr;
    int count = smmdb_len(LISTNO_OFFSET_GRADE + player);
    
	printf("Player %s's Grade\n", smm_players[player].name);
    for (i = 0; i < count; i++)
    {
        ptr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        printf("%s : %s (%i)\n", smmObj_getObjectName(ptr), 
				smmObj_getGradeName(smmObj_getObjectGrade(ptr)),
				smmObj_getObjectCredit(ptr));
    }
}

int rolldie(int player)
{
    char c;
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);

    return (rand()%MAX_DIE + 1);
}


//action code when a player stays at a node
void actionNode(int player)
{
	void *ptr = smmdb_getData(LISTNO_NODE,smm_players[player].pos);

	int type= smmObj_getObjectType(ptr);
    int credit=smmObj_getObjectCredit(ptr);
    int energy=smmObj_getObjectEnergy(ptr);
    
    int grade;
    void *gradePtr;
    void *cardPtr;
    int randIndex;
    int i, labIndex = -1;
    
    printf(" --> player%i pos :%i, type : %s, credit : %i, energy : %i\n",
       player, smm_players[player].pos, smmObj_getTypeName(type), credit, energy);

	
   switch(type) 
    {
        case SMMNODE_TYPE_LECTURE:
        	//이미 수강한 강의인지 확인 
            if (findGrade(player, smmObj_getObjectName(ptr)) == NULL) 
            {
            	//에너지 충분한지 확인 
                if (smm_players[player].energy >= energy) 
                {
                    smm_players[player].credit += credit;
                    smm_players[player].energy -= energy;
                    grade = rand() % SMMNODE_MAX_GRADE;

                    gradePtr = smmObj_genObject(smmObj_getObjectName(ptr), SMMNODE_OBJTYPE_GRADE, type, credit, energy, grade);
                    smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);

                    printf("[Lecture] Took Class '%s'. Grade: %s, Energy consumed: %i\n", 
                           smmObj_getObjectName(ptr), smmObj_getGradeName(grade), energy);
                }
                else 
                {
                    printf("[Lecture] Not enough energy to take this class! (Required: %i)\n", energy);
                }
            }
            else 
            {
                printf("[Lecture] Already taken this class.\n");
            }
            break;      
   
    	case SMMNODE_TYPE_RESTAURANT:
    		smm_players[player].energy+=energy;
    		printf("[Restaurant] Energy recharged (+%i) -> %i\n", energy, smm_players[player].energy);
    		break;
    		
    	case SMMNODE_TYPE_LABORATORY:
    		printf("[Laboratory] You are in the lab.\n");
            break;
    		
    	case SMMNODE_TYPE_HOME:
    		printf("[Home] Sweet Home ^3^\n");
    		if(smm_players[player].credit>=GRADUATE_CREDIT)
			{
				printf("CONGRATULATIONS! Player %s Graduated!\n", smm_players[player].name);
    			smm_players[player].flag_graduated=1;
			}
    		break;
    		
    	case SMMNODE_TYPE_GOTOLAB:
    		// 실험실 노드 위치 찾기
        	labIndex = -1;
        	for(i=0; i<smm_board_nr; i++) {
            	void* nPtr = smmdb_getData(LISTNO_NODE, i);
            	if (smmObj_getObjectType(nPtr) == SMMNODE_TYPE_LABORATORY) {
                	labIndex = i;
                	break;
            }
        }
        //실험실로 강제 이동시킴 
        if (labIndex != -1) {
            smm_players[player].pos = labIndex;
            smm_players[player].experimenting = 1;
            smm_players[player].experiment_threshold = (rand() % MAX_DIE) + 1; 
            
            printf("[Experiment] Moving to Laboratory (Index %i). You are now experimenting.\n", labIndex);
            printf("[Experiment] Success threshold randomly set: %i\n", smm_players[player].experiment_threshold);
        }
        break;
    		
    	case SMMNODE_TYPE_FOODCHANGE: 
            if (smm_food_nr > 0) {
                randIndex = rand() % smm_food_nr;
                cardPtr = smmdb_getData(LISTNO_FOODCARD, randIndex);
                int foodEnergy = smmObj_getObjectEnergy(cardPtr);
                smm_players[player].energy += foodEnergy;
                printf("[FoodChance] %s! Energy recharged (+%i) -> %i\n", 
                       smmObj_getObjectName(cardPtr), foodEnergy, smm_players[player].energy);
            }
            break;
    		
    	case SMMNODE_TYPE_FESTIVAL:
            if (smm_festival_nr > 0) {
                randIndex = rand() % smm_festival_nr;
                cardPtr = smmdb_getData(LISTNO_FESTCARD, randIndex);
                printf("[Festival] Mission: %s\n", smmObj_getObjectName(cardPtr));
            }
            break;
            
        default:
            break;
    }
}


int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int turn;
    void *ptr;
    
    smm_board_nr = 0;
    smm_food_nr = 0;
    smm_festival_nr = 0;
    
    srand(time(NULL));
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        void* ptr;
        //printf("%s %i %i %i\n", name, type, credit, energy);
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD,type, credit, energy,0);
        smmdb_addTail(LISTNO_NODE, ptr);

    }
    fclose(fp);
    smm_board_nr = smmdb_len(LISTNO_NODE);
    printf("Total number of board nodes : %i\n", smm_board_nr);
    
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        void* ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_FOOD, 0, 0, energy, 0);
        smmdb_addTail(LISTNO_FOODCARD, ptr);
        
    }
    fclose(fp);
    smm_food_nr = smmdb_len(LISTNO_FOODCARD);
    printf("Total number of food cards : %i\n", smm_food_nr);

    
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", name) == 1) 
    {
        //store the parameter set
        void* ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_FEST, 0, 0, 0, 0);
        smmdb_addTail(LISTNO_FESTCARD, ptr);
    }
    fclose(fp);
    smm_festival_nr = smmdb_len(LISTNO_FESTCARD);
    printf("Total number of festival cards : %i\n", smm_festival_nr);
     
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("Input player number:");
        scanf("%i", &smm_player_nr);
        fflush(stdin);
        
        if (smm_player_nr <= 0 || smm_player_nr > MAX_PLAYER)
           printf("Invalid player number!\n");
    }
    while (smm_player_nr<= 0 || smm_player_nr > MAX_PLAYER);
    
    
    
    
    generatePlayers(smm_player_nr, smmObj_getObjectEnergy(smmdb_getData(LISTNO_NODE, 0)));
   
	printf("\n---GAME START---\n");
    turn = 0;
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (isGraduated()==0) //is anybody graduated?
    {
    	printf("\n---Turn: Player %s ---\n", smm_players[turn].name);
        int die_result;
        
        //3-1. initial printing
        printPlayerStatus();
        
        //3-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        printf("[Dice] Rolled: %i\n", die_result);
        if (smm_players[turn].experimenting == 1) {
        	void* labPtr = smmdb_getData(LISTNO_NODE, smm_players[turn].pos);
        	int reqEnergy = smmObj_getObjectEnergy(labPtr);
        
        	smm_players[turn].energy -= reqEnergy; 
        	printf("[Experiment] Trying to escape(Energy -%i -> %i)\n", reqEnergy, smm_players[turn].energy);

        if (die_result >= smm_players[turn].experiment_threshold) {
            
			printf("[Experiment] You escaped the lab.\n");
            smm_players[turn].experimenting = 0;
            
            goForward(turn, die_result);
            actionNode(turn);
        }
        else {
            printf("[Experiment] FAIL. Threshold was %i. You remain in the lab.\n", 
                       smm_players[turn].experiment_threshold);
        }
    }
        //3-3. go forward
        else {
        	goForward(turn, die_result);
        	actionNode(turn);
    }
		//3-4. take action at the destination node of the board
        if (isGraduated()) {
            break;
        }
     
        //3-5. next turn
        turn = (turn + 1)%smm_player_nr;
    }
    
    int i;
    printf("GAME OVER!\n");
    for(i=0; i<smm_player_nr; i++) {
        if(smm_players[i].flag_graduated) {
            printf("Winner is %s\n", smm_players[i].name);
            printGrades(i);
        }
    }
	free(smm_players);
    system("PAUSE");
    return 0;
}
