# $FreeBSD$

# Validate that the user didn't try setting an env-only variable in
# their src.conf. This benefits from already including bsd.mkopt.mk.
# This is checked later in src.own.mk.
.for var in ${__ENV_ONLY_OPTIONS}
__presrcconf_${var}:=	${MK_${var}:U-}${WITHOUT_${var}:Uno:Dyes}${WITH_${var}:Uno:Dyes}
.endfor

# tempting, but bsd.compiler.mk causes problems this early
# probably need to remove dependence on bsd.own.mk 
#.include "src.opts.mk"
