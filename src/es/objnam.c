/*	SCCS Id: @(#)objnam.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* "an uncursed greased partly eaten guardian naga hatchling [corpse]" */
#define PREFIX	80	/* (56) */
#define SCHAR_LIM 127
/* FIXME:  The string handling needs to be rethought quite thoroughly */
#define NUMOBUF 24

/* For the wishing code */
#define LONGEST_WORD 40
#define MAX_WORDS 16

struct user_obj
{
    int cnt, spe, spesgn, rechrg;
    int blessed, uncursed, iscursed, ispoisoned, isgreased;
    int eroded, eroded2, erodeproof;
#ifdef INVISIBLE_OBJECTS
    int isinvisible;
#endif
    int halfeaten;
    int islit, unlabeled, ishistoric, isdiluted;
    boolean called;
    const char *name;
    int very;
    int num_keywords;
    char keywords[MAX_WORDS][LONGEST_WORD];
};

struct obj_match
{
    int num_matches;
    boolean full_match;
};

STATIC_DCL char *FDECL(strprepend,(char *,const char *));
STATIC_DCL char *FDECL(pluralize_one, (const char *, int));
STATIC_DCL void FDECL(make_called_objwords, (int,
	char words[MAX_WORDS][LONGEST_WORD]));
STATIC_DCL void FDECL(make_ided_objwords, (int,
	char words[MAX_WORDS][LONGEST_WORD]));
STATIC_DCL void FDECL(make_unided_objwords, (int,
	char words[MAX_WORDS][LONGEST_WORD]));
STATIC_DCL void FDECL(gen_keywords, (char words[MAX_WORDS][LONGEST_WORD],
	const char *, const char *));
