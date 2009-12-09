/*	SCCS Id: @(#)do_name.c	3.4	2003/01/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVLB

STATIC_DCL void FDECL(do_oname, (struct obj *));
static void FDECL(getpos_help, (BOOLEAN_P,const char *));

extern const char what_is_an_unknown_object[];		/* from pager.c */

/* the response for '?' help request in getpos() */
static void
getpos_help(force, goal)
boolean force;
const char *goal;
{
    char sbuf[BUFSZ];
    boolean doing_what_is;
    winid tmpwin = create_nhwindow(NHW_MENU);

    Sprintf(sbuf, "Usa [%s] para mover el cursor a %s.",
	    iflags.num_pad ? "2468" : "hjkl", goal);
    putstr(tmpwin, 0, sbuf);
    putstr(tmpwin, 0, "Usa [HJKL] para mover el cursor 8 unidades a la vez.");
    putstr(tmpwin, 0, "O entra un símbolo de fondo (ej. <).");
    /* disgusting hack; the alternate selection characters work for any
       getpos call, but they only matter for dowhatis (and doquickwhatis) */
    doing_what_is = (goal == what_is_an_unknown_object);
    Sprintf(sbuf, "Teclea un .%s cuando estés en el lugar correcto.",
	    doing_what_is ? " o , o ; o :" : "");
    putstr(tmpwin, 0, sbuf);
    if (!force)
	putstr(tmpwin, 0, "Teclea Espacio or Escape cuando termines.");
    putstr(tmpwin, 0, "");
    display_nhwindow(tmpwin, TRUE);
    destroy_nhwindow(tmpwin);
}

int
getpos(cc, force, goal)
coord *cc;
boolean force;
const char *goal;
{
    int result = 0;
    int cx, cy, i, c;
    int sidx, tx, ty;
    boolean msg_given = TRUE;	/* clear message window by default */
    static const char pick_chars[] = ".,;:";
    const char *cp;
    const char *sdp;
    if(iflags.num_pad) sdp = ndir; else sdp = sdir;	/* DICE workaround */

    if (flags.verbose) {
	pline("(Para más instrucciones teclea un ?)");
	msg_given = TRUE;
    }
    cx = cc->x;
    cy = cc->y;
#ifdef CLIPPING
    cliparound(cx, cy);
#endif
    curs(WIN_MAP, cx,cy);
    flush_screen(0);
#ifdef MAC
    lock_mouse_cursor(TRUE);
#endif
    for (;;) {
	c = nh_poskey(&tx, &ty, &sidx);
	if (c == '\033') {
	    cx = cy = -10;
	    msg_given = TRUE;	/* force clear */
	    result = -1;
	    break;
	}
	if(c == 0) {
	    if (!isok(tx, ty)) continue;
	    /* a mouse click event, just assign and return */
	    cx = tx;
	    cy = ty;
	    break;
	}
	if ((cp = index(pick_chars, c)) != 0) {
	    /* '.' => 0, ',' => 1, ';' => 2, ':' => 3 */
	    result = cp - pick_chars;
	    break;
	}
	for (i = 0; i < 8; i++) {
	    int dx, dy;

	    if (sdp[i] == c) {
		/* a normal movement letter or digit */
		dx = xdir[i];
		dy = ydir[i];
	    } else if (sdir[i] == tolower2((char)c)) {
		/* a shifted movement letter */
		dx = 8 * xdir[i];
		dy = 8 * ydir[i];
	    } else
		continue;

	    /* truncate at map edge; diagonal moves complicate this... */
	    if (cx + dx < 1) {
		dy -= sgn(dy) * (1 - (cx + dx));
		dx = 1 - cx;		/* so that (cx+dx == 1) */
	    } else if (cx + dx > COLNO-1) {
		dy += sgn(dy) * ((COLNO-1) - (cx + dx));
		dx = (COLNO-1) - cx;
	    }
	    if (cy + dy < 0) {
		dx -= sgn(dx) * (0 - (cy + dy));
		dy = 0 - cy;		/* so that (cy+dy == 0) */
	    } else if (cy + dy > ROWNO-1) {
		dx += sgn(dx) * ((ROWNO-1) - (cy + dy));
		dy = (ROWNO-1) - cy;
	    }
	    cx += dx;
	    cy += dy;
	    goto nxtc;
	}

	if(c == '?'){
	    getpos_help(force, goal);
	} else {
	    if (!index(quitchars, c)) {
		char matching[MAXPCHARS];
		int pass, lo_x, lo_y, hi_x, hi_y, k = 0;
		(void)memset((genericptr_t)matching, 0, sizeof matching);
		for (sidx = 1; sidx < MAXPCHARS; sidx++)
		    if (c == defsyms[sidx].sym || c == (int)showsyms[sidx])
			matching[sidx] = (char) ++k;
		if (k) {
		    for (pass = 0; pass <= 1; pass++) {
			/* pass 0: just past current pos to lower right;
			   pass 1: upper left corner to current pos */
			lo_y = (pass == 0) ? cy : 0;
			hi_y = (pass == 0) ? ROWNO - 1 : cy;
			for (ty = lo_y; ty <= hi_y; ty++) {
			    lo_x = (pass == 0 && ty == lo_y) ? cx + 1 : 1;
			    hi_x = (pass == 1 && ty == hi_y) ? cx : COLNO - 1;
			    for (tx = lo_x; tx <= hi_x; tx++) {
				k = levl[tx][ty].glyph;
				if (glyph_is_cmap(k) &&
					matching[glyph_to_cmap(k)]) {
				    cx = tx,  cy = ty;
				    if (msg_given) {
					clear_nhwindow(WIN_MESSAGE);
					msg_given = FALSE;
				    }
				    goto nxtc;
				}
			    }	/* column */
			}	/* row */
		    }		/* pass */
		    pline("No se puede encontrar el elemento del calabozo '%c'.", c);
		    msg_given = TRUE;
		    goto nxtc;
		} else {
		    pline("Dirección desconocida: '%s' (%s).",
			  visctrl((char)c),
			  !force ? "abortado" :
			  iflags.num_pad ? "usa 2468 o ." : "usa hjkl o .");
		    msg_given = TRUE;
		} /* k => matching */
	    } /* !quitchars */
	    if (force) goto nxtc;
	    pline("Listo.");
	    msg_given = FALSE;	/* suppress clear */
	    cx = -1;
	    cy = 0;
	    result = 0;	/* not -1 */
	    break;
	}
    nxtc:	;
#ifdef CLIPPING
	cliparound(cx, cy);
#endif
	curs(WIN_MAP,cx,cy);
	flush_screen(0);
    }
#ifdef MAC
    lock_mouse_cursor(FALSE);
#endif
    if (msg_given) clear_nhwindow(WIN_MESSAGE);
    cc->x = cx;
    cc->y = cy;
    return result;
}

