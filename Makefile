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

OPTIM_FLAGS = -mavx2

SANITIZE_FLAGS = -lasan -fsanitize=address,leak

TEXT_CFILES  = utils/text.cpp
LIST_CFILES  = list/list.cpp list/list_dump.cpp
LOG_CFILES   = log/log.cpp
UTILS_CFILES = utils/utils.cpp
HASH_CFILES  = hashtable/hashtable.cpp hashtable/optimisation.cpp

ASM_FILES = hashtable/crc32
CFILES = main.cpp $(TEXT_CFILES) $(LIST_CFILES) $(LOG_CFILES) $(UTILS_CFILES) $(HASH_CFILES)
GRAPH_CFILES = graph_hashes.cpp $(TEXT_CFILES) $(LIST_CFILES) $(LOG_CFILES) $(UTILS_CFILES) $(HASH_CFILES)


OUTPUT = hash-tables.out

IMG 		= list
IMG_FORMAT      = png
DOT_FILE_NAME   = list_graph.dot

all:
	@ nasm -f elf64 -l $(ASM_FILES).lst $(ASM_FILES).s
	@ g++ $(OPTIM_FLAGS) -masm=intel -o $(OUTPUT) $(CFLAGS) $(CFILES) $(ASM_FILES).o -no-pie -D$(DFLAG1) -D$(DFLAG2)
	@ echo Compiled c-files

.PHONY: sanitize
sanitize:
	@ clear
	  nasm -f elf64 -l $(ASM_FILES).lst $(ASM_FILES).s
	  g++ $(OPTIM_FLAGS) -o $(OUTPUT) $(CFLAGS) $(CFILES) $(ASM_FILES).o -no-pie -D$(DFLAG1) -D$(DFLAG2) $(SANITIZE_FLAGS)
	@ echo Compiled c-files

.PHONY: graph_hashes
graph_hashes:
	# @ clear
	@ nasm -f elf64 -l $(ASM_FILES).lst $(ASM_FILES).s
	@ g++ $(OPTIM_FLAGS) -masm=intel -o $(OUTPUT) $(CFLAGS) $(GRAPH_CFILES) $(ASM_FILES).o -no-pie -D$(DFLAG1)
	@ echo Compiled c-files
	@ ./$(OUTPUT)

.PHONY: def
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

.PHONY: graphviz
graphviz:
	dot -T $(IMG_FORMAT) -o $(IMG).$(IMG_FORMAT) $(DOT_FILE_NAME)

.PHONY: pygraph
pygraph:
	python graphics/graph.py