STATIC_DCL void FDECL(parse_user_obj, (const char *, struct user_obj *,
	BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(match_keywords, (const struct user_obj *,
	char words[MAX_WORDS][LONGEST_WORD], struct obj_match *match));
static char * FDECL(feminize_one, (const char *, int));
static void FDECL(add_erosion_words, (struct obj *, char *));

struct Jitem {
	int item;
	const char *name;
};

/* true for gems/rocks that should have " stone" appended to their names */
#define GemStone(typ)	(typ == FLINT ||				\
			 (objects[typ].oc_material == GEMSTONE &&	\
			  (typ != DILITHIUM_CRYSTAL && typ != RUBY &&	\
			   typ != DIAMOND && typ != SAPPHIRE &&		\
			   typ != BLACK_OPAL && 	\
			   typ != EMERALD && typ != OPAL)))

#ifndef OVLB

STATIC_DCL struct Jitem Japanese_items[];

#else /* OVLB */

STATIC_OVL struct Jitem Japanese_items[] = {
	{ SHORT_SWORD, "wakizashi" },
	{ BROADSWORD, "ninja-to" },
	{ FLAIL, "nunchaku" },
	{ GLAIVE, "naginata" },
	{ LOCK_PICK, "osaku" },
	{ WOODEN_HARP, "koto" },
	{ KNIFE, "shito" },
	{ PLATE_MAIL, "tanko" },
	{ HELMET, "kabuto" },
	{ LEATHER_GLOVES, "yugake" },
	{ FOOD_RATION, "gunyoki" },
	{ POT_BOOZE, "sake" },
	{0, "" }
};

#endif /* OVLB */

STATIC_DCL const char *FDECL(Japanese_item_name,(int i));

#ifdef OVL1

STATIC_OVL char *
strprepend(s,pref)
register char *s;
register const char *pref;
{
	register int i = (int)strlen(pref);

	if(i > PREFIX) {
		impossible("PREFIX too short (for %d).", i);
		return(s);
	}
	s -= i;
	(void) strncpy(s, pref, i);	/* do not copy trailing 0 */
	return(s);
}

#endif /* OVL1 */
#ifdef OVLB

/* manage a pool of BUFSZ buffers, so callers don't have to */
char *
nextobuf()
{
	static char NEARDATA bufs[NUMOBUF][BUFSZ];
	static int bufidx = 0;

	bufidx = (bufidx + 1) % NUMOBUF;
	return bufs[bufidx];
}

char *
obj_typename(otyp)
register int otyp;
{
	char *buf = nextobuf();
	register struct objclass *ocl = &objects[otyp];
	register const char *actualn = OBJ_NAME(*ocl);
	register const char *dn = OBJ_DESCR(*ocl);
	register const char *un = ocl->oc_uname;
	register int nn = ocl->oc_name_known;


	if (Role_if(PM_SAMURAI) && Japanese_item_name(otyp))
		actualn = Japanese_item_name(otyp);
	switch(ocl->oc_class) {
	case COIN_CLASS:
		Strcpy(buf, "moneda");
		break;
	case POTION_CLASS:
		Strcpy(buf, "poción");
		break;
	case SCROLL_CLASS:
		Strcpy(buf, "pergamino");
		break;
	case WAND_CLASS:
		Strcpy(buf, "varita");
		break;
	case SPBOOK_CLASS:
		Strcpy(buf, "libro de hechizo");
		break;
	case RING_CLASS:
		Strcpy(buf, "anillo");
		break;
	case AMULET_CLASS:
		if(nn)
			Strcpy(buf,actualn);
		else
			Strcpy(buf,"amuleto");
		if(un)
			/* Todos los amuletos son masculinos */
			Sprintf(eos(buf)," llamado %s",un);
		if(dn)
			Sprintf(eos(buf)," (%s)",dn);
		return(buf);
	default:
		buf[0] = 0;
		if(nn) {
			if (GemStone(otyp)) {
				/* avoid "piedra de gris" */
				Strcpy(buf, "piedra ");
				if (strcmp(actualn, "gris") != 0)
					Strcat(buf, "de ");
			}
			Strcat(buf, actualn);
			if(un)
				Sprintf(eos(buf), " llamad%c %s",
					isfeminine(buf)? 'a' : 'o',
					un);
			if(dn)
				Sprintf(eos(buf), " (%s)", dn);
		} else {
			if(ocl->oc_class == GEM_CLASS)
				Strcpy(buf, (ocl->oc_material == MINERAL) ?
						"piedra de " : "joya de ");
			Strcat(buf, dn ? dn : actualn);
			if(un)
				Sprintf(eos(buf), " llamad%c %s",
					isfeminine(buf)? 'a' : 'o',
					un);
		}
		return(buf);
	}
	/* here for ring/scroll/potion/wand */
	if(nn) {
	    if (ocl->oc_unique)
		Strcpy(buf, actualn); /* avoid spellbook of Book of the Dead */
	    else
		Sprintf(eos(buf), " de %s", actualn);
	}
	if(un)
		Sprintf(eos(buf), " llamad%c %s",
			isfeminine(buf)? 'a' : 'o',
			un);
	if(dn)
		Sprintf(eos(buf), " (%s)", dn);
	return(buf);
}

/* less verbose result than obj_typename(); either the actual name
   or the description (but not both); user-assigned name is ignored */
char *
simple_typename(otyp)
int otyp;
{
    char *bufp, *pp, *save_uname = objects[otyp].oc_uname;

    objects[otyp].oc_uname = 0;		/* suppress any name given by user */
    bufp = obj_typename(otyp);
    objects[otyp].oc_uname = save_uname;
    if ((pp = strstri(bufp, " (")) != 0)
	*pp = '\0';		/* strip the appended description */
    return bufp;
}

boolean
obj_is_pname(obj)
register struct obj *obj;
{
    return((boolean)(obj->dknown && obj->known && obj->onamelth &&
		     /* Since there aren't any objects which are both
			artifacts and unique, the last check is redundant. */
		     obj->oartifact && !objects[obj->otyp].oc_unique));
}

/* Give the name of an object seen at a distance.  Unlike xname/doname,
 * we don't want to set dknown if it's not set already.  The kludge used is
 * to temporarily set Blind so that xname() skips the dknown setting.  This
 * assumes that we don't want to do this too often; if this function becomes
 * frequently used, it'd probably be better to pass a parameter to xname()
 * or doname() instead.
 */
char *
distant_name(obj, func)
register struct obj *obj;
char *FDECL((*func), (OBJ_P));
{
	char *str;

	long save_Blinded = Blinded;
	Blinded = 1;
	str = (*func)(obj);
	Blinded = save_Blinded;
	return str;
}

/* convert player specified fruit name into corresponding fruit juice name
   ("slice of pizza" -> "pizza juice" rather than "slice of pizza juice") */
char *
fruitname(juice)
boolean juice;	/* whether or not to append " juice" to the name */
{
    char *buf = nextobuf();
    const char *fruit_nam = strstri(pl_fruit, " de ");

    if (fruit_nam)
	fruit_nam += 4;		/* skip past " de " */
    else
	fruit_nam = pl_fruit;	/* use it as is */

    Sprintf(buf, "%s%s", juice ? "jugo de " : "", makesingular(fruit_nam));
    return buf;
}

#endif /* OVLB */
#ifdef OVL1

char *
xname(obj)
register struct obj *obj;
{
	register char *buf;
	register int typ = obj->otyp;
	register struct objclass *ocl = &objects[typ];
	register int nn = ocl->oc_name_known;
	register const char *actualn = OBJ_NAME(*ocl);
	register const char *dn = OBJ_DESCR(*ocl);
	register const char *un = ocl->oc_uname;

	buf = nextobuf() + PREFIX;	/* leave room for "17 -3 " */
	if (Role_if(PM_SAMURAI) && Japanese_item_name(typ))
		actualn = Japanese_item_name(typ);

	buf[0] = '\0';
	/*
	 * clean up known when it's tied to oc_name_known, eg after AD_DRIN
	 * This is only required for unique objects since the article
	 * printed for the object is tied to the combination of the two
	 * and printing the wrong article gives away information.
	 */
	if (!nn && ocl->oc_uses_known && ocl->oc_unique) obj->known = 0;
	if (!Blind) obj->dknown = TRUE;
	if (Role_if(PM_PRIEST)) obj->bknown = TRUE;
	if (obj_is_pname(obj))
	    goto nameit;
	switch (obj->oclass) {
	    case AMULET_CLASS:
		if (!obj->dknown)
			Strcpy(buf, "amuleto");
		else if (typ == AMULET_OF_YENDOR ||
			 typ == FAKE_AMULET_OF_YENDOR)
			/* each must be identified individually */
			Strcpy(buf, obj->known ? actualn : dn);
		else if (nn)
			Strcpy(buf, actualn);
		else if (un)
			Sprintf(buf,"amuleto llamado %s", un);
		else
			Sprintf(buf,"amuleto %s", dn);
		break;
	    case WEAPON_CLASS:
	    case VENOM_CLASS:
	    case TOOL_CLASS:
		if (typ == LENSES)
			Strcpy(buf, "par de ");

		if (obj->dknown && nn)
			Strcat(buf, actualn);
		else
			Strcat(buf, dn ? dn : actualn);
		if (obj->oclass==WEAPON_CLASS && is_poisonable(obj) && obj->opoisoned)
			Sprintf(eos(buf), " envenenad%c",
				isfeminine(buf)? 'a' : 'o');
		if (obj->dknown && !nn && un) {
			Sprintf(eos(buf), " llamad%c%s ",
				isfeminine(buf)? 'a' : 'o',
				typ == LENSES? "s" : "");
			Strcat(buf, un);
		}
		/* If we use an() here we'd have to remember never to use */
		/* it whenever calling doname() or xname(). */
		if (typ == FIGURINE)
		    Sprintf(eos(buf), " de un%s %s",
			isfeminine(mons[obj->corpsenm].mname)? "a" : "",
			mons[obj->corpsenm].mname);
		break;
	    case ARMOR_CLASS:
		/* depends on order of the dragon scales objects */
		if (typ >= GRAY_DRAGON_SCALES && typ <= YELLOW_DRAGON_SCALES) {
			Sprintf(buf, "juego de %s", actualn);
			break;
		}
		if(is_boots(obj) || is_gloves(obj)) Strcpy(buf,"par de ");

		if(obj->otyp >= ELVEN_SHIELD && obj->otyp <= ORCISH_SHIELD
				&& !obj->dknown) {
			Strcpy(buf, "escudo");
			break;
		}
		if(obj->otyp == SHIELD_OF_REFLECTION && !obj->dknown) {
			Strcpy(buf, "escudo llano");
			break;
		}

		if(nn)	Strcat(buf, actualn);
		else if(un) {
			if(is_boots(obj))
				Strcat(buf,"botas llamadas ");
			else if(is_gloves(obj))
				Strcat(buf,"guantes llamados ");
			else if(is_cloak(obj))
				Strcpy(buf,"capa llamada ");
			else if(is_helmet(obj))
				Strcpy(buf,"yelmo llamado ");
			else if(is_shield(obj))
				Strcpy(buf,"escudo llamado ");
			else
				Strcpy(buf,"armadura llamada ");
			Strcat(buf, un);
		} else Strcat(buf, dn);
		break;
	    case FOOD_CLASS:
		if (typ == SLIME_MOLD) {
			register struct fruit *f;

			for(f=ffruit; f; f = f->nextf) {
				if(f->fid == obj->spe) {
					Strcpy(buf, f->fname);
					break;
				}
			}
			if (!f) impossible("Bad fruit #%d?", obj->spe);
			break;
		}

		Strcpy(buf, actualn);
		if (typ == TIN && obj->known) {
		    if(obj->spe > 0)
			Strcat(buf, " de espinaca");
		    else if (obj->corpsenm == NON_PM)
			Strcpy(buf, "lata vacía");
		    else if (vegetarian(&mons[obj->corpsenm]))
			Sprintf(eos(buf), " de %s", mons[obj->corpsenm].mname);
		    else
			Sprintf(eos(buf), " de carne de %s", mons[obj->corpsenm].mname);
		}
		break;
	    case COIN_CLASS:
	    case CHAIN_CLASS:
		Strcpy(buf, actualn);
		break;
	    case ROCK_CLASS:
		if (typ == STATUE)
		    Sprintf(buf, "%s%s de%s %s",
			actualn,
			(Role_if(PM_ARCHEOLOGIST) && (obj->spe & STATUE_HISTORIC))
				? " histórica" : "",
			type_is_pname(&mons[obj->corpsenm]) ? "" :
			  (mons[obj->corpsenm].geno & G_UNIQ)
				? isfeminine(mons[obj->corpsenm].mname)? " la" : "l"
				: isfeminine(mons[obj->corpsenm].mname)? " una" : " un",
			mons[obj->corpsenm].mname);
		else Strcpy(buf, actualn);
		break;
	    case BALL_CLASS:
		Sprintf(buf, "bola %spesada de hierro",
			(obj->owt > ocl->oc_weight) ? "muy " : "");
		break;
	    case POTION_CLASS:
		Strcat(buf, "poción");
		if(!obj->dknown) break;
		if (obj->odiluted)
			Strcat(buf, " diluida");
		if(nn) {
		    Strcat(buf, " de ");
		    Strcat(buf, actualn);
		    /* FIXME:  This could be removed */
		    if (typ == POT_WATER &&
			obj->bknown && (obj->blessed || obj->cursed)) {
			Strcat(buf, obj->blessed ? " bendita" : " maldita");
		    }
		} else if (un) {
			Strcat(buf, " llamada ");
			Strcat(buf, un);
		} else {
			Strcat(buf, " ");
			Strcat(buf, dn);
		}
		break;
	case SCROLL_CLASS:
		Strcpy(buf, "pergamino");
		if(!obj->dknown) break;
		if(nn) {
			Strcat(buf, " de ");
			Strcat(buf, actualn);
		} else if(un) {
			Strcat(buf, " llamado ");
			Strcat(buf, un);
		} else if (ocl->oc_magic) {
			Strcat(buf, " marcado ");
			Strcat(buf, dn);
		} else {
			Strcat(buf, " ");
			Strcat(buf, dn);
		}
		break;
	case WAND_CLASS:
		if(!obj->dknown)
			Strcpy(buf, "varita");
		else if(nn)
			Sprintf(buf, "varita de %s", actualn);
		else if(un)
			Sprintf(buf, "varita llamada %s", un);
		else
			Sprintf(buf, "varita %s", dn);
		break;
	case SPBOOK_CLASS:
		if (!obj->dknown) {
			Strcpy(buf, "libro de hechizo");
		} else if (nn) {
			if (typ != SPE_BOOK_OF_THE_DEAD)
			    Strcpy(buf, "libro de ");
			Strcat(buf, actualn);
		} else if (un) {
			Sprintf(buf, "libro de hechizo llamado %s", un);
		} else
			Sprintf(buf, "libro de hechizo %s", dn);
		break;
	case RING_CLASS:
		if(!obj->dknown)
			Strcpy(buf, "anillo");
		else if(nn)
			Sprintf(buf, "anillo de %s", actualn);
		else if(un)
			Sprintf(buf, "anillo llamado %s", un);
		else
			Sprintf(buf, "anillo %s", dn);
		break;
	case GEM_CLASS:
	    {
		const char *rock =
			    (ocl->oc_material == MINERAL) ? "piedra" : "joya";
		if (!obj->dknown) {
		    Strcpy(buf, rock);
		} else if (!nn) {
		    if (un) Sprintf(buf,"%s llamada %s", rock, un);
		    else Sprintf(buf, "%s %s", rock, dn);
		} else {
		    if (GemStone(typ)) Strcpy(buf, "piedra de ");
		    Strcat(buf, actualn);
		}
		break;
	    }
	default:
		Sprintf(buf,"glorkum %d %d %d", obj->oclass, typ, obj->spe);
	}
	if (obj->quan != 1L) Strcpy(buf, makeplural(buf));

	if (obj->onamelth && obj->dknown) {
		char *p;
		/* For statues, we want "nombrado" to agree with the monster
		   and not "estatua" */
		p = buf;
		if (strncmp(p, "estatua de ", 11) == 0)
		    p += 11;
		Sprintf(eos(buf), " nombrad%c%s ",
			isfeminine(p)? 'a' : 'o',
			obj->quan != 1L? "s" : "");
nameit:
		Strcat(buf, ONAME(obj));
	}

	if (!strncmpi2(buf, "el ",  3)) buf += 3;
	if (!strncmpi2(buf, "la ",  3)) buf += 3;
	if (!strncmpi2(buf, "los ", 4)) buf += 4;
	if (!strncmpi2(buf, "las ", 4)) buf += 4;
	return(buf);
}

/* xname() output augmented for multishot missile feedback */
char *
mshot_xname(obj)
struct obj *obj;
{
    char tmpbuf[BUFSZ];
    char *onm = xname(obj);

    if (m_shot.n > 1 && m_shot.o == obj->otyp) {
	boolean fem = isfeminine(onm);

	/* copy xname's result so that we can reuse its return buffer */
	Strcpy(tmpbuf, onm);
	/* "the Nth arrow"; value will eventually be passed to an() or
	   The(), both of which correctly handle this "the " prefix */
	Sprintf(onm, "%s %d%c %s", fem? "la" : "el", m_shot.i,
		fem? 'ª' : 'º', tmpbuf);
    }

    return onm;
}

#endif /* OVL1 */
#ifdef OVL0

/* used for naming "the unique_item" instead of "a unique_item" */
boolean
the_unique_obj(obj)
register struct obj *obj;
{
    if (!obj->dknown)
	return FALSE;
    else if (obj->otyp == FAKE_AMULET_OF_YENDOR && !obj->known)
	return TRUE;		/* lie */
    else
	return (boolean)(objects[obj->otyp].oc_unique &&
			 (obj->known || obj->otyp == AMULET_OF_YENDOR));
}

static void
add_erosion_words(obj,prefix)
struct obj *obj;
char *prefix;
{
	boolean iscrys = (obj->otyp == CRYSKNIFE);
	char *p = prefix, *q; /* To decline the adjectives */
	char gender;
	/* Plural if more than one object or if "par de" */
	char plural = obj->quan > 1L
		    || (obj->oclass == TOOL_CLASS && obj->otyp == LENSES)
		    || is_boots(obj) || is_gloves(obj);

	if (!is_damageable(obj) && !iscrys) return;

	/* For certain objects, the objects array does not contain the full
	   name; hence the gender must be fixed according to class for these
	   objects */
	switch (obj->oclass) {
	case RING_CLASS:	/* anillo */
	case SCROLL_CLASS:	/* pergamino */
	case SPBOOK_CLASS:	/* libro */
		gender = 'o';
		break;

	case POTION_CLASS:	/* poción */
	case GEM_CLASS:		/* joya or pieza */
	case WAND_CLASS:	/* varita */
		gender = 'a';
		break;

	default:
		gender = isfeminine(OBJ_NAME(objects[obj->otyp]))? 'a' : 'o';
		break;
	}

	/* The only cases where any of these bits do double duty are for
	 * rotted food and diluted potions, which are all not is_damageable().
	 */
	if (obj->oeroded && !iscrys) {
		switch (obj->oeroded) {
			case 2:	Strcat(prefix, " muy"); break;
			case 3:	Strcat(prefix, " completamente"); break;
		}			
		Strcat(prefix, is_rustprone(obj) ? " oxidad@#" : " quemad@#");
	}
	if (obj->oeroded2 && !iscrys) {
		switch (obj->oeroded2) {
			case 2:	Strcat(prefix, " muy"); break;
			case 3:	Strcat(prefix, " completamente"); break;
		}			
		Strcat(prefix, is_corrodeable(obj) ? " corroid@#" :
			" podrid@#");
	}
	if (obj->rknown && obj->oerodeproof)
		Strcat(prefix,
		       iscrys ? " fixad@#" :
		       is_rustprone(obj) ? " inoxidable#" :
		       is_corrodeable(obj) ? " incorroible#" :	/* "stainless"? */
		       is_flammable(obj) ? " inquemable#" : "");

	/* Decline the adjectives */
	/* p was set to prefix above */
	q = p;
	while (*p != 0) {
		switch (*p) {
		case '@':	/* Gender */
			*q++ = gender;
			break;

		case '#':	/* Number */
			if (plural)
				*q++ = 's';
			break;

		default:
			*q++ = *p;
			break;
		}
		p++;
	}
	*q = 0;
}
char *
doname(obj)
register struct obj *obj;
{
	char prefix[PREFIX];
	char tmpbuf[PREFIX+1];
	/* when we have to add something at the start of prefix instead of the
	 * end (Strcat is used on the end)
	 */
	register char *bp = xname(obj);
	boolean plur_adj;
	char declension1[3],  /* "o"  "a"  "os"  "as" */
	     declension2[2],  /* ""   ""   "s"   "s"  */
	     declension3[3];  /* ""   ""   "es"  "es" */

	if(obj->quan != 1L)
		Sprintf(prefix, "%ld ", obj->quan);
	else if (obj_is_pname(obj) || the_unique_obj(obj)) {
		prefix[0] = 0;
		if (strncmpi2(bp, "el ", 3) == 0)
		    bp += 3;
		if (strncmpi2(bp, "la ", 3) == 0)
		    bp += 3;
		/* Certain artifacts begin with "The " */
		if (strncmpi2(bp, "the ", 4) != 0)
		    Strcpy(prefix, isfeminine(bp)? "la " : "el ");
	} else {
		/* isfeminine() disregards "par de" so that following
		   adjectives are correct; but we want "un" to match "par" */
		if (strncmp(bp, "par de ", 7) == 0)
			Strcpy(prefix, "un ");
		else
			Strcpy(prefix, isfeminine(bp)? "una " : "un ");
	}

	/* If pair, use plural adjectives (un par de botas élficas benditas) */
	plur_adj = (obj->quan > 1L) || (strncmp(bp, "par de ", 7) == 0);
	Strcpy(declension2, plur_adj? "s" : "");
	Sprintf(declension1, "%c%s",
		isfeminine(bp)? 'a' : 'o',
		declension2);
	Strcpy(declension3, plur_adj? "es" : "");

#ifdef INVISIBLE_OBJECTS
	if (obj->oinvis) Sprintf(eos(bp)," invisible%s", declension2);
#endif

	if (obj->bknown &&
	    obj->oclass != COIN_CLASS &&
	    (obj->otyp != POT_WATER || !objects[POT_WATER].oc_name_known
		|| (!obj->cursed && !obj->blessed))) {
	    /* allow 'blessed clear potion' if we don't know it's holy water;
	     * always allow "uncursed potion of water"
	     */
	    if (obj->cursed)
		Sprintf(eos(bp), " maldit%s", declension1);
	    else if (obj->blessed)
		Sprintf(eos(bp), " bendit%s", declension1);
	    else if ((!obj->known || !objects[obj->otyp].oc_charged ||
		      (obj->oclass == ARMOR_CLASS ||
		       obj->oclass == RING_CLASS))
		/* For most items with charges or +/-, if you know how many
		 * charges are left or what the +/- is, then you must have
		 * totally identified the item, so "uncursed" is unneccesary,
		 * because an identified object not described as "blessed" or
		 * "cursed" must be uncursed.
		 *
		 * If the charges or +/- is not known, "uncursed" must be
		 * printed to avoid ambiguity between an item whose curse
		 * status is unknown, and an item known to be uncursed.
		 */
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
			&& obj->otyp != FAKE_AMULET_OF_YENDOR
			&& obj->otyp != AMULET_OF_YENDOR
			&& !Role_if(PM_PRIEST))
		Sprintf(eos(bp), " inmaldit%s", declension1);
	}

	if (obj->greased) Sprintf(eos(bp), " engrasad%s", declension1);

	switch(obj->oclass) {
	case AMULET_CLASS:
		if(obj->owornmask & W_AMUL)
			Sprintf(eos(bp), " (llevad%s)", declension1);
		break;
	case WEAPON_CLASS:
		add_erosion_words(obj, bp);
plus:
		if(obj->known) {
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case ARMOR_CLASS:
		add_erosion_words(obj, bp);
		if(obj->owornmask & W_ARMOR)
			Sprintf(eos(bp),
				(obj == uskin)
					? " (embedid%s en tu piel)"
					: " (llevad%s)",
				declension1);
		goto plus;
	case TOOL_CLASS:
		/* weptools already get this done when we go to the +n code */
                /* RLC It seems I've done something to keep that from happening */
		/*if (!is_weptool(obj))*/
		    add_erosion_words(obj, bp);
		if(obj->owornmask & (W_TOOL /* blindfold */
#ifdef STEED
				| W_SADDLE
#endif
				)) {
			Sprintf(eos(bp), " (llevad%s)", declension1);
			break;
		}
		if (obj->otyp == LEASH && obj->leashmon != 0) {
			Strcat(bp, " (en uso)");
			break;
		}
		if (is_weptool(obj))
			goto plus;
		if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
			if (!obj->spe)
			    Strcpy(tmpbuf, "no");
			else
			    Sprintf(tmpbuf, "%d", obj->spe);
			Sprintf(eos(bp), " (%s vela%s%s%s)",
				tmpbuf,
				plur(obj->spe),
				obj->lamplit? ", alumbrada" : " juntada",
				plur(obj->spe));
			break;
		} else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
			obj->otyp == BRASS_LANTERN || Is_candle(obj)) {
			if (Is_candle(obj) &&
			    obj->age < 20L * (long)objects[obj->otyp].oc_cost)
				Sprintf(eos(bp), "usad%s en parte", declension1);
			if(obj->lamplit)
				Sprintf(eos(bp), " (alumbrad%s)", declension1);
			break;
		}
		if(objects[obj->otyp].oc_charged)
		    goto charges;
		break;
	case WAND_CLASS:
		add_erosion_words(obj, bp);
charges:
		if(obj->known)
		    Sprintf(eos(bp), " (%d:%d)", (int)obj->recharged, obj->spe);
		break;
	case POTION_CLASS:
		if (obj->otyp == POT_OIL && obj->lamplit)
		    Sprintf(eos(bp), " (alumbrad%s)", declension1);
		break;
	case RING_CLASS:
		add_erosion_words(obj, bp);
ring:
		if (obj->owornmask & W_RING)
		{
			const char *part = body_part(HAND);
			boolean fem = isfeminine(part);

			sprintf(eos(bp), " (en %s %s", fem? "la" : "el", part);
			if(obj->owornmask & W_RINGR)
				Sprintf(eos(bp), " derech%c)", fem? 'a' : 'o');
			if(obj->owornmask & W_RINGL)
				Sprintf(eos(bp), " izquierd%c)", fem? 'a' : 'o');
		}
		if(obj->known && objects[obj->otyp].oc_charged) {
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case FOOD_CLASS:
		if (obj->otyp == CORPSE) {
		    /* Avoid "cadáver nombrado foo de bar" */
		    char buf2[BUFSZ];
		    char *suffix;
		    suffix = strchr(bp, ' ');
		    if (!suffix) suffix = eos(bp);
		    Sprintf(buf2, "%.*s de%s %s%s",
				(int)(suffix-bp), bp,
				((mons[obj->corpsenm].geno & G_UNIQ)
				&& !type_is_pname(&mons[obj->corpsenm]))
					? isfeminine(mons[obj->corpsenm].mname)? " la" : "l"
					: "",
					mons[obj->corpsenm].mname,
				suffix);
		    strcpy(bp, buf2);
		} else if (obj->otyp == EGG) {
		    if (obj->corpsenm >= LOW_PM &&
			    (obj->known ||
			    mvitals[obj->corpsenm].mvflags & MV_KNOWS_EGG)) {
			Strcat(bp, " de ");
			Strcat(bp, mons[obj->corpsenm].mname);
			if (obj->spe)
			    Sprintf(eos(bp), " (puest%s por ti)", declension1);
		    }
		}
		if (obj->oeaten)
		    Sprintf(eos(bp), " comid%s en parte", declension1);
		if (obj->otyp == MEAT_RING) goto ring;
		break;
	case BALL_CLASS:
	case CHAIN_CLASS:
		add_erosion_words(obj, bp);
		if(obj->owornmask & W_BALL)
			Sprintf(eos(bp), " (encadenad%s a ti)", declension1);
			break;
	}

	if((obj->owornmask & W_WEP) && !mrg_to_wielded) {
		if (obj->quan != 1L) {
			Sprintf(eos(bp), " (blandid%s)", declension1);
		} else {
			const char *hand_s = body_part(HAND);
			boolean fem = isfeminine(hand_s);
			boolean plu = bimanual(obj);

			if (plu) hand_s = makeplural(hand_s);
			Sprintf(eos(bp), " (arma en %s %s)",
				plu ? (fem? "las" : "los")
				    : (fem? "la"  : "el" ),
				hand_s);
		}
	}
	if(obj->owornmask & W_SWAPWEP) {
		if (u.twoweap) {
			const char *hand = body_part(HAND);
			Sprintf(eos(bp), " (blandid%s en otr%c %s)",
				declension1,
				isfeminine(hand)? 'a' : 'o',
				body_part(HAND));
		} else
			Strcat(bp, " (arma alterna; no blandida)");
	}
	if(obj->owornmask & W_QUIVER) Strcat(bp, " (en aljaba)");
	if(obj->unpaid) {
		xchar ox, oy; 
		long quotedprice = unpaid_cost(obj);
		struct monst *shkp = (struct monst *)0;

		if (Has_contents(obj) &&
		    get_obj_location(obj, &ox, &oy, BURIED_TOO|CONTAINED_TOO) &&
		    costly_spot(ox, oy) &&
		    (shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE))))
			quotedprice += contained_cost(obj, shkp, 0L, FALSE, TRUE);
		Sprintf(eos(bp), " (sin pagar, %ld %s)",
			quotedprice, currency(quotedprice));
	}
	bp = strprepend(bp, prefix);
	return(bp);
}

