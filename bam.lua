settings = NewSettings()

source = 
{
   "CompSystem.c",
   "testmain.c"
}
objects = Compile(settings, source)
exe = Link(settings, "test", objects)
