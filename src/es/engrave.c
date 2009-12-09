/*	SCCS Id: @(#)engrave.c	3.4	2001/11/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include <ctype.h>

STATIC_VAR NEARDATA struct engr *head_engr;

#ifdef OVLB
/* random engravings */
static const char *random_mesg[] = {
	"Elbereth",
	/* trap engravings */
	"Vlad estuvo aquí", "ad aerarium",
	/* take-offs and other famous engravings */
	"Owlbreath", "Galadriel",
	"Kilroy estaba aquí",
	"A.S. ->", "<- A.S.", /* Journey to the Center of the Earth */
	"No podrás subirlo por la escalera", /* Adventure */
	"Lasciate ogni speranza o voi ch'entrate.", /* Inferno */
	"Bien Venido", /* Prisoner */
	"Disculpe el contratiempo.", /* So Long... */
	"Hasta el próximo miércoles", /* Thriller */
	"notary sojak", /* Smokey Stover */
	"Para pasarla bien llama a 8?7-5309", /* A once-popular song */
	"Favor de no alimentar los animales.", /* Various zoos around the world */
	"Madam, in Eden, I'm Adam.", /* A palindrome */
	"Es Adán, ya ve yo soy Eva y nada sé." /* Y otro palindromo */
	"¡Dos pulgadas arriba!", /* Siskel & Ebert */
	"¡Hola, mundo!", /* The First C Program */
#ifdef MAIL
	"¡Tienes correo!", /* AOL */
#endif
	"¡Como si!", /* Clueless */
};

char *
random_engraving(outbuf)
char *outbuf;
{
	const char *rumor;

	/* a random engraving may come from the "rumors" file,
	   or from the list above */
	if (!rn2(4) || !(rumor = getrumor(0, outbuf, TRUE)) || !*rumor)
	    Strcpy(outbuf, random_mesg[rn2(SIZE(random_mesg))]);

	wipeout_text(outbuf, (int)(strlen(outbuf) / 4), 0);
	return outbuf;
}

/* Partial rubouts for engraving characters. -3. */
/* LENGUA:  Also deal with diacritics */
static const struct {
	char		wipefrom;
	const char *	wipeto;
} rubouts[] = {
	{'A', "^"},     {'B', "Pb["},   {'C', "("},     {'D', "|)["},
	{'E', "|FL[_"}, {'F', "|-"},    {'G', "C("},    {'H', "|-"},
	{'I', "|"},     {'K', "|<"},    {'L', "|_"},    {'M', "|"},
	{'N', "|\\"},   {'O', "C("},    {'P', "F"},     {'Q', "C("},
	{'R', "PF"},    {'T', "|"},     {'U', "J"},     {'V', "/\\"},
	{'W', "V/\\"},  {'Z', "/"},
	{'b', "|"},     {'d', "c|"},    {'e', "c"},     {'g', "c"},
	{'h', "n"},     {'j', "i"},     {'k', "|"},     {'l', "|"},
	{'m', "nr"},    {'n', "r"},     {'o', "c"},     {'q', "c"},
	{'w', "v"},     {'y', "v"},
	{':', "."},     {';', ","},
	{'0', "C("},    {'1', "|"},     {'6', "o"},     {'7', "/"},
	{'8', "3o"}
};

void
wipeout_text(engr, cnt, seed)
char *engr;
int cnt;
unsigned seed;		/* for semi-controlled randomization */
{
	char *s;
	int i, j, nxt, use_rubout, lth = (int)strlen(engr);

	if (lth && cnt > 0) {
	    while (cnt--) {
		/* pick next character */
		if (!seed) {
		    /* random */
		    nxt = rn2(lth);
		    use_rubout = rn2(4);
		} else {
		    /* predictable; caller can reproduce the same sequence by
		       supplying the same arguments later, or a pseudo-random
		       sequence by varying any of them */
		    nxt = seed % lth;
		    seed *= 31,  seed %= (BUFSZ-1);
		    use_rubout = seed & 3;
		}
		s = &engr[nxt];
		if (*s == ' ') continue;

		/* rub out unreadable & small punctuation marks */
		if (index("?.,'`-|_", *s)) {
		    *s = ' ';
		    continue;
		}

		if (!use_rubout)
		    i = SIZE(rubouts);
		else
		    for (i = 0; i < SIZE(rubouts); i++)
			if (*s == rubouts[i].wipefrom) {
			    /*
			     * Pick one of the substitutes at random.
			     */
			    if (!seed)
				j = rn2(strlen(rubouts[i].wipeto));
			    else {
				seed *= 31,  seed %= (BUFSZ-1);
				j = seed % (strlen(rubouts[i].wipeto));
			    }
			    *s = rubouts[i].wipeto[j];
			    break;
			}

		/* didn't pick rubout; use '?' for unreadable character */
		if (i == SIZE(rubouts)) *s = '?';
	    }
	}

	/* trim trailing spaces */
	while (lth && engr[lth-1] == ' ') engr[--lth] = 0;
}

boolean
can_reach_floor()
{
	return (boolean)(!u.uswallow &&
#ifdef STEED
			/* Restricted/unskilled riders can't reach the floor */
			!(u.usteed && P_SKILL(P_RIDING) < P_BASIC) &&
#endif
			 (!Levitation ||
			  Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)));
}
#endif /* OVLB */
#ifdef OVL0

