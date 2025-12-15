//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/
#define SMMNODE_TYPE_LECTURE                0
#define SMMNODE_TYPE_RESTAURANT             1
#define SMMNODE_TYPE_LABORATORY             2
#define SMMNODE_TYPE_HOME                   3
#define SMMNODE_TYPE_GOTOLAB                4
#define SMMNODE_TYPE_FOODCHANGE             5
#define SMMNODE_TYPE_FESTIVAL               6

#define SMMNODE_OBJTYPE_BOARD 0
#define SMMNODE_OBJTYPE_GRADE 1
#define SMMNODE_OBJTYPE_FOOD 2
#define SMMNODE_OBJTYPE_FEST 3
/* grade :
    A+,
    A0,
    A-,
    B+,
    B0,
    B-,
    C+,
    C0,
    C-
*/
#define SMMNODE_GRADE_AP  0   // A+
#define SMMNODE_GRADE_A0  1   // A0
#define SMMNODE_GRADE_AM  2   // A-
#define SMMNODE_GRADE_BP  3   // B+
#define SMMNODE_GRADE_B0  4   // B0
#define SMMNODE_GRADE_BM  5   // B-
#define SMMNODE_GRADE_CP  6   // C+
#define SMMNODE_GRADE_C0  7   // C0
#define SMMNODE_GRADE_CM  8   // C-
#define SMMNODE_GRADE_DP  9   // D+ 
#define SMMNODE_GRADE_D0  10  // D0
#define SMMNODE_GRADE_DM  11  // D-
#define SMMNODE_GRADE_F   12  // F
#define SMMNODE_MAX_GRADE 13



//object generation
void* smmObj_genObject(char* name,int objType, int type, int credit, int energy,int grade);

//member retrieving
char* smmObj_getObjectName(void* ptr);
int smmObj_getObjectType(void* ptr);
int smmObj_getObjectEnergy(void *ptr);
int smmObj_getObjectCredit(void* ptr);
int smmObj_getObjectGrade(void*ptr);

//element to string
char* smmObj_getTypeName(int type);


#endif /* smm_object_h */
