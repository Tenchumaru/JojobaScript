!IF "$(Configuration)" == "Debug"
CFLAGS=/JMC /permissive- /GS /W4 /Zc:wchar_t /ZI /Gm- /Od /sdl /Zc:inline /fp:precise /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX /Zc:forScope /RTC1 /Gd /MDd /FC /EHsc /nologo /diagnostics:column
LFLAGS=/MANIFEST /NXCOMPAT /DYNAMICBASE /DEBUG /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1
!ELSEIF "$(Configuration)" == "Release"
CFLAGS=/permissive- /GS /GL /W4 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline /fp:precise /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column
LFLAGS=/MANIFEST /LTCG:incremental /NXCOMPAT /DYNAMICBASE /DEBUG /MACHINE:X64 /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /TLBID:1
!ELSE
!ERROR Unknown configuration "$(Configuration)"
!ENDIF

all: JojobaScript.exe
	ECHO all

clean:
	DEL parser.cc
	DEL parser.h
	DEL scanner.cc

JojobaScript.exe: JojobaScript.obj parser.obj scanner.obj
	link /OUT:$@ JojobaScript.obj parser.obj scanner.obj $(LFLAGS)

JojobaScript.obj: JojobaScript.cc
	cl JojobaScript.cc /c $(CFLAGS)

parser.obj: parser.cc
	cl parser.cc /c $(CFLAGS)

parser.cc: parser.y
	win_bison --verbose --defines=parser.h --output=parser.cc parser.y

scanner.obj: scanner.cc
	cl scanner.cc /c $(CFLAGS)

scanner.cc: scanner.l parser.cc
	win_flex --fast --wincompat --outfile=scanner.cc scanner.l