struct monst *
christen_monst(mtmp, name)
struct monst *mtmp;
const char *name;
{
	int lth;
	struct monst *mtmp2;
	char buf[PL_PSIZ];

	/* dogname & catname are PL_PSIZ arrays; object names have same limit */
	lth = *name ? (int)(strlen(name) + 1) : 0;
	if(lth > PL_PSIZ){
		lth = PL_PSIZ;
		name = strncpy(buf, name, PL_PSIZ - 1);
		buf[PL_PSIZ - 1] = '\0';
	}
	if (lth == mtmp->mnamelth) {
		/* don't need to allocate a new monst struct */
		if (lth) Strcpy(NAME(mtmp), name);
		return mtmp;
	}
	mtmp2 = newmonst(mtmp->mxlth + lth);
	*mtmp2 = *mtmp;
	(void) memcpy((genericptr_t)mtmp2->mextra,
		      (genericptr_t)mtmp->mextra, mtmp->mxlth);
	mtmp2->mnamelth = lth;
	if (lth) Strcpy(NAME(mtmp2), name);
	replmon(mtmp,mtmp2);
	return(mtmp2);
}

int
do_mname()
{
	char buf[BUFSZ];
	coord cc;
	register int cx,cy;
	register struct monst *mtmp;
	char qbuf[QBUFSZ];

	if (Hallucination) {
		pline("Nunca lo reconocerías en cualquier caso.");
		return 0;
	}
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, FALSE, "el monstruo que quieres llamar") < 0 ||
			(cx = cc.x) < 0)
		return 0;
	cy = cc.y;

	if (cx == u.ux && cy == u.uy) {
#ifdef STEED
	    if (u.usteed && canspotmon(u.usteed))
		mtmp = u.usteed;
	    else {
#endif
		pline("Esta criatura %s se llama %s y no puede cambiar su nombre.",
		ACURR(A_CHA) > 14 ? "linda" : "fea",
		plname);
		return(0);
#ifdef STEED
	    }
#endif
	} else
	    mtmp = m_at(cx, cy);

	if (!mtmp || (!sensemon(mtmp) &&
			(!(cansee(cx,cy) || see_with_infrared(mtmp)) || mtmp->mundetected
			|| mtmp->m_ap_type == M_AP_FURNITURE
			|| mtmp->m_ap_type == M_AP_OBJECT
			|| (mtmp->minvis && !See_invisible)))) {
		pline("No veo ningún monstruo allí.");
		return(0);
	}
	/* special case similar to the one in lookat() */
	(void) distant_monnam(mtmp, ARTICLE_THE, buf);
	Sprintf(qbuf, "¿Cómo quieres llamar %s?", buf);
	getlin(qbuf,buf);
	if(!*buf || *buf == '\033') return(0);
	/* strip leading and trailing spaces; unnames monster if all spaces */
	(void)mungspaces(buf);

	if (mtmp->data->geno & G_UNIQ)
	    /* LENGUA:  This pun apparently doesn't work in Spanish */
	    pline("¡A %s no le gusta que le pongan nombres!",
		  Monnam(mtmp));
	else
	    (void) christen_monst(mtmp, buf);
	return(0);
}

/*
 * This routine changes the address of obj. Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when obj is in the inventory.
 */
STATIC_OVL
void
do_oname(obj)
register struct obj *obj;
{
	char *que;
	char buf[BUFSZ], qbuf[QBUFSZ];
	const char *aname;
	short objtyp;

	que = xname(obj);
	Sprintf(qbuf, "¿Cómo quieres nombrar %s %s?",
		isfeminine(que)? is_plural(obj) ? "estas" : "esta"
			       : is_plural(obj) ? "estos"  : "este",
		que);
	getlin(qbuf, buf);
	if(!*buf || *buf == '\033')	return;
	/* strip leading and trailing spaces; unnames item if all spaces */
	(void)mungspaces(buf);

	/* relax restrictions over proper capitalization for artifacts */
	if ((aname = artifact_name(buf, &objtyp)) != 0 && objtyp == obj->otyp)
		Strcpy(buf, aname);

	if (obj->oartifact) {
		pline("El artefacto parece resistirse al intento.");
		return;
	} else if (restrict_name(obj, buf) || exist_artifact(obj->otyp, buf)) {
		int n = rn2((int)strlen(buf));
		register char c1, c2;

		c1 = tolower2(buf[n]);
		do c2 = 'a' + rn2('z'-'a'); while (c1 == c2);
		buf[n] = (buf[n] == c1) ? c2 : toupper2(c2);  /* keep same case */
		pline("Mientras grabas, tu %s se resbala.", body_part(HAND));
		display_nhwindow(WIN_MESSAGE, FALSE);
		pline("Grabas: \"%s\".",buf);
	}
	obj = oname(obj, buf);
}

/*
 * Allocate a new and possibly larger storage space for an obj.
 */
