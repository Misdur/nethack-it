/*	SCCS Id: @(#)timeout.c	3.4	2002/12/17	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"	/* for checking save modes */

STATIC_DCL void NDECL(stoned_dialogue);
STATIC_DCL void NDECL(vomiting_dialogue);
STATIC_DCL void NDECL(choke_dialogue);
STATIC_DCL void NDECL(slime_dialogue);
STATIC_DCL void NDECL(slip_or_trip);
STATIC_DCL void FDECL(see_lamp_flicker, (struct obj *, const char *));
STATIC_DCL void FDECL(lantern_message, (struct obj *));
STATIC_DCL void FDECL(cleanup_burn, (genericptr_t,long));

#ifdef OVLB

/* He is being petrified - dialogue by inmet!tower */
static NEARDATA const char * const stoned_texts[] = {
	"Estás retardando.",			/* 5 */
	"Tus miembros se hacen inflexibles.",	/* 4 */
	"Tus miembros se han puesto piedra.",	/* 3 */
	"Te has puesto piedra.",		/* 2 */
	"Eres una estatua."			/* 1 */
};

STATIC_OVL void
stoned_dialogue()
{
	register long i = (Stoned & TIMEOUT);

	if(i > 0L && i <= SIZE(stoned_texts))
		pline(stoned_texts[SIZE(stoned_texts) - i]);
	if(i == 5L)
		HFast = 0L;
	if(i == 3L)
		nomul(-3);
	exercise(A_DEX, FALSE);
}

/* He is getting sicker and sicker prior to vomiting */
static NEARDATA const char * const vomiting_texts[] = {
	"Te sientes un poco nausead%c.",		/* 14 */
	"Te sientes un poco confundid%c.",		/* 11 */
	"Parece que no puedas pensar derecho.",	/* 8 */
	"Te sientes increíblemente mal%c.",	/* 5 */
	"¡De repente vomitas!"			/* 2 */
};

STATIC_OVL void
vomiting_dialogue()
{
	register long i = (Vomiting & TIMEOUT) / 3L;

	if ((((Vomiting & TIMEOUT) % 3L) == 2) && (i >= 0)
	    && (i < SIZE(vomiting_texts)))
		pline(vomiting_texts[SIZE(vomiting_texts) - i - 1],
			poly_gender()==1? 'a' : 'o');

	switch ((int) i) {
	case 0:
		vomit();
		morehungry(20);
		break;
	case 2:
		make_stunned(HStun + d(2,4), FALSE);
		/* fall through */
	case 3:
		make_confused(HConfusion + d(2,4), FALSE);
		break;
	}
	exercise(A_CON, FALSE);
}

static NEARDATA const char * const choke_texts[] = {
	"Es difícil respirar.",
	"Estás boqueando por aire.",
	"No puedes respirar más.",
	"Te pones %s.",
	"Sufocas."
};

static NEARDATA const char * const choke_texts2[] = {
	"Se aprieta tu %s.",
	"Tu sangre se encuentra difícil llegar a tu cerebro.",
	"La presión en tu %s se crece.",
	"Se marchita tu conciencia.",
	"Sufocas."
};

STATIC_OVL void
choke_dialogue()
{
	register long i = (Strangled & TIMEOUT);

	if(i > 0 && i <= SIZE(choke_texts)) {
	    if (Breathless || !rn2(50))
		pline(choke_texts2[SIZE(choke_texts2) - i], body_part(NECK));
	    else {
		const char *str = choke_texts[SIZE(choke_texts)-i];

		if (index(str, '%'))
		    pline(str, feminize(hcolor(NH_BLUE), poly_gender()==1));
		else
		    pline(str);
	    }
	}
	exercise(A_STR, FALSE);
}

static NEARDATA const char * const slime_texts[] = {
	"Te pones un poco %s.",             /* 5 */
	"Tus miembros se ponen rezumados.", /* 4 */
	"Tu piel empieza pelarse.",         /* 3 */
	"Te estás poniendo %s.",            /* 2 */
	"Te has puesto %s."                 /* 1 */
};

STATIC_OVL void
slime_dialogue()
{
	register long i = (Slimed & TIMEOUT) / 2L;

	if (((Slimed & TIMEOUT) % 2L) && i >= 0L
		&& i < SIZE(slime_texts)) {
	    const char *str = slime_texts[SIZE(slime_texts) - i - 1L];

	    if (index(str, '%')) {
		if (i == 4L) {	/* "you are turning green" */
		    if (!Blind)	/* [what if you're already green?] */
			pline(str, feminize(hcolor(NH_GREEN), poly_gender()==1));
		} else
		    pline(str, an(Hallucination ? rndmonnam() : "limo verde"));
	    } else
		pline(str);
	}
	if (i == 3L) {	/* limbs becoming oozy */
	    HFast = 0L;	/* lose intrinsic speed */
	    stop_occupation();
	    if (multi > 0) nomul(0);
	}
	exercise(A_DEX, FALSE);
}

void
burn_away_slime()
{
	if (Slimed) {
	    pline("¡El limo que te cubre se quema!");
	    Slimed = 0L;
	    flags.botl = 1;
	}
	return;
}


#endif /* OVLB */
#ifdef OVL0

