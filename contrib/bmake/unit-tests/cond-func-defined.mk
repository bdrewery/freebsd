# $NetBSD: cond-func-defined.mk,v 1.3 2020/08/20 17:23:43 rillig Exp $
#
# Tests for the defined() function in .if conditions.

DEF=		defined
${:UA B}=	variable name with spaces

.if !defined(DEF)
.error
.endif

# Horizontal whitespace after the opening parenthesis is ignored.
.if !defined( 	DEF)
.error
.endif

# Horizontal whitespace before the closing parenthesis is ignored.
.if !defined(DEF 	)
.error
.endif

# The argument of a function must not directly contain whitespace.
.if !defined(A B)
.error
.endif

# If necessary, the whitespace can be generated by a variable expression.
.if !defined(${:UA B})
.error
.endif

all:
	@:;
