# $NetBSD: varname-make_print_var_on_error-jobs.mk,v 1.1 2020/10/23 06:18:23 rillig Exp $
#
# Tests for the special MAKE_PRINT_VAR_ON_ERROR variable, which prints the
# values of selected variables on error.
#
# The variable .ERROR_CMD contains all commands of the target, with variable
# expressions expanded, just as they were printed to the shell command file.
#
# The commands in .ERROR_CMD are space-separated.  Since each command usually
# contains spaces as well, this value is only intended as a first hint to what
# happened.  For more details, use the debug options -de, -dj, -dl, -dn, -dx.

# As of 2020-10-23, .ERROR_CMD only works in parallel mode.
.MAKEFLAGS: -j1

MAKE_PRINT_VAR_ON_ERROR=	.ERROR_TARGET .ERROR_CMD

all:
	@: command before
	@echo fail; false
	@: command after${:U, with variable expressions expanded}
