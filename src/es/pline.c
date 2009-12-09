/*	SCCS Id: @(#)pline.c	3.4	1999/11/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS /* Uses ... */	/* comment line for pre-compiled headers */
#include "hack.h"
#include "epri.h"
#ifdef WIZARD
#include "edog.h"
#endif

#ifdef OVLB

static boolean no_repeat = FALSE;

static char *FDECL(You_buf, (int));

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vpline, (const char *, va_list));

void
pline VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, char *);
	vpline(line, VA_ARGS);
	VA_END();
}

# ifdef USE_STDARG
static void
vpline(const char *line, va_list the_args) {
# else
static void
vpline(line, the_args) const char *line; va_list the_args; {
# endif

#else	/* USE_STDARG | USE_VARARG */

#define vpline pline

void
pline VA_DECL(const char *, line)
#endif	/* USE_STDARG | USE_VARARG */

	char pbuf[BUFSZ];
/* Do NOT use VA_START and VA_END in here... see above */

	if (!line || !*line) return;
	if (index(line, '%')) {
	    Vsprintf(pbuf,line,VA_ARGS);
	    line = pbuf;
	}
	if (!iflags.window_inited) {
	    raw_print(line);
	    return;
	}
#ifndef MAC
	if (no_repeat && !strcmp(line, toplines))
	    return;
#endif /* MAC */
	if (vision_full_recalc) vision_recalc(0);
	if (u.ux) flush_screen(1);		/* %% */
	putstr(WIN_MESSAGE, 0, line);
}

/*VARARGS1*/
void
Norep VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, const char *);
	no_repeat = TRUE;
	vpline(line, VA_ARGS);
	no_repeat = FALSE;
	VA_END();
	return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char *
You_buf(siz)
int siz;
{
	if (siz > you_buf_siz) {
		if (you_buf) free((genericptr_t) you_buf);
		you_buf_siz = siz + 10;
		you_buf = (char *) alloc((unsigned) you_buf_siz);
	}
	return you_buf;
}

void
free_youbuf()
{
	if (you_buf) free((genericptr_t) you_buf),  you_buf = (char *)0;
	you_buf_siz = 0;
}

/* LENGUA:  YouMessage formerly just concatenated the prefix to the text.
   In the Spanish version, it also prefixes the ¡ and ¿ characters if needed. */
static char *
YouMessage(prefix, text)
const char *prefix;
const char *text;
{
    unsigned len;
    char *pointer;

    pointer = You_buf(strlen(prefix) + strlen(text) + 2);
    pointer[0] = 0;
    len = strlen(text);
    if (len != 0) {
        if (text[len-1] == '!' && strchr(text, '¡') == 0)
            Strcpy(pointer, "¡");
        if (text[len-1] == '?' && strchr(text, '¿') == 0)
            Strcpy(pointer, "¿");
    }
    Strcat(pointer, prefix);
    Strcat(pointer, text);
    return pointer;
}

#if 0
/* LENGUA:  You() will disappear without replacement */
/*VARARGS1*/
void
You VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("You ", line), VA_ARGS);
	VA_END();
}

/* LENGUA:  Your() will disappear without replacement */
/*VARARGS1*/
void
Your VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("Your ", line), VA_ARGS);
	VA_END();
}

/* LENGUA:  You_feel() will disappear; to be replaced by Sientes() */
/*VARARGS1*/
void
You_feel VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("You feel ", line), VA_ARGS);
	VA_END();
}
#endif
/*VARARGS1*/
void
Sientes VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("Sientes ", line), VA_ARGS);
	VA_END();
}


void
Te_sientes VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("Te sientes ", line), VA_ARGS);
	VA_END();
}


/* LENGUA:  You_cant() will disappear; to be replaced by No_puedes() */
/*VARARGS1*/
#if 0
void
You_cant VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("You can't ", line), VA_ARGS);
	VA_END();
}
#endif
/*VARARGS1*/
void
No_puedes VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("No puedes ", line), VA_ARGS);
	VA_END();
}

#if 0
/* LENGUA:  pline_The() will disappear without replacement */
/*VARARGS1*/
void
pline_The VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("The ", line), VA_ARGS);
	VA_END();
}

/* LENGUA:  There() will disappear, to be replaced by Hay() */
/*VARARGS1*/
void
There VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("There ", line), VA_ARGS);
	VA_END();
}
#endif
/*VARARGS1*/
void
Hay VA_DECL(const char *,line)
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage("Hay ", line), VA_ARGS);
	VA_END();
}