#endif /* OVL0 */
#ifdef OVLB

/* used from invent.c */
boolean
not_fully_identified(otmp)
register struct obj *otmp;
{
#ifdef GOLDOBJ
    /* gold doesn't have any interesting attributes [yet?] */
    if (otmp->oclass == COIN_CLASS) return FALSE;	/* always fully ID'd */
#endif
    /* check fundamental ID hallmarks first */
    if (!otmp->known || !otmp->dknown ||
#ifdef MAIL
	    (!otmp->bknown && otmp->otyp != SCR_MAIL) ||
#else
	    !otmp->bknown ||
#endif
	    !objects[otmp->otyp].oc_name_known)	/* ?redundant? */
	return TRUE;
    if (otmp->oartifact && undiscovered_artifact(otmp->oartifact))
	return TRUE;
    /* otmp->rknown is the only item of interest if we reach here */
       /*
	*  Note:  if a revision ever allows scrolls to become fireproof or
	*  rings to become shockproof, this checking will need to be revised.
	*  `rknown' ID only matters if xname() will provide the info about it.
	*/
    if (otmp->rknown || (otmp->oclass != ARMOR_CLASS &&
			 otmp->oclass != WEAPON_CLASS &&
			 !is_weptool(otmp) &&		    /* (redunant) */
			 otmp->oclass != BALL_CLASS))	    /* (useless) */
	return FALSE;
    else	/* lack of `rknown' only matters for vulnerable objects */
	return (boolean)(is_rustprone(otmp) ||
			 is_corrodeable(otmp) ||
			 is_flammable(otmp));
}

char *
corpse_xname(otmp, ignore_oquan)
struct obj *otmp;
boolean ignore_oquan;	/* to force singular */
{
	char *nambuf = nextobuf();

	Sprintf(nambuf, "cadáver de %s", mons[otmp->corpsenm].mname);

	if (ignore_oquan || otmp->quan < 2)
	    return nambuf;
	else
	    return makeplural(nambuf);
}

/* xname, unless it's a corpse, then corpse_xname(obj, FALSE) */
char *
cxname(obj)
struct obj *obj;
{
	if (obj->otyp == CORPSE)
	    return corpse_xname(obj, FALSE);
	return xname(obj);
}

/* treat an object as fully ID'd when it might be used as reason for death */
char *
killer_xname(obj)
struct obj *obj;
{
    struct obj save_obj;
    unsigned save_ocknown;
    char *buf, *save_ocuname;

    /* remember original settings for core of the object;
       oname and oattached extensions don't matter here--since they
       aren't modified they don't need to be saved and restored */
    save_obj = *obj;
    /* killer name should be more specific than general xname; however, exact
       info like blessed/cursed and rustproof makes things be too verbose */
    obj->known = obj->dknown = 1;
    obj->bknown = obj->rknown = obj->greased = 0;
    /* if character is a priest[ess], bknown will get toggled back on */
    obj->blessed = obj->cursed = 0;
    /* "killed by poisoned <obj>" would be misleading when poison is
       not the cause of death and "poisoned by poisoned <obj>" would
       be redundant when it is, so suppress "poisoned" prefix */
    obj->opoisoned = 0;
    /* strip user-supplied name; artifacts keep theirs */
    if (!obj->oartifact) obj->onamelth = 0;
    /* temporarily identify the type of object */
    save_ocknown = objects[obj->otyp].oc_name_known;
    objects[obj->otyp].oc_name_known = 1;
    save_ocuname = objects[obj->otyp].oc_uname;
    objects[obj->otyp].oc_uname = 0;	/* avoid "foo called bar" */

    buf = xname(obj);
    if (obj->quan == 1L) buf = obj_is_pname(obj) ? the(buf) : an(buf);

    objects[obj->otyp].oc_name_known = save_ocknown;
    objects[obj->otyp].oc_uname = save_ocuname;
    *obj = save_obj;	/* restore object's core settings */

    return buf;
}

/*
 * Used if only one of a collection of objects is named (e.g. in eat.c).
 */
const char *
singular(otmp, func)
register struct obj *otmp;
char *FDECL((*func), (OBJ_P));
{
	long savequan;
	char *nam;

	/* Note: using xname for corpses will not give the monster type */
	if (otmp->otyp == CORPSE && func == xname)
		return corpse_xname(otmp, TRUE);

	savequan = otmp->quan;
	otmp->quan = 1L;
	nam = (*func)(otmp);
	otmp->quan = savequan;
	return nam;
}

char *
an(str)
register const char *str;
{
	char *buf = nextobuf();

	buf[0] = '\0';

	if (strncmpi2(str, "el ",  3)!=0 &&
	    strncmpi2(str, "la ",  3)!=0 &&
	    strncmpi2(str, "los ", 4)!=0 &&
	    strncmpi2(str, "las ", 4)!=0 &&
	    strcmp(str, "lava derretida")!=0 &&
	    strcmp(str, "barras de hierro")!=0 &&
	    strcmp(str, "hielo")!=0) {
		/* isfeminine() is set up to ignore "par de", so that "par de
		   botas" gets "malditas", for example, rather than "maldito";
		   nonetheless, we want "un par" and not "una par" */
		if (strncmpi2(str, "par de ", 7) == 0)
			Strcpy(buf, "un ");
		else
			Strcpy(buf, isfeminine(str)? "una " : "un ");
	}

	Strcat(buf, str);
	return buf;
}

char *
An(str)
const char *str;
{
	register char *tmp = an(str);
	*tmp = toupper2(*tmp);
	return tmp;
}

/*
 * Prepend "the" if necessary; assumes str is a subject derived from xname.
 * Use type_is_pname() for monster names, not the().  the() is idempotent.
 */
char *
the(str)
const char *str;
{
	char *buf = nextobuf();
	boolean insert_the = FALSE;

	if (strncmpi2(str, "el ",  3)==0
	||  strncmpi2(str, "la ",  3)==0
	||  strncmpi2(str, "los ", 4)==0
	||  strncmpi2(str, "las ", 4)==0
	||  strncmpi2(str, "the ", 4)==0) { /* certain artifacts */
	    Strcpy(buf, str);
	    return buf;
	} else if (!isupper2(str[0])) {
	    /* not a proper name, needs an article */
	    insert_the = TRUE;
	} else {
	    /* Probably a proper name, might not need an article */
	    register char *tmp, *named, *called;
	    int l;

	    /* some objects have capitalized adjectives in their names */
	    if(((tmp = rindex(str, ' ')) || (tmp = rindex(str, '-'))) &&
	       (!isupper2(tmp[1])))
		insert_the = TRUE;
	    else if (tmp && index(str, ' ') < tmp) {	/* has spaces */
		/* it needs an article if the name contains "de" */
		tmp = strstri(str, " de ");
		if (!tmp) tmp = strstri(str, " del ");
		named = strstri(str, " nombrado ");
		if (!named) named = strstri(str, " nombrada ");
		if (!named) named = strstri(str, " nombrados ");
		if (!named) named = strstri(str, " nombradas ");
		called = strstri(str, " llamado ");
		if (!called) called = strstri(str, " llamada ");
		if (!called) called = strstri(str, " llamados ");
		if (!called) called = strstri(str, " llamadas ");
		if (called && (!named || called < named)) named = called;

		if (tmp && (!named || tmp < named))	/* found an "of" */
		    insert_the = TRUE;
		/* stupid special case: lacks "of" but needs "the" */
		else if (!named && (l = strlen(str)) >= 31 &&
		      !strcmp(&str[l - 31], "Platinum Yendorian Express Card"))
		    insert_the = TRUE;
	    }
	}
	if (insert_the) {
	    /* isfeminine() is set up to ignore "par de", so that "par de
	       botas" gets "malditas", for example, rather than "maldito";
	       nonetheless, we want "un par" and not "una par" */
	    if (strncmpi2(str, "par de ", 7) == 0)
		Strcpy(buf, "el ");
	    else if (isplural(str))
		Strcpy(buf, isfeminine(str)? "las " : "los ");
	    else {
		if (needs_el(str))
		    Strcpy(buf, "el ");
		else
		    Strcpy(buf, isfeminine(str)? "la "  : "el ");
	    }
	} else
	    buf[0] = '\0';
	Strcat(buf, str);

	return buf;
}

char *
The(str)
const char *str;
{
    register char *tmp = the(str);
    *tmp = toupper2(*tmp);
    return tmp;
}


/* Identify nouns beginning with stressed A or HA.  These require _el_
   regardless of their gender. */
boolean
needs_el(str)
const char *str;
{
    unsigned i;
    unsigned syllables;

    i = 0;
    if (tolower2(str[0]) == 'h') i = 1;	/* Ignore initial h */

    /* Easy case: words beginning with Á */
    if ((char)tolower2(str[i]) == 'á') return TRUE;

    /* Easy case: words not beginning with A or Á */
    if (tolower2(str[i]) != 'a') return FALSE;

    /* Easy case: an accent on a vowel that is not initial A */
    for (i=0; str[i]!=0; i++) {
	if (index("áéíóú", tolower2(str[i])))
	    return FALSE;
    }

    /* Two exceptions:  these are names of letters */
    if (!strcmpi2(str, "a") || !strcmpi2(str, "hache"))
	return FALSE;

    /* The only words that get this far are the ones that begin with A or HA
       and do not contain accented vowels.  Since there is no accent, default
       stress rules apply.  Such words may or may not require _el_ depending
       on the number of syllables and the final letter:

       o One syllable:  always requires _el_
       o Two syllables:  requires _el_ only if stress on first syllable, i.e.,
         if final letter is a vowel, n, or s
       o Three or more syllables:  never requires _el_.

       We only need to consider the first word. */

    syllables = 0;
    for (i=0; isalpha2(str[i]) && syllables<3; i++) {
        /* Count a syllable when we see its first vowel */
	if (index("aeo", tolower2(str[i]))) {
	    /* Strong vowel:  count a syllable if initial or not after I or U */
	    if (i==0 || !index("iu", tolower2(str[i-1])))
		syllables++;
	}
	else if (index("iu", tolower2(str[i]))) {
	    /* Weak vowel:  count a syllable if initial or not after a vowel */
	    if (i==0 || !index("aeiou", tolower2(str[i-1])))
		syllables++;
	}
    }

    if (syllables < 2) return TRUE;
    if (syllables > 2) return FALSE;

    /* Two syllable word; str[i-1] is the final character */
    return index("aeiouns", str[i-1]) != 0;
}


/* Prepend "a" and combine correctly with "el" */
char *
al(str)
const char *str;
{
    char *buf = nextobuf();

    /* We match "el " but not "El " because we don't want to handle proper
       names specially */
    if (strncmp(str, "el ", 3) == 0) {
	Strcpy(buf, str);
	buf[0] = 'a';
    } else {
	Strcpy(buf, "a ");
	Strcat(buf, str);
    }
    return buf;
}

char *
Al(str)
const char *str;
{
    register char *tmp = al(str);
    *tmp = toupper2(*tmp);
    return tmp;
}


/* Prepend "de" and combine correctly with "el" */
char *
del(str)
const char *str;
{
    char *buf = nextobuf();

    /* We match "el " but not "El " because we don't want to handle proper
       names specially */
    Strcpy(buf, strncmp(str, "el ", 3) == 0? "d" : "de ");
    Strcat(buf, str);
    return buf;
}

char *
Del(str)
const char *str;
{
    register char *tmp = del(str);
    *tmp = toupper2(*tmp);
    return tmp;
}

/* Capitalize the first letter of a possibly-nonmodifiable string */
char *
capitalize(str)
const char *str;
{
	char *buf = nextobuf();

	Strcpy(buf, str);
	*buf = toupper2(*buf);
	return buf;
}

