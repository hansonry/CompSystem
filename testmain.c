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
static void addActors(CompSystem_T sys, comptypeid_t * types, int start, int count);
static void loop(CompSystem_T sys, comptypeid_t * types);
static void jumptest(CompSystem_T sys, comptypeid_t * types, actorid_t actor);
static void destroy(int * comp, CompSystem_T sys, comptypeid_t type, actorid_t actor);

int main(int argc, char * args[])
{
   CompSystem_T sys;
   comptypeid_t types[eComp_Last];
   
   sys = CompSystem_Create();

   createTypes(sys, types);   
   addActors(sys, types, 0, 20);
   jumptest(sys, types, 3);
   
   loop(sys, types);
   
   // Remove First Element
   CompSystem_RemoveActor(sys, 0);
   jumptest(sys, types, 19);
   loop(sys, types);
   
   // Remove Second Element
   CompSystem_RemoveActor(sys, 18);
   jumptest(sys, types, 17);
   loop(sys, types);
   
   addActors(sys, types, 40, 2);
   jumptest(sys, types, 1);
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
      CompSystem_SetType(sys, types[i], sizeof(int), (CompSystem_DestroyFunc_T)&destroy);
   }
}

static void addActors(CompSystem_T sys, comptypeid_t * types, int start, int count)
{
   actorid_t actor1;
   int *rawValue, *array, i;
   for(i = 0; i < count; i++)
   {
      CompSystem_NewActor(sys, &actor1);      
      CompSystem_SetComponent(sys, actor1, types[eComp_Position], (void**)&rawValue);
      (*rawValue) = start + i;

      CompSystem_SetComponent(sys, actor1, types[eComp_Physics], (void**)&rawValue);
      (*rawValue) = (start + i) << 1;
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

static void destroy(int * comp, CompSystem_T sys, comptypeid_t type, actorid_t actor)
{
   printf("Destroy: (a, t, v) = (%i, %i, %i)\n", actor, type, *comp);
}

