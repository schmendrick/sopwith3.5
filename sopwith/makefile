sopwith.exe: sopwith.obj sopasm.obj response.rsp
  otlink @response.rsp

sopwith2.exe: sopwith2.obj sopasm.obj respons2.rsp
  otlink @respons2.rsp

mkg.exe: mkg.cpp
  gpp mkg.cpp -omkg.exe

sprites.c: mkg.exe mkg.txt sopwith.spr
  mkg

coll.c: mkg.exe mkg.txt sopwith.spr
  mkg

.asm.obj:
  a86 +o +c $<

.c.obj:
  tcc -v -y -I\prog\include -L\prog\lib -c -w $<

sopwith.obj: sopwith.c def.h sopasm.h sprites.c ground.c

sopwith2.obj: sopwith2.c def.h sopasm.h sprites.c ground.c

sopasm.obj: sopasm.asm

clean:
  del *.bak
  del *.obj
  del *.map
  del *.sym
  del *.exe
  del *.tr
  del *.td
  del *.ms
  del coll.c
  del sprites.c