void
nh_timeout()
{
	register struct prop *upp;
	int sleeptime;
	int m_idx;
	int baseluck = (flags.moonphase == FULL_MOON) ? 1 : 0;

	if (flags.friday13) baseluck -= 1;

	if (u.uluck != baseluck &&
		moves % (u.uhave.amulet || u.ugangr ? 300 : 600) == 0) {
	/* Cursed luckstones stop bad luck from timing out; blessed luckstones
	 * stop good luck from timing out; normal luckstones stop both;
	 * neither is stopped if you don't have a luckstone.
	 * Luck is based at 0 usually, +1 if a full moon and -1 on Friday 13th
	 */
	    register int time_luck = stone_luck(FALSE);
	    boolean nostone = !carrying(LUCKSTONE) && !stone_luck(TRUE);

	    if(u.uluck > baseluck && (nostone || time_luck < 0))
		u.uluck--;
	    else if(u.uluck < baseluck && (nostone || time_luck > 0))
		u.uluck++;
	}
	if(u.uinvulnerable) return; /* things past this point could kill you */
	if(Stoned) stoned_dialogue();
	if(Slimed) slime_dialogue();
	if(Vomiting) vomiting_dialogue();
	if(Strangled) choke_dialogue();
	if(u.mtimedone && !--u.mtimedone) {
		if (Unchanging)
			u.mtimedone = rnd(100*youmonst.data->mlevel + 1);
		else
			rehumanize();
	}
	if(u.ucreamed) u.ucreamed--;

	/* Dissipate spell-based protection. */
	if (u.usptime) {
	    if (--u.usptime == 0 && u.uspellprot) {
		u.usptime = u.uspmtime;
		u.uspellprot--;
		find_ac();
		if (!Blind)
		    Norep("La niebla %s alrededor de ti %s.", feminize(hcolor(NH_GOLDEN), TRUE),
			  u.uspellprot ? "se hace menos densa" : "desaparece");
	    }
	}

#ifdef STEED
	if (u.ugallop) {
	    if (--u.ugallop == 0L && u.usteed)
	    	pline("%s deja de galopear.", Monnam(u.usteed));
	}
#endif

	for(upp = u.uprops; upp < u.uprops+SIZE(u.uprops); upp++)
	    if((upp->intrinsic & TIMEOUT) && !(--upp->intrinsic & TIMEOUT)) {
		switch(upp - u.uprops){
		case STONED:
			if (delayed_killer && !killer) {
				killer = delayed_killer;
				delayed_killer = 0;
			}
			if (!killer) {
				/* leaving killer_format would make it
				   "petrified by petrification" */
				killer_format = NO_KILLER_PREFIX;
				killer = "murió de petrificación";
			}
			done(STONING);
			break;
		case SLIMED:
			if (delayed_killer && !killer) {
				killer = delayed_killer;
				delayed_killer = 0;
			}
			if (!killer) {
				killer_format = NO_KILLER_PREFIX;
				killer = "se puso limo verde";
			}
			done(TURNED_SLIME);
			break;
		case VOMITING:
			make_vomiting(0L, TRUE);
			break;
		case SICK:
			pline("Te mueres de tu enfermedad.");
			killer_format = KILLED_BY_AN;
			killer = u.usick_cause;
			if ((m_idx = name_to_mon(killer)) >= LOW_PM) {
			    if (type_is_pname(&mons[m_idx])) {
				killer_format = KILLED_BY;
			    } else if (mons[m_idx].geno & G_UNIQ) {
				killer = the(killer);
				Strcpy(u.usick_cause, killer);
				killer_format = KILLED_BY;
			    }
			}
			u.usick_type = 0;
			done(POISONING);
			break;
		case FAST:
			if (!Very_fast)
				pline("Te sientes retardando%s.",
							Fast ? " un poco" : "");
			break;
		case CONFUSION:
			HConfusion = 1; /* So make_confused works properly */
			make_confused(0L, TRUE);
			stop_occupation();
			break;
		case STUNNED:
			HStun = 1;
			make_stunned(0L, TRUE);
			stop_occupation();
			break;
		case BLINDED:
			Blinded = 1;
			make_blinded(0L, TRUE);
			stop_occupation();
			break;
		case INVIS:
			newsym(u.ux,u.uy);
			if (!Invis && !BInvis && !Blind) {
			    pline(!See_invisible ?
				      "Ya no estás invisible." :
				      "Ya no puedes ver través de ti.");
			    stop_occupation();
			}
			break;
		case SEE_INVIS:
			set_mimic_blocking(); /* do special mimic handling */
			see_monsters();		/* make invis mons appear */
			newsym(u.ux,u.uy);	/* make self appear */
			stop_occupation();
			break;
		case WOUNDED_LEGS:
			heal_legs();
			stop_occupation();
			break;
		case HALLUC:
			HHallucination = 1;
			(void) make_hallucinated(0L, TRUE, 0L);
			stop_occupation();
			break;
		case SLEEPING:
			if (unconscious() || Sleep_resistance)
				HSleeping += rnd(100);
			else if (Sleeping) {
				pline("Te duermes.");
				sleeptime = rnd(20);
				fall_asleep(-sleeptime, TRUE);
				HSleeping += sleeptime + rnd(100);
			}
			break;
		case LEVITATION:
			(void) float_down(I_SPECIAL|TIMEOUT, 0L);
			break;
		case STRANGLED:
			killer_format = KILLED_BY;
			killer = (u.uburied) ? "sufocación" : "estrangulación";
			done(DIED);
			break;
		case FUMBLING:
			/* call this only when a move took place.  */
			/* otherwise handle fumbling msgs locally. */
			if (u.umoved && !Levitation) {
			    slip_or_trip();
			    nomul(-2);
			    nomovemsg = "";
			    /* The more you are carrying the more likely you
			     * are to make noise when you fumble.  Adjustments
			     * to this number must be thoroughly play tested.
			     */
			    if ((inv_weight() > -500)) {
				pline("¡Haces mucho ruido!");
				wake_nearby();
			    }
			}
			/* from outside means slippery ice; don't reset
			   counter if that's the only fumble reason */
			HFumbling &= ~FROMOUTSIDE;
			if (Fumbling)
			    HFumbling += rnd(20);
			break;
		case DETECT_MONSTERS:
			see_monsters();
			break;
		}
	}

	run_timers();
}

#endif /* OVL0 */
#ifdef OVL1

void
fall_asleep(how_long, wakeup_msg)
int how_long;
boolean wakeup_msg;
{
	stop_occupation();
	nomul(how_long);
	/* generally don't notice sounds while sleeping */
	if (wakeup_msg && multi == how_long) {
	    /* caller can follow with a direct call to Hear_again() if
	       there's a need to override this when wakeup_msg is true */
	    flags.soundok = 0;
	    afternmv = Hear_again;	/* this won't give any messages */
	}
	/* early wakeup from combat won't be possible until next monster turn */
	u.usleep = monstermoves;
	nomovemsg = wakeup_msg ? "Te despiertas." : You_can_move_again;
}

/* Attach an egg hatch timeout to the given egg. */
void
attach_egg_hatch_timeout(egg)
struct obj *egg;
{
	int i;

	/* stop previous timer, if any */
	(void) stop_timer(HATCH_EGG, (genericptr_t) egg);

	/*
	 * Decide if and when to hatch the egg.  The old hatch_it() code tried
	 * once a turn from age 151 to 200 (inclusive), hatching if it rolled
	 * a number x, 1<=x<=age, where x>150.  This yields a chance of
	 * hatching > 99.9993%.  Mimic that here.
	 */
	for (i = (MAX_EGG_HATCH_TIME-50)+1; i <= MAX_EGG_HATCH_TIME; i++)
	    if (rnd(i) > 150) {
		/* egg will hatch */
		(void) start_timer((long)i, TIMER_OBJECT,
						HATCH_EGG, (genericptr_t)egg);
		break;
	    }
}

/* prevent an egg from ever hatching */
void
kill_egg(egg)
struct obj *egg;
{
	/* stop previous timer, if any */
	(void) stop_timer(HATCH_EGG, (genericptr_t) egg);
}

