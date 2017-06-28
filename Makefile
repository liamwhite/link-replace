all: link-replace
link-replace: link-replace.o traverse.o replace.o str-replace.o

link-replace.o: link-replace.c traverse.h
traverse.o: traverse.c traverse.h replace.h error.h
replace.o: replace.c replace.h str-replace.h error.h
str-replace.o: str-replace.c str-replace.h
