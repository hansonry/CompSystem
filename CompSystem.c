/*******************************************************************************
 * Copyright (c) 2014, Ryan Hanson
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL RYAN HANSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
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
   CompSystem_DestroyFunc_T destroyFunc;
   
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
static int CompSystem_FindActorFromID(CompSystem_T sys, actorid_t actor);
static void CompSystem_UpdateAllActorPointers(CompSystem_T sys);
static void CompSystem_UpdateActorPointers(CompSystem_T sys, Actor_T * actorPtr);
static void CompSystem_MoveMemory(void * dest, void * src, int elementSize);
static void CompSystem_DestroyComponent(CompSystem_T sys, comptypeid_t type, 
                                        actorid_t actor, 
                                        CompSystem_DestroyFunc_T destroyFunc, 
                                        void * comp);


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
      actorPtr->compIndexArray[(*type)] = COMPSYSTEM_INVALID_INDEX;
      
   }
}

void CompSystem_SetType(CompSystem_T sys, comptypeid_t type, int elementSize, CompSystem_DestroyFunc_T destroyFunc)
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
   compTypePtr->destroyFunc       = destroyFunc;
   compTypePtr->compInfo.arySize  = GROW_BY;
   compTypePtr->compInfo.eleCount = 0;
   compTypePtr->compArray         = calloc(GROW_BY, elementSize);
   compTypePtr->actorPtrArray     = calloc(GROW_BY, sizeof(Actor_T*));
}


void CompSystem_NewActor(CompSystem_T sys, actorid_t * actor)
{
   Actor_T * actorPtr;
   int actorIndex, i;
   if(sys->actorInfo.eleCount >= sys->actorInfo.arySize)
   {
      sys->actorInfo.arySize = CompSystem_GrowArraySize((void**)&sys->actorArray,
                                                         sizeof(Actor_T),
                                                         sys->actorInfo.arySize,
                                                         GROW_BY);
                                                         
      CompSystem_UpdateAllActorPointers(sys);
   }
   actorIndex = sys->actorInfo.eleCount;
   sys->actorInfo.eleCount ++;
   
   // Init Actor
   actorPtr = &sys->actorArray[actorIndex];
   actorPtr->id = sys->nextActorID;
   sys->nextActorID ++;
   actorPtr->compIndexArray = calloc(sys->typeInfo.eleCount, sizeof(int));
   for(i = 0; i < sys->typeInfo.eleCount; i ++)
   {
      actorPtr->compIndexArray[i] = COMPSYSTEM_INVALID_INDEX;
   }
   (*actor) = actorPtr->id;
}

void CompSystem_RemoveActor(CompSystem_T sys, actorid_t actor)
{
   int actorIndex, compType, compIndex, compIndexLast, actorIndexLast;
   int compByteIndex, compByteIndexLast;
   Actor_T * actorPtr, * actorPtrLast;
   CompType_T * compTypePtr;
   
   actorIndex = CompSystem_FindActorFromID(sys, actor);
   if(actorIndex != COMPSYSTEM_INVALID_INDEX)
   {
      actorPtr = &sys->actorArray[actorIndex];
      // Overwrite each component and re-attach back to orignal Actor
      for(compType = 0; compType < sys->typeInfo.eleCount; compType++)
      {         
         compIndex = actorPtr->compIndexArray[compType];
         if(compIndex != COMPSYSTEM_INVALID_INDEX)
         {
            compTypePtr = &sys->typeArray[compType];
            compIndexLast = compTypePtr->compInfo.eleCount - 1;
            actorPtrLast = compTypePtr->actorPtrArray[compIndexLast];         
            compByteIndex =     compIndex     * compTypePtr->elementSize;
            compByteIndexLast = compIndexLast * compTypePtr->elementSize;

            
            CompSystem_DestroyComponent(sys, compType, actor,
                                        compTypePtr->destroyFunc,
                                        &compTypePtr->compArray[compByteIndex]);
                                        
            
            // Move Last Element into this one
            CompSystem_MoveMemory(&compTypePtr->compArray[compByteIndex], 
                                  &compTypePtr->compArray[compByteIndexLast],
                                  compTypePtr->elementSize);
                                  
            // Re-attach Actor to component
            
            compTypePtr->actorPtrArray[compIndex] = actorPtrLast;
            actorPtrLast->compIndexArray[compType] = compIndex;
            
            // Decrement Size
            compTypePtr->compInfo.eleCount --;
         }
      }
      
      // Overwrite the Actor
      actorIndexLast = sys->actorInfo.eleCount - 1;
      actorPtrLast = &sys->actorArray[actorIndexLast];
      
      free(actorPtr->compIndexArray);
      if(actorPtr != actorPtrLast)
      {
         actorPtr->id = actorPtrLast->id;
         actorPtr->compIndexArray = actorPtrLast->compIndexArray;
         
         CompSystem_UpdateActorPointers(sys, actorPtr);
      }
      actorPtrLast->compIndexArray = NULL;
      
      // Decrement Size
      sys->actorInfo.eleCount --;
      
      
   }
}

void CompSystem_SetComponent(CompSystem_T sys, actorid_t actor, comptypeid_t type, void ** compOut)
{
   Actor_T * actorPtr;
   CompType_T * compTypePtr;
   byte_t * dest;
   int destOffset;
   int destIndex;
   int actorIndex;
   
   compTypePtr = &sys->typeArray[type];
   actorIndex = CompSystem_FindActorFromID(sys, actor);
   if(actorIndex != COMPSYSTEM_INVALID_INDEX && compTypePtr != NULL)
   {
      actorPtr = &sys->actorArray[actorIndex];
      if(actorPtr->compIndexArray[type] == COMPSYSTEM_INVALID_INDEX)
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
   }
   else
   {
      dest = NULL;
   }
   
   (*compOut) = dest;
   
   
}

void CompSystem_GetComponent(const CompSystem_T sys, actorid_t actor, comptypeid_t type, int * outIndex, void ** outPointer)
{
   Actor_T * actorPtr;
   CompType_T * compTypePtr;
   byte_t * outPtr;
   int outInd, offset, actorIndex;   
   
   
   compTypePtr = &sys->typeArray[type];
   actorIndex = CompSystem_FindActorFromID(sys, actor);
   if(actorIndex != COMPSYSTEM_INVALID_INDEX)
   {
      actorPtr = &sys->actorArray[actorIndex];
      outInd = actorPtr->compIndexArray[type];
      offset = outInd * compTypePtr->elementSize;
      outPtr = &compTypePtr->compArray[offset];
   }
   else
   {
      outPtr = NULL;
      outInd = COMPSYSTEM_INVALID_INDEX;
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
   int i, j;
   CompType_T * compTypePtr;
   Actor_T * actorPtr;
   byte_t * comp;
   
   
   // Clean Components
   for(i = 0; i < sys->typeInfo.eleCount; i++)
   {
      compTypePtr = &sys->typeArray[i];
      
      if(compTypePtr->compArray != NULL)
      {
         comp = compTypePtr->compArray;
         for(j = 0; j < compTypePtr->compInfo.eleCount; j++)
         {
            CompSystem_DestroyComponent(sys, i,
                                        compTypePtr->actorPtrArray[j]->id,
                                        compTypePtr->destroyFunc,
                                        comp);
            comp += compTypePtr->elementSize;
                                           
         }

      
         free(compTypePtr->compArray);
         free(compTypePtr->actorPtrArray);
      }
   }
   
   // Clean Actors
   for(i = 0; i < sys->actorInfo.eleCount; i++)
   {
      actorPtr = &sys->actorArray[i];
      free(actorPtr->compIndexArray);
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


static int CompSystem_FindActorFromID(CompSystem_T sys, actorid_t actor)
{   
   int i, outIndex;
   
   outIndex = COMPSYSTEM_INVALID_INDEX;
   
   for(i = 0; i < sys->actorInfo.eleCount; i++)
   {
      if(sys->actorArray[i].id == actor)
      {
         outIndex = i;
         break;
      }
   }
   return outIndex;
}

static void CompSystem_UpdateAllActorPointers(CompSystem_T sys)
{
   int actorIndex;
   Actor_T * actorPtr;
   
   for(actorIndex = 0; actorIndex < sys->actorInfo.eleCount; actorIndex++)
   {
      actorPtr = &sys->actorArray[actorIndex];
      CompSystem_UpdateActorPointers(sys, actorPtr);
   }
}

static void CompSystem_UpdateActorPointers(CompSystem_T sys, Actor_T * actorPtr)
{
   int typeIndex, dataIndex;
   CompType_T * compTypePtr;
   for(typeIndex = 0; typeIndex < sys->typeInfo.eleCount; typeIndex++)
   {
      dataIndex = actorPtr->compIndexArray[typeIndex];
      if(dataIndex != COMPSYSTEM_INVALID_INDEX)
      {
         compTypePtr = &sys->typeArray[typeIndex];
         compTypePtr->actorPtrArray[dataIndex] = actorPtr;
      }
   }

}

static void CompSystem_MoveMemory(void * dest, void * src, int elementSize)
{
   if(dest != src)
   {
      memcpy(dest, src, elementSize);
   }   
}

static void CompSystem_DestroyComponent(CompSystem_T sys, comptypeid_t type, 
                                        actorid_t actor, 
                                        CompSystem_DestroyFunc_T destroyFunc, 
                                        void * comp)
{
   if(destroyFunc != NULL)
   {
      destroyFunc(comp, sys, type, actor);
   }
}