/* timer callback routine: hatch the given egg */
void
hatch_egg(arg, timeout)
genericptr_t arg;
long timeout;
{
	struct obj *egg;
	struct monst *mon, *mon2;
	coord cc;
	xchar x, y;
	boolean yours, silent, knows_egg = FALSE;
	boolean cansee_hatchspot = FALSE;
	int i, mnum, hatchcount = 0;

	egg = (struct obj *) arg;
	/* sterilized while waiting */
	if (egg->corpsenm == NON_PM) return;

	mon = mon2 = (struct monst *)0;
	mnum = big_to_little(egg->corpsenm);
	/* The identity of one's father is learned, not innate */
	yours = (egg->spe || (!flags.female && carried(egg) && !rn2(2)));
	silent = (timeout != monstermoves);	/* hatched while away */

	/* only can hatch when in INVENT, FLOOR, MINVENT */
	if (get_obj_location(egg, &x, &y, 0)) {
	    hatchcount = rnd((int)egg->quan);
	    cansee_hatchspot = cansee(x, y) && !silent;
	    if (!(mons[mnum].geno & G_UNIQ) &&
		   !(mvitals[mnum].mvflags & (G_GENOD | G_EXTINCT))) {
		for (i = hatchcount; i > 0; i--) {
		    if (!enexto(&cc, x, y, &mons[mnum]) ||
			 !(mon = makemon(&mons[mnum], cc.x, cc.y, NO_MINVENT)))
			break;
		    /* tame if your own egg hatches while you're on the
		       same dungeon level, or any dragon egg which hatches
		       while it's in your inventory */
		    if ((yours && !silent) ||
			(carried(egg) && mon->data->mlet == S_DRAGON)) {
			if ((mon2 = tamedog(mon, (struct obj *)0)) != 0) {
			    mon = mon2;
			    if (carried(egg) && mon->data->mlet != S_DRAGON)
				mon->mtame = 20;
			}
		    }
		    if (mvitals[mnum].mvflags & G_EXTINCT)
			break;	/* just made last one */
		    mon2 = mon;	/* in case makemon() fails on 2nd egg */
		}
		if (!mon) mon = mon2;
		hatchcount -= i;
		egg->quan -= (long)hatchcount;
	    }
	}
#if 0
	/*
	 * We could possibly hatch while migrating, but the code isn't
	 * set up for it...
	 */
	else if (obj->where == OBJ_MIGRATING) {
	    /*
	    We can do several things.  The first ones that come to
	    mind are:

	    + Create the hatched monster then place it on the migrating
	      mons list.  This is tough because all makemon() is made
	      to place the monster as well.    Makemon() also doesn't
	      lend itself well to splitting off a "not yet placed"
	      subroutine.

	    + Mark the egg as hatched, then place the monster when we
	      place the migrating objects.

	    + Or just kill any egg which gets sent to another level.
	      Falling is the usual reason such transportation occurs.
	    */
	    cansee_hatchspot = FALSE;
	    mon = ???
	    }
#endif

	if (mon) {
	    char monnambuf[BUFSZ], carriedby[BUFSZ];
	    boolean siblings = (hatchcount > 1), redraw = FALSE;

	    if (cansee_hatchspot) {
		/* LENGUA:  We should use "unas" if all monsters hatched are
		   female, but that complicates the code too much */
		Sprintf(monnambuf, "%s%s",
			siblings ? "unos " : "",
			siblings ?
			makeplural(m_monnam(mon)) : an(m_monnam(mon)));
		/* we don't learn the egg type here because learning
		   an egg type requires either seeing the egg hatch
		   or being familiar with the egg already,
		   as well as being able to see the resulting
		   monster, checked below
		*/
	    }
	    switch (egg->where) {
		case OBJ_INVENT:
		    knows_egg = TRUE; /* true even if you are blind */
		    if (!cansee_hatchspot)
			Sientes("%s %s de tu carga!", something,
			    locomotion(mon->data, "cayendo"));
		    else
			pline("¡Ves %s %s de tu carga!",
			    monnambuf, locomotion(mon->data, "cayendo"));
		    if (yours) {
			pline("Sus gritas suenan como \"%s%s%s\"",
			    egg->spe ? ""  : "¿",
			    flags.female ? "mamá" : "papá",
			    egg->spe ? "." : "?");
		    } else if (mon->data->mlet == S_DRAGON) {
			verbalize("¡Glip!");		/* Mything eggs :-) */
		    }
		    break;

		case OBJ_FLOOR:
		    if (cansee_hatchspot) {
			knows_egg = TRUE;
			pline("Ves %s empollándose.", monnambuf);
			redraw = TRUE;	/* update egg's map location */
		    }
		    break;

		case OBJ_MINVENT:
		    if (cansee_hatchspot) {
			/* egg carring monster might be invisible */
			if (canseemon(egg->ocarry)) {
			    Sprintf(carriedby, "la carga de %s",
				     a_monnam(egg->ocarry));
			    knows_egg = TRUE;
			}
			else if (is_pool(mon->mx, mon->my))
			    Strcpy(carriedby, "agua vacía");
			else
			    Strcpy(carriedby, "el aire");
			pline("¡Ves %s %s desde %s!", monnambuf,
			    locomotion(mon->data, "cayendo"), carriedby);
		    }
		    break;
#if 0
		case OBJ_MIGRATING:
		    break;
#endif
		default:
		    impossible("¿Huevo se empolló dónde? (%d)", (int)egg->where);
		    break;
	    }

	    if (cansee_hatchspot && knows_egg)
		learn_egg_type(mnum);

	    if (egg->quan > 0) {
		/* still some eggs left */
		attach_egg_hatch_timeout(egg);
		if (egg->timed) {
		    /* replace ordinary egg timeout with a short one */
		    (void) stop_timer(HATCH_EGG, (genericptr_t)egg);
		    (void) start_timer((long)rnd(12), TIMER_OBJECT,
					HATCH_EGG, (genericptr_t)egg);
		}
	    } else if (carried(egg)) {
		useup(egg);
	    } else {
		/* free egg here because we use it above */
		obj_extract_self(egg);
		obfree(egg, (struct obj *)0);
	    }
	    if (redraw) newsym(x, y);
	}
}

/* Learn to recognize eggs of the given type. */
void
learn_egg_type(mnum)
int mnum;
{
	/* baby monsters hatch from grown-up eggs */
	mnum = little_to_big(mnum);
	mvitals[mnum].mvflags |= MV_KNOWS_EGG;
	/* we might have just learned about other eggs being carried */
	update_inventory();
}

/* Attach a fig_transform timeout to the given figurine. */
void
attach_fig_transform_timeout(figurine)
struct obj *figurine;
{
	int i;

	/* stop previous timer, if any */
	(void) stop_timer(FIG_TRANSFORM, (genericptr_t) figurine);

	/*
	 * Decide when to transform the figurine.
	 */
	i = rnd(9000) + 200;
	/* figurine will transform */
	(void) start_timer((long)i, TIMER_OBJECT,
				FIG_TRANSFORM, (genericptr_t)figurine);
}

