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

void CompSystem_SetComponent(CompSystem_T sys, actorid_t actor, comptypeid_t type, void ** compOut);
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