/* returns "count cxname(otmp)" or just cxname(otmp) if count == 1 */
char *
aobjnam(otmp,verb)
register struct obj *otmp;
register const char *verb;
{
	register char *bp = cxname(otmp);
	char prefix[PREFIX];

	if(otmp->quan != 1L) {
		Sprintf(prefix, "%ld ", otmp->quan);
		bp = strprepend(bp, prefix);
	}

	if(verb) {
	    Strcat(bp, " ");
	    Strcat(bp, otense(otmp, verb));
	}
	return(bp);
}

/* like aobjnam, but prepend "The", not count, and use xname */
char *
Tobjnam(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
	char *bp = The(xname(otmp));

	if(verb) {
	    Strcat(bp, " ");
	    Strcat(bp, otense(otmp, verb));
	}
	return(bp);
}

/* LENGUA:  The exact usage of the verb tense functions, otense and
   vtense, is changed slightly in Spanish Nethack:  the English version
   accepts a plural verb and possibly returns a singular (speak -> speaks)
   and the Spanish version accepts a singular verb and possibly returns a
   plural (habla -> hablan). */

/* return form of the verb (input singular) if xname(otmp) were the subject */
char *
otense(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
	char *buf;

	/*
	 * verb is given in singular (without trailing n).  Return as input
	 * if the result of xname(otmp) would be singular.  Don't bother
	 * recomputing xname(otmp) at this time.
	 */
	if (is_plural(otmp))
	    return vtense((char *)0, verb);

	buf = nextobuf();
	Strcpy(buf, verb);
	return buf;
}

/* return form of the verb (input singular) for present tense 3rd person subj */
char *
vtense(subj, verb)
register const char *subj;
register const char *verb;
{
	char *buf = nextobuf();

	/*
	 * verb is given in singular (without trailing n).  Return as input
	 * if subj appears to be singular.  Add special cases as necessary.
	 * Many hard cases can already be handled by using otense() instead.
	 * If this gets much bigger, consider decomposing makeplural.
	 * Note: monster names are not expected here (except before corpse).
	 *
	 * special case: allow null sobj to get the plural 3rd person
	 * present tense form so we don't duplicate this code elsewhere.
	 */
	Strcpy(buf, verb);
	/* The only special case is es -> son */
	if (!subj || isplural(subj)) {
	    if (strcmp(verb, "es") == 0)
		Strcpy(buf, "son");
	    else if (strcmp(verb, "no es") == 0)
		Strcpy(buf, "no son");
	    else
		Strcat(buf, "n");
	}
	else if (strcmpi2(subj, "tú") == 0) {
	    if (strcmp(verb, "es") == 0)
		Strcpy(buf, "eres");
	    else if (strcmp(verb, "no es") == 0)
		Strcpy(buf, "no eres");
	    else
		Strcat(buf, "s");
	}

	return buf;
}

/* capitalized variant of doname() */
char *
Doname2(obj)
register struct obj *obj;
{
	register char *s = doname(obj);

	*s = toupper2(*s);
	return(s);
}

/* returns "tu(s) xname(obj)" or "el xname(obj) de Foobar" or "el xname(obj)" */
char *
yname(obj)
struct obj *obj;
{
	char *outbuf = nextobuf();

	shk_your(outbuf, obj, (char *)0);
	return outbuf;
}

/* capitalized variant of yname() */
char *
Yname2(obj)
struct obj *obj;
{
	char *s = yname(obj);

	*s = toupper2(*s);
	return s;
}

/* returns "your simple_typename(obj->otyp)"
 * or "Foobar's simple_typename(obj->otyp)"
 * or "the simple_typename(obj-otyp)"
 */
char *
ysimple_name(obj)
struct obj *obj;
{
	char *outbuf = nextobuf();
	shk_your(outbuf, obj, simple_typename(obj->otyp));	/* assert( s == outbuf ); */
	return outbuf;
}

/* capitalized variant of ysimple_name() */
char *
Ysimple_name2(obj)
struct obj *obj;
{
	char *s = ysimple_name(obj);

	*s = toupper2(*s);
	return s;
}

#endif /* OVLB */
#ifdef OVL0

/* "Feminize" a single word */
/* Use for both nouns and adjectives */
static char *
feminize_one(str, len)
const char *str;
int len;
{
	/* List of exceptions */
	/* If "fem" is NULL, the masculine and feminine are the same */
	static const struct
	{
		char *masc;
		char *fem;
	} excepciones[] =
	{
		{ "él", "ella" },
		{ "el", "la" },
		{ "un", "una" },
		{ "este", "esta" },
		{ "éste", "ésta" },
		{ "ese", "esa" },
		{ "ése", "ésa" },
		{ "aquel", "aquella" },
		{ "aquél", "aquélla" },
		{ "abad", "abadesa" },
		{ "hombre", "mujer" },
		{ "señor", "señora" },
		{ "rey", "reina" },
		{ "sacerdote", "sacerdotisa" },
		{ "osobúho", "osabúha" },
		{ "caballo", "yegua" },
		{ "caballito", "yegüita" },
		{ "potrillo", "potranca" },
		{ "vampiro", "vampiresa" },
		{ "soldado", (char *)0 },
		{ "oráculo", (char *)0 },

		/* Adverbs */
		{ "alrededor", (char *)0 },

		/* List terminator */
		{ (char *)0, (char *)0 }
	};

	char *buf = nextobuf();
	unsigned i;

	strncpy(buf, str, len);
	buf[len] = 0;

	/* Look for the word in the list of exceptions */
	for (i=0; excepciones[i].masc!=(char *)0; i++)
		if (strcmpi2(buf, excepciones[i].masc)==0)
			break;

	/* If the word is found, return the feminine */
	if (excepciones[i].masc != (char *)0) {
		if (excepciones[i].fem != (char *)0) {
			Strcpy(buf, excepciones[i].fem);
			/* Preserve capitalization */
			if (isupper2(str[0]))
				buf[0] = toupper2(buf[0]);
		}
		return buf;
	}

	/*
	 * Regular rules to form Spanish feminines:
	 *
	 * 1) Words in -o change to -a.
	 *
	 * 2) Words in -dor change to -dora.
	 *
	 * 3) Words in -ón but not in -ión change to -ona.
	 *
	 * 4) Words in -és change to -esa.
	 *
	 * 5) There may be other rules, but for now, other words don't change
	 *
	 * Plurals are also considered.
	 */

	/* Gerunds */
	if (len>=4 && (strcmpi2(buf+len-4, "ando")==0))
		return buf;
	if (len>=5 && (strcmpi2(buf+len-5, "iendo")==0))
		return buf;

	/* -o */
	if (len>=1 && (strcmp(buf+len-1, "o")==0)) {
		buf[len-1] = 'a';
		return buf;
	}
	if (len>=1 && (strcmp(buf+len-1, "O")==0)) {
		buf[len-1] = 'A';
		return buf;
	}
	if (len>=2 && (strcmp(buf+len-2, "os")==0)) {
		buf[len-2] = 'a';
		return buf;
	}
	if (len>=2 && (strcmp(buf+len-2, "OS")==0)) {
		buf[len-2] = 'A';
		return buf;
	}

	/* -ión does not change */
	if (len>=3 && strcmpi2(buf+len-3, "ión")==0)
		return buf;
	if (len>=5 && strcmpi2(buf+len-5, "iones")==0)
		return buf;

	/* -ón -> -ona */
	if (len>=2 && strcmp(buf+len-2, "ón")==0) {
		Strcpy(buf+len-2, "ona");
		return buf;
	}
	if (len>=2 && strcmp(buf+len-2, "ÓN")==0) {
		Strcpy(buf+len-2, "ONA");
		return buf;
	}
	if (len>=4 && strcmp(buf+len-4, "ones")==0) {
		buf[len-2] = 'a';
		return buf;
	}
	if (len>=4 && strcmp(buf+len-4, "ONES")==0) {
		buf[len-2] = 'A';
		return buf;
	}

	/* -és -> -esa */
	if (len>=2 && strcmp(buf+len-2, "és")==0) {
		Strcpy(buf+len-2, "esa");
		return buf;
	}
	if (len>=2 && strcmp(buf+len-2, "ÉS")==0) {
		Strcpy(buf+len-2, "ESA");
		return buf;
	}
	if (len>=4 && strcmp(buf+len-4, "eses")==0) {
		buf[len-2] = 'a';
		return buf;
	}
	if (len>=4 && strcmp(buf+len-4, "ESES")==0) {
		buf[len-2] = 'A';
		return buf;
	}

	/* -dor -> -dora */
	if (len>=3 && strcmp(buf+len-3, "dor")==0) {
		Strcpy(buf+len, "a");
		return buf;
	}
	if (len>=3 && strcmp(buf+len-3, "DOR")==0) {
		Strcpy(buf+len, "A");
		return buf;
	}
	if (len>=5 && strcmp(buf+len-5, "dores")==0) {
		buf[len-2] = 'a';
		return buf;
	}
	if (len>=5 && strcmp(buf+len-5, "DORES")==0) {
		buf[len-2] = 'A';
		return buf;
	}

	/* other words */
	return buf;
}

/*
 * feminize() is used when inserting words (chiefly adjectives) into messages
 * when it is inconvenient to do the declension locally.  When fem is true,
 * we convert the given string to a feminine form; when fem is false, we
 * leave it alone.
 *
 * mon_nam() also uses feminize() to convert the names of certain monsters,
 * such as enfermero -> enfermera.
 */
char *
feminize(oldstr, fem)
const char *oldstr;
boolean fem;
{
	char *str = nextobuf();
	unsigned i, j;
	unsigned len;
	boolean preposition_found;
	char *pl;

	if (!fem) { /* Masculine form requested */
		strcpy(str, oldstr);
		return str;
	}

	/*
	 * The method used here is to "feminize" everything until we see a
	 * preposition.  Once a preposition is seen, we stop "feminizing",
	 * except for certain adjectives and participles appended by xname().
	 *
	 * This works similarly to singularize() and pluralize().
	 *
	 * feminize_one() knows about adverbs and will not alter them.
	 */

	i = 0;
	j = 0;
	preposition_found = FALSE;

	/* Find the start and end of a word */

	while (oldstr[i] != 0) {
		/* Copy spaces, digits, punctuation, etc */
		while (oldstr[i]!=0 && !isalpha2(oldstr[i]))
			str[j++] = oldstr[i++];
		if (oldstr[i] == 0) break;

		/* Found a word.  Locate its end */
		for (len=0; isalpha2(oldstr[i+len]); len++) {}

		/* Is it a preposition? */
		if ((len==2 && strncmp(oldstr+i, "de", len)==0)
		||  (len==3 && strncmp(oldstr+i, "del", len)==0)
		||  (len==1 && strncmp(oldstr+i, "a", len)==0)
		||  (len==2 && strncmp(oldstr+i, "al", len)==0)
		||  (len==2 && strncmp(oldstr+i, "en", len)==0)
		||  (len==3 && strncmp(oldstr+i, "con", len)==0)
		||  (len==3 && strncmp(oldstr+i, "sin", len)==0)
		||  (len==3 && strncmp(oldstr+i, "por", len)==0)
		||  (len==4 && strncmp(oldstr+i, "como", len)==0)
		||  (len==4 && strncmp(oldstr+i, "para", len)==0)
		||  (len==6 && strncmp(oldstr+i, "contra", len)==0)) {
			preposition_found = TRUE;
			/* Copy without change */
			strncpy(str+j, oldstr+i, len);
			j += len;
		/* Unlike singularize() and pluralize() we don't need to
		   consider the feminine forms here */
		} else if ((len==7 && strncmp(oldstr+i, "marcado", len)==0)
		||         (len==10 && strncmp(oldstr+i, "envenenado", len)==0)
		||         (len==7 && strncmp(oldstr+i, "llamado", len)==0)
		||         (len==9 && strncmp(oldstr+i, "nombrado", len)==0)) {
			/* Participles appended by xname() */
			/* Make it feminine */
			pl = feminize_one(oldstr+i, len);

			/* Add it to the output */
			Strcpy(str+j, pl);
			j += strlen(pl);

			/* What follows is a name */
			preposition_found = TRUE;
		} else if (preposition_found) {
			/* Copy without change */
			strncpy(str+j, oldstr+i, len);
			j += len;
		} else {
			/* Make it feminine */
			pl = feminize_one(oldstr+i, len);

			/* Add it to the output */
			Strcpy(str+j, pl);
			j += strlen(pl);
		}
		i += len;
	}

	str[j] = 0;
	return str;
}


/* Pluralize a single word */
/* Use for both nouns and adjectives */
static char *
pluralize_one(str, len)
const char *str;
int len;
{
	/* Lista de excepciones */
	/* Si "plural" es (char *)0, es lo mismo que el singular */
	static const struct
	{
		char *singular;
		char *plural;
	} excepciones[] =
	{
		{ "c", (char *)0 },		/* for ración C */
		{ "k", (char *)0 },		/* for ración K */
		{ "uruk", (char *)0 },          /* for Uruk-hai */
		{ "olog", (char *)0 },          /* for Olog-hai */
		{ "hai", (char *)0 },
		{ "el", "los" },
		{ "un", "unos" },
		{ "este", "estos" },
		{ "éste", "éstos" },
		{ "bumerán", "bumeranes" },
		{ "etin", "étines" },

		/* Foreign plurals */
		{ "mumak", "mumakil" },		/* Elvish */
		{ "bec", "becs" },		/* French */
		{ "hobbit", "hobbits" },	/* English */
		{ "kobold", "kobolds" },	/* English */
		{ "ettin", "ettins" },		/* English */
		{ "márilith", "máriliths" },	/* English */

		/* Adverbios no en -mente */
		{ "no", (char *)0 },
		{ "arriba", (char *)0 },

		/* Terminador de lista */
		{ (char *)0, (char *)0 }
	};
	static const char accvowels[] = "áéíóú";
	static const char    vowels[] = "aeiou";
	static const char Accvowels[] = "ÁÉÍÓÚ";
	static const char    Vowels[] = "AEIOU";
	char *p;

	char *buf = nextobuf();
	unsigned i;

	strncpy(buf, str, len);
	buf[len] = 0;

	/* Buscamos la palabra en la lista de excepciones */
	for (i=0; excepciones[i].singular!=(char *)0; i++)
		if (strcmpi2(buf, excepciones[i].singular)==0)
			break;

	/* Si hemos buscado la palabra, devolvemos el plural */
	if (excepciones[i].singular != (char *)0) {
		if (excepciones[i].plural != (char *)0) {
			Strcpy(buf, excepciones[i].plural);
			/* Conservemos la capitalización */
			if (isupper2(str[0]))
				buf[0] = toupper2(buf[0]);
		}
		return buf;
	}

	/*
	 * Reglas regulares de nombres plurales en español:
	 *
	 * 1) Después de -es o -is, no hay que cambiar la palabra.
	 *
	 * 2) Después de una vocal sin acento, añadamos "s".
	 *
	 * 3) Por otras palabras, añadamos "es".
	 *
	 * Es *mucho* más facil que en inglés. :-)  También, no cambiamos
	 * las palabras en -mente (son adverbios).
	 */

	/* -es e -is */
	if (len>=2
	&&  (strcmpi2(buf+len-2, "es")==0 || strcmpi2(buf+len-2, "is")==0))
		return buf;

	/* adverbios en -mente */
	/* otros adverbios deben estar en la lista de excepciones */
	if (len>=5 && strcmpi2(buf+len-5, "mente")==0)
		return buf;

    /* Palabras que terminan en una vocal con tilde y 's' o 'n' */
    if (len>=2
    &&  (p=strchr(accvowels, buf[len-2])) != (char *)0
    &&  strchr("ns", buf[len-1]) != (char *)0)
    {
	buf[len-2] = vowels[(int)(p-accvowels)];
	Strcat(buf, "es");
	return buf;
    }
    if (len>=2
    &&  (p=strchr(Accvowels, buf[len-2])) != (char *)0
    &&  strchr("NS", buf[len-1]) != (char *)0)
    {
	buf[len-2] = Vowels[(int)(p-accvowels)];
	Strcat(buf, "ES");
	return buf;
    }
	/* FIXME:  Hay otras palabras que ganan un acento */

	/* vocales sin acento, y é */
	if (len>=1
        &&  (buf[len-1]=='É' || strchr(Vowels, buf[len-1])!=(char *)0)) {
		Strcat(buf, "S");
		return buf;
	}
	if (len>=1
        &&  (buf[len-1]=='é' || strchr(vowels, buf[len-1])!=(char *)0)) {
		Strcat(buf, "s");
		return buf;
	}

	/* otras palabras */
	if (len>=1 && isupper2(buf[len-1]))
		Strcat(buf, "ES");
	else
		Strcat(buf, "es");
	return buf;
}

