all: Value.cc.g.inl Value.h.g.inl parser.cc scanner.cc

clean:
	DEL Value.cc.g.inl
	DEL Value.h.g.inl
	DEL parser.cc
	DEL parser.h
	DEL scanner.cc

Value.cc.g.inl: Value.cc.tt Value.txt
	TextTransform Value.cc.tt

Value.h.g.inl: Value.h.tt Value.txt
	TextTransform Value.h.tt

parser.cc: parser.y
	win_bison --verbose --defines=parser.h --output=parser.cc -Wcounterexamples parser.y

scanner.cc: scanner.l parser.cc
	win_flex --fast --wincompat --outfile=scanner.cc scanner.l