struct obj *
realloc_obj(obj, oextra_size, oextra_src, oname_size, name)
struct obj *obj;
int oextra_size;		/* storage to allocate for oextra            */
genericptr_t oextra_src;
int oname_size;			/* size of name string + 1 (null terminator) */
const char *name;
{
	struct obj *otmp;

	otmp = newobj(oextra_size + oname_size);
	*otmp = *obj;	/* the cobj pointer is copied to otmp */
	if (oextra_size) {
	    if (oextra_src)
		(void) memcpy((genericptr_t)otmp->oextra, oextra_src,
							oextra_size);
	} else {
	    otmp->oattached = OATTACHED_NOTHING;
	}
	otmp->oxlth = oextra_size;

	otmp->onamelth = oname_size;
	otmp->timed = 0;	/* not timed, yet */
	otmp->lamplit = 0;	/* ditto */
	/* __GNUC__ note:  if the assignment of otmp->onamelth immediately
	   precedes this `if' statement, a gcc bug will miscompile the
	   test on vax (`insv' instruction used to store bitfield does
	   not set condition codes, but optimizer behaves as if it did).
	   gcc-2.7.2.1 finally fixed this. */
	if (oname_size) {
	    if (name)
		Strcpy(ONAME(otmp), name);
	}

	if (obj->owornmask) {
		boolean save_twoweap = u.twoweap;
		/* unwearing the old instance will clear dual-wield mode
		   if this object is either of the two weapons */
		setworn((struct obj *)0, obj->owornmask);
		setworn(otmp, otmp->owornmask);
		u.twoweap = save_twoweap;
	}

	/* replace obj with otmp */
	replace_object(obj, otmp);

	/* fix ocontainer pointers */
	if (Has_contents(obj)) {
		struct obj *inside;

		for(inside = obj->cobj; inside; inside = inside->nobj)
			inside->ocontainer = otmp;
	}

	/* move timers and light sources from obj to otmp */
	if (obj->timed) obj_move_timers(obj, otmp);
	if (obj->lamplit) obj_move_light_source(obj, otmp);

	/* objects possibly being manipulated by multi-turn occupations
	   which have been interrupted but might be subsequently resumed */
	if (obj->oclass == FOOD_CLASS)
	    food_substitution(obj, otmp);	/* eat food or open tin */
	else if (obj->oclass == SPBOOK_CLASS)
	    book_substitution(obj, otmp);	/* read spellbook */

	/* obfree(obj, otmp);	now unnecessary: no pointers on bill */
	dealloc_obj(obj);	/* let us hope nobody else saved a pointer */
	return otmp;
}

struct obj *
oname(obj, name)
struct obj *obj;
const char *name;
{
	int lth;
	char buf[PL_PSIZ];

	lth = *name ? (int)(strlen(name) + 1) : 0;
	if (lth > PL_PSIZ) {
		lth = PL_PSIZ;
		name = strncpy(buf, name, PL_PSIZ - 1);
		buf[PL_PSIZ - 1] = '\0';
	}
	/* If named artifact exists in the game, do not create another.
	 * Also trying to create an artifact shouldn't de-artifact
	 * it (e.g. Excalibur from prayer). In this case the object
	 * will retain its current name. */
	if (obj->oartifact || (lth && exist_artifact(obj->otyp, name)))
		return obj;

	if (lth == obj->onamelth) {
		/* no need to replace entire object */
		if (lth) Strcpy(ONAME(obj), name);
	} else {
		obj = realloc_obj(obj, obj->oxlth,
			      (genericptr_t)obj->oextra, lth, name);
	}
	if (lth) artifact_exists(obj, name, TRUE);
	if (obj->oartifact) {
	    /* can't dual-wield with artifact as secondary weapon */
	    if (obj == uswapwep) untwoweapon();
	    /* activate warning if you've just named your weapon "Sting" */
	    if (obj == uwep) set_artifact_intrinsic(obj, TRUE, W_WEP);
	}
	if (carried(obj)) update_inventory();
	return obj;
}

static NEARDATA const char callable[] = {
	SCROLL_CLASS, POTION_CLASS, WAND_CLASS, RING_CLASS, AMULET_CLASS,
	GEM_CLASS, SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0 };

int
ddocall()
{
	register struct obj *obj;
#ifdef REDO
	char	ch;
#endif
	char allowall[2];

	switch(
#ifdef REDO
		ch =
#endif
		ynq("¿Nombrar un objeto individual?")) {
	case 'q':
		break;
	case 'y':
#ifdef REDO
		savech(ch);
#endif
		allowall[0] = ALL_CLASSES; allowall[1] = '\0';
		obj = getobj(allowall, "", "nombrar");
		if(obj) do_oname(obj);
		break;
	default :
#ifdef REDO
		savech(ch);
#endif
		obj = getobj(callable, "", "llamar");
		if (obj) {
			/* behave as if examining it in inventory;
			   this might set dknown if it was picked up
			   while blind and the hero can now see */
			(void) xname(obj);

			if (!obj->dknown) {
				pline("Nunca reconocerías otro.");
				return 0;
			}
			docall(obj);
		}
		break;
	}
	return 0;
}

void
docall(obj)
register struct obj *obj;
{
	char buf[BUFSZ], qbuf[QBUFSZ];
	struct obj otemp;
	register char **str1;

	if (!obj->dknown) return; /* probably blind */
	otemp = *obj;
	otemp.quan = 1L;
	otemp.onamelth = 0;
	otemp.oxlth = 0;
	if (objects[otemp.otyp].oc_class == POTION_CLASS && otemp.fromsink)
	    /* kludge, meaning it's sink water */
	    Sprintf(qbuf,"Llamar una corriente de flúido %s:",
		    OBJ_DESCR(objects[otemp.otyp]));
	else
	    Sprintf(qbuf, "Llamar %s:", an(xname(&otemp)));
	getlin(qbuf, buf);
	if(!*buf || *buf == '\033')
		return;

	/* clear old name */
	str1 = &(objects[obj->otyp].oc_uname);
	if(*str1) free((genericptr_t)*str1);

	/* strip leading and trailing spaces; uncalls item if all spaces */
	(void)mungspaces(buf);
	if (!*buf) {
	    if (*str1) {	/* had name, so possibly remove from disco[] */
		/* strip name first, for the update_inventory() call
		   from undiscover_object() */
		*str1 = (char *)0;
		undiscover_object(obj->otyp);
	    }
	} else {
	    *str1 = strcpy((char *) alloc((unsigned)strlen(buf)+1), buf);
	    discover_object(obj->otyp, FALSE, TRUE); /* possibly add to disco[] */
	}
}