const char *
surface(x, y)
register int x, y;
{
	register struct rm *lev = &levl[x][y];

	if ((x == u.ux) && (y == u.uy) && u.uswallow &&
		is_animal(u.ustuck->data))
	    return "el buche";
	else if (IS_AIR(lev->typ) && Is_airlevel(&u.uz))
	    return "el aire";
	else if (is_pool(x,y))
	    return (Underwater && !Is_waterlevel(&u.uz)) ? "el fondo" : "el agua";
	else if (is_ice(x,y))
	    return "el hielo";
	else if (is_lava(x,y))
	    return "la lava";
	else if (lev->typ == DRAWBRIDGE_DOWN)
	    return "el puente";
	else if(IS_ALTAR(levl[x][y].typ))
	    return "el altar";
	else if(IS_GRAVE(levl[x][y].typ))
	    return "la lápida";
	else if(IS_FOUNTAIN(levl[x][y].typ))
	    return "la fuente";
	else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz)) ||
		 IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
	    return "el piso";
	else
	    return "el suelo";
}

const char *
ceiling(x, y)
register int x, y;
{
	register struct rm *lev = &levl[x][y];
	const char *what;

	/* other room types will no longer exist when we're interested --
	 * see check_special_room()
	 */
	if (*in_rooms(x,y,VAULT))
	    what = "el techo de la bóveda";
	else if (*in_rooms(x,y,TEMPLE))
	    what = "el techo del templo";
	else if (*in_rooms(x,y,SHOPBASE))
	    what = "el techo de la tienda";
	else if (IS_AIR(lev->typ))
	    what = "el cielo";
	else if (Underwater)
	    what = "la superficie del agua";
	else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz)) ||
		 IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
	    what = "el techo";
	else
	    what = "la piedra de encima";

	return what;
}

struct engr *
engr_at(x, y)
xchar x, y;
{
	register struct engr *ep = head_engr;

	while(ep) {
		if(x == ep->engr_x && y == ep->engr_y)
			return(ep);
		ep = ep->nxt_engr;
	}
	return((struct engr *) 0);
}

#ifdef ELBERETH
/* Decide whether a particular string is engraved at a specified
 * location; a case-insensitive substring match used.
 * Ignore headstones, in case the player names herself "Elbereth".
 */
int
sengr_at(s, x, y)
	const char *s;
	xchar x, y;
{
	register struct engr *ep = engr_at(x,y);

	return (ep && ep->engr_type != HEADSTONE &&
		ep->engr_time <= moves && strstri(ep->engr_txt, s) != 0);
}
#endif /* ELBERETH */

#endif /* OVL0 */
#ifdef OVL2

void
u_wipe_engr(cnt)
register int cnt;
{
	if (can_reach_floor())
		wipe_engr_at(u.ux, u.uy, cnt);
}

#endif /* OVL2 */
#ifdef OVL1

void
wipe_engr_at(x,y,cnt)
register xchar x,y,cnt;
{
	register struct engr *ep = engr_at(x,y);

	/* Headstones are indelible */
	if(ep && ep->engr_type != HEADSTONE){
	    if(ep->engr_type != BURN || is_ice(x,y)) {
		if(ep->engr_type != DUST && ep->engr_type != ENGR_BLOOD) {
			cnt = rn2(1 + 50/(cnt+1)) ? 0 : 1;
		}
		wipeout_text(ep->engr_txt, (int)cnt, 0);
		while(ep->engr_txt[0] == ' ')
			ep->engr_txt++;
		if(!ep->engr_txt[0]) del_engr(ep);
	    }
	}
}

#endif /* OVL1 */
#ifdef OVL2

void
read_engr_at(x,y)
register int x,y;
{
	register struct engr *ep = engr_at(x,y);
	register int	sensed = 0;
	char buf[BUFSZ];
	
	/* Sensing an engraving does not require sight,
	 * nor does it necessarily imply comprehension (literacy).
	 */
	if(ep && ep->engr_txt[0]) {
	    switch(ep->engr_type) {
	    case DUST:
		if(!Blind) {
			sensed = 1;
			Hay("%s escrito aquí en el %s.", something,
				is_ice(x,y) ? "hielo" : "polvo");
		}
		break;
	    case ENGRAVE:
	    case HEADSTONE:
		if (!Blind || can_reach_floor()) {
			sensed = 1;
			Hay("%s grabado aquí en %s.",
				something,
				surface(x,y));
		}
		break;
	    case BURN:
		if (!Blind || can_reach_floor()) {
			sensed = 1;
			pline("Algo de texto ha sido %s aquí en %s.",
				is_ice(x,y) ? "fundido" : "grabado a fuego",
				surface(x,y));
		}
		break;
	    case MARK:
		if(!Blind) {
			sensed = 1;
			Hay("algunos graffitis aquí en %s.",
				surface(x,y));
		}
		break;
	    case ENGR_BLOOD:
		/* "It's a message!  Scrawled in blood!"
		 * "What's it say?"
		 * "It says... `See you next Wednesday.'" -- Thriller
		 */
		if(!Blind) {
			sensed = 1;
			pline("Ves aquí un mensaje escrito en sangre.");
		}
		break;
	    default:
		impossible("%s está escrito de un modo muy extraño.",
				Something);
		sensed = 1;
	    }
	    if (sensed) {
	    	char *et;
	    	unsigned maxelen = BUFSZ - sizeof("Sientes las palabras: \"\". ");
	    	if (strlen(ep->engr_txt) > maxelen) {
	    		(void) strncpy(buf,  ep->engr_txt, (int)maxelen);
			buf[maxelen] = '\0';
			et = buf;
		} else
			et = ep->engr_txt;
		pline("%s: \"%s\".",
		      (Blind) ? "Sientes las palabras" : "Lees",  et);
		if(flags.run > 1) nomul(0);
	    }
	}
}

#endif /* OVL2 */
#ifdef OVLB

