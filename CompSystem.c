#include <stdlib.h>
#include <string.h>
#include "CompSystem.h"

#define GROW_BY 16
#define MIN(a, b) (((a) < (b)) ? (a) : (b))



typedef unsigned char byte_t;

typedef struct arrayinfo_s
{
   int arySize;
   int eleCount;
} ArrayInfo_T;

typedef struct actor_s
{
   actorid_t id;
   int * compIndexArray;
} Actor_T;

typedef struct comptype_s
{
   byte_t * compArray;
   Actor_T ** actorPtrArray;
   ArrayInfo_T compInfo;
   int elementSize;
   
} CompType_T;

struct compsystem_s
{
   CompType_T  * typeArray;
   Actor_T     * actorArray;
   ArrayInfo_T   typeInfo;
   ArrayInfo_T   actorInfo;
   actorid_t     nextActorID;
};

static int CompSystem_SetArraySize(void ** array, int elementSize, int size, int newSize);
static int CompSystem_GrowArraySize(void ** array, int elementSize, int size, int delta);
static Actor_T * CompSystem_FindActorFromID(CompSystem_T sys, actorid_t actor);
static void CompSystem_UpdateActorPointers(CompSystem_T sys);


CompSystem_T CompSystem_Create(void)
{
   CompSystem_T sys = malloc(sizeof(struct compsystem_s));
   // Create Empty Type Array
   sys->typeArray = calloc(GROW_BY, sizeof(CompType_T));
   sys->typeInfo.arySize = GROW_BY;
   sys->typeInfo.eleCount = 0;
   
   // Create Empty Actor Array
   sys->actorArray = calloc(GROW_BY, sizeof(Actor_T));
   sys->actorInfo.arySize = GROW_BY;
   sys->actorInfo.eleCount = 0;

   sys->nextActorID = 0;
   return sys;
}

void CompSystem_NewType(CompSystem_T sys, comptypeid_t * type)
{   
   CompType_T * compTypePtr;
   Actor_T * actorPtr;
   int i, oldSize;
   
   if(sys->typeInfo.eleCount >= sys->typeInfo.arySize)
   {
      sys->typeInfo.arySize = CompSystem_GrowArraySize((void **)&sys->typeArray, 
                                                       sizeof(CompType_T),
                                                       sys->typeInfo.arySize,
                                                       GROW_BY);
   }
   oldSize = sys->typeInfo.eleCount;
   (*type) = oldSize;
   sys->typeInfo.eleCount ++;
   
   // Set Default Values
   compTypePtr = &sys->typeArray[(*type)];
   compTypePtr->compArray         = NULL;
   compTypePtr->actorPtrArray     = NULL;
   compTypePtr->compInfo.arySize  = 0;
   compTypePtr->compInfo.eleCount = 0;
   compTypePtr->elementSize       = 0;
   
   
   // If there are any actors, expand their type pointers
   for(i = 0; i < sys->actorInfo.eleCount; i++)
   {
      actorPtr = &sys->actorArray[i];
      CompSystem_SetArraySize((void**)actorPtr->compIndexArray, 
                              sizeof(byte_t *), 
                              oldSize,
                              sys->typeInfo.eleCount);
      // Clear New Value
      actorPtr->compIndexArray[(*type)] = -1;
      
   }
}

void CompSystem_SetType(CompSystem_T sys, comptypeid_t type, int elementSize)
{
   CompType_T * compTypePtr;
   compTypePtr = &sys->typeArray[type];
   
   // Remove old data if present
   if(compTypePtr->compArray != NULL)
   {
      free(compTypePtr->compArray);
      free(compTypePtr->actorPtrArray);
   }
   
   // Create new buffers
   compTypePtr->elementSize       = elementSize;
   compTypePtr->compInfo.arySize  = GROW_BY;
   compTypePtr->compInfo.eleCount = 0;
   compTypePtr->compArray         = calloc(GROW_BY, elementSize);
   compTypePtr->actorPtrArray     = calloc(GROW_BY, sizeof(Actor_T*));
}