/* give a fumble message */
STATIC_OVL void
slip_or_trip()
{
	struct obj *otmp = vobj_at(u.ux, u.uy);
	const char *what, *pronoun;
	char buf[BUFSZ];
	boolean on_foot = TRUE;
#ifdef STEED
	if (u.usteed) on_foot = FALSE;
#endif

	if (otmp && on_foot && !u.uinwater && is_pool(u.ux, u.uy)) otmp = 0;

	if (otmp && on_foot) {		/* trip over something in particular */
	    /*
		If there is only one item, it will have just been named
		during the move, so refer to by via pronoun; otherwise,
		if the top item has been or can be seen, refer to it by
		name; if not, look for rocks to trip over; trip over
		anonymous "something" if there aren't any rocks.
	     */
	    pronoun = otmp->quan == 1L ? "él" : "ellos";
	    /* Don't call feminize() because if we can't see it, we don't know
	       if it's feminine */
	    what = !otmp->nexthere ? pronoun :
		  (otmp->dknown || !Blind) ? doname(otmp) :
		  ((otmp = sobj_at(ROCK, u.ux, u.uy)) == 0 ? something :
		  (otmp->quan == 1L ? "una piedra" : "unas piedras"));
	    if (Hallucination) {
		what = strcpy(buf, what);
		buf[0] = toupper2(buf[0]);
		pline("¡Ay!  ¡%s muerde%s tu %s!",
			what, (!otmp || otmp->quan == 1L) ? "" : "n",
			body_part(FOOT));
	    } else {
		pline("Tropezas sobre %s.", what);
	    }
	} else if (rn2(3) && is_ice(u.ux, u.uy)) {
#ifdef STEED
	    if (u.usteed)
		pline("%s se %s en el hielo.",
		      upstart(x_monnam(u.usteed,
				u.usteed->mnamelth ? ARTICLE_NONE : ARTICLE_THE,
				(char *)0, SUPPRESS_SADDLE, FALSE)),
		      rn2(2) ? "desliza" : "resbala");
	    else
#endif
		pline("Te %s en el hielo.", rn2(2) ? "deslizas" : "resbalas");
	} else {
	    if (on_foot) {
		switch (rn2(4)) {
		  case 1:
			{
			    const char *pies = body_part(FOOT);
			    const char *plural = Hallucination ? "" : "s";
			    boolean fem = Hallucination ? FALSE : isfeminine(pies);

			    pline("Tropezas sobre tu%s propi%c%s %s.",
				plural,
				fem? 'a' : 'o',
				plural,
				Hallucination ? "codo" : makeplural(pies));
			}
			break;
		  case 2:
			pline("Te resbalas %s.", Hallucination ?
				"sobre una cáscara de banana" : "y casi te caes");
			break;
		  case 3:
			pline("Pataleas.");
			break;
		  default:
			pline("Te resbalas.");
			break;
		}
	    }
#ifdef STEED
	    else {
		switch (rn2(4)) {
		  case 1:
			pline("Tus %s se resbalan fuera de los estribos.", makeplural(body_part(FOOT)));
			break;
		  case 2:
			pline("Sueltas las riendas.");
			break;
		  case 3:
			pline("Golpeas el cuerno de la silla.");
			break;
		  default:
			pline("Te resbalas a un lado de la silla.");
			break;
		}
		dismount_steed(DISMOUNT_FELL);
	    }
#endif
	}
}

/* Print a lamp flicker message with tailer. */
STATIC_OVL void
see_lamp_flicker(obj, tailer)
struct obj *obj;
const char *tailer;
{
	switch (obj->where) {
	    case OBJ_INVENT:
	    case OBJ_MINVENT:
		pline("%s tembla%s.", Yname2(obj), tailer);
		break;
	    case OBJ_FLOOR:
		pline("Ves %s temblando%s.", an(xname(obj)), tailer);
		break;
	}
}

/* Print a dimming message for brass lanterns. */
STATIC_OVL void
lantern_message(obj)
struct obj *obj;
{
	/* from adventure */
	switch (obj->where) {
	    case OBJ_INVENT:
		pline("Tu linterna se hace obscura.");
		if (Hallucination)
		    pline("Ya no se han inventado las pilas.");
		break;
	    case OBJ_FLOOR:
		pline("Ves una linterna hiciéndose obscura.");
		break;
	    case OBJ_MINVENT:
		pline("La linterna %s se hace obscura.",
		    del(mon_nam(obj->ocarry)));
		break;
	}
}

/*
 * Timeout callback for for objects that are burning. E.g. lamps, candles.
 * See begin_burn() for meanings of obj->age and obj->spe.
 */
