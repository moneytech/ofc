#include <ofc/parse.h>

unsigned ofc_parse_stmt__do_while_block(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	ofc_parse_stmt_t* stmt)
{
	unsigned dpos = ofc_parse_debug_position(debug);

	unsigned i = ofc_parse_keyword(
		src, ptr, debug,
		OFC_PARSE_KEYWORD_WHILE);
	if (i == 0) return 0;

	if (ptr[i++] != '(')
	{
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}

	unsigned len;
	stmt->do_while_block.cond = ofc_parse_expr(
		src, &ptr[i], debug, &len);
	if (!stmt->do_while_block.cond)
	{
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	if (ptr[i++] != ')')
	{
		ofc_parse_expr_delete(stmt->do_while_block.cond);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}

	/* TODO - Make optional? */
	if (!ofc_is_end_statement(&ptr[i], &len))
	{
		ofc_parse_expr_delete(stmt->do_while_block.cond);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	stmt->do_while_block.block
		= ofc_parse_stmt_list(src, &ptr[i], debug, &len);
	if (stmt->do_while_block.block) i += len;

	len = ofc_parse_keyword_end(
		src, &ptr[i], debug,
		OFC_PARSE_KEYWORD_DO, false);
	if (len == 0)
	{
		ofc_parse_stmt_list_delete(
			stmt->do_while_block.block);
		ofc_parse_expr_delete(stmt->do_while_block.cond);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	stmt->type = OFC_PARSE_STMT_DO_WHILE_BLOCK;
	return i;
}

unsigned ofc_parse_stmt__do_while(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	ofc_parse_stmt_t* stmt)
{
	unsigned dpos = ofc_parse_debug_position(debug);

	unsigned i = ofc_parse_label(
		src, ptr, debug,
		&stmt->do_while.end_label);
	if (i == 0) return 0;

	if (ptr[i] == ',')
		i += 1;

	unsigned len = ofc_parse_keyword(
		src, &ptr[i], debug,
		OFC_PARSE_KEYWORD_WHILE);
	if (len == 0)
	{
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	if (ptr[i++] != '(')
	{
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}

	stmt->do_while.cond = ofc_parse_expr(
		src, &ptr[i], debug, &len);
	if (!stmt->do_while.cond)
	{
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	if (ptr[i++] != ')')
	{
		ofc_parse_expr_delete(stmt->do_while.cond);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}

	stmt->type = OFC_PARSE_STMT_DO_WHILE;
	return i;
}

unsigned ofc_parse_stmt__do_label(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	ofc_parse_stmt_t* stmt)
{
	unsigned dpos = ofc_parse_debug_position(debug);

	unsigned i = ofc_parse_label(
		src, ptr, debug,
		&stmt->do_label.end_label);
	if (i == 0) return 0;

	if (ptr[i] != ',')
	{
		/* We only support numeric labels without a comma,
		   because identifiers would be ambiguous. */
		if (stmt->do_label.end_label.type
			!= OFC_PARSE_LABEL_NUMBER)
		{
			ofc_parse_debug_rewind(debug, dpos);
			return 0;
		}
	}
	else
	{
		i += 1;
	}

	unsigned len;
	stmt->do_label.init
		= ofc_parse_assign_init(
			src, &ptr[i], debug, &len);
	if (!stmt->do_label.init)
	{
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	stmt->type = OFC_PARSE_STMT_DO_LABEL;
	stmt->do_label.last = NULL;
	stmt->do_label.step = NULL;

	if (ptr[i++] != ',')
	{
		ofc_parse_assign_delete(stmt->do_label.init);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}

	stmt->do_label.last = ofc_parse_expr(
		src, &ptr[i], debug, &len);
	if (!stmt->do_label.last)
	{
		ofc_parse_assign_delete(stmt->do_label.init);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	if (ptr[i] == ',')
	{
		i += 1;

		stmt->do_label.step = ofc_parse_expr(
			src, &ptr[i], debug, &len);
		if (!stmt->do_label.step)
		{
			ofc_parse_expr_delete(stmt->do_label.last);
			ofc_parse_assign_delete(stmt->do_label.init);
			ofc_parse_debug_rewind(debug, dpos);
			return 0;
		}
		i += len;
	}

	return i;
}

unsigned ofc_parse_stmt__do_block(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	ofc_parse_stmt_t* stmt)
{
	unsigned dpos = ofc_parse_debug_position(debug);

	unsigned i = 0;
	stmt->do_block.init
		= ofc_parse_assign_init(
			src, &ptr[i], debug, &i);
	if (!stmt->do_block.init)
		return 0;

	stmt->type = OFC_PARSE_STMT_DO_BLOCK;
	stmt->do_block.last = NULL;
	stmt->do_block.step = NULL;

	if (ptr[i++] != ',')
	{
		ofc_parse_assign_delete(stmt->do_block.init);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}

	unsigned len;
	stmt->do_block.last = ofc_parse_expr(
		src, &ptr[i], debug, &len);
	if (!stmt->do_block.last)
	{
		ofc_parse_assign_delete(stmt->do_block.init);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	if (ptr[i] == ',')
	{
		i += 1;

		stmt->do_block.step = ofc_parse_expr(
			src, &ptr[i], debug, &len);
		if (!stmt->do_block.step)
		{
			ofc_parse_expr_delete(stmt->do_block.last);
			ofc_parse_assign_delete(stmt->do_block.init);
			ofc_parse_debug_rewind(debug, dpos);
			return 0;
		}
		i += len;
	}

	/* TODO - Make optional? */
	if (!ofc_is_end_statement(&ptr[i], &len))
	{
		ofc_parse_expr_delete(stmt->do_block.last);
		ofc_parse_assign_delete(stmt->do_block.init);
		ofc_parse_debug_rewind(debug, dpos);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	stmt->do_block.block
		= ofc_parse_stmt_list(src, &ptr[i], debug, &len);
	if (stmt->do_block.block) i += len;

	len = ofc_parse_keyword_end(
		src, &ptr[i], debug,
		OFC_PARSE_KEYWORD_DO, false);
	if (len == 0)
	{
		ofc_parse_stmt_list_delete(
			stmt->do_block.block);
		ofc_parse_expr_delete(stmt->do_block.last);
		ofc_parse_assign_delete(stmt->do_block.init);
		ofc_parse_debug_rewind(debug, dpos);
		ofc_parse_debug_rewind(debug, dpos);
		return 0;
	}
	i += len;

	return i;
}

unsigned ofc_parse_stmt_do(
	const ofc_sparse_t* src, const char* ptr,
	ofc_parse_debug_t* debug,
	ofc_parse_stmt_t* stmt)
{
	unsigned dpos = ofc_parse_debug_position(debug);

	unsigned i = ofc_parse_keyword(
		src, ptr, debug, OFC_PARSE_KEYWORD_DO);
	if (i == 0) return 0;

	unsigned l;

	l = ofc_parse_stmt__do_while(
		src, &ptr[i], debug, stmt);
	if (l > 0) return (i + l);

	l = ofc_parse_stmt__do_while_block(
		src, &ptr[i], debug, stmt);
	if (l > 0) return (i + l);

	/* This must come before do_block. */
	l = ofc_parse_stmt__do_label(
		src, &ptr[i], debug, stmt);
	if (l > 0) return (i + l);

	l = ofc_parse_stmt__do_block(
		src, &ptr[i], debug, stmt);
	if (l > 0) return (i + l);

	ofc_parse_debug_rewind(debug, dpos);
	return 0;
}



bool ofc_parse_stmt__do_while_block_print(
	ofc_colstr_t* cs, const ofc_parse_stmt_t* stmt, unsigned indent)
{
	if (!ofc_colstr_atomic_writef(cs, "DO WHILE(")
		|| !ofc_parse_expr_print(cs, stmt->do_while_block.cond)
		|| !ofc_colstr_atomic_writef(cs, ")"))
		return false;

	if (stmt->do_while_block.block && !ofc_parse_stmt_list_print(
		cs, stmt->do_while_block.block, (indent + 1)))
		return false;

	if (!ofc_colstr_newline(cs, NULL))
		return false;

	unsigned j;
	for (j = 0; j < indent; j++)
	{
		if (!ofc_colstr_atomic_writef(cs, "  "))
			return false;
	}

	return ofc_colstr_atomic_writef(cs, "END DO");
}

bool ofc_parse_stmt__do_while_print(
	ofc_colstr_t* cs, const ofc_parse_stmt_t* stmt)
{
	return (ofc_colstr_atomic_writef(cs, "DO ")
		&& ofc_parse_label_print(cs, stmt->do_while.end_label)
		&& ofc_colstr_atomic_writef(cs, ", WHILE(")
		&& ofc_parse_expr_print(cs, stmt->do_while.cond)
		&& ofc_colstr_atomic_writef(cs, ")"));
}

bool ofc_parse_stmt__do_label_print(
	ofc_colstr_t* cs, const ofc_parse_stmt_t* stmt)
{
	if (!ofc_colstr_atomic_writef(cs, "DO ")
		|| !ofc_parse_label_print(cs, stmt->do_label.end_label)
		|| !ofc_colstr_atomic_writef(cs, ", ")
		|| !ofc_parse_assign_print(cs, stmt->do_label.init)
		|| !ofc_colstr_atomic_writef(cs, ", ")
		|| !ofc_parse_expr_print(cs, stmt->do_label.last))
		return false;

	if (stmt->do_label.step)
	{
		if (!ofc_colstr_atomic_writef(cs, ", ")
			|| !ofc_parse_expr_print(
				cs, stmt->do_label.step))
			return false;
	}

	return true;
}

bool ofc_parse_stmt__do_block_print(
	ofc_colstr_t* cs, const ofc_parse_stmt_t* stmt, unsigned indent)
{
	if (!ofc_colstr_atomic_writef(cs, "DO ")
		|| !ofc_parse_assign_print(cs, stmt->do_block.init)
		|| !ofc_colstr_atomic_writef(cs, ", ")
		|| !ofc_parse_expr_print(cs, stmt->do_block.last))
		return false;

	if (stmt->do_block.step)
	{
		if (!ofc_colstr_atomic_writef(cs, ", ")
			|| !ofc_parse_expr_print(
				cs, stmt->do_block.step))
			return false;
	}

	if (stmt->do_block.block && !ofc_parse_stmt_list_print(
			cs, stmt->do_block.block, (indent + 1)))
		return false;

	if (!ofc_colstr_newline(cs, NULL))
		return false;

	unsigned j;
	for (j = 0; j < indent; j++)
	{
		if (!ofc_colstr_atomic_writef(cs, "  "))
			return false;
	}

	return ofc_colstr_atomic_writef(cs, "END DO");
}

bool ofc_parse_stmt_do_print(
	ofc_colstr_t* cs, const ofc_parse_stmt_t* stmt, unsigned indent)
{
	if (!stmt)
		return false;

	switch (stmt->type)
	{
		case OFC_PARSE_STMT_DO_LABEL:
			return ofc_parse_stmt__do_label_print(cs, stmt);
		case OFC_PARSE_STMT_DO_BLOCK:
			return ofc_parse_stmt__do_block_print(
				cs, stmt, indent);
		case OFC_PARSE_STMT_DO_WHILE:
			return ofc_parse_stmt__do_while_print(cs, stmt);
		case OFC_PARSE_STMT_DO_WHILE_BLOCK:
			return ofc_parse_stmt__do_while_block_print(
				cs, stmt, indent);
		default:
			break;
	}

	return false;
}
