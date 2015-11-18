#include <ofc/sema.h>


bool ofc_sema_stmt_data(
	ofc_sema_scope_t* scope,
	const ofc_parse_stmt_t* stmt)
{
	if (!scope || !stmt
		|| (stmt->type != OFC_PARSE_STMT_DATA)
		|| !stmt->data)
		return false;

	unsigned j;
	for (j = 0; j < stmt->data->count; j++)
	{
		const ofc_parse_data_entry_t* entry
			= stmt->data->entry[j];
		if (!entry) continue;

		if (!entry->nlist || !entry->clist
			|| (entry->nlist->count == 0))
			return false;

		ofc_sema_decl_t*       decl[entry->nlist->count];
		const ofc_parse_lhs_t* lhs[entry->nlist->count];

		unsigned elems = 0;
		unsigned i;
		for (i = 0; i < entry->nlist->count; i++)
		{
			lhs[i] = entry->nlist->lhs[i];

			ofc_str_ref_t base_name;
			if (!ofc_parse_lhs_base_name(
				*(lhs[i]), &base_name))
				return false;

			decl[i] = ofc_sema_scope_decl_find_modify(
				scope, base_name);
			if (!decl[i])
			{
				if (lhs[i]->type != OFC_PARSE_LHS_VARIABLE)
				{
					ofc_sema_scope_error(scope, stmt->src,
						"Can only implicitly declare scalar variables in DATA statment");
					return false;
				}

				decl[i] = ofc_sema_decl_implicit_name(
					scope, base_name);
				if (!decl[i])
				{
					ofc_str_ref_t n = base_name;
					ofc_sema_scope_error(scope, stmt->src,
						"No declaration for '%.*s' and no valid IMPLICIT rule.",
						n.size, n.base);
					return false;
				}

				if (!ofc_sema_decl_list_add(
					scope->decl, decl[i]))
				{
					ofc_sema_decl_delete(decl[i]);
					return false;
				}
			}

			ofc_sema_lhs_t* slhs
				= ofc_sema_lhs(scope, lhs[i]);
			elems += ofc_sema_type_elem_count(
				ofc_sema_lhs_type(slhs));
			ofc_sema_lhs_delete(slhs);
		}

		unsigned ctotal = 0;
		for (i = 0; i < entry->clist->count; i++)
		{
			if (!entry->clist->entry[i])
				continue;
			ctotal += (entry->clist->entry[i]->repeat == 0 ? 1
				: entry->clist->entry[i]->repeat);
		}

		ofc_sema_expr_t* bexpr[entry->clist->count];
        const ofc_sema_expr_t* cexpr[ctotal];

		unsigned k = 0;
		for (i = 0; i < entry->clist->count; i++)
		{
			if (!entry->clist->entry[i])
				continue;

			unsigned count = (entry->clist->entry[i]->repeat == 0 ? 1
				: entry->clist->entry[i]->repeat);

			bexpr[i] = ofc_sema_expr(scope,
				entry->clist->entry[i]->expr);
			if (!bexpr[i])
			{
				unsigned l;
				for (l = 0; l < i; l++)
					ofc_sema_expr_delete(bexpr[i]);
				return false;
			}

			unsigned l;
			for (l = 0; l < count; k++, l++)
				cexpr[k] = bexpr[i];
		}

		if (ctotal < elems)
		{
			ofc_sema_scope_warning(scope, stmt->src,
				"Not enough initializer elements in DATA statement.");
		}
		else if (ctotal > elems)
		{
			ofc_sema_scope_warning(scope, stmt->src,
				"Too many initializer elements in DATA statement, ignoring.");
			elems = ctotal;
		}

		for (i = 0, k = 0; i < entry->nlist->count; i++)
		{
			ofc_sema_array_index_t* index = NULL;
			ofc_sema_array_t*       slice = NULL;

			unsigned elem_count;
			if (lhs[i]->type == OFC_PARSE_LHS_VARIABLE)
			{
				elem_count = ofc_sema_decl_elem_count(decl[i]);
				if (elem_count > (ctotal - k))
					elem_count = (ctotal - k);
			}
			else if (lhs[i]->type == OFC_PARSE_LHS_ARRAY)
			{
				if (!ofc_sema_decl_is_array(decl[i]))
				{
					ofc_sema_scope_error(scope, lhs[i]->src,
						"Can't index non-array type.");
					for (i = 0; i < entry->clist->count; i++)
						ofc_sema_expr_delete(bexpr[i]);
					return false;
				}

				index = ofc_sema_array_index(
					scope, decl[i]->type->array,
					lhs[i]->array.index);
				elem_count = 1;

				if (!index)
				{
					slice = ofc_sema_array(
						scope, decl[i]->type->array,
						lhs[i]->array.index);
					/*elem_count = ofc_sema_array_elem_count(slice);*/
				}
			}
			else
			{
				ofc_sema_scope_error(scope, lhs[i]->src,
					"Invalid LHS in DATA statement.");
				for (i = 0; i < entry->clist->count; i++)
					ofc_sema_expr_delete(bexpr[i]);
				return false;
			}

			if (ofc_sema_decl_is_array(decl[i]))
			{
				if (index)
				{
					/* TODO - Support array index initializers. */
					ofc_sema_scope_error(scope, stmt->src,
						"Array index initializers not yet supported.");
					ofc_sema_array_index_delete(index);
					for (i = 0; i < entry->clist->count; i++)
						ofc_sema_expr_delete(bexpr[i]);
					return false;
				}
				else if (slice)
				{
					/* TODO - Support array slice initializers. */
					ofc_sema_scope_error(scope, stmt->src,
						"Array slice initializers not yet supported.");
					ofc_sema_array_delete(slice);
					for (i = 0; i < entry->clist->count; i++)
						ofc_sema_expr_delete(bexpr[i]);
					return false;
				}
				else
				{
					if (!ofc_sema_decl_init_array(
						scope, decl[i], NULL, elem_count, &cexpr[k]))
					{
						for (i = 0; i < entry->clist->count; i++)
							ofc_sema_expr_delete(bexpr[i]);
						return false;
					}
				}
			}
			else
			{
				if (!ofc_sema_decl_init(
					scope, decl[i], cexpr[k]))
				{
					for (i = 0; i < entry->clist->count; i++)
						ofc_sema_expr_delete(bexpr[i]);
					return false;
				}
			}

			k += elem_count;
		}

		for (i = 0; i < entry->clist->count; i++)
			ofc_sema_expr_delete(bexpr[i]);
	}

	return true;
}