void
burn_object(arg, timeout)
genericptr_t arg;
long timeout;
{
	struct obj *obj = (struct obj *) arg;
	boolean canseeit, many, menorah, need_newsym;
	xchar x, y;
	char whose[BUFSZ];

	menorah = obj->otyp == CANDELABRUM_OF_INVOCATION;
	many = menorah ? obj->spe > 1 : obj->quan > 1L;

	/* timeout while away */
	if (timeout != monstermoves) {
	    long how_long = monstermoves - timeout;

	    if (how_long >= obj->age) {
		obj->age = 0;
		end_burn(obj, FALSE);

		if (menorah) {
		    obj->spe = 0;	/* no more candles */
		} else if (Is_candle(obj) || obj->otyp == POT_OIL) {
		    /* get rid of candles and burning oil potions */
		    obj_extract_self(obj);
		    obfree(obj, (struct obj *)0);
		    obj = (struct obj *) 0;
		}

	    } else {
		obj->age -= how_long;
		begin_burn(obj, TRUE);
	    }
	    return;
	}

	/* only interested in INVENT, FLOOR, and MINVENT */
	if (get_obj_location(obj, &x, &y, 0)) {
	    canseeit = !Blind && cansee(x, y);
	    /* set up `whose[]' to be "Your" or "Fred's" or "The goblin's" */
	    (void) Shk_Your(whose, obj, (char *)0);
	} else {
	    canseeit = FALSE;
	}
	need_newsym = FALSE;

	/* obj->age is the age remaining at this point.  */
	switch (obj->otyp) {
	    case POT_OIL:
		    /* this should only be called when we run out */
		    if (canseeit) {
			switch (obj->where) {
			    case OBJ_INVENT:
			    case OBJ_MINVENT:
				pline("%s se ha quemado.", whose);
				break;
			    case OBJ_FLOOR:
				pline("Ves una pocion de huelo dejando de quemar.");
				need_newsym = TRUE;
				break;
			}
		    }
		    end_burn(obj, FALSE);	/* turn off light source */
		    obj_extract_self(obj);
		    obfree(obj, (struct obj *)0);
		    obj = (struct obj *) 0;
		    break;

	    case BRASS_LANTERN:
	    case OIL_LAMP:
		switch((int)obj->age) {
		    case 150:
		    case 100:
		    case 50:
			if (canseeit) {
			    if (obj->otyp == BRASS_LANTERN)
				lantern_message(obj);
			    else
				see_lamp_flicker(obj,
				    obj->age == 50L ? " considerablemente" : "");
			}
			break;

		    case 25:
			if (canseeit) {
			    if (obj->otyp == BRASS_LANTERN)
				lantern_message(obj);
			    else {
				switch (obj->where) {
				    case OBJ_INVENT:
				    case OBJ_MINVENT:
					pline("%s parece al punto de extinguirse.", whose);
					break;
				    case OBJ_FLOOR:
					pline("Ves %s al punto de extinguirse.",
					    an(xname(obj)));
					break;
				}
			    }
			}
			break;

		    case 0:
			/* even if blind you'll know if holding it */
			if (canseeit || obj->where == OBJ_INVENT) {
			    switch (obj->where) {
				case OBJ_INVENT:
				case OBJ_MINVENT:
				    if (obj->otyp == BRASS_LANTERN)
					pline("%s ha usado todo su poder.",
					    whose);
				    else
					pline("%s %s se ha extinguido.",
					    whose, xname(obj));
				    break;
				case OBJ_FLOOR:
				    if (obj->otyp == BRASS_LANTERN)
					pline("Ves una linterna extinguiéndose.");
				    else
					pline("Ves %s extinguiéndose.",
					    an(xname(obj)));
				    break;
			    }
			}
			end_burn(obj, FALSE);
			break;

		    default:
			/*
			 * Someone added fuel to the lamp while it was
			 * lit.  Just fall through and let begin burn
			 * handle the new age.
			 */
			break;
		}

		if (obj->age)
		    begin_burn(obj, TRUE);

		break;

	    case CANDELABRUM_OF_INVOCATION:
	    case TALLOW_CANDLE:
	    case WAX_CANDLE:
		switch (obj->age) {
		    case 75:
			if (canseeit)
			    switch (obj->where) {
				case OBJ_INVENT:
				    pline("La%s vela%s%s se está%s hiciendo corta%s.",
					  many ? "s" : "",
					  many ? "s" : "",
					  menorah ? " del candelabro" : "",
					  many ? "n" : "",
					  many ? "s" : "");
				    break;
				case OBJ_MINVENT:
				    pline("La%s vela%s%s %s se está%s hiciendo corta%s.",
					  many ? "s" : "",
					  many ? "s" : "",
					  menorah ? " del candelabro" : "",
					  del(mon_nam(obj->ocarry)),
					  many ? "n" : "",
					  many ? "s" : "");
				    break;
				case OBJ_FLOOR:
				    pline("Ves la%s vela%s%s hiciendose corta%s.",
					  many ? "s" : "",
					  many ? "s" : "",
					  menorah ? " de un candelabro" : "",
					  many ? "s" : "");
				    break;
			    }
			break;

		    case 15:
			if (canseeit)
			    switch (obj->where) {
				case OBJ_INVENT:
				    pline("La%s vela%s%s tembla%s abajo.",
					  many ? "s" : "",
					  many ? "s" : "",
					  menorah ? " del candelabro" : "",
					  many ? "n" : "");
				    break;
				case OBJ_MINVENT:
				    pline("La%s vela%s%s %s tembla%s abajo.",
					  many ? "s" : "",
					  many ? "s" : "",
					  menorah ? " del candelabro" : "",
					  del(mon_nam(obj->ocarry)),
					  many ? "n" : "");
				    break;
				case OBJ_FLOOR:
				    pline("Ves la%s vela%s%s temblando abajo.",
					  many ? "s" : "",
					  many ? "s" : "",
					  menorah ? " de un candelabro" : "");
				    break;
			    }
			break;

		    case 0:
			/* we know even if blind and in our inventory */
			if (canseeit || obj->where == OBJ_INVENT) {
			    if (menorah) {
				switch (obj->where) {
				    case OBJ_INVENT:
					pline("La%s llama%s del candelabro se muere%s.",
						many ? "s" : "",
						many ? "s" : "",
						many ? "n" : "");
					break;
				    case OBJ_MINVENT:
					pline("La%s llama%s del candelabro %s se muere%s.",
						many ? "s" : "",
						many ? "s" : "",
						del(mon_nam(obj->ocarry)),
						many ? "n" : "");
					break;
				    case OBJ_FLOOR:
					pline("Ves la%s llama%s del candelabro muriéndose.",
						many ? "s" : "",
						many ? "s" : "");
					break;
				}
			    } else {
				switch (obj->where) {
				    case OBJ_INVENT:
				    case OBJ_MINVENT:
					pline("¡Se consume%s %s!",
					    many ? "n" : "",
					    xname(obj));
					break;
				    case OBJ_FLOOR:
					/*
					You see some wax candles consumed!
					You see a wax candle consumed!
					*/
					pline("¡Ves una%s %s consumiéndose!",
					    many ? "s" : "",
					    xname(obj));
					need_newsym = TRUE;
					break;
				}

				/* post message */
				pline(Hallucination ?
					(many ? "¡Grita!" :
						"¡Gritan!") :
					Blind ? "" :
					    (many ? "Se mueren las llamas." :
						    "Se muere la llama."));
			    }
			}
			end_burn(obj, FALSE);

			if (menorah) {
			    obj->spe = 0;
			} else {
			    obj_extract_self(obj);
			    obfree(obj, (struct obj *)0);
			    obj = (struct obj *) 0;
			}
			break;

		    default:
			/*
			 * Someone added fuel (candles) to the menorah while
			 * it was lit.  Just fall through and let begin burn
			 * handle the new age.
			 */
			break;
		}

		if (obj && obj->age)
		    begin_burn(obj, TRUE);

		break;

	    default:
		impossible("burn_object: obj inesperado %s", xname(obj));
		break;
	}
	if (need_newsym) newsym(x, y);
}

/*
 * Start a burn timeout on the given object. If not "already lit" then
 * create a light source for the vision system.  There had better not
 * be a burn already running on the object.
 *
 * Magic lamps stay lit as long as there's a genie inside, so don't start
 * a timer.
 *
 * Burn rules:
 *	potions of oil, lamps & candles:
 *		age = # of turns of fuel left
 *		spe = <unused>
 *
 *	magic lamps:
 *		age = <unused>
 *		spe = 0 not lightable, 1 lightable forever
 *
 *	candelabrum:
 *		age = # of turns of fuel left
 *		spe = # of candles
 *
 * Once the burn begins, the age will be set to the amount of fuel
 * remaining _once_the_burn_finishes_.  If the burn is terminated
 * early then fuel is added back.
 *
 * This use of age differs from the use of age for corpses and eggs.
 * For the latter items, age is when the object was created, so we
 * know when it becomes "bad".
 *
 * This is a "silent" routine - it should not print anything out.
 */
void
begin_burn(obj, already_lit)
	struct obj *obj;
	boolean already_lit;
{
	int radius = 3;
	long turns = 0;
	boolean do_timer = TRUE;

	if (obj->age == 0 && obj->otyp != MAGIC_LAMP && !artifact_light(obj))
	    return;

	switch (obj->otyp) {
	    case MAGIC_LAMP:
		obj->lamplit = 1;
		do_timer = FALSE;
		break;

	    case POT_OIL:
		turns = obj->age;
		radius = 1;	/* very dim light */
		break;

	    case BRASS_LANTERN:
	    case OIL_LAMP:
		/* magic times are 150, 100, 50, 25, and 0 */
		if (obj->age > 150L)
		    turns = obj->age - 150L;
		else if (obj->age > 100L)
		    turns = obj->age - 100L;
		else if (obj->age > 50L)
		    turns = obj->age - 50L;
		else if (obj->age > 25L)
		    turns = obj->age - 25L;
		else
		    turns = obj->age;
		break;

	    case CANDELABRUM_OF_INVOCATION:
	    case TALLOW_CANDLE:
	    case WAX_CANDLE:
		/* magic times are 75, 15, and 0 */
		if (obj->age > 75L)
		    turns = obj->age - 75L;
		else if (obj->age > 15L)
		    turns = obj->age - 15L;
		else
		    turns = obj->age;
		radius = candle_light_range(obj);
		break;

	    default:
                /* [ALI] Support artifact light sources */
                if (artifact_light(obj)) {
		    obj->lamplit = 1;
		    do_timer = FALSE;
		    radius = 2;
		} else {
		    const char *que = xname(obj);
		    impossible("begin burn: %s inesperad%c%s",
				que, isfeminine(que)? 'a' : 'o',
				plur(obj->quan));
		    turns = obj->age;
		}
		break;
	}

	if (do_timer) {
	    if (start_timer(turns, TIMER_OBJECT,
					BURN_OBJECT, (genericptr_t)obj)) {
		obj->lamplit = 1;
		obj->age -= turns;
		if (carried(obj) && !already_lit)
		    update_inventory();
	    } else {
		obj->lamplit = 0;
	    }
	} else {
	    if (carried(obj) && !already_lit)
		update_inventory();
	}

	if (obj->lamplit && !already_lit) {
	    xchar x, y;

	    if (get_obj_location(obj, &x, &y, CONTAINED_TOO|BURIED_TOO))
		new_light_source(x, y, radius, LS_OBJECT, (genericptr_t) obj);
	    else
		impossible("begin_burn: no puedo obtener posición del obj");
	}
}