void CompSystem_NewActor(CompSystem_T sys, actorid_t * actor)
{
   Actor_T * actorPtr;
   int i;
   if(sys->actorInfo.eleCount >= sys->actorInfo.arySize)
   {
      sys->actorInfo.arySize = CompSystem_GrowArraySize((void**)&sys->actorArray,
                                                         sizeof(Actor_T),
                                                         sys->actorInfo.arySize,
                                                         GROW_BY);
                                                         
      CompSystem_UpdateActorPointers(sys);
   }
   (*actor) = sys->actorInfo.eleCount;
   sys->actorInfo.eleCount ++;
   
   // Init Actor
   actorPtr = &sys->actorArray[(*actor)];
   actorPtr->id = sys->nextActorID;
   sys->nextActorID ++;
   actorPtr->compIndexArray = calloc(sys->typeInfo.eleCount, sizeof(int));
   for(i = 0; i < sys->typeInfo.eleCount; i ++)
   {
      actorPtr->compIndexArray[i] = -1;
   }   
}

void CompSystem_SetComponent(CompSystem_T sys, actorid_t actor, comptypeid_t type, const void * comp)
{
   Actor_T * actorPtr;
   CompType_T * compTypePtr;
   byte_t * dest;
   int destOffset;
   int destIndex;
   
   compTypePtr = &sys->typeArray[type];
   actorPtr = CompSystem_FindActorFromID(sys, actor);
   if(actorPtr != NULL && compTypePtr != NULL)
   {
      if(actorPtr->compIndexArray[type] == -1)
      {
         // Grow if necessary
         if(compTypePtr->compInfo.eleCount >= compTypePtr->compInfo.arySize)
         {
            

            (void)CompSystem_GrowArraySize((void**)&compTypePtr->compArray, 
                                           compTypePtr->elementSize,
                                           compTypePtr->compInfo.arySize,
                                           GROW_BY);
            compTypePtr->compInfo.arySize = CompSystem_GrowArraySize((void**)&compTypePtr->actorPtrArray, 
                                                                     sizeof(Actor_T*),
                                                                     compTypePtr->compInfo.arySize,
                                                                     GROW_BY);
            


         }
         // Get offsets
         destIndex  = compTypePtr->compInfo.eleCount;

         // Set up references 
         compTypePtr->actorPtrArray[destIndex] = actorPtr;
         actorPtr->compIndexArray[type] = destIndex;
         
         // Inc count
         compTypePtr->compInfo.eleCount ++;
      }
      else
      {
         destIndex = actorPtr->compIndexArray[type];
      }
      
      // do the copy
      
      destOffset = destIndex * compTypePtr->elementSize;
      dest = &compTypePtr->compArray[destOffset];

      memcpy(dest, comp, compTypePtr->elementSize);
   }
   
   
   
}

void CompSystem_GetComponent(const CompSystem_T sys, actorid_t actor, comptypeid_t type, int * outIndex, void ** outPointer)
{
   Actor_T * actorPtr;
   CompType_T * compTypePtr;
   byte_t * outPtr;
   int outInd, offset;
   
   
   compTypePtr = &sys->typeArray[type];
   actorPtr = CompSystem_FindActorFromID(sys, actor);
   if(actorPtr != NULL)
   {
      outInd = actorPtr->compIndexArray[type];
      offset = outInd * compTypePtr->elementSize;
      outPtr = &compTypePtr->compArray[offset];
   }
   else
   {
      outPtr = NULL;
      outInd = -1;
   }
   
   if(outPointer != NULL)
   {
      (*outPointer) = outPtr;
   }
   
   if(outIndex != NULL)
   {
      (*outIndex) = outInd;
   }

}

void CompSystem_GetComponentActor(const CompSystem_T sys, comptypeid_t type, int index, actorid_t * actor)
{
   CompType_T * compTypePtr;
   
   compTypePtr = &sys->typeArray[type];
      
   (*actor) = compTypePtr->actorPtrArray[index]->id;
}