/* Plural routine; chiefly used for user-defined fruits.  We have to try to
 * account for everything reasonable the player has; something unreasonable
 * can still break the code.  However, it's still a lot more accurate than
 * "just add an s at the end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi.")
 * and body parts.
 */
char *
makeplural(oldstr)
const char *oldstr;
{
	char *str = nextobuf();
	unsigned i, j;
	unsigned len;
	boolean preposition_found;
	char *pl;

	/*
	 * Pluralization is both easier and harder in Spanish than in English.
	 * Easier, because there are so few exceptions to the rules.  Harder,
	 * because adjectives and participles also have to be made plural.
	 *
	 * The method used here is to pluralize everything until we see a
	 * preposition.  Once a preposition is seen, we stop pluralizing,
	 * except for certain adjectives and participles appended by xname().
	 *
	 * pluralize_one() knows about adverbs and will not alter them.
	 *
	 * The idea is that
	 *
	 *    poción de curación total bendita
	 *
	 * should become
	 *
	 *    pociones de curación total benditas
	 *
	 * Don't forget the feminine forms!
	 */

	i = 0;
	j = 0;
	preposition_found = FALSE;

	/* Find the start and end of a word */

	while (oldstr[i] != 0) {
		/* Copy spaces, digits, punctuation, etc */
		while (oldstr[i]!=0 && !isalpha2(oldstr[i]))
			str[j++] = oldstr[i++];
		if (oldstr[i] == 0) break;

		/* Found a word.  Locate its end */
		for (len=0; isalpha2(oldstr[i+len]); len++) {}

		/* Is it a preposition? */
		if ((len==2 && strncmp(oldstr+i, "de", len)==0)
		||  (len==3 && strncmp(oldstr+i, "del", len)==0)
		||  (len==1 && strncmp(oldstr+i, "a", len)==0)
		||  (len==2 && strncmp(oldstr+i, "al", len)==0)
		||  (len==2 && strncmp(oldstr+i, "en", len)==0)
		||  (len==3 && strncmp(oldstr+i, "con", len)==0)
		||  (len==3 && strncmp(oldstr+i, "sin", len)==0)
		||  (len==3 && strncmp(oldstr+i, "por", len)==0)
		||  (len==4 && strncmp(oldstr+i, "para", len)==0)
		||  (len==6 && strncmp(oldstr+i, "contra", len)==0)) {
			preposition_found = TRUE;
			/* Copy without change */
			strncpy(str+j, oldstr+i, len);
			j += len;
		} else if ((len==7 && strncmp(oldstr+i, "marcado", len)==0)
		||         (len==7 && strncmp(oldstr+i, "marcada", len)==0)
		||         (len==10 && strncmp(oldstr+i, "envenenado", len)==0)
		||         (len==10 && strncmp(oldstr+i, "envenenada", len)==0)
		||         (len==7 && strncmp(oldstr+i, "llamado", len)==0)
		||         (len==7 && strncmp(oldstr+i, "llamada", len)==0)
		||         (len==9 && strncmp(oldstr+i, "nombrado", len)==0)
		||         (len==9 && strncmp(oldstr+i, "nombrada", len)==0)) {
			/* Participles appended by xname() */
			/* Make it plural */
			pl = pluralize_one(oldstr+i, len);

			/* Add it to the output */
			Strcpy(str+j, pl);
			j += strlen(pl);

			/* What follows is a name */
			preposition_found = TRUE;
		} else if (preposition_found) {
			/* Copy without change */
			strncpy(str+j, oldstr+i, len);
			j += len;
		} else {
			/* Make it plural */
			pl = pluralize_one(oldstr+i, len);

			/* Add it to the output */
			Strcpy(str+j, pl);
			j += strlen(pl);
		}
		i += len;
	}

	str[j] = 0;
	return str;
}

#endif /* OVL0 */

#ifdef OVLB

/* Singularize a single word */
/* Use for both nouns and adjectives */
static char *
singularize_one(str, len)
const char *str;
int len;
{
	/* Lista de excepciones */
	/* Si "singular" es (char *)0, es lo mismo que el plural */
	static const struct
	{
		char *plural;
		char *singular;
	} excepciones[] =
	{
		/* Formas irregulares */
		{ "los", "el" },
		{ "unos", "un" },
		{ "estos", "este" },
		{ "éstos", "éste" },
		{ "mumakil", "mumak" },

		/* Palabras que no cambian en plural */
		{ "desuellamentes", "desuellamentes" },
		{ "manes", "manes" },
		{ "wumpus", "wumpus" },
		{ "ciempiés", "ciempiés" },
		{ "erinias", "erinias" },
		{ "Orcus", "Orcus" },
		{ "Asmodeus", "Asmodeus" },
		{ "Pelias", "Pelias" },
		{ "Hipócrates", "Hipócrates" },
		{ "dos", "dos" },	/* "Dos Flores" */
		{ "aklys", "aklys" },
		{ "abrelatas", "abrelatas" },
		{ "gris", "gris" },

		/* Objetos cuyos nombres son normalmente plurales */
		{ "escamas", "escamas" }, /* escamas de dragon... */
		{ "guantes", "guantes" },
		{ "guanteletes", "guanteletes" },
		{ "botas", "botas" },
		{ "zapatos", "zapatos" },
		{ "anteojos", "anteojos" },

		/* Terminador de lista */
		{ (char *)0, (char *)0 }
	};

	char *buf = nextobuf();
	unsigned i;

	strncpy(buf, str, len);
	buf[len] = 0;

	/* Buscamos la palabra en la lista de excepciones */
	for (i=0; excepciones[i].plural!=(char *)0; i++)
		if (strcmpi2(buf, excepciones[i].plural)==0)
			break;

	/* Si hemos buscado la palabra, devolvemos el singular */
	if (excepciones[i].plural != (char *)0) {
		if (excepciones[i].singular != (char *)0) {
			Strcpy(buf, excepciones[i].singular);
			/* Conservemos la capitalización */
			if (isupper2(str[0]))
				buf[0] = toupper2(buf[0]);
		}
		return buf;
	}

	/* Aquí quitamos el sufijo plural */

	if (len==0 || (buf[len-1]!='S' && buf[len-1]!='s'))
		return buf;		/* No es un plural */

	/* Palabras en -s */
	if (len==1 || (buf[len-2]!='E' && buf[len-2]!='e')) {
		/* -s pero no -es */
		/* Usamos la lista de excepciones por palabras en -es e -is
		   que no cambian en el plural */
		buf[len-1] = 0;
		return buf;
	}

	/* Palabras en -es */
	if (len>=5
	&&  (strcmpi2(buf+len-5, "antes")==0 || strcmpi2(buf+len-5, "entes")==0)) {
		/* -antes y -entes; quitamos la s */
		buf[len-1] = 0;
		return buf;
	}

	if (len>=4 && strcmpi2(buf+len-4, "ones")==0) {
		/* -ones */
		Strcpy(buf+len-4, "ón");
		return buf;
	}

	/* Quitamos el sufijo -es */
	buf[len-2] = 0;
	return buf;
}

/*
 * Singularize a string the user typed in; this helps reduce the complexity
 * of readobjnam, and is also used in pager.c to singularize the string
 * for which help is sought.
 */

char *
makesingular(oldstr)
const char *oldstr;
{
	char *str = nextobuf();
	unsigned i, j;
	unsigned len;
	boolean preposition_found;
	char *sing;

	i = 0;
	j = 0;
	preposition_found = FALSE;

	/* Find the start and end of a word */

	while (oldstr[i] != 0) {
		/* Copy spaces, digits, punctuation, etc */
		while (oldstr[i]!=0 && !isalpha2(oldstr[i]))
			str[j++] = oldstr[i++];
		if (oldstr[i] == 0) break;

		/* Found a word.  Locate its end */
		for (len=0; isalpha2(oldstr[i+len]); len++) {}

		/* Is it a preposition? */
		if ((len==2 && strncmp(oldstr+i, "de", len)==0)
		||  (len==3 && strncmp(oldstr+i, "del", len)==0)
		||  (len==1 && strncmp(oldstr+i, "a", len)==0)
		||  (len==2 && strncmp(oldstr+i, "al", len)==0)
		||  (len==2 && strncmp(oldstr+i, "en", len)==0)
		||  (len==3 && strncmp(oldstr+i, "con", len)==0)
		||  (len==3 && strncmp(oldstr+i, "sin", len)==0)
		||  (len==3 && strncmp(oldstr+i, "por", len)==0)
		||  (len==4 && strncmp(oldstr+i, "para", len)==0)
		||  (len==6 && strncmp(oldstr+i, "contra", len)==0)) {
			preposition_found = TRUE;
			/* Copy without change */
			strncpy(str+j, oldstr+i, len);
			j += len;
		} else if ((len==7 && strncmp(oldstr+i, "marcados", len)==0)
		||         (len==7 && strncmp(oldstr+i, "marcadas", len)==0)
		||         (len==10 && strncmp(oldstr+i, "envenenados", len)==0)
		||         (len==10 && strncmp(oldstr+i, "envenenadas", len)==0)
		||         (len==7 && strncmp(oldstr+i, "llamados", len)==0)
		||         (len==7 && strncmp(oldstr+i, "llamadas", len)==0)
		||         (len==9 && strncmp(oldstr+i, "nombrados", len)==0)
		||         (len==9 && strncmp(oldstr+i, "nombradas", len)==0)) {
			/* Participles appended by xname() */
			/* Make it singular */
			sing = singularize_one(oldstr+i, len);

			/* Add it to the output */

			Strcpy(str+j, sing);
			j += strlen(sing);
		} else if (preposition_found) {
			/* Copy without change */
			strncpy(str+j, oldstr+i, len);
			j += len;
		} else {
			/* Make it singular */
			sing = singularize_one(oldstr+i, len);

			/* Add it to the output */

			Strcpy(str+j, sing);
			j += strlen(sing);
		}
		i += len;
	}

	str[j] = 0;
	return str;
}

/* alternate spellings; if the difference is only the presence or
   absence of spaces and/or hyphens (such as "pickaxe" vs "pick axe"
   vs "pick-axe") then there is no need for inclusion in this list;
   likewise for ``"of" inversions'' ("boots of speed" vs "speed boots") */
struct alt_spellings {
	const char *sp;
	int ob;
} spellings[] = {
	{ "escudo llano", SHIELD_OF_REFLECTION },
	{ "bola de hierro", HEAVY_IRON_BALL },
	{ "linterna", BRASS_LANTERN },
	{ "azadón", DWARVISH_MATTOCK },
	{ "amuleto de resistancia de veneno", AMULET_VERSUS_POISON },
	{ "piedra", ROCK },
#ifdef TOURIST
	{ "cámara", EXPENSIVE_CAMERA },
#endif
	{ "eucalipto", EUCALYPTUS_LEAF },
	{ (const char *)0, 0 },
};


/* Determines whether word matches the given keyword */

boolean
cmp_keyword(word, keyword)
const char *word;
const char *keyword;
{
	unsigned len1;
	unsigned len2 = strlen(keyword);
	char declension[4][40];
	unsigned i;

	/* Determine the length of the word */
	for (len1=0; isalpha2(word[len1]); len1++) {}

	/* Decline the keyword */
	strcpy(declension[0], keyword);
	if (len2!=0 && keyword[len2-1]=='@') {
		/* Keyword inflects for gender */
		strcpy(declension[1], declension[0]);
		declension[0][len2-1] = 'o'; /* masculine */
		declension[1][len2-1] = 'a'; /* feminine */
		/* Get the plurals */
		strcpy(declension[2], pluralize_one(declension[0], len2));
		strcpy(declension[3], pluralize_one(declension[1], len2));
	} else {
		/* Keyword does not inflect for gender */
		strcpy(declension[1], pluralize_one(declension[0], len2));
		declension[2][0] = 0;
		declension[3][0] = 0;
	}

	for (i=0; i<4; i++) {
		if (len1==strlen(declension[i])
		&&  strncmpi2a(word, declension[i], len1)==0)
			return TRUE;
	}

	return FALSE;
}


/* make keyword list from user-supplied object name */