void
make_engr_at(x,y,s,e_time,e_type)
register int x,y;
register const char *s;
register long e_time;
register xchar e_type;
{
	register struct engr *ep;

	if ((ep = engr_at(x,y)) != 0)
	    del_engr(ep);
	ep = newengr(strlen(s) + 1);
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = x;
	ep->engr_y = y;
	ep->engr_txt = (char *)(ep + 1);
	Strcpy(ep->engr_txt, s);
	/* engraving Elbereth shows wisdom */
	if (!in_mklev && !strcmp(s, "Elbereth")) exercise(A_WIS, TRUE);
	ep->engr_time = e_time;
	ep->engr_type = e_type > 0 ? e_type : rnd(N_ENGRAVE-1);
	ep->engr_lth = strlen(s) + 1;
}

/* delete any engraving at location <x,y> */
void
del_engr_at(x, y)
int x, y;
{
	register struct engr *ep = engr_at(x, y);

	if (ep) del_engr(ep);
}

/*
 *	freehand - returns true if player has a free hand
 */
int
freehand()
{
	return(!uwep || !welded(uwep) ||
	   (!bimanual(uwep) && (!uarms || !uarms->cursed)));
/*	if ((uwep && bimanual(uwep)) ||
	    (uwep && uarms))
		return(0);
	else
		return(1);*/
}

static NEARDATA const char styluses[] =
	{ ALL_CLASSES, ALLOW_NONE, TOOL_CLASS, WEAPON_CLASS, WAND_CLASS,
	  GEM_CLASS, RING_CLASS, 0 };

