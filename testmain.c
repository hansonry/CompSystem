#include <stdio.h>
#include "CompSystem.h"

typedef enum comp_e
{
   eComp_Position,
   eComp_Physics,
   eComp_Render,
   eComp_Last
} Comp_T;

static void createTypes(CompSystem_T sys, comptypeid_t * types);
static void addActors(CompSystem_T sys, comptypeid_t * types);
static void loop(CompSystem_T sys, comptypeid_t * types);
static void jumptest(CompSystem_T sys, comptypeid_t * types, actorid_t actor);

int main(int argc, char * args[])
{
   CompSystem_T sys;
   comptypeid_t types[eComp_Last];
   
   sys = CompSystem_Create();

   createTypes(sys, types);   
   addActors(sys, types);
   jumptest(sys, types, 3);
   
   loop(sys, types);
   
   
   CompSystem_Destroy(sys);
   printf("HelloWorld\n");
   return 0;
}

static void createTypes(CompSystem_T sys, comptypeid_t * types)
{
   int i;
   for(i = 0; i < eComp_Last; i ++)
   {
      CompSystem_NewType(sys, &types[i]);
      CompSystem_SetType(sys, types[i], sizeof(int));
   }
}

static void addActors(CompSystem_T sys, comptypeid_t * types)
{
   actorid_t actor1;
   int rawValue, *array, i;
   for(i = 0; i < 20; i++)
   {
      CompSystem_NewActor(sys, &actor1);
      rawValue = i;
      CompSystem_SetComponent(sys, actor1, types[eComp_Position], &rawValue);
      rawValue = i << 1;
      CompSystem_SetComponent(sys, actor1, types[eComp_Physics], &rawValue);
   }
}

static void loop(CompSystem_T sys, comptypeid_t * types)
{
   int count, i;
   int * data;
   actorid_t actorid;
   
   CompSystem_GetActorCount(sys, &count);
   for(i = 0; i < count; i++)
   {
      CompSystem_GetActor(sys, i, &actorid);
      printf("ActorID: %i\n", actorid);
   }
   
   
   CompSystem_ComponentFor(sys, types[eComp_Position], (void**)&data, &count);
   
   for(i = 0; i < count; i++)
   {
      printf("Position: (i, v) = (%i, %i)\n", i, data[i]);
   }
   
   CompSystem_ComponentFor(sys, types[eComp_Physics], (void**)&data, &count);
   
   for(i = 0; i < count; i++)
   {
      printf("Physics: (i, v) = (%i, %i)\n", i, data[i]);
   }

}

static void jumptest(CompSystem_T sys, comptypeid_t * types, actorid_t actor)
{
   
   int index, *ptr, index2, *ptr2;
   actorid_t outActor;
   
      
   CompSystem_GetComponent(sys, actor, types[eComp_Position], &index, (void**)&ptr);
   printf("GetComponent: (a, i, v) = (%i, %i, %i)\n", actor, index, *ptr);
   CompSystem_GetComponentActor(sys, types[eComp_Position], index, &outActor);
   printf("GetComponentActor: (oa) = (%i)\n", outActor);
   CompSystem_GetComponentFromComponent(sys, types[eComp_Position], index, types[eComp_Physics], &index2, (void**)&ptr2);
   printf("GetComponentFromComponent: (i2, v2) = (%i, %i)\n", index2, *ptr2);
}
