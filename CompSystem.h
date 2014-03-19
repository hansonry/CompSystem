#ifndef __COMPSYSTEM_H__
#define __COMPSYSTEM_H__

#define COMPSYSTEM_INVALID_INDEX -1

typedef struct compsystem_s * CompSystem_T;

typedef unsigned int actorid_t;
typedef unsigned int comptypeid_t;
typedef void (*CompSystem_DestroyFunc_T)(void * comp, CompSystem_T sys, 
                                         comptypeid_t type, actorid_t actor);




CompSystem_T CompSystem_Create(void);

void CompSystem_NewType(CompSystem_T sys, comptypeid_t * type);
void CompSystem_SetType(CompSystem_T sys, comptypeid_t type, int elementSize, CompSystem_DestroyFunc_T destroyFunc);

void CompSystem_NewActor(CompSystem_T sys, actorid_t * actor);
void CompSystem_RemoveActor(CompSystem_T sys, actorid_t actor);

void CompSystem_SetComponent(CompSystem_T sys, actorid_t actor, comptypeid_t type, const void * comp);
void CompSystem_GetComponent(const CompSystem_T sys, actorid_t actor, comptypeid_t type, int * outIndex, void ** outPointer);
void CompSystem_GetComponentActor(const CompSystem_T sys, comptypeid_t type, int index, actorid_t * actor);
void CompSystem_GetComponentFromComponent(const CompSystem_T sys, 
                                          comptypeid_t sourceType, 
                                          int sourceIndex, 
                                          comptypeid_t destType, 
                                          int * destIndex, 
                                          void ** destPointer);

void CompSystem_ComponentFor(const CompSystem_T sys, comptypeid_t type, void ** array, int * size);
void CompSystem_GetActorCount(const CompSystem_T sys, int * actorCount);
void CompSystem_GetActor(const CompSystem_T sys, int index, actorid_t * actor);

void CompSystem_Destroy(CompSystem_T sys);

#endif // __COMPSYSTEM_H__