static void
make_called_objwords(index, words)
int index;
char words[MAX_WORDS][LONGEST_WORD];
{
	/* "llamado" won't be part of the parsed keyword list and so it isn't
	   included in the prefixes */
	switch (objects[index].oc_class) {
	case RING_CLASS:
		gen_keywords(words, "anillo", objects[index].oc_uname);
		break;

	case AMULET_CLASS:
		gen_keywords(words, "amuleto", objects[index].oc_uname);
		break;

	case POTION_CLASS:
		gen_keywords(words, "poción", objects[index].oc_uname);
		break;

	case SCROLL_CLASS:
		gen_keywords(words, "pergamino", objects[index].oc_uname);
		break;

	case SPBOOK_CLASS:
		gen_keywords(words, "libro", objects[index].oc_uname);
		break;

	case WAND_CLASS:
		gen_keywords(words, "varita", objects[index].oc_uname);
		break;

	case GEM_CLASS:
		if (strcmp(obj_descr[index].oc_name, "roca") == 0)
			gen_keywords(words, "roca", objects[index].oc_uname);
		else
			gen_keywords(words, "joya", objects[index].oc_uname);
		break;

	default:
		gen_keywords(words, (char *)0, objects[index].oc_uname);
		break;
	}
}


/* make keyword list from identified object name */

static void
make_ided_objwords(index, words)
int index;
char words[MAX_WORDS][LONGEST_WORD];
{
	switch (objects[index].oc_class) {
	case RING_CLASS:
		gen_keywords(words, "anillo de", obj_descr[index].oc_name);
		break;

	case AMULET_CLASS:
		gen_keywords(words, (char *)0, obj_descr[index].oc_name);
		break;

	case POTION_CLASS:
		gen_keywords(words, "poción de", obj_descr[index].oc_name);
		break;

	case SCROLL_CLASS:
		gen_keywords(words, "pergamino de", obj_descr[index].oc_name);
		break;

	case SPBOOK_CLASS:
		gen_keywords(words, "libro de", obj_descr[index].oc_name);
		break;

	case WAND_CLASS:
		gen_keywords(words, "varita de", obj_descr[index].oc_name);
		break;

	case GEM_CLASS:
		if (objects[index].oc_material == GLASS)
			gen_keywords(words, (char *)0, obj_descr[index].oc_name);
		else if (strcmp(obj_descr[index].oc_name, "roca") == 0)
			gen_keywords(words, (char *)0, obj_descr[index].oc_name);
		else
			gen_keywords(words, "joya de", obj_descr[index].oc_name);
		break;

	default:
		gen_keywords(words, (char *)0, obj_descr[index].oc_name);
		break;
	}
}


/* make keyword list from unidentified object name */

static void
make_unided_objwords(index, words)
int index;
char words[MAX_WORDS][LONGEST_WORD];
{
	switch (objects[index].oc_class) {
	case RING_CLASS:
		gen_keywords(words, "anillo", obj_descr[index].oc_descr);
		break;

	case AMULET_CLASS:
		gen_keywords(words, "amuleto", obj_descr[index].oc_descr);
		break;

	case POTION_CLASS:
		gen_keywords(words, "poción", obj_descr[index].oc_descr);
		break;

	case SCROLL_CLASS:
		gen_keywords(words, "pergamino marcado", obj_descr[index].oc_descr);
		break;

	case SPBOOK_CLASS:
		gen_keywords(words, "libro", obj_descr[index].oc_descr);
		break;

	case WAND_CLASS:
		gen_keywords(words, "varita", obj_descr[index].oc_descr);
		break;

	case GEM_CLASS:
		if (objects[index].oc_material == GLASS)
			gen_keywords(words, "joya", obj_descr[index].oc_descr);
		else if (strcmp(obj_descr[index].oc_name, "roca") == 0)
			gen_keywords(words, (char *)0, obj_descr[index].oc_descr);
		else
			gen_keywords(words, "joya", obj_descr[index].oc_descr);
		break;

	default:
		gen_keywords(words, (char *)0, obj_descr[index].oc_descr);
		break;
	}
}

static void
gen_keywords(words, prefix, name)
char words[MAX_WORDS][LONGEST_WORD];
const char *prefix;
const char *name;
{
    const char *nm;
    unsigned i, j, len, len2;
    char word[LONGEST_WORD];
    boolean is_a_noun;
    boolean prefix_done;

	if (name == (char *)0) return;

	is_a_noun = TRUE;
	/* Ugly kludge to process the prefix (if it isn't null) and then the
	   name */
	if (prefix == (char *)0) {
		nm = name;
		prefix_done = TRUE;
	} else {
		nm = prefix;
		prefix_done = FALSE;
	}
	i = 0;
	j = 0;
	while ((nm[i]!=0 || !prefix_done) && j<MAX_WORDS-1) {
		if (nm[i]==0 && !prefix_done) {
			nm = name;
			i = 0;
			prefix_done = TRUE;
		}

		/* Skip over any non-alphanumeric */
		while (nm[i]!=0 && !isalnum2(nm[i]))
			i++;
		if (nm[i] == 0) continue;

		/* Determine the length of the word */
		for (len=0; isalnum2(nm[i+len]); len++) {}

		/* Isolate the word */
		len2 = len;
		if (len2 >= sizeof(word)) len2 = sizeof(word)-1;
		memcpy(word, nm+i, len2);
		word[len2] = 0;

		/* Skip to the next word */
		i += len;

		/* Ignore articles and conjunctions */
		if (strcmp(word, "y") == 0
		||  strcmp(word, "e") == 0
		||  strcmp(word, "o") == 0
		||  strcmp(word, "u") == 0
		||  strcmp(word, "pero") == 0
		||  strcmp(word, "sino") == 0
		||  strcmp(word, "el") == 0
		||  strcmp(word, "la") == 0
		||  strcmp(word, "lo") == 0
		||  strcmp(word, "los") == 0
		||  strcmp(word, "las") == 0)
			continue;

		/*
		 * Assume the first word after a preposition is a noun.
		 * Don't enter the preposition into the keyword list, unless
		 * it is _contra_ (so _amuleto de cambio_ and _amuleto contra
		 * el cambio_ will be distinct).
		 */

		if (strcmp(word, "a") == 0
		||  strcmp(word, "al") == 0
		||  strcmp(word, "de") == 0
		||  strcmp(word, "en") == 0
		||  strcmp(word, "del") == 0
		||  strcmp(word, "contra") == 0
		||  strcmp(word, "por") == 0
		||  strcmp(word, "para") == 0) {
			is_a_noun = TRUE;
			if (strcmp(word, "contra") == 0) {
				strcpy(words[j], word);
				j++;
			}
			continue;
		}

		/* Undecline nouns and adjectives */
		if (len == len2) { /* Otherwise the declension is already lost */
			/* Convert to the singular form */
			if (len2>=2 && strcmp(word+len2-2, "es")==0
			&& !(len2>=5 && strcmp(word+len2-5, "antes")==0)
			&& !(len2>=5 && strcmp(word+len2-5, "entes")==0)
			&& strcmp(word, "guanteletes")!=0) {
				/* Remove -es */
				word[len2-2] = 0;
				len2 -= 2;
			}
			else if (word[len2-1]=='s'
			&&  strcmp(word, "dos")!=0	/* "two" */
			&&  strcmp(word, "aklys")!=0	/* Not a plural */
			&&  strcmp(word, "abrelatas")!=0	/* Not a plural */
			&&  strcmp(word, "lembas")!=0	/* Not a plural */
			&&  strcmp(word, "parálisis")!=0	/* Not a plural */
			&&  strcmp(word, "matalobos")!=0	/* Not a plural */
			&&  strcmp(word, "gris")!=0) {  /* Not a plural */
				word[len2-1] = 0;
				len2--;
			}

			/*
			 * Assume the first word, and any word following a
			 * preposition, is a noun:  leave it alone.  Other
			 * words are adjectives, unless specifically tested
			 * for.  Look for a gender inflection and mark it with
			 * an @ character.
			 */
			if (!is_a_noun
			&&  strcmp(word, "arriba")!=0) {  /* An adverb */
				if (len2>0 && (word[len2-1]=='a' || word[len2-1]=='o'))
					word[len2-1] = '@';
			}
		}

		if (strcmp(word, "dos") != 0) { /* An adjective before the noun */
			/* The next word will not be a noun */
			is_a_noun = FALSE;
		}
		if (len2>=2
		&&  (strcmp(word+len2-1, "ar")==0
		  || strcmp(word+len2-1, "er")==0
		  || strcmp(word+len2-1, "ir")==0)
		&&  strcmp(word, "circular")!=0
		&&  strcmp(word, "triangular")!=0
		&&  strcmp(word, "vulgar")!=0) {
			/* Infinitives; a direct object may follow */
			is_a_noun = TRUE;
		}

		/* Write the word to the output */
		strcpy(words[j], word);
		j++;
	}

	/* Mark the end of the keyword list */
	words[j][0] = 0;
}


/* Process user-provided string into keywords */

static void
parse_user_obj(name, uobj, isfood, wiz)
const char *name;
struct user_obj *uobj;
boolean isfood;
boolean wiz;
{
    char word[LONGEST_WORD];
    int i;
    int len, len2;

    /* Initialize the parsed-object structure */
    uobj->cnt         = 0;	/* bare number appearing at start */
    uobj->spe         = 0;	/* absolute value of +x or -y, or x in (x:y) */
    uobj->spesgn      = 0;	/* sign of +x or -y */
    uobj->rechrg      = 0;	/* y in (x:y) */
    uobj->blessed     = 0;	/* "bendito" */
    uobj->uncursed    = 0;	/* "inmaldito" */
    uobj->iscursed    = 0;	/* "maldito" */
    uobj->ispoisoned  = 0;	/* "envenenado" */
    uobj->isgreased   = 0;	/* "engrasado" */
    uobj->eroded      = 0;	/* "oxidado" o "quemado" */
    uobj->eroded2     = 0;	/* "quemado", "podrido", o "pudrido" */
    uobj->erodeproof  = 0;	/* "inoxidable" etc. */
#ifdef INVISIBLE_OBJECTS
    uobj->isinvisible = 0;	/* "invisible" */
#endif
    uobj->halfeaten   = 0;	/* "comido" */
    uobj->islit       = 0;	/* "(in)alumbrado" */
    uobj->unlabeled   = 0;	/* "inmarcado" */
    uobj->ishistoric  = 0;	/* "histórico" */
    uobj->isdiluted   = 0;	/* "diluido" */
    uobj->very        = 0;	/* "muy" */
    uobj->called = FALSE;	/* "llamado" */
    uobj->name  = (char *)0;	/* "nombrado" */

    uobj->num_keywords = 0;

    /* Look for keywords */
    i = 0;
    while (name[i] != 0 && uobj->num_keywords < MAX_WORDS)
    {
	/* Isolate a keyword */
	while (name[i]==' ') i++;
	if (name[i]==0) break;
	if (isalnum2(name[i])) {
	    /* A keyword that begins with a letter or a digit continues to the
	       next non-alphanumeric character, so that e.g. "varita(0:2)" is
	       correctly parsed. */
	    for (len=0; isalnum2(name[i+len]); len++) {}
	} else {
	    /* Parse such things as +2, (0:3), etc.  These continue to the next
	       whitespace character. */
	    for (len=0; name[i+len]!=' '; len++) {}
	}
	len2 = len;
	if (len2 >= LONGEST_WORD) len2 = LONGEST_WORD-1;
	memcpy(word, name+i, len2);
	word[len2] = 0;

	/* Deal with certain special cases */
	if (digit(word[0]) && uobj->num_keywords==0) {
	    uobj->cnt = atoi(word);
	} else if (!isfood && (word[0]=='+' || word[0]=='-')) {
	    uobj->spesgn = (word[0]=='+')? 1 : -1;
	    uobj->spe = abs(atoi(word));
	} else if (!isfood && word[0]=='(') {
	    char *colon;
	    /* Charges on wands and certain tools; (3) or (0:5) */
	    colon = strchr(word, ':');
	    if (colon == (char *)0) {
		uobj->rechrg = 0;
		uobj->spe    = abs(atoi(word+1));
	    } else {
		uobj->rechrg = abs(atoi(word +1));
		uobj->spe    = abs(atoi(colon+1));
	    }
	    uobj->spesgn = 1;
	/* Modifiers such as "bendito" */
	} else if (cmp_keyword(word, "llamad@")!=0) {
	    uobj->called = TRUE;
	} else if (cmp_keyword(word, "nombrad@")!=0) {
	    /* The item may be "named" anything, and we should end on seeing
	       "nombrado" */
	    uobj->name = name+i+len;
	    while (*uobj->name==' ') uobj->name++;
	    if (*uobj->name==0) uobj->name = (char *)0;
	    break;
	} else if (!strncmpi2(word, "un ", 3) ||
		   !strncmpi2(word, "uno ", 4) ||
		   !strncmpi2(word, "una ", 4)) {
	    uobj->cnt = 1;
	} else if (cmp_keyword(word, "bendit@")!=0 ||
		   cmp_keyword(word, "bendecid@")!=0) {
	    uobj->blessed = 1;
	} else if (cmp_keyword(word, "maldit@")!=0 ||
		   cmp_keyword(word, "maldecid@")!=0) {
	    uobj->iscursed = 1;
	} else if (cmp_keyword(word, "inmaldit@")!=0 ||
		   cmp_keyword(word, "inmaldecid@")!=0) {
	    uobj->uncursed = 1;
#ifdef INVISIBLE_OBJECTS
	} else if (cmp_keyword(word, "invisible")!=0) {
	    uobj->isinvisible = 1;
#endif
	} else if (!isfood &&
		  (cmp_keyword(word, "inoxidable")!=0 ||
		   cmp_keyword(word, "inerodable")!=0 || /*?*/
		   cmp_keyword(word, "incoroible")!=0 || /*?*/
		   cmp_keyword(word, "fij@")!=0 ||
		   cmp_keyword(word, "fijad@")!=0 ||
		   cmp_keyword(word, "inquemable")!=0 ||
		   cmp_keyword(word, "impodrible")!=0 ||
		   cmp_keyword(word, "impudrible")!=0)) {
	    uobj->erodeproof = 1;
	} else if (!isfood && cmp_keyword(word, "alumbrad@")!=0) {
	    uobj->islit = 1;
	} else if (!isfood && cmp_keyword(word, "inalumbrad@")!=0) {
	    uobj->islit = 0;
	} else if (!isfood && cmp_keyword(word, "inmarcad@")!=0) {
	    uobj->unlabeled = 1;
	} else if (!isfood && (cmp_keyword(word, "envenenad@")!=0
#ifdef WIZARD
	  || (wiz && cmp_keyword(word, "trampad@")!=0)
#endif
	  )) {
	    uobj->ispoisoned=1;
	} else if (cmp_keyword(word, "engrasad@")!=0) {
	    uobj->isgreased=1;
	} else if (!strncmpi2(word, "muy ", 4)) {
	    /* muy oxidado bola de hierro muy pesado */
	    uobj->very = 1;
	} else if (!isfood &&
		  (!strncmpi2(word, "completamente ", 14) ||
		   !strncmpi2(word, "enteramente ", 12))) {
	    uobj->very = 2;
	} else if (!isfood &&
		  (cmp_keyword(word, "oxidad@")!=0 ||
		   cmp_keyword(word, "quemad@")!=0)) {
	    uobj->eroded = 1 + uobj->very;
	    uobj->very = 0;
	} else if (!isfood &&
		  (cmp_keyword(word, "coroid@")!=0 ||
		   cmp_keyword(word, "podrid@")!=0 ||
		   cmp_keyword(word, "pudrid@")!=0)) {
	    uobj->eroded2 = 1 + uobj->very;
	    uobj->very = 0;
	/* FIXME:  Comido cannot be distinguished from a keyword in ración de
	   comida.  Until this is resolved, wishing for halfeaten food is not
	   supported. */
	/*} else if (cmp_keyword(word, "comid@")!=0) {
	    uobj->halfeaten = 1;*/
	} else if (!isfood &&
		  (cmp_keyword(word, "históric@")!=0 ||
		   cmp_keyword(word, "historic@")!=0)) {
	    uobj->ishistoric = 1;
	} else if (!isfood && cmp_keyword(word, "diluid@")!=0) {
	    uobj->isdiluted = 1;
	} else if (cmp_keyword(word, "par")!=0) {
	    /* This is OK:  boots, gloves, and lenses are
		   not mergeable, and ignore cnt */
	    uobj->cnt *= 2;
	} else {
	    Strcpy(uobj->keywords[uobj->num_keywords], word);
	    uobj->num_keywords++;
	}

	i += len;
    }
}