/*
 * Stop a burn timeout on the given object if timer attached.  Darken
 * light source.
 */
void
end_burn(obj, timer_attached)
	struct obj *obj;
	boolean timer_attached;
{
	if (!obj->lamplit) {
	    const char *que = xname(obj);
	    impossible("end_burn: obj %s no %s alumbrad%c%s",
			que, otense(obj, "está"),
			isfeminine(que)? 'a' : 'o', plur(obj->quan));
	    return;
	}

	if (obj->otyp == MAGIC_LAMP || artifact_light(obj))
	    timer_attached = FALSE;

	if (!timer_attached) {
	    /* [DS] Cleanup explicitly, since timer cleanup won't happen */
	    del_light_source(LS_OBJECT, (genericptr_t)obj);
	    obj->lamplit = 0;
	    if (obj->where == OBJ_INVENT)
		update_inventory();
	} else if (!stop_timer(BURN_OBJECT, (genericptr_t) obj))
	    impossible("end_burn: ¡obj %s no tiene temporizador!", xname(obj));
}

#endif /* OVL1 */
#ifdef OVL0

/*
 * Cleanup a burning object if timer stopped.
 */
static void
cleanup_burn(arg, expire_time)
    genericptr_t arg;
    long expire_time;
{
    struct obj *obj = (struct obj *)arg;
    if (!obj->lamplit) {
	const char *que = xname(obj);
	impossible("cleanup_burn: obj %s no %s alumbrad%c%s",
		    que, otense(obj, "está"),
		    isfeminine(que)? 'a' : 'o', plur(obj->quan));
	return;
    }

    del_light_source(LS_OBJECT, arg);

    /* restore unused time */
    obj->age += expire_time - monstermoves;

    obj->lamplit = 0;

    if (obj->where == OBJ_INVENT)
	update_inventory();
}

#endif /* OVL0 */
#ifdef OVL1

void
do_storms()
{
    int nstrike;
    register int x, y;
    int dirx, diry;
    int count;

    /* no lightning if not the air level or too often, even then */
    if(!Is_airlevel(&u.uz) || rn2(8))
	return;

    /* the number of strikes is 8-log2(nstrike) */
    for(nstrike = rnd(64); nstrike <= 64; nstrike *= 2) {
	count = 0;
	do {
	    x = rnd(COLNO-1);
	    y = rn2(ROWNO);
	} while (++count < 100 && levl[x][y].typ != CLOUD);

	if(count < 100) {
	    dirx = rn2(3) - 1;
	    diry = rn2(3) - 1;
	    if(dirx != 0 || diry != 0)
		buzz(-15, /* "monster" LIGHTNING spell */
		     8, x, y, dirx, diry);
	}
    }

    if(levl[u.ux][u.uy].typ == CLOUD) {
	/* inside a cloud during a thunder storm is deafening */
	pline("¡¡Pum!!!  ¡¡Bum!!  ¡¡Bum!!");
	if(!u.uinvulnerable) {
	    stop_occupation();
	    nomul(-3);
	}
    } else
	Oyes("un ruido retumbante.");
}
#endif /* OVL1 */


#ifdef OVL0
/* ------------------------------------------------------------------------- */
/*
 * Generic Timeout Functions.
 *
 * Interface:
 *
 * General:
 *	boolean start_timer(long timeout,short kind,short func_index,
 *							genericptr_t arg)
 *		Start a timer of kind 'kind' that will expire at time
 *		monstermoves+'timeout'.  Call the function at 'func_index'
 *		in the timeout table using argument 'arg'.  Return TRUE if
 *		a timer was started.  This places the timer on a list ordered
 *		"sooner" to "later".  If an object, increment the object's
 *		timer count.
 *
 *	long stop_timer(short func_index, genericptr_t arg)
 *		Stop a timer specified by the (func_index, arg) pair.  This
 *		assumes that such a pair is unique.  Return the time the
 *		timer would have gone off.  If no timer is found, return 0.
 *		If an object, decrement the object's timer count.
 *
 *	void run_timers(void)
 *		Call timers that have timed out.
 *
 *
 * Save/Restore:
 *	void save_timers(int fd, int mode, int range)
 *		Save all timers of range 'range'.  Range is either global
 *		or local.  Global timers follow game play, local timers
 *		are saved with a level.  Object and monster timers are
 *		saved using their respective id's instead of pointers.
 *
 *	void restore_timers(int fd, int range, boolean ghostly, long adjust)
 *		Restore timers of range 'range'.  If from a ghost pile,
 *		adjust the timeout by 'adjust'.  The object and monster
 *		ids are not restored until later.
 *
 *	void relink_timers(boolean ghostly)
 *		Relink all object and monster timers that had been saved
 *		using their object's or monster's id number.
 *
 * Object Specific:
 *	void obj_move_timers(struct obj *src, struct obj *dest)
 *		Reassign all timers from src to dest.
 *
 *	void obj_split_timers(struct obj *src, struct obj *dest)
 *		Duplicate all timers assigned to src and attach them to dest.
 *
 *	void obj_stop_timers(struct obj *obj)
 *		Stop all timers attached to obj.
 */


#ifdef WIZARD
STATIC_DCL const char *FDECL(kind_name, (SHORT_P));
STATIC_DCL void FDECL(print_queue, (winid, timer_element *));
#endif
STATIC_DCL void FDECL(insert_timer, (timer_element *));
STATIC_DCL timer_element *FDECL(remove_timer, (timer_element **, SHORT_P,
								genericptr_t));
STATIC_DCL void FDECL(write_timer, (int, timer_element *));
STATIC_DCL boolean FDECL(mon_is_local, (struct monst *));
STATIC_DCL boolean FDECL(timer_is_local, (timer_element *));
STATIC_DCL int FDECL(maybe_write_timer, (int, int, BOOLEAN_P));

/* ordered timer list */
static timer_element *timer_base;		/* "active" */
static unsigned long timer_id = 1;

/* If defined, then include names when printing out the timer queue */
#define VERBOSE_TIMER

typedef struct {
    timeout_proc f, cleanup;
#ifdef VERBOSE_TIMER
    const char *name;
# define TTAB(a, b, c) {a,b,c}
#else
# define TTAB(a, b, c) {a,b}
#endif
} ttable;

/* table of timeout functions */
static const ttable timeout_funcs[NUM_TIME_FUNCS] = {
    TTAB(rot_organic,	(timeout_proc)0,	"rot_organic"),
    TTAB(rot_corpse,	(timeout_proc)0,	"rot_corpse"),
    TTAB(revive_mon,	(timeout_proc)0,	"revive_mon"),
    TTAB(burn_object,	cleanup_burn,		"burn_object"),
    TTAB(hatch_egg,	(timeout_proc)0,	"hatch_egg"),
    TTAB(fig_transform,	(timeout_proc)0,	"fig_transform")
};
#undef TTAB