/* Mohs' Hardness Scale:
 *  1 - Talc		 6 - Orthoclase
 *  2 - Gypsum		 7 - Quartz
 *  3 - Calcite		 8 - Topaz
 *  4 - Fluorite	 9 - Corundum
 *  5 - Apatite		10 - Diamond
 *
 * Since granite is a igneous rock hardness ~ 7, anything >= 8 should
 * probably be able to scratch the rock.
 * Devaluation of less hard gems is not easily possible because obj struct
 * does not contain individual oc_cost currently. 7/91
 *
 * steel     -	5-8.5	(usu. weapon)
 * diamond    - 10			* jade	     -	5-6	 (nephrite)
 * ruby       -  9	(corundum)	* turquoise  -	5-6
 * sapphire   -  9	(corundum)	* opal	     -	5-6
 * topaz      -  8			* glass      - ~5.5
 * emerald    -  7.5-8	(beryl)		* dilithium  -	4-5??
 * aquamarine -  7.5-8	(beryl)		* iron	     -	4-5
 * garnet     -  7.25	(var. 6.5-8)	* fluorite   -	4
 * agate      -  7	(quartz)	* brass      -	3-4
 * amethyst   -  7	(quartz)	* gold	     -	2.5-3
 * jasper     -  7	(quartz)	* silver     -	2.5-3
 * onyx       -  7	(quartz)	* copper     -	2.5-3
 * moonstone  -  6	(orthoclase)	* amber      -	2-2.5
 */

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
doengrave()
{
	boolean dengr = FALSE;	/* TRUE if we wipe out the current engraving */
	boolean doblind = FALSE;/* TRUE if engraving blinds the player */
	boolean doknown = FALSE;/* TRUE if we identify the stylus */
	boolean eow = FALSE;	/* TRUE if we are overwriting oep */
	boolean jello = FALSE;	/* TRUE if we are engraving in slime */
	boolean ptext = TRUE;	/* TRUE if we must prompt for engrave text */
	boolean teleengr =FALSE;/* TRUE if we move the old engraving */
	boolean zapwand = FALSE;/* TRUE if we remove a wand charge */
	xchar type = DUST;	/* Type of engraving made */
	char buf[BUFSZ];	/* Buffer for final/poly engraving text */
	char ebuf[BUFSZ];	/* Buffer for initial engraving text */
	char qbuf[QBUFSZ];	/* Buffer for query text */
	char post_engr_text[BUFSZ]; /* Text displayed after engraving prompt */
	const char *everb;	/* Present tense of engraving type */
	const char *everb_inf;	/* Infinitive of engraving type */
	const char *everb_what;	/* The thing affected by engraving */
	const char *eloc;	/* Where the engraving is (ie dust/floor/...) */
	char *sp;		/* Place holder for space count of engr text */
	int len;		/* # of nonspace chars of new engraving text */
	int maxelen;		/* Max allowable length of engraving text */
	struct engr *oep = engr_at(u.ux,u.uy);
				/* The current engraving */
	struct obj *otmp;	/* Object selected with which to engrave */
	char *writer;

	multi = 0;		/* moves consumed */
	nomovemsg = (char *)0;	/* occupation end message */

	buf[0] = (char)0;
	ebuf[0] = (char)0;
	post_engr_text[0] = (char)0;
	maxelen = BUFSZ - 1;
	if (is_demon(youmonst.data) || youmonst.data->mlet == S_VAMPIRE)
	    type = ENGR_BLOOD;

	/* Can the adventurer engrave at all? */

	if(u.uswallow) {
		if (is_animal(u.ustuck->data)) {
			pline("¿Qué escribirías?  ¿\"Jonás estuvo aquí\"?");
			return(0);
		} else if (is_whirly(u.ustuck->data)) {
			No_puedes("alcanzar %s.", surface(u.ux,u.uy));
			return(0);
		} else
			jello = TRUE;
	} else if (is_lava(u.ux, u.uy)) {
		No_puedes("escribir en la lava!");
		return(0);
	} else if (is_pool(u.ux,u.uy) || IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		No_puedes("escribir en el agua!");
		return(0);
	}
	if(Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)/* in bubble */) {
		No_puedes("escribir en el aire!");
		return(0);
	}
	if (cantwield(youmonst.data)) {
		pline("¡Ya no puedes retener nada!");
		return(0);
	}
	if (check_capacity((char *)0)) return (0);

	/* One may write with finger, or weapon, or wand, or..., or...
	 * Edited by GAN 10/20/86 so as not to change weapon wielded.
	 */

	otmp = getobj(styluses, "con", "escribir");
	if(!otmp) return(0);		/* otmp == zeroobj if fingers */

	if (otmp == &zeroobj) writer = makeplural(body_part(FINGER));
	else writer = xname(otmp);

	/* There's no reason you should be able to write with a wand
	 * while both your hands are tied up.
	 */
	if (!freehand() && otmp != uwep && !otmp->owornmask) {
		pline("¡No tienes %s libre para escribir!", an(body_part(HAND)));
		return(0);
	}

	if (jello) {
		pline("%s con tu %s.", mon_nam_acc(u.ustuck, "Cosquilleas"), writer);
		pline("Tu mensaje se disuelve...");
		return(0);
	}
	if (otmp->oclass != WAND_CLASS && !can_reach_floor()) {
		No_puedes("alcanzar %s!", surface(u.ux,u.uy));
		return(0);
	}
	if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
		pline("Haces un movimiento hacia el altar con tu %s.", writer);
		altar_wrath(u.ux, u.uy);
		return(0);
	}
	if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
	    if (otmp == &zeroobj) { /* using only finger */
		pline("Sólo harías una pequeña mancha en %s.",
			surface(u.ux, u.uy));
		return(0);
	    } else if (!levl[u.ux][u.uy].disturbed) {
		pline("¡Perturbas a los no-muertos!");
		levl[u.ux][u.uy].disturbed = 1;
		(void) makemon(&mons[PM_GHOUL], u.ux, u.uy, NO_MM_FLAGS);
		exercise(A_WIS, FALSE);
		return(1);
	    }
	}

	/* SPFX for items */

	switch (otmp->oclass) {
	    default:
	    case AMULET_CLASS:
	    case CHAIN_CLASS:
	    case POTION_CLASS:
	    case COIN_CLASS:
		break;

	    case RING_CLASS:
		/* "diamond" rings and others should work */
	    case GEM_CLASS:
		/* diamonds & other hard gems should work */
		if (objects[otmp->otyp].oc_tough) {
			type = ENGRAVE;
			break;
		}
		break;

	    case ARMOR_CLASS:
		if (is_boots(otmp)) {
			type = DUST;
			break;
		}
		/* fall through */
	    /* Objects too large to engrave with */
	    case BALL_CLASS:
	    case ROCK_CLASS:
		No_puedes("escribir con un objeto tan grande!");
		ptext = FALSE;
		break;

	    /* Objects too silly to engrave with */
	    case FOOD_CLASS:
	    case SCROLL_CLASS:
	    case SPBOOK_CLASS:
		{
		    const char *que = xname(otmp);

		    pline("Tu %s se %s.", que,
			    is_ice(u.ux,u.uy) ? "congelaría" : "ensuciaría demasiado");
		}
		ptext = FALSE;
		break;

	    case RANDOM_CLASS:	/* This should mean fingers */
		break;

	    /* The charge is removed from the wand before prompting for
	     * the engraving text, because all kinds of setup decisions
	     * and pre-engraving messages are based upon knowing what type
	     * of engraving the wand is going to do.  Also, the player
	     * will have potentially seen "You wrest .." message, and
	     * therefore will know they are using a charge.
	     */
	    case WAND_CLASS:
		if (zappable(otmp)) {
		    check_unpaid(otmp);
		    zapwand = TRUE;
		    if (Levitation) ptext = FALSE;

		    switch (otmp->otyp) {
		    /* DUST wands */
		    default:
			break;

			/* NODIR wands */
		    case WAN_LIGHT:
		    case WAN_SECRET_DOOR_DETECTION:
		    case WAN_CREATE_MONSTER:
		    case WAN_WISHING:
		    case WAN_ENLIGHTENMENT:
			zapnodir(otmp);
			break;

			/* IMMEDIATE wands */
			/* If wand is "IMMEDIATE", remember to affect the
			 * previous engraving even if turning to dust.
			 */
		    case WAN_STRIKING:
			Strcpy(post_engr_text,
			"¡La varita lucha sin éxito contra tu intento de escribir!"
			);
			break;
		    case WAN_SLOW_MONSTER:
			if (!Blind) {
			   Sprintf(post_engr_text,
				   "¡Los insectos en %s se mueven más lentamente!",
				   surface(u.ux, u.uy));
			}
			break;
		    case WAN_SPEED_MONSTER:
			if (!Blind) {
			   Sprintf(post_engr_text,
				   "¡Los insectos en %s se mueven más rápidamente!",
				   surface(u.ux, u.uy));
			}
			break;
		    case WAN_POLYMORPH:
			if(oep)  {
			    if (!Blind) {
				type = (xchar)0;	/* random */
				(void) random_engraving(buf);
			    }
			    dengr = TRUE;
			}
			break;
		    case WAN_NOTHING:
		    case WAN_UNDEAD_TURNING:
		    case WAN_OPENING:
		    case WAN_LOCKING:
		    case WAN_PROBING:
			break;

			/* RAY wands */
		    case WAN_MAGIC_MISSILE:
			ptext = TRUE;
			if (!Blind) {
			   const char *piso = surface(u.ux, u.uy);
			   Sprintf(post_engr_text,
				   "¡%s es acribillad%c por agujeros de balas!",
				   capitalize(piso),
				   isfeminine(piso)? 'a' : 'o');
			}
			break;

		    /* can't tell sleep from death - Eric Backus */
		    case WAN_SLEEP:
		    case WAN_DEATH:
			if (!Blind) {
			   Sprintf(post_engr_text,
				   "¡Los insectos en %s dejan de moverse!",
				   surface(u.ux, u.uy));
			}
			break;

		    case WAN_COLD:
			if (!Blind)
			    Strcpy(post_engr_text,
				"Unos cubitos de hielo caen de la varita.");
			if(!oep || (oep->engr_type != BURN))
			    break;
		    case WAN_CANCELLATION:
		    case WAN_MAKE_INVISIBLE:
			if (oep && oep->engr_type != HEADSTONE) {
			    if (!Blind)
				pline("¡La inscripción en %s se desvanece!",
					surface(u.ux,u.uy));
			    dengr = TRUE;
			}
			break;
		    case WAN_TELEPORTATION:
			if (oep && oep->engr_type != HEADSTONE) {
			    if (!Blind)
				pline("¡La inscripción en %s se desvanece!",
					surface(u.ux,u.uy));
			    teleengr = TRUE;
			}
			break;

		    /* type = ENGRAVE wands */
		    case WAN_DIGGING:
			ptext = TRUE;
			type  = ENGRAVE;
			if(!objects[otmp->otyp].oc_name_known) {
			    if (flags.verbose)
				pline("¡Esta %s es una varita de cavar!",
				xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind)
			    Strcpy(post_engr_text,
				IS_GRAVE(levl[u.ux][u.uy].typ) ?
				"Vuelan astillas de la lápida." :
				is_ice(u.ux,u.uy) ?
				"¡Vuelan astillas de la superficie del hielo!" :
				"Se vuela la grava del piso.");
			else
			    Strcpy(post_engr_text, "¡Oyes un taladro!");
			break;

		    /* type = BURN wands */
		    case WAN_FIRE:
			ptext = TRUE;
			type  = BURN;
			if(!objects[otmp->otyp].oc_name_known) {
			if (flags.verbose)
			    pline("¡Esta %s es una varita de fuego!", xname(otmp));
			    doknown = TRUE;
			}
			Strcpy(post_engr_text,
				Blind ? "Sientes que la varita se calienta." :
					"Unas llamas salen de la varita.");
			break;
		    case WAN_LIGHTNING:
			ptext = TRUE;
			type  = BURN;
			if(!objects[otmp->otyp].oc_name_known) {
			    if (flags.verbose)
				pline("¡Esta %s es una varita de relámpagos!",
					xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind) {
			    Strcpy(post_engr_text,
				    "Un relámpago se arquea desde la varita.");
			    doblind = TRUE;
			} else
			    Strcpy(post_engr_text, "¡Oyes crujidos!");
			break;

		    /* type = MARK wands */
		    /* type = ENGR_BLOOD wands */
		    }
		} else /* end if zappable */
		    if (!can_reach_floor()) {
			No_puedes("alcanzar %s!", surface(u.ux,u.uy));
			return(0);
		    }
		break;

	    case WEAPON_CLASS:
		if (is_blade(otmp)) {
		    if ((int)otmp->spe > -3)
			type = ENGRAVE;
		    else {
			char gen = isfeminine(xname(otmp))? 'a' : 'o';
			pline("Tu %s demasiado embotad%c para grabar.",
				aobjnam(otmp,"es"), gen);
		    }
		}
		break;

	    case TOOL_CLASS:
		if(otmp == ublindf) {
		    pline("Es un poco difícil escribir con eso, ¿no crees?");
		    return(0);
		}
		switch (otmp->otyp)  {
		    case MAGIC_MARKER:
			if (otmp->spe <= 0)
			    pline("Tu marcador se ha secado.");
			else
			    type = MARK;
			break;
		    case TOWEL:
			/* Can't really engrave with a towel */
			ptext = FALSE;
			if (oep) {
			    if ((oep->engr_type == DUST ) ||
				(oep->engr_type == ENGR_BLOOD) ||
				(oep->engr_type == MARK )) {
				if (!Blind)
				    pline("Borras el mensaje aquí.");
				else
				    pline("Tu %s se %s de %s.", xname(otmp),
					  otense(otmp, "cubre"),
					  is_ice(u.ux,u.uy) ?
					  "hielo" : "polvo");
				dengr = TRUE;
			    } else
				pline("Tu %s no puede borrar esta inscripción.",
				     xname(otmp));
			}
			else {
			    pline("Tu %s se %s con %s.", xname(otmp),
				  otense(otmp, "cubre"),
				  is_ice(u.ux,u.uy) ? "hielo" : "polvo");
			}
			break;
		    default:
			break;
		}
		break;

	    case VENOM_CLASS:
#ifdef WIZARD
		if (wizard) {
		    pline("¿¿Escribir una carta venenosa??");
		    break;
		}
#endif
	    case ILLOBJ_CLASS:
		impossible("¡Estás escribiendo con un objeto ilegal!");
		break;
	}

	if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
	    if (type == ENGRAVE || type == 0)
		type = HEADSTONE;
	    else {
		/* ensures the "cannot wipe out" case */
		type = DUST;
		dengr = FALSE;
		teleengr = FALSE;
		buf[0] = (char)0;
	    }
	}

	/* End of implement setup */

	/* Identify stylus */
	if (doknown) {
	    makeknown(otmp->otyp);
	    more_experienced(0,10);
	}

	if (teleengr) {
	    rloc_engr(oep);
	    oep = (struct engr *)0;
	}

	if (dengr) {
	    del_engr(oep);
	    oep = (struct engr *)0;
	}

	/* Something has changed the engraving here */
	if (*buf) {
	    make_engr_at(u.ux, u.uy, buf, moves, type);
	    pline("La inscripción dice ahora: \"%s\".", buf);
	    ptext = FALSE;
	}

	if (zapwand && (otmp->spe < 0)) {
	    pline("%s %sse vuelve polvo.",
		  The(xname(otmp)), Blind ? "" : "brilla violentemente, y entonces ");
	    if (!IS_GRAVE(levl[u.ux][u.uy].typ))
		pline("No vas a tener ningún éxito escribiendo en el %s con polvo.",
		      is_ice(u.ux,u.uy) ? "hielo" : "polvo");
	    useup(otmp);
	    ptext = FALSE;
	}

	if (!ptext) {		/* Early exit for some implements. */
	    if (otmp->oclass == WAND_CLASS && !can_reach_floor())
		No_puedes("alcanzar %s!", surface(u.ux,u.uy));
	    return(1);
	}

	/* Special effects should have deleted the current engraving (if
	 * possible) by now.
	 */

	if (oep) {
	    register char c = 'n';

	    /* Give player the choice to add to engraving. */

	    if (type == HEADSTONE) {
		/* no choice, only append */
		c = 'y';
	    } else if ( (type == oep->engr_type) && (!Blind ||
		 (oep->engr_type == BURN) || (oep->engr_type == ENGRAVE)) ) {
		c = yn_function("¿Quieres agregarlo a la inscripción actual?",
				es_ynqchars, ynqchars, 'y');
		if (c == 'q') {
		    pline(Never_mind);
		    return(0);
		}
	    }

	    if (c == 'n' || Blind) {

		if( (oep->engr_type == DUST) || (oep->engr_type == ENGR_BLOOD) ||
		    (oep->engr_type == MARK) ) {
		    if (!Blind) {
			pline("Borras el mensaje que estaba escrito %s aquí.",
			    ((oep->engr_type == DUST)  ? "en el polvo" :
			    ((oep->engr_type == ENGR_BLOOD) ? "en sangre"   :
							 "")));
			del_engr(oep);
			oep = (struct engr *)0;
		    } else
		   /* Don't delete engr until after we *know* we're engraving */
			eow = TRUE;
		} else
		    if ( (type == DUST) || (type == MARK) || (type == ENGR_BLOOD) ) {
			No_puedes("borrar el mensaje que está %s en %s aquí.",
			 oep->engr_type == BURN ?
			   (is_ice(u.ux,u.uy) ? "fundido" : "grabado a fuego") :
			   "grabado", surface(u.ux,u.uy));
			return(1);
		    } else
			if ( (type != oep->engr_type) || (c == 'n') ) {
			    if (!Blind || can_reach_floor())
				pline("Sobrescribirás el mensaje actual.");
			    eow = TRUE;
			}
	    }
	}

	eloc = surface(u.ux,u.uy);
	switch(type){
	    default:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = " a la escritura extraña";
		} else {
			everb = "Escribes";
			everb_inf = "escribir";
			everb_what = " extrañamente";
		}
		break;
	    case DUST:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = " a la escritura";
		} else {
			everb = "Escribes";
			everb_inf = "escribir";
			everb_what = "";
		}
		eloc = is_ice(u.ux,u.uy) ? "el hielo" : "el polvo";
		break;
	    case HEADSTONE:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = " al epitafio";
		} else {
			everb = "Grabas";
			everb_inf = "grabar";
			everb_what = "";
		}
	    case ENGRAVE:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = " al grabado";
		} else {
			everb = "Grabas";
			everb_inf = "grabar";
			everb_what = "";
		}
		break;
	    case BURN:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = is_ice(u.ux,u.uy) ? " al texto fundido"
						       : " a la inscripción a fuego";
		} else {
			everb = is_ice(u.ux,u.uy) ? "Fundes" : "Grabas a fuego";
			everb_inf = is_ice(u.ux,u.uy) ? "fundir" : "grabas a fuego";
			everb_what = "";;
		}
		break;
	    case MARK:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = " a los graffitis";
		} else {
			everb = "Garrapateas";
			everb_inf = "garrapatear";
			everb_what = "";
		}
		break;
	    case ENGR_BLOOD:
		if (oep && !eow) {
			everb = "Añades";
			everb_inf = "añadir";
			everb_what = " a los graffitis";
		} else {
			everb = "Garrapateas";
			everb_inf = "garrapatear";
			everb_what = "";
		}
		break;
	}

	/* Tell adventurer what is going on */
	if (otmp != &zeroobj)
	    pline("%s%s en %s con %s.", everb, everb_what, eloc, doname(otmp));
	else
	    pline("%s%s en %s con tus %s.", everb, everb_what, eloc,
		makeplural(body_part(FINGER)));

	/* Prompt for engraving! */
	Sprintf(qbuf,"¿Qué quieres %s%s en %s aquí?", everb_inf, everb_what, eloc);
	getlin(qbuf, ebuf);

	/* Count the actual # of chars engraved not including spaces */
	len = strlen(ebuf);
	for (sp = ebuf; *sp; sp++) if (isspace(*sp)) len -= 1;

	if (len == 0 || index(ebuf, '\033')) {
	    if (zapwand) {
		if (!Blind)
		    pline("%s, y entonces se %s.",
			  Tobjnam(otmp, "brila"), otense(otmp, "desvanece"));
		return(1);
	    } else {
		pline(Never_mind);
		return(0);
	    }
	}

	/* A single `x' is the traditional signature of an illiterate person */
	if (len != 1 || (!index(ebuf, 'x') && !index(ebuf, 'X')))
	    u.uconduct.literate++;

	/* Mix up engraving if surface or state of mind is unsound.
	   Note: this won't add or remove any spaces. */
	for (sp = ebuf; *sp; sp++) {
	    if (isspace(*sp)) continue;
	    if (((type == DUST || type == ENGR_BLOOD) && !rn2(25)) ||
		    (Blind && !rn2(11)) || (Confusion && !rn2(7)) ||
		    (Stunned && !rn2(4)) || (Hallucination && !rn2(2)))
		*sp = ' ' + rnd(96 - 2);	/* ASCII '!' thru '~'
						   (excludes ' ' and DEL) */
	}

	/* Previous engraving is overwritten */
	if (eow) {
	    del_engr(oep);
	    oep = (struct engr *)0;
	}

	/* Figure out how long it took to engrave, and if player has
	 * engraved too much.
	 */
	switch(type){
	    default:
		multi = -(len/10);
		if (multi) nomovemsg = "Acabas con tu grabado extraño.";
		break;
	    case DUST:
		multi = -(len/10);
		if (multi) nomovemsg = "Acabas de escribir en el polvo.";
		break;
	    case HEADSTONE:
	    case ENGRAVE:
		multi = -(len/10);
		if ((otmp->oclass == WEAPON_CLASS) &&
		    ((otmp->otyp != ATHAME) || otmp->cursed)) {
		    multi = -len;
		    maxelen = ((otmp->spe + 3) * 2) + 1;
			/* -2 = 3, -1 = 5, 0 = 7, +1 = 9, +2 = 11
			 * Note: this does not allow a +0 anything (except
			 *	 an athame) to engrave "Elbereth" all at once.
			 *	 However, you could now engrave "Elb", then
			 *	 "ere", then "th".
			 */
		    pline("Tu %s.", aobjnam(otmp, "se embota"));
		    if (otmp->unpaid) {
			struct monst *shkp = shop_keeper(*u.ushops);
			if (shkp) {
			    pline("¡Usted lo daña, usted lo paga!");
			    bill_dummy_object(otmp);
			}
		    }
		    if (len > maxelen) {
			multi = -maxelen;
			otmp->spe = -3;
		    } else if (len > 1)
			otmp->spe -= len >> 1;
		    else otmp->spe -= 1; /* Prevent infinite engraving */
		} else
		    if ( (otmp->oclass == RING_CLASS) ||
			 (otmp->oclass == GEM_CLASS) )
			multi = -len;
		if (multi) nomovemsg = "Acabas de escribir.";
		break;
	    case BURN:
		multi = -(len/10);
		if (multi)
		    nomovemsg = is_ice(u.ux,u.uy) ?
			"Acabas de fundir tu mensaje en el hielo.":
			"Acabas de grabar a fuego tu mensaje en el piso.";
		break;
	    case MARK:
		multi = -(len/10);
		if ((otmp->oclass == TOOL_CLASS) &&
		    (otmp->otyp == MAGIC_MARKER)) {
		    maxelen = (otmp->spe) * 2; /* one charge / 2 letters */
		    if (len > maxelen) {
			pline("Tu marcador se ha secado.");
			otmp->spe = 0;
			multi = -(maxelen/10);
		    } else
			if (len > 1) otmp->spe -= len >> 1;
			else otmp->spe -= 1; /* Prevent infinite grafitti */
		}
		if (multi) nomovemsg = "Acabas de estropear el calabozo.";
		break;
	    case ENGR_BLOOD:
		multi = -(len/10);
		if (multi) nomovemsg = "Acabas de garrapatear.";
		break;
	}

	/* Chop engraving down to size if necessary */
	if (len > maxelen) {
	    for (sp = ebuf; (maxelen && *sp); sp++)
		if (!isspace(*sp)) maxelen--;
	    if (!maxelen && *sp) {
		*sp = (char)0;
		if (multi) nomovemsg = "No puedes escribir más.";
		pline("Sólo puedes escribir \"%s\"", ebuf);
	    }
	}

	/* Add to existing engraving */
	if (oep) Strcpy(buf, oep->engr_txt);

	(void) strncat(buf, ebuf, (BUFSZ - (int)strlen(buf) - 1));

	make_engr_at(u.ux, u.uy, buf, (moves - multi), type);

	if (post_engr_text[0]) pline(post_engr_text);

	if (doblind && !resists_blnd(&youmonst)) {
	    pline("¡Te ciega el foganozo!");
	    make_blinded((long)rnd(50),FALSE);
	    if (!Blind) pline(vision_clears);
	}

	return(1);
}