#endif /*OVLB*/
#ifdef OVL0

static const char * const ghostnames[] = {
	/* these names should have length < PL_NSIZ */
	/* Capitalize the names for aesthetics -dgk */
	"Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
	"Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
	"Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Robert",
	"Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Jiro", "Mizue",
	"Stephan", "Lance Braccus", "Shadowhawk",
	/* Según Andrés y Javier */
	"Adán", "Adolfo", "Adrián", "Agustín", "Alberto", "Alejandro",
	"Alfredo", "Alicia", "Ana", "Andrés", "Aníbal", "Antonio", "Beatriz",
	"Benito", "Bernardo", "Bruno", "Camilo", "Carla", "Carlos", "Carolina",
	"Cecilia", "César", "Clemente", "Conrado", "Cristina", "Darío",
	"Diego", "Domingo", "Dorotea", "Edmundo", "Eduardo", "Elena", "Elisa",
	"Emilio", "Enrique", "Ernesto", "Esteban", "Eugenia", "Eva",
	"Ezequiel", "Federico", "Felipe", "Fernando", "Florencia", "Francisco",
	"Gregorio", "Guido", "Guillermo", "Gustavo", "Horacio", "Hortensia",
	"Hugo", "Ignacio", "Inés", "Isabel", "Javier", "Jerónimo", "Jesús",
	"Joaquín", "Jorge", "José", "Juan", "Juana", "Julia", "Julián",
	"Julio", "Lázaro", "Leandro", "Leonardo", "Leonor", "Leopoldo",
	"Leticia", "Lisandro", "Lorenzo", "Lucas", "Lucía", "Luciano", "Lucio",
	"Lucrecia", "Luis", "Magdalena", "Manuel", "Marcelo", "Marcos",
	"Margarita", "María", "Mariana", "Mario", "Marta", "Matías",
	"Mauricio", "Maximiliano", "Miguel", "Nicolás", "Octavio", "Pablo",
	"Patricio", "Paula", "Pedro", "Rafael", "Ramón", "Ricardo", "Roberto",
	"Rodolfo", "Rodrigo", "Rosa", "Rubén", "Samuel", "Santiago", "Sara",
	"Silvestre", "Sofía", "Susana", "Teodoro", "Teresa", "Tomás",
	"Valentín", "Vicente", "Zacarías"
};

/* ghost names formerly set by x_monnam(), now by makemon() instead */
const char *
rndghostname()
{
    return rn2(7) ? ghostnames[rn2(SIZE(ghostnames))] : (const char *)plname;
}

/* Monster naming functions:
 * x_monnam is the generic monster-naming function.
 *		  seen	      unseen	   detected		  named
 * mon_nam:	the newt	it	the invisible orc	Fido
 * noit_mon_nam:the newt (as if detected) the invisible orc	Fido
 * l_monnam:	newt		it	invisible orc		dog called fido
 * Monnam:	The newt	It	The invisible orc	Fido
 * noit_Monnam: The newt (as if detected) The invisible orc	Fido
 * Adjmonnam:	The poor newt	It	The poor invisible orc	The poor Fido
 * Amonnam:	A newt		It	An invisible orc	Fido
 * a_monnam:	a newt		it	an invisible orc	Fido
 * m_monnam:	newt		xan	orc			Fido
 * y_monnam:	your newt     your xan	your invisible orc	Fido
 */

/* Bug: if the monster is a priest or shopkeeper, not every one of these
 * options works, since those are special cases.
 */
char *
x_monnam(mtmp, article, adjective, suppress, called)
register struct monst *mtmp;
int article;
/* ARTICLE_NONE, ARTICLE_THE, ARTICLE_A: obvious
 * ARTICLE_YOUR: "your" on pets, "the" on everything else
 *
 * If the monster would be referred to as "it" or if the monster has a name
 * _and_ there is no adjective, "invisible", "saddled", etc., override this
 * and always use no article.
 */
const char *adjective;
int suppress;
/* SUPPRESS_IT, SUPPRESS_INVISIBLE, SUPPRESS_HALLUCINATION, SUPPRESS_SADDLE.
 * EXACT_NAME: combination of all the above
 */
