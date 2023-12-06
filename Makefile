BASH="C:\Program Files\Git\usr\bin\bash.exe"
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
	$(BASH) -c ". bashrc;sed -i -e '/yylineno/,$$d' parser.inl"

scanner.inl: scanner.l parser.inl
	win_flex --fast --wincompat --outfile=scanner.inl scanner.l
	$(BASH) -c ". bashrc;sed -i -e s/TO.*DO// scanner.inl"