/* Compare the parsed object name to a keyword list */

static void
match_keywords(uobj, words, match)
const struct user_obj *uobj;
char words[MAX_WORDS][LONGEST_WORD];
struct obj_match *match;
{
    int i, j;

    match->num_matches = 0;
    for (i=0; words[i][0]!=0; i++)
    {
	for (j=0; j<uobj->num_keywords; j++)
	{
	    if (cmp_keyword(uobj->keywords[j], words[i]))
	    {
		match->num_matches++;
	    }
	}
    }
    match->full_match = match->num_matches==i;
}

/*
 * Return something wished for.  Specifying a null pointer for
 * the user request string results in a random object.  Otherwise,
 * if asking explicitly for "nothing" (or "nil") return no_wish;
 * if not an object return &zeroobj; if an error (no matching object),
 * return null.
 * If from_user is false, we're reading from the wizkit, nothing was typed in.
 */
struct obj *
readobjnam(bp, no_wish, from_user, wiz)
register char *bp;
struct obj *no_wish;
boolean from_user;
boolean wiz;
{
	register int i;
	register struct obj *otmp;
	int typ, mntmp, contents;
	struct user_obj uobj;
	struct obj_match match, best_match;
	int ftype = current_fruit;
	/* Fruits may not mess up the ability to wish for real objects (since
	 * you can leave a fruit in a bones file and it will be added to
	 * another person's game), so they must be checked for last, after
	 * stripping all the possible prefixes and seeing if there's a real
	 * name in there.  So we have to save the full original name.  However,
	 * it's still possible to do things like "uncursed burnt Alaska",
	 * or worse yet, "2 burned 5 course meals", so we need to loop to
	 * strip off the prefixes again, this time stripping only the ones
	 * possible on food.
	 * We could get even more detailed so as to allow food names with
	 * prefixes that _are_ possible on food, so you could wish for
	 * "2 3 alarm chilis".  Currently this isn't allowed; options.c
	 * automatically sticks 'candied' in front of such names.
	 */

	char oclass;
	char *un, *dn, *actualn;
	int match_list[NUM_OBJECTS];
	int mmatch_list[NUMMONS];
	int num_matches;

	typ = 0;
	mntmp = NON_PM;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
	contents = UNDEFINED;
	oclass = 0;
	actualn = dn = un = 0;

	/* Unlimited wishing is allowed only in wizard mode and only for the
	   ^W command; wands of wishing etc. are unaffected */
#ifdef WIZARD
	if (!wizard)
#endif
	    wiz = FALSE;

	if (!bp) bp = "";
	/* first, remove extra whitespace they may have typed */
	mungspaces(bp);
	/* allow wishing for "nada" to preserve wishless conduct...
	   [now requires "varita de nada" if that's what was really wanted] */
	if (!strcmpi2(bp, "nada") || !strcmpi2(bp, "nil") ||
	    !strcmpi2(bp, "ningún") || !strcmpi2(bp, "ninguno") ||
	    !strcmpi2(bp, "ninguna")) return no_wish;
	/* Break the object name into keywords */
	parse_user_obj(bp, &uobj, FALSE, wiz);

	/* Look for empty tin or tin of spinach */
	for (i=0; i<uobj.num_keywords; i++) {
		if (strcmpi2(uobj.keywords[i], "espinaca") == 0)
			contents = SPINACH;
		else if (cmp_keyword(uobj.keywords[i], "vaci@"))
			contents = EMPTY;
	}

	/* Determine what object is requested */
	/* Among those that have all keywords present, we use the one with the
	   most keywords, so that e.g. "gray dragon scale mail" gives that
	   item and not scale mail or gray dragon scales. */

	typ = 0;
	best_match.num_matches = 0;
	best_match.full_match = FALSE;
	num_matches = 0;
	/* Start at index 1 to skip the "strange object" */
	for (i=1; i<NUM_OBJECTS; i++) {
		char words[MAX_WORDS][LONGEST_WORD];
		boolean got_words;

		/* First check the identified name, or user name if "called" */
		got_words = FALSE;
		if (uobj.called) {
			if (objects[i].oc_uname) {
				make_called_objwords(i, words);
				got_words = TRUE;
			}
		} else {
			make_ided_objwords(i, words);
			got_words = TRUE;
		}
		if (got_words) {
			match_keywords(&uobj, words, &match);
			if (match.full_match && !best_match.full_match) {
				/* A full match trumps a partial match */
				best_match = match;
				num_matches = 1;
				match_list[0] = i;
			} else if (best_match.full_match==match.full_match &&
				   best_match.num_matches<match.num_matches) {
				/* A long match trumps a short match */
				best_match = match;
				num_matches = 1;
				match_list[0] = i;
			} else if (match.num_matches!=0 &&
				   best_match.full_match==match.full_match &&
				   best_match.num_matches==match.num_matches) {
				match_list[num_matches++] = i;
			}
		}

		/* Then check the unidentified name */
		make_unided_objwords(i, words);
		match_keywords(&uobj, words, &match);
		if (match.full_match && !best_match.full_match) {
			/* A full match trumps a partial match */
			best_match = match;
			num_matches = 1;
			match_list[0] = i;
		} else if (best_match.full_match==match.full_match &&
			   best_match.num_matches<match.num_matches) {
			/* A long match trumps a short match */
			best_match = match;
			num_matches = 1;
			match_list[0] = i;
		} else if (match.num_matches!=0 &&
			   best_match.full_match==match.full_match &&
			   best_match.num_matches==match.num_matches) {
			/* This match is as good as the last one */
			if (num_matches==0 || match_list[num_matches-1]!=i)
				match_list[num_matches++] = i;
		}
	}

	if (num_matches != 0) {
		/* num_matches give the number of potential matches, and
		   match_list gives the matching indices.  Randomly select
		   from the potential matches. */
		typ = match_list[rn2(num_matches)];
	}

	if (typ==0 && contents==SPINACH) {
		/* A wish for "espinaca", without qualification, should yield
		   a tin of spinach */
		typ = TIN;
	}

	/* Items dependent on a monster type */
	switch (typ) {
	case TIN:
	case FIGURINE:
	case EGG:
	case CORPSE:
	case STATUE:
		mntmp = 0;
		best_match.num_matches = 0;
		best_match.full_match = FALSE;
		num_matches = 0;
		for (i=0; i<NUMMONS; i++) {
			char words[MAX_WORDS][LONGEST_WORD];

			gen_keywords(words, (char *)0, mons[i].mname);
			match_keywords(&uobj, words, &match);
			if (match.full_match && !best_match.full_match) {
				/* A full match trumps a partial match */
				best_match = match;
				num_matches = 1;
				mmatch_list[0] = i;
			} else if (best_match.full_match==match.full_match &&
				   best_match.num_matches<match.num_matches) {
				/* A long match trumps a short match */
				best_match = match;
				num_matches = 1;
				mmatch_list[0] = i;
			} else if (match.num_matches!=0 &&
				   best_match.full_match==match.full_match &&
				   best_match.num_matches==match.num_matches) {
				/* This match is as good as the last one */
				mmatch_list[num_matches++] = i;
			}
		}
		if (num_matches != 0)
			mntmp = mmatch_list[rn2(num_matches)];
		break;
	}

	if (typ == 0 || !best_match.full_match) {
	    short objtyp;
	    const char *name;

	    /* Perhaps it's an artifact specified by name, not type */
	    /* We try this on partial matches because sometimes a word in an
	       artifact name matches a word in an ordinary object name
	       (Hoja Vorpal vs. hoja de eucalipto is one) */
	    name = artifact_name(bp, &objtyp);
	    if(name) {
		typ = objtyp;
		uobj.name = name;
		best_match.full_match = TRUE;	/* don't try a fruit */
	    }
	}

	if (typ == 0 || !best_match.full_match) {
		/* Try a fruit */
		struct fruit *f, *f2;
		struct user_obj uobjf;

		parse_user_obj(bp, &uobjf, TRUE, wiz);
		f2 = (struct fruit *)0;
		best_match.num_matches = 0;
		best_match.full_match = FALSE;
		num_matches = 0;
		for (f=ffruit; f; f=f->nextf) {
			char words[MAX_WORDS][LONGEST_WORD];

			gen_keywords(words, (char *)0, f->fname);
			match_keywords(&uobjf, words, &match);
			if (match.full_match && !best_match.full_match) {
				/* A full match trumps a partial match */
				best_match = match;
				f2 = f;
			} else if (best_match.full_match==match.full_match &&
				   best_match.num_matches<match.num_matches) {
				/* A long match trumps a short match */
				best_match = match;
				num_matches = 1;
				f2 = f;
			}
		}
		if (f2) {
			typ = SLIME_MOLD;
			uobj = uobjf;
			ftype = f2->fid;
		}
	}

	/* Translator's Note:  From here, the code is mostly the same as the
	   English version, save for a translated message near the end and
	   insertion of uobj. in front of certain flags */
	/* Further note: the wiz flag is added so that the use of a wand of
	   wishing is unaffected in wizard mode.  This allows debugging of the
	   wishing code itself by wishing for a wand of wishing and then
	   zapping it. */

	/* check for some objects that are not allowed */
	if (typ && objects[typ].oc_unique) {
#ifdef WIZARD
	    if (wiz)
		;	/* allow unique objects */
	    else
#endif
	    switch (typ) {
		case AMULET_OF_YENDOR:
		    typ = FAKE_AMULET_OF_YENDOR;
		    break;
		case CANDELABRUM_OF_INVOCATION:
		    typ = rnd_class(TALLOW_CANDLE, WAX_CANDLE);
		    break;
		case BELL_OF_OPENING:
		    typ = BELL;
		    break;
		case SPE_BOOK_OF_THE_DEAD:
		    typ = SPE_BLANK_PAPER;
		    break;
	    }
	}

	/* catch any other non-wishable objects */
	if (objects[typ].oc_nowish
#ifdef WIZARD
	    && !wiz
#endif
	    )
	    return((struct obj *)0);

	/* convert magic lamps to regular lamps before lighting them or setting
	   the charges */
	if (typ == MAGIC_LAMP
#ifdef WIZARD
				&& !wiz
#endif
						)
	    typ = OIL_LAMP;

	if(typ) {
		otmp = mksobj(typ, TRUE, FALSE);
	} else {
		/* FIXME:  Haven't added the wish-for-class code yet */
		otmp = mkobj(oclass, FALSE);
		if (otmp) typ = otmp->otyp;
	}
	oclass = otmp->oclass;

	if (uobj.islit &&
		(typ == OIL_LAMP || typ == MAGIC_LAMP || typ == BRASS_LANTERN ||
		 Is_candle(otmp) || typ == POT_OIL)) {
	    place_object(otmp, u.ux, u.uy);  /* make it viable light source */
	    begin_burn(otmp, FALSE);
	    obj_extract_self(otmp);	 /* now release it for caller's use */
	}

	if(uobj.cnt > 0 && objects[typ].oc_merge && oclass != SPBOOK_CLASS &&
		(uobj.cnt < rnd(6) ||
#ifdef WIZARD
		wiz ||
#endif
		 (uobj.cnt <= 7 && Is_candle(otmp)) ||
		 (uobj.cnt <= 20 &&
		  ((oclass == WEAPON_CLASS && is_ammo(otmp))
				|| typ == ROCK || is_missile(otmp)))))
			otmp->quan = (long) uobj.cnt;

#ifdef WIZARD
	if (oclass == VENOM_CLASS) otmp->spe = 1;
#endif

	if (uobj.spesgn == 0) uobj.spe = otmp->spe;
#ifdef WIZARD
	else if (wiz) /* no alteration to uobj.spe */ ;
#endif
	else if (oclass == ARMOR_CLASS || oclass == WEAPON_CLASS ||
		 is_weptool(otmp) ||
			(oclass==RING_CLASS && objects[typ].oc_charged)) {
		if(uobj.spe > rnd(5) && uobj.spe > otmp->spe) uobj.spe = 0;
		if(uobj.spe > 2 && Luck < 0) uobj.spesgn = -1;
	} else {
		if (oclass == WAND_CLASS) {
			if (uobj.spe > 1 && uobj.spesgn == -1) uobj.spe = 1;
		} else {
			if (uobj.spe > 0 && uobj.spesgn == -1) uobj.spe = 0;
		}
		if (uobj.spe > otmp->spe) uobj.spe = otmp->spe;
	}

	if (uobj.spesgn == -1) uobj.spe = -uobj.spe;

	/* set otmp->spe.  This may, or may not, use uobj.spe... */
	switch (typ) {
		case TIN: if (contents==EMPTY) {
				otmp->corpsenm = NON_PM;
				otmp->spe = 0;
			} else if (contents==SPINACH) {
				otmp->corpsenm = NON_PM;
				otmp->spe = 1;
			}
			break;
		case SLIME_MOLD: otmp->spe = ftype;
			/* Fall through */
		case SKELETON_KEY: case CHEST: case LARGE_BOX:
		case HEAVY_IRON_BALL: case IRON_CHAIN: case STATUE:
			/* otmp->cobj already done in mksobj() */
				break;
#ifdef MAIL
		case SCR_MAIL: otmp->spe = 1; break;
#endif
		case WAN_WISHING:
#ifdef WIZARD
			if (!wiz) {
#endif
				otmp->spe = (rn2(10) ? -1 : 0);
				break;
#ifdef WIZARD
			}
			/* fall through, if wizard */
#endif
		default: otmp->spe = uobj.spe;
	}

	/* set otmp->corpsenm or dragon scale [mail] */
	if (mntmp >= LOW_PM) {
		if (mntmp == PM_LONG_WORM_TAIL) mntmp = PM_LONG_WORM;

		switch (typ) {
		case TIN:
			otmp->spe = 0; /* No spinach */
			if (dead_species(mntmp, FALSE)) {
			    otmp->corpsenm = NON_PM;	/* it's empty */
			} else if (!(mons[mntmp].geno & G_UNIQ) &&
				   !(mvitals[mntmp].mvflags & G_NOCORPSE) &&
				   mons[mntmp].cnutrit != 0) {
			    otmp->corpsenm = mntmp;
			}
			break;
		case CORPSE:
			if (!(mons[mntmp].geno & G_UNIQ) &&
				   !(mvitals[mntmp].mvflags & G_NOCORPSE)) {
			    /* beware of random troll or lizard corpse,
			       or of ordinary one being forced to such */
			    if (otmp->timed) obj_stop_timers(otmp);
			    if (mons[mntmp].msound == MS_GUARDIAN)
			    	otmp->corpsenm = genus(mntmp,1);
			    else
				otmp->corpsenm = mntmp;
			    start_corpse_timeout(otmp);
			}
			break;
		case FIGURINE:
			if (!(mons[mntmp].geno & G_UNIQ)
			    && !is_human(&mons[mntmp])
#ifdef MAIL
			    && mntmp != PM_MAIL_DAEMON
#endif
							)
				otmp->corpsenm = mntmp;
			break;
		case EGG:
			mntmp = can_be_hatched(mntmp);
			if (mntmp != NON_PM) {
			    otmp->corpsenm = mntmp;
			    if (!dead_species(mntmp, TRUE))
				attach_egg_hatch_timeout(otmp);
			    else
				kill_egg(otmp);
			}
			break;
		case STATUE: otmp->corpsenm = mntmp;
			if (Has_contents(otmp) && verysmall(&mons[mntmp]))
			    delete_contents(otmp);	/* no spellbook */
			otmp->spe = uobj.ishistoric ? STATUE_HISTORIC : 0;
			break;
		case SCALE_MAIL:
			/* Dragon mail - depends on the order of objects */
			/*		 & dragons.			 */
			if (mntmp >= PM_GRAY_DRAGON &&
						mntmp <= PM_YELLOW_DRAGON)
			    otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
						    mntmp - PM_GRAY_DRAGON;
			break;
		}
	}

	/* set blessed/cursed -- setting the fields directly is safe
	 * since weight() is called below and addinv() will take care
	 * of luck */
	if (uobj.iscursed) {
		curse(otmp);
	} else if (uobj.uncursed) {
		otmp->blessed = 0;
		otmp->cursed = (Luck < 0
#ifdef WIZARD
					 && !wiz
#endif
							);
	} else if (uobj.blessed) {
		otmp->blessed = (Luck >= 0
#ifdef WIZARD
					 || wiz
#endif
							);
		otmp->cursed = (Luck < 0
#ifdef WIZARD
					 && !wiz
#endif
							);
	} else if (uobj.spesgn < 0) {
		curse(otmp);
	}

#ifdef INVISIBLE_OBJECTS
	if (uobj.isinvisible) otmp->oinvis = 1;
#endif

	/* set uobj.eroded */
	if (is_damageable(otmp) || otmp->otyp == CRYSKNIFE) {
	    if (uobj.eroded && (is_flammable(otmp) || is_rustprone(otmp)))
		    otmp->oeroded = uobj.eroded;
	    if (uobj.eroded2 && (is_corrodeable(otmp) || is_rottable(otmp)))
		    otmp->oeroded2 = uobj.eroded2;

	    /* set uobj.erodeproof */
	    if (uobj.erodeproof && !uobj.eroded && !uobj.eroded2)
		    otmp->oerodeproof = (Luck >= 0
#ifdef WIZARD
					     || wiz
#endif
					);
	}

	/* set otmp->recharged */
	if (oclass == WAND_CLASS) {
	    /* prevent wishing abuse */
	    if (otmp->otyp == WAN_WISHING
#ifdef WIZARD
		    && !wiz
#endif
		) uobj.rechrg = 1;
	    otmp->recharged = (unsigned)uobj.rechrg;
	}

	/* set poisoned */
	if (uobj.ispoisoned) {
	    if (is_poisonable(otmp))
		otmp->opoisoned = (Luck >= 0);
	    else if (Is_box(otmp) || typ == TIN)
		otmp->otrapped = 1;
	    else if (oclass == FOOD_CLASS)
		/* try to taint by making it as old as possible */
		otmp->age = 1L;
	}

	if (uobj.isgreased) otmp->greased = 1;

	if (uobj.isdiluted && otmp->oclass == POTION_CLASS &&
			otmp->otyp != POT_WATER)
		otmp->odiluted = 1;

	if (uobj.name) {
		const char *aname;
		short objtyp;

		/* an artifact name might need capitalization fixing */
		aname = artifact_name(uobj.name, &objtyp);
		if (aname && objtyp == otmp->otyp) uobj.name = aname;

		otmp = oname(otmp, uobj.name);
		if (otmp->oartifact) {
			otmp->quan = 1L;
			u.uconduct.wisharti++;	/* KMH, conduct */
		}
	}

	/* more wishing abuse: don't allow wishing for certain artifacts */
	/* and make them pay; charge them for the wish anyway! */
	if ((is_quest_artifact(otmp) ||
	     (otmp->oartifact && rn2(nartifact_exist()) > 1))
#ifdef WIZARD
	    && !wiz
#endif
	    ) {
	    artifact_exists(otmp, ONAME(otmp), FALSE);
	    obfree(otmp, (struct obj *) 0);
	    otmp = &zeroobj;
	    pline("¡Por un momento, sientes %s en tus %s, pero se desaparece!",
		  something,
		  makeplural(body_part(HAND)));
	}

	if (uobj.halfeaten && otmp->oclass == FOOD_CLASS) {
		if (otmp->otyp == CORPSE)
			otmp->oeaten = mons[otmp->corpsenm].cnutrit;
		else otmp->oeaten = objects[otmp->otyp].oc_nutrition;
		/* (do this adjustment before setting up object's weight) */
		consume_oeaten(otmp, 1);
	}
	otmp->owt = weight(otmp);
	if (uobj.very && otmp->otyp == HEAVY_IRON_BALL) otmp->owt += 160;

	return(otmp);
}