boolean called;
{
	char *buf = nextobuf();
	struct permonst *mdat = mtmp->data;
	boolean do_hallu, do_invis, do_it, do_saddle;
	boolean name_at_start, has_adjectives;
	char *bp;

	if (program_state.gameover)
	    suppress |= SUPPRESS_HALLUCINATION;
	if (article == ARTICLE_YOUR && !mtmp->mtame)
	    article = ARTICLE_THE;

	do_hallu = Hallucination && !(suppress & SUPPRESS_HALLUCINATION);
	do_invis = mtmp->minvis && !(suppress & SUPPRESS_INVISIBLE);
	do_it = !canspotmon(mtmp) && 
	    article != ARTICLE_YOUR &&
	    !program_state.gameover &&
#ifdef STEED
	    mtmp != u.usteed &&
#endif
	    !(u.uswallow && mtmp == u.ustuck) &&
	    !(suppress & SUPPRESS_IT);
	do_saddle = !(suppress & SUPPRESS_SADDLE);

	buf[0] = 0;

	/* unseen monsters, etc.  Use "it" */
	if (do_it) {
	    Strcpy(buf, "él");
	    return buf;
	}

	/* priests and minions: don't even use this function */
	if (mtmp->ispriest || mtmp->isminion) {
	    char priestnambuf[BUFSZ];
	    char *name;
	    long save_prop = EHalluc_resistance;
	    unsigned save_invis = mtmp->minvis;

	    /* when true name is wanted, explicitly block Hallucination */
	    if (!do_hallu) EHalluc_resistance = 1L;
	    if (!do_invis) mtmp->minvis = 0;
	    name = priestname(mtmp, priestnambuf);
	    EHalluc_resistance = save_prop;
	    mtmp->minvis = save_invis;
	    if (article == ARTICLE_NONE) {
		if (!strncmp(name, "el ", 3)) name += 3;
		else if (!strncmp(name, "la ", 3)) name += 3;
	    }
	    return strcpy(buf, name);
	}

	/* Shopkeepers: use shopkeeper name.  For normal shopkeepers, just
	 * "Asidonhopo"; for unusual ones, "Asidonhopo the invisible
	 * shopkeeper" or "Asidonhopo the blue dragon".  If hallucinating,
	 * none of this applies.
	 */
	if (mtmp->isshk && !do_hallu) {
	    if (adjective && article == ARTICLE_THE) {
		/* pathological case: "the angry Asidonhopo the blue dragon"
		   sounds silly */
		Strcpy(buf, mon_article(mtmp)? "la " : "el ");
		Strcat(strcat(buf, adjective), " ");
		Strcat(buf, shkname(mtmp));
		return buf;
	    }
	    Strcat(buf, shkname(mtmp));
	    if (mdat == &mons[PM_SHOPKEEPER] && !do_invis)
		return buf;
	    Strcat(buf, mon_article(mtmp)? " la " : " el ");
	    Strcat(buf, feminize(mdat->mname, mon_gender(mtmp)));
	    if (do_invis)
		Strcat(buf, " invisible");
	    return buf;
	}

	/* Put the actual monster name or type into the buffer now */
	/* Be sure to remember whether the buffer starts with a name */
	if (do_hallu) {
	    Strcat(buf, rndmonnam());
	    name_at_start = FALSE;
	} else if (mtmp->mnamelth) {
	    char *name = NAME(mtmp);

	    if (mdat == &mons[PM_GHOST]) {
		Sprintf(eos(buf), "fantasma de %s", name);
		name_at_start = FALSE;
	    } else if (called) {
		Sprintf(eos(buf), "%s llamad%c %s",
			feminize(mdat->mname, mon_gender(mtmp)),
			mon_gender(mtmp)? 'a' : 'o', name);
		name_at_start = (boolean)type_is_pname(mdat);
	    } else if (is_mplayer(mdat)
		   && ((bp = strstri(name, " the ")) != 0
		    || (bp = strstri(name, " el " )) != 0
		    || (bp = strstri(name, " la " )) != 0
		    || (bp = strstri(name, " los ")) != 0
		    || (bp = strstri(name, " las ")) != 0)) {
		/* <name> the <adjective> <invisible> <saddled> <rank> */
		char pbuf[BUFSZ];

		Strcpy(pbuf, name);
		bp = index(bp+1, ' ');
		pbuf[bp - name + 1] = '\0'; /* adjectives right after the article */
/*
		if (has_adjectives)
		    Strcat(pbuf, buf);
*/
		Strcat(pbuf, bp + 1);	/* append the rest of the name */
		Strcpy(buf, pbuf);
		article = ARTICLE_NONE;
		name_at_start = TRUE;
	    } else {
		Strcat(buf, name);
		name_at_start = TRUE;
	    }
	} else if (is_mplayer(mdat) && !In_endgame(&u.uz)) {
	    char pbuf[BUFSZ];
	    Strcpy(pbuf, rank_of((int)mtmp->m_lev,
				 monsndx(mdat),
				 (boolean)mtmp->female));
	    Strcat(buf, lcase(pbuf));
	    name_at_start = FALSE;
	} else {
	    Strcat(buf, feminize(mdat->mname, mon_gender(mtmp)));
	    name_at_start = (boolean)type_is_pname(mdat);
	}

	/* Put the adjectives in the buffer */
	has_adjectives = FALSE;
	if (adjective) {
	    Strcat(buf, " ");
	    Strcat(buf, feminize(adjective, mon_gender(mtmp)));
	    has_adjectives = TRUE;
	}
	if (do_invis) {
	    Strcat(buf, " invisible");
	    has_adjectives = TRUE;
	}
#ifdef STEED
	if (do_saddle && (mtmp->misc_worn_check & W_SADDLE) &&
	    !Blind && !Hallucination) {
	    Strcat(buf, mon_gender(mtmp)? " ensillada" : " ensillado");
	    has_adjectives = TRUE;
	}
#endif

	if (name_at_start && (article == ARTICLE_YOUR || !has_adjectives)) {
	    if (mdat == &mons[PM_WIZARD_OF_YENDOR])
		article = ARTICLE_THE;
	    else
		article = ARTICLE_NONE;
	} else if ((mdat->geno & G_UNIQ) && article == ARTICLE_A) {
	    article = ARTICLE_THE;
	}

	{
	    char buf2[BUFSZ];

	    switch(article) {
		case ARTICLE_YOUR:
		    Strcpy(buf2, "tu ");
		    Strcat(buf2, buf);
		    Strcpy(buf, buf2);
		    return buf;
		case ARTICLE_THE:
            /* Don't duplicate articles */
            if (strncmpi(buf, "el " , 3) != 0
            &&  strncmpi(buf, "la " , 3) != 0
            &&  strncmpi(buf, "the ", 4) != 0) {
		        Strcpy(buf2, mon_article(mtmp)? "la " : "el ");
		        Strcat(buf2, buf);
		        Strcpy(buf, buf2);
            }
		    return buf;
		case ARTICLE_A:
		    return(an(buf));
		case ARTICLE_NONE:
		default:
		    return buf;
	    }
	}
}