void
save_engravings(fd, mode)
int fd, mode;
{
	register struct engr *ep = head_engr;
	register struct engr *ep2;
	unsigned no_more_engr = 0;

	while (ep) {
	    ep2 = ep->nxt_engr;
	    if (ep->engr_lth && ep->engr_txt[0] && perform_bwrite(mode)) {
		bwrite(fd, (genericptr_t)&(ep->engr_lth), sizeof(ep->engr_lth));
		bwrite(fd, (genericptr_t)ep, sizeof(struct engr) + ep->engr_lth);
	    }
	    if (release_data(mode))
		dealloc_engr(ep);
	    ep = ep2;
	}
	if (perform_bwrite(mode))
	    bwrite(fd, (genericptr_t)&no_more_engr, sizeof no_more_engr);
	if (release_data(mode))
	    head_engr = 0;
}

void
rest_engravings(fd)
int fd;
{
	register struct engr *ep;
	unsigned lth;

	head_engr = 0;
	while(1) {
		mread(fd, (genericptr_t) &lth, sizeof(unsigned));
		if(lth == 0) return;
		ep = newengr(lth);
		mread(fd, (genericptr_t) ep, sizeof(struct engr) + lth);
		ep->nxt_engr = head_engr;
		head_engr = ep;
		ep->engr_txt = (char *) (ep + 1);	/* Andreas Bormann */
		/* mark as finished for bones levels -- no problem for
		 * normal levels as the player must have finished engraving
		 * to be able to move again */
		ep->engr_time = moves;
	}
}

