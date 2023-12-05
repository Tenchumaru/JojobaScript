all: Value.cc.inl Value.h.inl parser.inl scanner.inl

clean:
	DEL Value.cc.inl
	DEL Value.h.inl
	DEL parser.inl
	DEL yy.h
	DEL scanner.inl

Value.cc.inl: Value.cc.tt Value.txt
	TextTransform Value.cc.tt

Value.h.inl: Value.h.tt Value.txt
	TextTransform Value.h.tt

parser.inl: parser.y
	win_bison --verbose --defines=yy.h --output=parser.inl -Wcounterexamples parser.y

scanner.inl: scanner.l parser.inl
	win_flex --fast --wincompat --outfile=scanner.inl scanner.l