int
rnd_class(first,last)
int first,last;
{
	int i, x, sum=0;

	if (first == last)
	    return (first);
	for(i=first; i<=last; i++)
		sum += objects[i].oc_prob;
	if (!sum) /* all zero */
		return first + rn2(last-first+1);
	x = rnd(sum);
	for(i=first; i<=last; i++)
		if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
			return i;
	return 0;
}

STATIC_OVL const char *
Japanese_item_name(i)
int i;
{
	struct Jitem *j = Japanese_items;

	while(j->item) {
		if (i == j->item)
			return j->name;
		j++;
	}
	return (const char *)0;
}

boolean
isfeminine(str)
const char *str;
{
	const struct {
		const char *word;
		int isfem;	/* TRUE, FALSE, or -1 if we don't know */
	} wordlist[] = {
		/* Words whose genders are not determinable by their endings
		   go here */
		{ "el",      -1 },   /* -1 because "el agua" is feminine */
		{ "cavernícola", -1 },
		{ "dragón",      -1 },
		{ "hombre",  FALSE },
		{ "rey",     FALSE },
		{ "cadáver", FALSE },
		{ "altar",   FALSE },
		{ "gólem",   FALSE },
		{ "mano",    TRUE },
		{ "mujer",   TRUE },
		{ "fuente",  TRUE },
		{ "puente",  FALSE },
		{ "oráculo", TRUE },
		{ "nube",    TRUE },
		{ "llave",   TRUE },
		{ "gofre",   TRUE },
		{ "serpiente", TRUE },
		{ "escorpión", FALSE },
		{ "arma", TRUE },
		{ "bisarma", TRUE },
		{ "alma", TRUE },
		{ "forma", TRUE },
		{ "gema", TRUE },
		/* Not a real word, but formed from abrelatas by spurious
		   singularization */
		{ "abrelata", FALSE },
		{ "sable", FALSE },
		{ "abad", FALSE },
		{ "par", FALSE },
		{ "pez", FALSE },
		{ "guante", FALSE },
		/* Array terminator */
		{ (char *)0, 0 }
	};
	char *sing;
	unsigned len, len2;
	unsigned i;
	int fem;

	/* isfeminine() should ignore "par de", so that "par de botas" gets
	   "malditas", for example, rather than "maldito".  the() and an()
	   recognize "par de" and give it a masculine article. */
	if (strncmp(str, "par de ", 7) == 0)
		str += 7;
	else if (strncmp(str, "un par de ", 10) == 0)
		str += 10;

	while (*str != 0) {
		/* Find the word */
		while (*str!=0 && !isalpha2(*str)) str++;
		if (*str==0) continue;

		/* Find the length of the word */
		for (len=0; isalpha2(str[len]); len++) {}

		/* Get the singular */
		sing = singularize_one(str, len);
		len2 = strlen(sing);

		/* Check the word list */
		fem = -1;
		for (i=0; wordlist[i].word!=(char *)0; i++) {
			if (strcmpi2(wordlist[i].word, sing) == 0) {
				fem = wordlist[i].isfem;
				break;
			}
		}
		if (wordlist[i].word != (char *)0) {
			/* The word was on the list */
			if (fem != -1) return fem;
		} else {
			/* Words in -ista are androgynous */
			if (len2>=4 && strcmpi2(sing+len2-4, "ista")==0)
				goto next_word;

			/* Endings that determine genders */
			/* These should appear in descending order by length,
			   so that e.g. -ma is checked before -a */
			if (len2>=4 && strcmpi2(sing+len2-1, "mbre")==0)
				return TRUE;
			if (len2>=3 && strcmpi2(sing+len2-3, "aje")==0)
				return FALSE;
			if (len2>=3 && strcmpi2(sing+len2-3, "dor")==0)
				return FALSE;
			if (len2>=3 && strcmpi2(sing+len2-3, "ión")==0)
				return TRUE;
			if (len2>=2 && strcmpi2(sing+len2-2, "ma")==0)
				return FALSE;
			if (len2>=2 && strcmpi2(sing+len2-2, "ón")==0)
				return FALSE;
			if (len2>=1 && strcmpi2(sing+len2-1, "o")==0)
				return FALSE;
			if (len2>=1 && strcmpi2(sing+len2-1, "a")==0)
				return TRUE;
			if (len2>=1 && strcmpi2(sing+len2-1, "z")==0)
				return TRUE;
			if (len2>=1 && strcmpi2(sing+len2-1, "d")==0)
				return TRUE;
		}
		/* Else try another word */
		next_word:
		str += len;
	}
	/* Give up and say it's masculine */
	return FALSE;
}

boolean
isplural(str)
const char *str;
{
	const struct {
		const char *word;
		int isplu;	/* TRUE, FALSE, or -1 if we don't know */
	} wordlist[] = {
		/* Words whose numbers are not determinable by their endings
		   go here */
		/* Plurals not in -s (mostly if not all foreign) */
		{ "mumakil",   TRUE  },

		/* Singulars in -s */
		{ "aklys",      FALSE },
		{ "matalobos",  FALSE },
		{ "abrelatas",  FALSE },
		{ "lembas",     FALSE },
		{ "erinias",    FALSE },
		{ "Hipócrates", FALSE },
		{ "Pelias",     FALSE },
		{ "parálisis",  FALSE },

		/* Words that do not change when plural */
		/* Uruk-hai and Olog-hai are plural in Tolkien (the singulars
		   are Uruk and Olog) but the error of using the -hai forms
		   as singular exists in the English version, and is not
		   changed here */
		{ "manes",     -1    },
		{ "gris",      -1    },
		{ "Uruk-hai",  -1    },
		{ "Olog-hai",  -1    },

		/* Array terminator */
		{ (char *)0, 0 }
	};
	unsigned len;
	unsigned i;
	int plu;

	while (*str != 0) {
		/* Find the word */
		while (*str!=0 && !isalpha2(*str)) str++;
		if (*str==0) continue;

		/* Find the length of the word */
		for (len=0; isalpha2(str[len]); len++) {}

		/* Check the word list */
		plu = -1;
		for (i=0; wordlist[i].word!=(char *)0; i++) {
			if (len == strlen(wordlist[i].word)
			&&  strncmpi2(wordlist[i].word, str, len) == 0) {
				plu = wordlist[i].isplu;
				break;
			}
		}
		if (wordlist[i].word != (char *)0) {
			/* The word was on the list */
			if (plu != -1) return plu;
		} else {
			/* Endings that determine numbers */
			if (len>=2
			&&  str[len-1]=='s'
			&&  strchr("aeiou", str[len-2])!=(char *)0)
				return TRUE;
			return FALSE;
		}
		/* Else try another word */
		str += len;
	}
	/* Give up and say it's singular */
	return FALSE;
}

const char *
cloak_simple_name(cloak)
struct obj *cloak;
{
    if (cloak) {
	switch (cloak->otyp) {
	case ROBE:
	    return "manto";
	case MUMMY_WRAPPING:
	    return "abrigo";
	case ALCHEMY_SMOCK:
	    return (objects[cloak->otyp].oc_name_known &&
			cloak->dknown) ? "batín" : "delantal";
	default:
	    break;
	}
    }
    return "capa";
}

const char *
mimic_obj_name(mtmp)
struct monst *mtmp;
{
	if (mtmp->m_ap_type == M_AP_OBJECT && mtmp->mappearance != STRANGE_OBJECT) {
		int idx = objects[mtmp->mappearance].oc_descr_idx;
		if (mtmp->mappearance == GOLD_PIECE) return "oro";
		return obj_descr[idx].oc_name;
	}
	return "lo-que-se-llame";
}
#endif /* OVLB */

/*objnam.c*/
