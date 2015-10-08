#ifndef __parse_file_h__
#define __parse_file_h__

#include "parse.h"

parse_stmt_list_t* parse_file(const sparse_t* src);

bool parse_file_print(
	colstr_t* cs,
	const parse_stmt_list_t* list);

#endif
