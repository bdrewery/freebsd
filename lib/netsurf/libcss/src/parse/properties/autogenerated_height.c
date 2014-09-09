/*
 * This file was generated by LibCSS gen_parser 
 * 
 * Generated from:
 *
 * height:CSS_PROP_HEIGHT IDENT:( INHERIT: AUTO:0,HEIGHT_AUTO IDENT:) LENGTH_UNIT:( UNIT_PX:HEIGHT_SET DISALLOW:unit&UNIT_ANGLE||unit&UNIT_TIME||unit&UNIT_FREQ RANGE:<0 LENGTH_UNIT:)
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
 * Parse height
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
css_error css__parse_height(css_language *c,
		const parserutils_vector *vector, int *ctx,
		css_style *result)
{
	int orig_ctx = *ctx;
	css_error error;
	const css_token *token;
	bool match;

	token = parserutils_vector_iterate(vector, ctx);
	if (token == NULL) {
		*ctx = orig_ctx;
		return CSS_INVALID;
	}

	if ((token->type == CSS_TOKEN_IDENT) && (lwc_string_caseless_isequal(token->idata, c->strings[INHERIT], &match) == lwc_error_ok && match)) {
			error = css_stylesheet_style_inherit(result, CSS_PROP_HEIGHT);
	} else if ((token->type == CSS_TOKEN_IDENT) && (lwc_string_caseless_isequal(token->idata, c->strings[AUTO], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_HEIGHT, 0,HEIGHT_AUTO);
	} else {
		css_fixed length = 0;
		uint32_t unit = 0;
		*ctx = orig_ctx;

		error = css__parse_unit_specifier(c, vector, ctx, UNIT_PX, &length, &unit);
		if (error != CSS_OK) {
			*ctx = orig_ctx;
			return error;
		}

		if (unit&UNIT_ANGLE||unit&UNIT_TIME||unit&UNIT_FREQ) {
			*ctx = orig_ctx;
			return CSS_INVALID;
		}

		if (length <0) {
			*ctx = orig_ctx;
			return CSS_INVALID;
		}

		error = css__stylesheet_style_appendOPV(result, CSS_PROP_HEIGHT, 0, HEIGHT_SET);
		if (error != CSS_OK) {
			*ctx = orig_ctx;
			return error;
		}

		error = css__stylesheet_style_vappend(result, 2, length, unit);
	}

	if (error != CSS_OK)
		*ctx = orig_ctx;
	
	return error;
}