#if defined(WIZARD)

STATIC_OVL const char *
kind_name(kind)
    short kind;
{
    switch (kind) {
	case TIMER_LEVEL: return "nivel";
	case TIMER_GLOBAL: return "global";
	case TIMER_OBJECT: return "objeto";
	case TIMER_MONSTER: return "monstruo";
    }
    return "desconocido";
}

STATIC_OVL void
print_queue(win, base)
    winid win;
    timer_element *base;
{
    timer_element *curr;
    char buf[BUFSZ], arg_address[20];

    if (!base) {
	putstr(win, 0, "<vacío>");
    } else {
	putstr(win, 0, "timeout  id   tipo   llamar");
	for (curr = base; curr; curr = curr->next) {
#ifdef VERBOSE_TIMER
	    Sprintf(buf, " %4ld   %4ld  %-6s %s(%s)",
		curr->timeout, curr->tid, kind_name(curr->kind),
		timeout_funcs[curr->func_index].name,
		fmt_ptr((genericptr_t)curr->arg, arg_address));
#else
	    Sprintf(buf, " %4ld   %4ld  %-6s #%d(%s)",
		curr->timeout, curr->tid, kind_name(curr->kind),
		curr->func_index,
		fmt_ptr((genericptr_t)curr->arg, arg_address));
#endif
	    putstr(win, 0, buf);
	}
    }
}

int
wiz_timeout_queue()
{
    winid win;
    char buf[BUFSZ];

    win = create_nhwindow(NHW_MENU);	/* corner text window */
    if (win == WIN_ERR) return 0;

    Sprintf(buf, "Tiempo actual = %ld.", monstermoves);
    putstr(win, 0, buf);
    putstr(win, 0, "");
    putstr(win, 0, "Cola activa de tiempo:");
    putstr(win, 0, "");
    print_queue(win, timer_base);

    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);

    return 0;
}

void
timer_sanity_check()
{
    timer_element *curr;
    char obj_address[20];

    /* this should be much more complete */
    for (curr = timer_base; curr; curr = curr->next)
	if (curr->kind == TIMER_OBJECT) {
	    struct obj *obj = (struct obj *) curr->arg;
	    if (obj->timed == 0) {
		pline("temporizador razón: obj %s sin temporizador, temporizador %ld",
		      fmt_ptr((genericptr_t)obj, obj_address), curr->tid);
	    }
	}
}

#endif /* WIZARD */


/*
 * Pick off timeout elements from the global queue and call their functions.
 * Do this until their time is less than or equal to the move count.
 */
void
run_timers()
{
    timer_element *curr;

    /*
     * Always use the first element.  Elements may be added or deleted at
     * any time.  The list is ordered, we are done when the first element
     * is in the future.
     */
    while (timer_base && timer_base->timeout <= monstermoves) {
	curr = timer_base;
	timer_base = curr->next;

	if (curr->kind == TIMER_OBJECT) ((struct obj *)(curr->arg))->timed--;
	(*timeout_funcs[curr->func_index].f)(curr->arg, curr->timeout);
	free((genericptr_t) curr);
    }
}


/*
 * Start a timer.  Return TRUE if successful.
 */
boolean
start_timer(when, kind, func_index, arg)
long when;
short kind;
short func_index;
genericptr_t arg;
{
    timer_element *gnu;

    if (func_index < 0 || func_index >= NUM_TIME_FUNCS)
	panic("start_timer");

    gnu = (timer_element *) alloc(sizeof(timer_element));
    gnu->next = 0;
    gnu->tid = timer_id++;
    gnu->timeout = monstermoves + when;
    gnu->kind = kind;
    gnu->needs_fixup = 0;
    gnu->func_index = func_index;
    gnu->arg = arg;
    insert_timer(gnu);

    if (kind == TIMER_OBJECT)	/* increment object's timed count */
	((struct obj *)arg)->timed++;

    /* should check for duplicates and fail if any */
    return TRUE;
}


/*
 * Remove the timer from the current list and free it up.  Return the time
 * it would have gone off, 0 if not found.
 */
long
stop_timer(func_index, arg)
short func_index;
genericptr_t arg;
{
    timer_element *doomed;
    long timeout;

    doomed = remove_timer(&timer_base, func_index, arg);

    if (doomed) {
	timeout = doomed->timeout;
	if (doomed->kind == TIMER_OBJECT)
	    ((struct obj *)arg)->timed--;
	if (timeout_funcs[doomed->func_index].cleanup)
	    (*timeout_funcs[doomed->func_index].cleanup)(arg, timeout);
	free((genericptr_t) doomed);
	return timeout;
    }
    return 0;
}


/*
 * Move all object timers from src to dest, leaving src untimed.
 */
void
obj_move_timers(src, dest)
    struct obj *src, *dest;
{
    int count;
    timer_element *curr;

    for (count = 0, curr = timer_base; curr; curr = curr->next)
	if (curr->kind == TIMER_OBJECT && curr->arg == (genericptr_t)src) {
	    curr->arg = (genericptr_t) dest;
	    dest->timed++;
	    count++;
	}
    if (count != src->timed)
	panic("obj_move_timers");
    src->timed = 0;
}


/*
 * Find all object timers and duplicate them for the new object "dest".
 */
void
obj_split_timers(src, dest)
    struct obj *src, *dest;
{
    timer_element *curr, *next_timer=0;

    for (curr = timer_base; curr; curr = next_timer) {
	next_timer = curr->next;	/* things may be inserted */
	if (curr->kind == TIMER_OBJECT && curr->arg == (genericptr_t)src) {
	    (void) start_timer(curr->timeout-monstermoves, TIMER_OBJECT,
					curr->func_index, (genericptr_t)dest);
	}
    }
}


/*
 * Stop all timers attached to this object.  We can get away with this because
 * all object pointers are unique.
 */
void
obj_stop_timers(obj)
    struct obj *obj;
{
    timer_element *curr, *prev, *next_timer=0;

    for (prev = 0, curr = timer_base; curr; curr = next_timer) {
	next_timer = curr->next;
	if (curr->kind == TIMER_OBJECT && curr->arg == (genericptr_t)obj) {
	    if (prev)
		prev->next = curr->next;
	    else
		timer_base = curr->next;
	    if (timeout_funcs[curr->func_index].cleanup)
		(*timeout_funcs[curr->func_index].cleanup)(curr->arg,
			curr->timeout);
	    free((genericptr_t) curr);
	} else {
	    prev = curr;
	}
    }
    obj->timed = 0;
}


/* Insert timer into the global queue */
STATIC_OVL void
insert_timer(gnu)
    timer_element *gnu;
{
    timer_element *curr, *prev;

    for (prev = 0, curr = timer_base; curr; prev = curr, curr = curr->next)
	if (curr->timeout >= gnu->timeout) break;

    gnu->next = curr;
    if (prev)
	prev->next = gnu;
    else
	timer_base = gnu;
}


