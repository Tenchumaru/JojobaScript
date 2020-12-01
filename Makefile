all: parser.cc scanner.cc

clean:
	DEL parser.cc
	DEL parser.h
	DEL scanner.cc

parser.cc: parser.y
	win_bison --verbose --defines=parser.h --output=parser.cc parser.y

scanner.cc: scanner.l parser.cc
	win_flex --fast --wincompat --outfile=scanner.cc scanner.l
