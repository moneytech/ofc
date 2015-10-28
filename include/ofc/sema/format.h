#ifndef __ofc_sema_format_h__
#define __ofc_sema_format_h__

typedef struct
{
	const ofc_parse_format_desc_list_t* src;
	ofc_parse_format_desc_list_t*       format;
} ofc_sema_format_t;


bool ofc_sema_format(
	ofc_sema_scope_t* scope,
	const ofc_parse_stmt_t* stmt);
void ofc_sema_format_delete(
	ofc_sema_format_t* format);

#endif