void
del_engr(ep)
register struct engr *ep;
{
	if (ep == head_engr) {
		head_engr = ep->nxt_engr;
	} else {
		register struct engr *ept;

		for (ept = head_engr; ept; ept = ept->nxt_engr)
		    if (ept->nxt_engr == ep) {
			ept->nxt_engr = ep->nxt_engr;
			break;
		    }
		if (!ept) {
		    impossible("¿Error en del_engr?");
		    return;
		}
	}
	dealloc_engr(ep);
}

/* randomly relocate an engraving */
void
rloc_engr(ep)
struct engr *ep;
{
	int tx, ty, tryct = 200;

	do  {
	    if (--tryct < 0) return;
	    tx = rn1(COLNO-3,2);
	    ty = rn2(ROWNO);
	} while (engr_at(tx, ty) ||
		!goodpos(tx, ty, (struct monst *)0, 0));

	ep->engr_x = tx;
	ep->engr_y = ty;
}


/* Epitaphs for random headstones */
static const char *epitaphs[] = {
	"Que en paz descanse",
	"Q.E.P.D.",
	/* LENGUA:  English, "Rest in pieces" */
	"Que en pedazos descanse",
	"Nota -- no hay NINGÚN artículo valioso en esta tumba",
	"1994-1995. El Hacker más Longevo de Todos los Tiempos",
	"La Tumba del Hacker Desconocido",
	"No estábamos seguros de quién era este, pero lo enterramos aquí de todos modos",
	"Sparky -- era un perro muy bueno",
	"Cuidado con el Tercer Riel Eléctrico",
	"Hecho en Taiwan",
	/* LENGUA:  English, "Og friend. Og good dude. Og died. Og now food" */
	"Og amigo. Og buen tipo. Og muerto. Og comido.",
	"Beetlejuice Beetlejuice Beetlejuice",
	"¡Cuidado abajo!",
	"Por favor no me desentierres.  Estoy perfectamente feliz aquí. -- Residente",
	"Cartero, por favor reenviar a la siguiente dirección: Gehena, Fortaleza de Asmodeo, quinto lémur a la izquierda",
	/* LENGUA:  need the usual form of this rhyme in Spanish */
	/* "Mary had a little lamb/Its fleece was white as snow/When Mary was in trouble/The lamb was first to go" */

	"¡Ten cuidado, o esto te podría pasar a ti!",
	"¡Te unirás muy pronto a este tipo en el infierno! -- el Mago de Yéndor",
	"¡Precaución! Esta tumba contiene desechos tóxicos",
	"Sum quod eris",
	"Aquí yace un Ateo, vestido elegantemente y sin ningún lugar adonde ir",
	"Aquí yace Ezequiel, edad 102 años.  Los buenos mueren jóvenes.",
	/* LENGUA:  English, "Here lies my wife: Here let her lie! Now she's at rest and so am I." */
	"Aquí en la tierra yace mi esposo.  Tanto él como yo ya tenemos reposo.",
	/* LENGUA:  English, "Here lies Johnny Yeast. Pardon me for not rising." */
	"Aquí yace Fernando Fermento.  Disculpen si no me levanto.",
	/* LENGUA:  These puns don't work in Spanish */
	/* "He always lied while on the earth and now he's lying in it", */
	/* "I made an ash of myself", */
	/* LENGUA:  English, "Soon ripe. Soon rotten. Soon gone. But not forgotten." */
	"Pronto madura.  Pronto podrida.  No olvidada.  Solamente ida.",
	/* LENGUA:  English, "Here lies the body of Jonathan Blake. Stepped on the gas instead of the brake." */
	"Aquí yace Carlos Acevedo. Pisó el acelerador en lugar del freno.",
	"¡Fuera!"

	/* Según Andrés */
	"Aquí yace quien nació y murió, sin saber nunca para qué vivió.",
	"Todos somos iguales en el nacer y en el morir, aunque no sea en el vivir.",
	"Aquí yace Beatriz de Figueredo, muerta muy joven por meterse el dedo. Por meterse el dedo a la nariz, de una hemorragia se murió Beatriz.",
	"Aquí yace un abogado, un hombre honrado, un hombre íntegro. ¡Enterraron a tres hombres en la misma fosa!",
	"Aquí yace Fulanito de Tal, de joven gastó su salud para conseguir dinero y de viejo gastó su dinero para conseguir salud. Sin salud y sin dinero, aquí yace Fulanito de Tal.",
	"Aquí descansa Joan Pons, pastelero de la Bisbal, que se acabó los turrones la vigilia de Navidad.",
	"Eadem mutata resurgo",
	"Aquí yace María, tan fría como siempre. Junto a ella yace Pepe, tieso y duro como nunca",
};

/* Create a headstone at the given location.
 * The caller is responsible for newsym(x, y).
 */
void
make_grave(x, y, str)
int x, y;
const char *str;
{
	/* Can we put a grave here? */
	if ((levl[x][y].typ != ROOM && levl[x][y].typ != GRAVE) || t_at(x,y)) return;

	/* Make the grave */
	levl[x][y].typ = GRAVE;

	/* Engrave the headstone */
	if (!str) str = epitaphs[rn2(SIZE(epitaphs))];
	del_engr_at(x, y);
	make_engr_at(x, y, str, 0L, HEADSTONE);
	return;
}


#endif /* OVLB */

/*engrave.c*/