/* Return 1 if the noun describing the monster is feminine (which does not
   necessarily correspond to mtmp->female, as this is grammatical rather than
   natural gender). */

int
mon_gender(mtmp)
register const struct monst *mtmp;
{
	/* Special cases */
	switch (mtmp->data - mons)
	{
	case PM_SALAMANDER:
	case PM_WERERAT:
	case PM_WEREJACKAL:
	case PM_WEREWOLF:
	    return mtmp->female;

	case PM_PANTHER:
	case PM_GARGOYLE:
	case PM_WINGED_GARGOYLE:
	case PM_SUCCUBUS:
	    return isfeminine(mtmp->data->mname);
	}

	/* By class */
	switch (mtmp->data->mlet)
	{
	case S_ANT:
	case S_BLOB:
	case S_EYE:
	case S_JELLY:
	case S_MIMIC:
	case S_NAGA:
	case S_PIERCER:
	case S_RODENT:
	case S_SPIDER:
	case S_TRAPPER:
	case S_VORTEX:
	case S_WORM:
	case S_XAN:
	case S_LIGHT:
	case S_BAT:
	case S_ELEMENTAL:
	case S_FUNGUS:
	case S_MUMMY:
	case S_PUDDING:
	case S_RUSTMONST:
	case S_SNAKE:
	case S_LIZARD:
	case S_EEL:
	case S_GHOST:
	    return isfeminine(mtmp->data->mname);

	default:
	    return mtmp->female;
	}
}


/* Return 1 if the monster takes "la" as its article */
int
mon_article(mtmp)
register const struct monst *mtmp;
{
    return !needs_el(mtmp->data->mname) && mon_gender(mtmp);
}


#endif /* OVL0 */
#ifdef OVLB

char *
l_monnam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, ARTICLE_NONE, (char *)0, 
		mtmp->mnamelth ? SUPPRESS_SADDLE : 0, TRUE));
}

#endif /* OVLB */
#ifdef OVL0

char *
mon_nam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, ARTICLE_THE, (char *)0,
		mtmp->mnamelth ? SUPPRESS_SADDLE : 0, FALSE));
}

char *
mon_verb_mon(magr, verb, mdef)
struct monst *magr;  /* This monster... */
const char *verb;    /* does this...    */
struct monst *mdef;  /* to this monster */
{
    /* The verb should be third person singular.  _Ser_ is supported, as
       are all tenses except the preterite indicative.
       The verb may be second person singular only if magr == &youmonst. */
    char *buf = nextobuf();
    const char *name;

    buf[0] = 0;

    /* First comes the subject, if it is present and is a noun */
    if (magr && magr != &youmonst) {
	name = mon_nam(magr);
	if (strcmp(name, "él") != 0) {
	    Strcat(buf, name);
	    Strcat(buf, " ");
	}
    }

    /* The next part is the object pronoun if present */
    name = "";
    if (mdef == &youmonst) {
	Strcat(buf, "te ");
    }
    else if (mdef) {
	name = mon_nam(mdef);
	if (strcmp(name, "él") == 0)
	    Strcat(buf, "lo ");
    }

    /* Now comes the verb */
    if (magr == &youmonst) {
	/* Conjugate for the second person if it isn't already */
	if (strcmp(verb, "es") == 0)
	    Strcat(buf, "eres");
	else {
	    /* Permit the verb to be passed in the second person if magr
	       is always &youmonst */
	    int len = strlen(verb);
	    Strcat(buf, verb);
	    if (len == 0 || verb[len-1] != 's')
		Strcat(buf, "s");
	}
    }
    else {
	Strcat(buf, verb);
    }

    /* Finally we add the object, if it is a noun */
    if (strcmp(name, "") != 0 && strcmp(name, "él") != 0) {
	Strcat(buf, " ");
	Strcat(buf, al(name));
    }

    return buf;
}

char *
Mon_verb_mon(magr, verb, mdef)
struct monst *magr;
const char *verb;
struct monst *mdef;
{
    return upstart(mon_verb_mon(magr, verb, mdef));
}

char *
mon_verb_you(magr, verb)
struct monst *magr;  /* This monster... */
const char *verb;    /* does this...    */
                     /* to you          */
{
    return mon_verb_mon(magr, verb, &youmonst);
}

char *
Mon_verb_you(magr, verb)
struct monst *magr;
const char *verb;
{
    return Mon_verb_mon(magr, verb, &youmonst);
}

char *
mon_nam_acc(mtmp, verb)
register struct monst *mtmp;
const char *verb;
{
	char *mon = mon_nam(mtmp);
	char *buf = nextobuf();

	if (strcmpi2(mon, "él") == 0) {
	    /* Place object pronoun correctly */
	    int len = strlen(verb);

	    /* For infinitives and gerunds, the pronoun is enclitic.  Gerunds
	       also need an accent on the stressed syllable. */ 
	    if (len>=2
	    &&  verb[len-1]=='r'
	    &&  strchr("aeií", verb[len-2]) != (char *)0)
		/* Infinitive */
		Sprintf(buf, "%slo", verb);
	    else if (len>=4 && strcmp(verb+len-4, "ando")==0)
		/* Gerund, first conjugation */
		Sprintf(buf, "%.*sándolo", len-4, verb);
	    else if (len>=4 && strcmp(verb+len-4, "endo")==0)
		/* Gerund, second or third conjugation */
		Sprintf(buf, "%.*séndolo", len-4, verb);
	    else if (isupper2(verb[0]))
		/* Finite verb, capitalized */
		Sprintf(buf, "Lo %c%s", tolower2(verb[0]), verb+1);
	    else
		/* Finite verb, not capitalized */
		Sprintf(buf, "lo %s", verb);
	} else {
	    Sprintf(buf, "%s %s", verb, al(mon));
	}
	return buf;
}