/* LENGUA:  You_hear() will disappear, to be replaced by Oyes() */
#if 0
/*VARARGS1*/
void
You_hear VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	if (Underwater)
		tmp = YouMessage("You barely hear ", line);
	else if (u.usleep)
		tmp = YouMessage("You dream that you hear ", line);
	else
		tmp = YouMessage("You hear ", line);
	vpline(tmp, VA_ARGS);
	VA_END();
}
#endif
/*VARARGS1*/
void
Oyes VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	if (Underwater)
		tmp = YouMessage("Oyes débilmente ", line);
	else if (u.usleep)
		tmp = YouMessage("Oyes en tus sueños ", line);
	else
		tmp = YouMessage("Oyes ", line);
	vpline(tmp, VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
verbalize VA_DECL(const char *,line)
	char *tmp;
	if (!flags.soundok) return;
	VA_START(line);
	VA_INIT(line, const char *);
	tmp = You_buf((int)strlen(line) + sizeof "\"\"");
	Strcpy(tmp, "\"");
	Strcat(tmp, line);
	Strcat(tmp, "\"");
	vpline(tmp, VA_ARGS);
	VA_END();
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vraw_printf,(const char *,va_list));

void
raw_printf VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, char *);
	vraw_printf(line, VA_ARGS);
	VA_END();
}

# ifdef USE_STDARG
static void
vraw_printf(const char *line, va_list the_args) {
# else
static void
vraw_printf(line, the_args) const char *line; va_list the_args; {
# endif

#else  /* USE_STDARG | USE_VARARG */

void
raw_printf VA_DECL(const char *, line)
#endif
/* Do NOT use VA_START and VA_END in here... see above */

	if(!index(line, '%'))
	    raw_print(line);
	else {
	    char pbuf[BUFSZ];
	    Vsprintf(pbuf,line,VA_ARGS);
	    raw_print(pbuf);
	}
}


/*VARARGS1*/
void
impossible VA_DECL(const char *, s)
	VA_START(s);
	VA_INIT(s, const char *);
	if (program_state.in_impossible)
		panic("impossible invocó impossible");
	program_state.in_impossible = 1;
	{
	    char pbuf[BUFSZ];
	    Vsprintf(pbuf,s,VA_ARGS);
	    paniclog("impossible", pbuf);
	}
	vpline(s,VA_ARGS);
	pline("Programa en desorden - tal vez haya que #salir.");
	program_state.in_impossible = 0;
	VA_END();
}

/* LENGUA:  Need to call feminize() after using this */
const char *
align_str(alignment)
    aligntyp alignment;
{
    switch ((int)alignment) {
	case A_CHAOTIC: return "caótico";
	case A_NEUTRAL: return "neutral";
	case A_LAWFUL:	return "justo";
	case A_NONE:	return "inalineado";
    }
    return "desconocido";
}

void
mstatusline(mtmp)
register struct monst *mtmp;
{
	aligntyp alignment;
	char info[BUFSZ], monnambuf[BUFSZ];
	char fem;
	unsigned i;

	if (mtmp->ispriest || mtmp->data == &mons[PM_ALIGNED_PRIEST]
				|| mtmp->data == &mons[PM_ANGEL])
		alignment = EPRI(mtmp)->shralign;
	else
		alignment = mtmp->data->maligntyp;
	alignment = (alignment > 0) ? A_LAWFUL :
		(alignment < 0) ? A_CHAOTIC :
		A_NEUTRAL;

	info[0] = 0;
	if (mtmp->mtame) {	  Strcat(info, ", mascota");
#ifdef WIZARD
	    if (wizard) {
		Sprintf(eos(info), " (%d", mtmp->mtame);
		if (!mtmp->isminion)
		    Sprintf(eos(info), "; hambre %ld; apport %d",
			EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
		Strcat(info, ")");
	    }
#endif
	}
	else if (mtmp->mpeaceful) Strcat(info, ", pacífic@");
	if (mtmp->meating)	  Strcat(info, ", comiendo");
	if (mtmp->mcan)		  Strcat(info, ", cancelad@");
	if (mtmp->mconf)	  Strcat(info, ", confundid@");
	if (mtmp->mblinded || !mtmp->mcansee)
				  Strcat(info, ", cieg@");
	if (mtmp->mstun)	  Strcat(info, ", aturdid@");
	if (mtmp->msleeping)	  Strcat(info, ", durmiendo");
#if 0	/* unfortunately mfrozen covers temporary sleep and being busy
	   (donning armor, for instance) as well as paralysis */
	else if (mtmp->mfrozen)	  Strcat(info, ", paralyzed");
#else
	else if (mtmp->mfrozen || !mtmp->mcanmove)
				  Strcat(info, ", no puede mover");
#endif
				  /* [arbitrary reason why it isn't moving] */
	else if (mtmp->mstrategy & STRAT_WAITMASK)
				  Strcat(info, ", meditando");
	else if (mtmp->mflee)	  Strcat(info, ", espantad@");
	if (mtmp->mtrapped)	  Strcat(info, ", trampad@");
	if (mtmp->mspeed)	  Strcat(info,
					mtmp->mspeed == MFAST ? ", rápid@" :
					mtmp->mspeed == MSLOW ? ", lent@" :
					", ???? rapidez");
	if (mtmp->mundetected)	  Strcat(info, ", escondid@");
	if (mtmp->minvis)	  Strcat(info, ", invisible");
	if (mtmp == u.ustuck)	  Strcat(info,
			(sticks(youmonst.data)) ? ", detenid@ por ti" :
				u.uswallow ? (is_animal(u.ustuck->data) ?
				", te ha tragado" :
				", te ha engolfado") :
				", deteniéndote");
#ifdef STEED
	if (mtmp == u.usteed)	  Strcat(info, ", trayéndote");
#endif

	/* Replace the @'s with the proper vowel for gender agreement */
	fem = mon_gender(mtmp)? 'a' : 'o';
	for (i=0; info[i]!=0; i++) {
	    if (info[i] == '@')
		info[i] = fem;
	}

	/* avoid "Status of the invisible newt ..., invisible" */
	/* and unlike a normal mon_nam, use "saddled" even if it has a name */
	Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_THE, (char *)0,
	    (SUPPRESS_IT|SUPPRESS_INVISIBLE), FALSE));

	pline("Estado %s (%s):  Nivel %d  HP %d(%d)  AC %d%s.",
		del(monnambuf),
		feminize(align_str(alignment), mon_gender(mtmp)),
		mtmp->m_lev,
		mtmp->mhp,
		mtmp->mhpmax,
		find_mac(mtmp),
		info);
}

