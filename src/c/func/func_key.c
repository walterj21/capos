#include <stddef.h>
#include "include/func.h"
#include "include/ufunc.h"
#include "include/terminal.h"

struct func func_key[] = {
	{"clear", t_clear},			// in terminal.c
	{"add", add},
	{"mul", mul},
	{"help", help},
	{"ufuncs", ufuncs},
	{"sans_undertale", sans_undertale},	// in music.c
	{"happy_birthday", happy_birthday},	// in music.c
	{"mem_status", umem_status},
	{"mem_hops", mem_hops},
	{"print_mem", print_mem},
	{"mem_poke", mem_poke},
	{"defragment", udefrag},
	{"echo", echo},
	{"time", time},
	{"", NULL}
};
