CompSystem
==========

A component based architecture library for game development in c

Simple Usage Example
----------

```
actorid_t actor;
comptypeid_t type;
ComponentStruct_T *comp;
int size, i;

// Create new system
CompSystem_T compSys = CompSystem_Create();

// Create new Component Type
CompSystem_NewType(compSys, &type);
CompSystem_SetType(compSys, type, sizeof(ComponentStruct_T), destroyFunc);

// Create new Actor
CompSystem_NewActor(compSys, &actor);
CompSystem_SetComponent(compSys, actor, type, (void **)&comp);
comp->member = 5;

// Iterate over all components of a type
CompSystem_ComponentFor(compSys, type, &comp, &size);
for(i = 0; i < size; i++)
{
   printf("Member: %i\n", comp[i].member);
}
```

Build
----------
You can build it using bam http://matricks.github.io/bam/ or just build it by hand. Should work without special settings.