STATIC_OVL timer_element *
remove_timer(base, func_index, arg)
timer_element **base;
short func_index;
genericptr_t arg;
{
    timer_element *prev, *curr;

    for (prev = 0, curr = *base; curr; prev = curr, curr = curr->next)
	if (curr->func_index == func_index && curr->arg == arg) break;

    if (curr) {
	if (prev)
	    prev->next = curr->next;
	else
	    *base = curr->next;
    }

    return curr;
}


STATIC_OVL void
write_timer(fd, timer)
    int fd;
    timer_element *timer;
{
    genericptr_t arg_save;

    switch (timer->kind) {
	case TIMER_GLOBAL:
	case TIMER_LEVEL:
	    /* assume no pointers in arg */
	    bwrite(fd, (genericptr_t) timer, sizeof(timer_element));
	    break;

	case TIMER_OBJECT:
	    if (timer->needs_fixup)
		bwrite(fd, (genericptr_t)timer, sizeof(timer_element));
	    else {
		/* replace object pointer with id */
		arg_save = timer->arg;
		timer->arg = (genericptr_t)((struct obj *)timer->arg)->o_id;
		timer->needs_fixup = 1;
		bwrite(fd, (genericptr_t)timer, sizeof(timer_element));
		timer->arg = arg_save;
		timer->needs_fixup = 0;
	    }
	    break;

	case TIMER_MONSTER:
	    if (timer->needs_fixup)
		bwrite(fd, (genericptr_t)timer, sizeof(timer_element));
	    else {
		/* replace monster pointer with id */
		arg_save = timer->arg;
		timer->arg = (genericptr_t)((struct monst *)timer->arg)->m_id;
		timer->needs_fixup = 1;
		bwrite(fd, (genericptr_t)timer, sizeof(timer_element));
		timer->arg = arg_save;
		timer->needs_fixup = 0;
	    }
	    break;

	default:
	    panic("write_timer");
	    break;
    }
}


/*
 * Return TRUE if the object will stay on the level when the level is
 * saved.
 */
boolean
obj_is_local(obj)
    struct obj *obj;
{
    switch (obj->where) {
	case OBJ_INVENT:
	case OBJ_MIGRATING:	return FALSE;
	case OBJ_FLOOR:
	case OBJ_BURIED:	return TRUE;
	case OBJ_CONTAINED:	return obj_is_local(obj->ocontainer);
	case OBJ_MINVENT:	return mon_is_local(obj->ocarry);
    }
    panic("obj_is_local");
    return FALSE;
}


/*
 * Return TRUE if the given monster will stay on the level when the
 * level is saved.
 */
STATIC_OVL boolean
mon_is_local(mon)
struct monst *mon;
{
    struct monst *curr;

    for (curr = migrating_mons; curr; curr = curr->nmon)
	if (curr == mon) return FALSE;
    /* `mydogs' is used during level changes, never saved and restored */
    for (curr = mydogs; curr; curr = curr->nmon)
	if (curr == mon) return FALSE;
    return TRUE;
}


/*
 * Return TRUE if the timer is attached to something that will stay on the
 * level when the level is saved.
 */
STATIC_OVL boolean
timer_is_local(timer)
    timer_element *timer;
{
    switch (timer->kind) {
	case TIMER_LEVEL:	return TRUE;
	case TIMER_GLOBAL:	return FALSE;
	case TIMER_OBJECT:	return obj_is_local((struct obj *)timer->arg);
	case TIMER_MONSTER:	return mon_is_local((struct monst *)timer->arg);
    }
    panic("timer_is_local");
    return FALSE;
}


/*
 * Part of the save routine.  Count up the number of timers that would
 * be written.  If write_it is true, actually write the timer.
 */
STATIC_OVL int
maybe_write_timer(fd, range, write_it)
    int fd, range;
    boolean write_it;
{
    int count = 0;
    timer_element *curr;

    for (curr = timer_base; curr; curr = curr->next) {
	if (range == RANGE_GLOBAL) {
	    /* global timers */

	    if (!timer_is_local(curr)) {
		count++;
		if (write_it) write_timer(fd, curr);
	    }

	} else {
	    /* local timers */

	    if (timer_is_local(curr)) {
		count++;
		if (write_it) write_timer(fd, curr);
	    }

	}
    }

    return count;
}


/*
 * Save part of the timer list.  The parameter 'range' specifies either
 * global or level timers to save.  The timer ID is saved with the global
 * timers.
 *
 * Global range:
 *		+ timeouts that follow the hero (global)
 *		+ timeouts that follow obj & monst that are migrating
 *
 * Level range:
 *		+ timeouts that are level specific (e.g. storms)
 *		+ timeouts that stay with the level (obj & monst)
 */
void
save_timers(fd, mode, range)
    int fd, mode, range;
{
    timer_element *curr, *prev, *next_timer=0;
    int count;

    if (perform_bwrite(mode)) {
	if (range == RANGE_GLOBAL)
	    bwrite(fd, (genericptr_t) &timer_id, sizeof(timer_id));

	count = maybe_write_timer(fd, range, FALSE);
	bwrite(fd, (genericptr_t) &count, sizeof count);
	(void) maybe_write_timer(fd, range, TRUE);
    }

    if (release_data(mode)) {
	for (prev = 0, curr = timer_base; curr; curr = next_timer) {
	    next_timer = curr->next;	/* in case curr is removed */

	    if ( !(!!(range == RANGE_LEVEL) ^ !!timer_is_local(curr)) ) {
		if (prev)
		    prev->next = curr->next;
		else
		    timer_base = curr->next;
		free((genericptr_t) curr);
		/* prev stays the same */
	    } else {
		prev = curr;
	    }
	}
    }
}


/*
 * Pull in the structures from disk, but don't recalculate the object and
 * monster pointers.
 */
void
restore_timers(fd, range, ghostly, adjust)
    int fd, range;
    boolean ghostly;	/* restoring from a ghost level */
    long adjust;	/* how much to adjust timeout */
{
    int count;
    timer_element *curr;

    if (range == RANGE_GLOBAL)
	mread(fd, (genericptr_t) &timer_id, sizeof timer_id);

    /* restore elements */
    mread(fd, (genericptr_t) &count, sizeof count);
    while (count-- > 0) {
	curr = (timer_element *) alloc(sizeof(timer_element));
	mread(fd, (genericptr_t) curr, sizeof(timer_element));
	if (ghostly)
	    curr->timeout += adjust;
	insert_timer(curr);
    }
}


/* reset all timers that are marked for reseting */
void
relink_timers(ghostly)
    boolean ghostly;
{
    timer_element *curr;
    unsigned nid;

    for (curr = timer_base; curr; curr = curr->next) {
	if (curr->needs_fixup) {
	    if (curr->kind == TIMER_OBJECT) {
		if (ghostly) {
		    if (!lookup_id_mapping((unsigned)curr->arg, &nid))
			panic("relink_timers 1");
		} else
		    nid = (unsigned) curr->arg;
		curr->arg = (genericptr_t) find_oid(nid);
		if (!curr->arg) panic("cant find o_id %d", nid);
		curr->needs_fixup = 0;
	    } else if (curr->kind == TIMER_MONSTER) {
		panic("relink_timers: no monster timer implemented");
	    } else
		panic("relink_timers 2");
	}
    }
}

#endif /* OVL0 */

/*timeout.c*/