void CompSystem_GetComponentFromComponent(const CompSystem_T sys, 
                                          comptypeid_t sourceType, 
                                          int sourceIndex, 
                                          comptypeid_t destType, 
                                          int * destIndex, 
                                          void ** destPointer)
{
   CompType_T * sourceCompTypePtr;
   CompType_T * destCompTypePtr;
   Actor_T * actorPtr;
   int destInd, destOffset;
   
   sourceCompTypePtr = &sys->typeArray[sourceType];
   
   actorPtr = sourceCompTypePtr->actorPtrArray[sourceIndex];
   destInd = actorPtr->compIndexArray[destType];
   
   if(destIndex != NULL)
   {
      (*destIndex) = destInd;
   }
   
   if(destPointer != NULL)
   {
      destCompTypePtr = &sys->typeArray[destType];
      destOffset      = destInd * destCompTypePtr->elementSize;
      (*destPointer)  = &destCompTypePtr->compArray[destOffset];
   }
}

void CompSystem_ComponentFor(const CompSystem_T sys, comptypeid_t type, void ** array, int * size)
{
   CompType_T * compTypePtr;
   compTypePtr = &sys->typeArray[type];
   if(array != NULL)
   {
      (*array) = compTypePtr->compArray;
   }
   
   if(size != NULL)
   {
      (*size) = compTypePtr->compInfo.eleCount;
   }
}

void CompSystem_GetActorCount(const CompSystem_T sys, int * actorCount)
{
   (*actorCount) = sys->actorInfo.eleCount;
}

void CompSystem_GetActor(const CompSystem_T sys, int index, actorid_t * actor)
{
   (*actor) = sys->actorArray[index].id;
}


void CompSystem_Destroy(CompSystem_T sys)
{
   int i;
   CompType_T * compTypePtr;
   Actor_T * actorPtr;
   
   // Clean Actors
   for(i = 0; i < sys->actorInfo.eleCount; i++)
   {
      actorPtr = &sys->actorArray[i];
      free(actorPtr->compIndexArray);
   }
   
   // Clean Components
   for(i = 0; i < sys->typeInfo.eleCount; i++)
   {
      compTypePtr = &sys->typeArray[i];
      if(compTypePtr->compArray != NULL)
      {
         free(compTypePtr->compArray);
         free(compTypePtr->actorPtrArray);
      }
   }
   
   free(sys->typeArray);
   free(sys->actorArray);
   free(sys);
}


static int CompSystem_GrowArraySize(void ** array, int elementSize, int size, int delta)
{
   return CompSystem_SetArraySize(array, elementSize, size, size + delta);
}

static int CompSystem_SetArraySize(void ** array, int elementSize, int size, int newSize)
{
   int minSize;
   void * temp;
   temp = calloc(newSize, elementSize);
   minSize = MIN(size, newSize);
   memcpy(temp, (*array), minSize * elementSize);
   free(*array);
   (*array) = temp;
   return newSize;
}


static Actor_T * CompSystem_FindActorFromID(CompSystem_T sys, actorid_t actor)
{
   Actor_T * actorPtr;
   int i;
   
   actorPtr = NULL;
   
   for(i = 0; i < sys->actorInfo.eleCount; i++)
   {
      if(sys->actorArray[i].id == actor)
      {
         actorPtr = &sys->actorArray[i];
         break;
      }
   }
   return actorPtr;
}

static void CompSystem_UpdateActorPointers(CompSystem_T sys)
{
   int typeIndex, actorIndex, dataIndex;
   CompType_T * compTypePtr;
   Actor_T * actorPtr;
   
   for(actorIndex = 0; actorIndex < sys->actorInfo.eleCount; actorIndex++)
   {
      actorPtr = &sys->actorArray[actorIndex];
      for(typeIndex = 0; typeIndex < sys->typeInfo.eleCount; typeIndex++)
      {
         compTypePtr = &sys->typeArray[typeIndex];
         dataIndex = actorPtr->compIndexArray[typeIndex];
         if(dataIndex != -1)
         {
            compTypePtr->actorPtrArray[dataIndex] = actorPtr;
         }
      }
   }
}