/* print the name as if mon_nam() was called, but assume that the player
 * can always see the monster--used for probing and for monsters aggravating
 * the player with a cursed potion of invisibility
 */
char *
noit_mon_nam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, ARTICLE_THE, (char *)0,
		mtmp->mnamelth ? (SUPPRESS_SADDLE|SUPPRESS_IT) :
		    SUPPRESS_IT, FALSE));
}

char *
Monnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = mon_nam(mtmp);

	*bp = toupper2(*bp);
	return(bp);
}

char *
noit_Monnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = noit_mon_nam(mtmp);

	*bp = toupper2(*bp);
	return(bp);
}

/* monster's own name */
char *
m_monnam(mtmp)
struct monst *mtmp;
{
	return x_monnam(mtmp, ARTICLE_NONE, (char *)0, EXACT_NAME, FALSE);
}

/* pet name: "your little dog" */
char *
y_monnam(mtmp)
struct monst *mtmp;
{
	int prefix, suppression_flag;

	prefix = mtmp->mtame ? ARTICLE_YOUR : ARTICLE_THE;
	suppression_flag = (mtmp->mnamelth
#ifdef STEED
			    /* "saddled" is redundant when mounted */
			    || mtmp == u.usteed
#endif
			    ) ? SUPPRESS_SADDLE : 0;

	return x_monnam(mtmp, prefix, (char *)0, suppression_flag, FALSE);
}

#endif /* OVL0 */
#ifdef OVLB

char *
Adjmonnam(mtmp, adj)
register struct monst *mtmp;
register const char *adj;
{
	register char *bp = x_monnam(mtmp, ARTICLE_THE, adj,
		mtmp->mnamelth ? SUPPRESS_SADDLE : 0, FALSE);

	*bp = toupper2(*bp);
	return(bp);
}

char *
a_monnam(mtmp)
register struct monst *mtmp;
{
	return x_monnam(mtmp, ARTICLE_A, (char *)0,
		mtmp->mnamelth ? SUPPRESS_SADDLE : 0, FALSE);
}

char *
Amonnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = a_monnam(mtmp);

	*bp = toupper2(*bp);
	return(bp);
}

/* used for monster ID by the '/', ';', and 'C' commands to block remote
   identification of the endgame altars via their attending priests */
char *
distant_monnam(mon, article, outbuf)
struct monst *mon;
int article;	/* only ARTICLE_NONE and ARTICLE_THE are handled here */
char *outbuf;
{
    /* high priest(ess)'s identity is concealed on the Astral Plane,
       unless you're adjacent (overridden for hallucination which does
       its own obfuscation) */
    if (mon->data == &mons[PM_HIGH_PRIEST] && !Hallucination &&
	    Is_astralevel(&u.uz) && distu(mon->mx, mon->my) > 2) {
	Strcpy(outbuf, article == ARTICLE_THE ? (mon->female ? "la " : "el ")
					      : "");
	Strcat(outbuf, mon->female ? "alta sacerdotisa" : "alto sacerdote");
    } else {
	Strcpy(outbuf, x_monnam(mon, article, (char *)0, 0, TRUE));
    }
    return outbuf;
}

static const char * const bogusmons[] = {
	/* LENGUA:  "jumbo shrimp" -> "gamba gigante" */
	"gamba gigante", "pigmeo gigante", "ñu", "pingüino asesino",
	"cucaracha gigante", "babosa gigante", "cresa", "pterodáctilo",
	"tyrannosaurus rex", "basilisco", "espectador", "pesadilla",
	/* LENGUA:  No me satisface bookworm -> ratón de biblioteca */
	"efrit", "marid", "larva pudradora", "ratón de biblioteca", "maestro liquen",
	"sombra", "holograma", "bufón", "abogado", "flacucho",
	"tomate asesino", "amazona", "robot", "battlemech",
	/* LENGUA:  ¿Cómo se dice "bug"? */
	"rhinovirus", "arpía", "perro-león", "hormiga-rata", "problema Y2K",
						/* misc. */
	"grue", "monstruo árbol de Navidad", "chupasuerte", "paskald",
	"brogmoide", "bestiadorn",		/* Quendor (Zork, &c.) */
	"Viejo Dragón Multicolor", "Iggy el Malvado",
						/* Moria */
	"emu", "cernícalo", "xeroc", "dionea atrapamoscas",	/* Rogue */
	/* LENGUA:  creeping coins -> monedas mordientes para conservar
	   la aliteración */
	"monedas mordientes",			/* Wizardry */
	"hidra", "sirena",			/* Greek legend */
	"conejito asesino",			/* Monty Python */
	"roedor de tamaño inusual",		/* The Princess Bride */
	"Smokey el oso",	/* "Only you can prevent forest fires!" */
	"Equipaje",				/* Discworld */
	"Ent",					/* Lord of the Rings */
	"árbol enredador", "niquelpiés", "meneo",	/* Xanth */
	"conejito blanco", "snark",		/* Lewis Carroll */
	/* LENGUA:  There is no way in heck I can make this pun work in
	   Spanish */
	/* Whaddya know, Javier pulled it off */
	"mempujas-tejalo",			/* Dr. Doolittle */
	"pitufo",				/* The Smurfs */
	"tribble", "Klingon", "Borg",		/* Star Trek */
	"Ewok", "Wookie",				/* Star Wars */
	"Totoro",				/* Tonari no Totoro */
	"ohmu",					/* Nausicaa */
	"youma",				/* Sailor Moon */
	"nyaasu",				/* Pokemon (Meowth) */
	"Godzilla", "King Kong",		/* monster movies */
	"bestia de terremoto",			/* old L of SH */
	"Invid",				/* Robotech */
	"Terminator",				/* The Terminator */
	"boomer",				/* Bubblegum Crisis */
	"Dalek",				/* Dr. Who ("Exterminate!") */
	"flota espacial microscópica", "Voraz Bestia Bugblatter de Traal",
						/* HGttG */
	"ninja-tortuga adolescente mutante",	/* TMNT */
	"conejo samurái",			/* Usagi Yojimbo */
	/* LENGUA:  ¿Cómo se dice "aardvark"? */
	"cerdo hormiguero",			/* Cerebus */
	"Audrey II",				/* Little Shop of Horrors */
	"médico brujo", "come-gente púrpura volador de un ojo y un cuerno",
						/* 50's rock 'n' roll */
	"Barney el dinosaurio",			/* saccharine kiddy TV */
	"Morgoth",				/* Angband */
	"Vorlon",				/* Babylon 5 */
	"bestia aventurera",			/* King Arthur */
	"Depredador",				/* Movie */
	"suegra",				/* common pest */
	/* LENGUA: some bogus monsters specific to Spanish */
	"gigante del helado",	/* un invento de algún gringo */
	/* Según Javier y Andrés */
	"chancha con cadenas",
	"Curupí",
	"Llorona",
	"mulánima",
	"Pombero",
	"Umita",
	"La Calaca Tilica y Flaca",
	"El Nahual",
	"El Chupacabras",
	"El Coco",
	"La Bruja de las Luces",
	"El Robachicos",
	"La Mujer de Blanco",
	"El Grinch",
	"Gollum",
	"El Monje Loco",
	"El Conde Contar", /* Plaza Sésamo */
	"Morticia", "Tío Cosa", "Dedos", "Largo", /* Los Locos Adams */
	"El Santo", "Blue Demon"
	"Sully", "Michael Washawski", /* Monsters Inc. */
	"Shrek", "Stitch" /* sus correspondientes películas */
	"Momia Blanca", "Momia Negra" /* Titanes en el Ring */
};