void
ustatusline()
{
	char info[BUFSZ];
	char fem;
	unsigned i;

	info[0] = '\0';
	if (Sick) {
		Strcat(info, ", muriendo de");
		if (u.usick_type & SICK_VOMITABLE)
			Strcat(info, " comida podrida");
		if (u.usick_type & SICK_NONVOMITABLE) {
			if (u.usick_type & SICK_VOMITABLE)
				Strcat(info, " y");
			Strcat(info, " enfermedad");
		}
	}
	if (Stoned)		Strcat(info, ", petrificando");
	if (Slimed)		Strcat(info, ", haciéndose limo");
	if (Strangled)		Strcat(info, ", sufocando");
	if (Vomiting)		Strcat(info, ", nausead@"); /* !"nauseous" */
	if (Confusion)		Strcat(info, ", confundid@");
	if (Blind) {
	    if (u.ucreamed) {
		if ((long)u.ucreamed < Blinded || Blindfolded
						|| !haseyes(youmonst.data))
		    Strcat(info, ", cieg@, cubiert@");
		else
		    Strcat(info, ", encegad@");
		Strcat(info, " por sucio pegajoso");
	    } else
		Strcat(info, ", cieg@");
	}
	if (Stunned)		Strcat(info, ", aturdid@");
#ifdef STEED
	if (!u.usteed)
#endif
	if (Wounded_legs) {
	    const char *what = body_part(LEG);
	    if ((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
		what = makeplural(what);
				Sprintf(eos(info), ", %s danad%cs", what,
					isfeminine(what)? 'a' : 'o');
	}
	if (Glib)		Sprintf(eos(info), ", %s resbalos%cs",
					makeplural(body_part(HAND)),
					isfeminine(body_part(HAND))? 'a' : 'o');
	if (u.utrap)		Strcat(info, ", trampad@");
	if (Fast)		Strcat(info, Very_fast ?
						", muy rápid@" : ", rápid@");
	if (u.uundetected)	Strcat(info, ", escondid@");
	if (Invis)		Strcat(info, ", invisible");
	if (u.ustuck) {
	    if (sticks(youmonst.data))
		Sprintf(info, ", %s", mon_nam_acc(u.ustuck, "deteniendo"));
	    else
		Sprintf(info, ", detenid@ por %s", mon_nam(u.ustuck));
	}

	/* Replace the @'s with the proper vowel for gender agreement */
	fem = poly_gender()==1? 'a' : 'o';
	for (i=0; info[i]!=0; i++) {
	    if (info[i] == '@')
		info[i] = fem;
	}

	pline("Estado %s (%s%s):  Nivel %d  HP %d(%d)  AC %d%s.",
		    del(plname),
		    (u.ualign.record >= 20) ? "píamente " :
		    (u.ualign.record > 13) ? "devotamente " :
		    (u.ualign.record > 8) ? "fervientamente " :
		    (u.ualign.record > 3) ? "fuertemente " :
		    (u.ualign.record == 3) ? "" :
		    (u.ualign.record >= 1) ? "vacilantemente " :
		    (u.ualign.record == 0) ? "nominalmente " :
					    "insuficientemente ",
		feminize(align_str(u.ualign.type), poly_gender()==1),
		Upolyd ? mons[u.umonnum].mlevel : u.ulevel,
		Upolyd ? u.mh : u.uhp,
		Upolyd ? u.mhmax : u.uhpmax,
		u.uac,
		info);
}

void
self_invis_message()
{
	char str[BUFSZ];
	char *p;

	Strcpy(str, Hallucination ? "¡Caramba, hombre!  "
				  : "¡Ay!  De repente, ");
	p = eos(str);
	Strcat(str, See_invisible ? "puedes ver través de ti"
				  : "no puedes verte");
	if (Hallucination)
	    /* Becuase the second sentence begins here */
	    *p = toupper2(*p);
	pline("%s.", str);
}

#endif /* OVLB */
/*pline.c*/
