CFLAGS= -Wshadow    						\
	-Winit-self 						\
	-Wredundant-decls 					\
	-Wcast-align						\
	-Wundef 						\
	-Wfloat-equal 						\
	-Winline 						\
	-Wunreachable-code 					\
	-Wmissing-declarations 					\
	-Wmissing-include-dirs 					\
	-Wswitch-enum 						\
	-Wswitch-default 					\
	-Weffc++ 						\
	-Wmain 							\
	-Wextra 						\
	-Wall 							\
	-g 							\
	-pipe 							\
	-fexceptions 						\
	-Wcast-qual 						\
	-Wconversion 						\
	-Wctor-dtor-privacy 					\
	-Wempty-body 						\
	-Wformat-security 					\
	-Wformat=2 						\
	-Wignored-qualifiers 					\
	-Wlogical-op 						\
	-Wmissing-field-initializers 				\
	-Wnon-virtual-dtor 					\
	-Woverloaded-virtual 					\
	-Wpointer-arith 					\
	-Wsign-promo 						\
	-Wstack-usage=8192 					\
	-Wstrict-aliasing 					\
	-Wstrict-null-sentinel 					\
	-Wtype-limits 						\
	-Wwrite-strings 					\
	-D_DEBUG 						\
	-D_EJUDGE_CLIENT_SIDE

TEXT_CFILES  = text.cpp
LIST_CFILES  = list/list.cpp
LOG_CFILES   = log/log.cpp
UTILS_CFILES = utils.cpp
HASH_CFILES  = hashtable/hashtable.cpp

CFILES = main.cpp $(TEXT_CFILES) $(LIST_CFILES) $(LOG_CFILES) $(UTILS_CFILES) $(HASH_CFILES)

OUTPUT = hash-tables.out

all:
	@ clear
	@ g++ -o $(OUTPUT) $(CFLAGS) $(CFILES) -lasan
	@ echo Compiled c-files
def:
	@ g++ -E $(OUTPUT) $(CFLAGS) $(CFILES) >> defines.txt

.PHONY: run
run:
	@ ./$(OUTPUT)
	@ echo Run

.PHONY: clean
clean:
	ifneq ($(OUTPUT))
		del -f $(OUTPUT)
	endif

.PHONY: clean_cmd
clean_cmd:
	@ cls
