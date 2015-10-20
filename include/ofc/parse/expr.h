#ifndef __ofc_parse_expr_h__
#define __ofc_parse_expr_h__

typedef enum
{
	OFC_PARSE_EXPR_CONSTANT,
	OFC_PARSE_EXPR_VARIABLE,
	OFC_PARSE_EXPR_BRACKETS,
	OFC_PARSE_EXPR_UNARY,
	OFC_PARSE_EXPR_BINARY,
} ofc_parse_expr_e;


typedef struct ofc_parse_expr_list_s ofc_parse_expr_list_t;

struct ofc_parse_expr_s
{
	ofc_parse_expr_e type;

	union
	{
		ofc_parse_literal_t literal;
		ofc_parse_lhs_t*    variable;

		struct
		{
			ofc_parse_expr_t* expr;
		} brackets;

		struct
		{
			ofc_parse_expr_t*    a;
			ofc_parse_operator_e operator;
		} unary;

		struct
		{
			ofc_parse_expr_t*    a;
			ofc_parse_expr_t*    b;
			ofc_parse_operator_e operator;
		} binary;
	};
};

struct ofc_parse_expr_list_s
{
	unsigned           count;
	ofc_parse_expr_t** expr;
};



/* Returns a literal unsigned integer wrapped in an expression type. */
ofc_parse_expr_t* ofc_parse_expr_integer(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	unsigned* len);

ofc_parse_expr_t* ofc_parse_expr(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	unsigned* len);

void ofc_parse_expr_delete(
	ofc_parse_expr_t* expr);
ofc_parse_expr_t* ofc_parse_expr_copy(
	const ofc_parse_expr_t* expr);

bool ofc_parse_expr_print(
	ofc_colstr_t* cs, const ofc_parse_expr_t* expr);

ofc_parse_expr_list_t* ofc_parse_expr_list(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	unsigned* len);
void ofc_parse_expr_list_delete(
	ofc_parse_expr_list_t* list);

#endif