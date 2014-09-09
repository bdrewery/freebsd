/*
 * This file was generated by LibCSS gen_parser 
 * 
 * Generated from:
 *
 * padding_top:CSS_PROP_PADDING_TOP WRAP:css__parse_padding_side
 * 
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2010 The NetSurf Browser Project.
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse padding_top
 *
 * \param c	  Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx	  Pointer to vector iteration context
 * \param result  resulting style
 * \return CSS_OK on success,
 *	   CSS_NOMEM on memory exhaustion,
 *	   CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *		   If the input is invalid, then \a *ctx remains unchanged.
 */
css_error css__parse_padding_top(css_language *c,
		const parserutils_vector *vector, int *ctx,
		css_style *result)
{
	return css__parse_padding_side(c, vector, ctx, result, CSS_PROP_PADDING_TOP);
}
