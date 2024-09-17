prefix=sos
debug_syntax=0
debug_syntax_avance=0

src=$(prefix).tab.c lex.yy.c analyse.c symbol_table.c quad.c mips.c mips_lib.c
src+= chaines_litterales.c

obj=$(patsubst %.c,%.o,$(src))
dep=$(patsubst %.c,%.d,$(src))

ifneq ($(debug_syntax),0)
	CFLAGS += -DSYNTAX_DEBUG
endif
ifneq ($(debug_syntax_avance),0)
	CFLAGS += -DSYNTAX_DEBUG_AVANCE
endif

# exige 3 fichiers:
# - $(prefix).y (fichier bison)
# - $(prefix).lex (fichier flex)
# - main.c (programme principal)
# construit un exécutable nommé "sos"

# note : le programme principal ne doit surtout pas s'appeler $(prefix).c
# (make l'écraserait parce qu'il a une règle "%.c: %.y")

all: sos

sos: $(obj) main.o
	$(CC) $(LDFLAGS) $^ -o sos $(LDLIBS)

test: $(obj) unit_test.o lib_test.o
	$(CC) $(LDFLAGS) $^ -o sos_test $(LDLIBS)

$(prefix).tab.c: $(prefix).y
	bison -t -d $(prefix).y

lex.yy.c: $(prefix).lex $(prefix).tab.h
	flex $(prefix).lex

doc:
	doxygen
# bison --report=all --report-file=$(prefix).output \
# 	--graph=$(prefix).dot --output=/dev/null \
# 	$(prefix).y
# dot -Tpdf < $(prefix).dot > $(prefix).pdf

-include $(dep)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@ $(LDLIBS)

debug_syntax:
	make debug_syntax=1

debug_syntax_avance:
	make debug_syntax_avance=1

clean:
	rm -f *.o *.d $(prefix).tab.c $(prefix).tab.h lex.yy.c sos sos_test \
		$(prefix).output $(prefix).dot $(prefix).pdf

	rm -rf doc