/* Return a random monster name, for hallucination.
 * KNOWN BUG: May be a proper name (Godzilla, Barney), may not
 * (the Terminator, a Dalek).  There's no elegant way to deal
 * with this without radically modifying the calling functions.
 */
const char *
rndmonnam()
{
	int name;

	do {
	    name = rn1(SPECIAL_PM + SIZE(bogusmons) - LOW_PM, LOW_PM);
	} while (name < SPECIAL_PM &&
	    (type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN)));

	if (name >= SPECIAL_PM) return bogusmons[name - SPECIAL_PM];
	return feminize(mons[name].mname, rn2(2));
}

#ifdef REINCARNATION
const char *
roguename() /* Name of a Rogue player */
{
	char *i, *opts;

	if ((opts = nh_getenv("ROGUEOPTS")) != 0) {
		for (i = opts; *i; i++)
			if (!strncmp("name=",i,5)) {
				char *j;
				if ((j = index(i+5,',')) != 0)
					*j = (char)0;
				return i+5;
			}
	}
	return rn2(3) ? (rn2(2) ? "Michael Toy" : "Kenneth Arnold")
		: "Glenn Wichman";
}
#endif /* REINCARNATION */
#endif /* OVLB */

#ifdef OVL2

static NEARDATA const char * const hcolors[] = {
	"ultravioleta", "infrarojo", "anaranjado-azulado",
	"verde-rojado", "blanco oscuro", "negro claro", "rosado-celeste",
	"salado", "dulce", "agrio", "amargo",
	"rayado", "espiral", "arremolinado", "cuadriculado", "damero", "a rombos",
	"ondulado", "manchado", "punteado", "a lunares",
	"cuadro", "redondo", "triangular",
	"cabernet", "sangría", "fuchsia", "glicina",
	"lima-limón", "fresa-banana", "menta",
	"romántico", "incandescente"
	/* Según Javier */
	"rosa mexicano",
	"tecni-color",
	"tuti-fruti",
	"rojo colorado", "negro oscuro" "blanco claro",
	"amarillo con lunares negros" /* "El bikini amarillo" */
	"picante", "picoso", "insípido", "ácido",
	"transparente",
	"verde fosforecente", "amarillo fosforecente", "rosa fosforecente",
	"naranja fosforecente",
	"negro fosforecente",
	"rosa neón",
	"azul cuasi-morado",
	"gris normal"
};

const char *
hcolor(colorpref)
const char *colorpref;
{
	return (Hallucination || !colorpref) ?
		hcolors[rn2(SIZE(hcolors))] : colorpref;
}

/* return a random real color unless hallucinating */
const char *
rndcolor()
{
	/* LENGUA:  Need to see how this is used so an appropriate translation
	   of "colorless" can be provided */
	int k = rn2(CLR_MAX);
	return Hallucination ? hcolor((char *)0) : (k == NO_COLOR) ?
		"descolorido" : c_obj_colors[k];
}

/* Aliases for road-runner nemesis
 */
static const char * const coynames[] = {
	"Carnivurus Vulgaris","Correcaminus Digestus",
	"Comelus Todus"  ,"Hambrientus-Hambrientus",
	"Comelus Casi Todus","Come Pajarus",
	"Famelicus Fantasticus","Eternalii Hambrientiis",
	"Famelius Vulgarus","Famelicus Vulgaris Ingeniusi",
	"Comelus-Babeantus","Edipus Cabezadurius",
	"Carnivurus Babentus","Cabezadurius Voracicus",
	"Siemprelistii Comelonus","Apetitus Giganticus",
	"Pulguientus Hambrii","Sobreconfiadii Vulgaris",
	"Caninus Nervus Rex","Grotesques Apetitus",
	"Nemesis Ridiculii","Canis latrans"
};
	
char *
coyotename(mtmp, buf)
struct monst *mtmp;
char *buf;
{
    if (mtmp && buf) {
	Sprintf(buf, "%s - %s",
	    x_monnam(mtmp, ARTICLE_NONE, (char *)0, 0, TRUE),
	    mtmp->mcan ? coynames[SIZE(coynames)-1] : coynames[rn2(SIZE(coynames)-1)]);
    }
    return buf;
}
#endif /* OVL2 */

/*do_name.c*/
