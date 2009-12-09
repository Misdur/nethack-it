/*	SCCS Id: @(#)wield.c	3.4	2003/01/29	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* KMH -- Differences between the three weapon slots.
 *
 * The main weapon (uwep):
 * 1.  Is filled by the (w)ield command.
 * 2.  Can be filled with any type of item.
 * 3.  May be carried in one or both hands.
 * 4.  Is used as the melee weapon and as the launcher for
 *     ammunition.
 * 5.  Only conveys intrinsics when it is a weapon, weapon-tool,
 *     or artifact.
 * 6.  Certain cursed items will weld to the hand and cannot be
 *     unwielded or dropped.  See erodeable_wep() and will_weld()
 *     below for the list of which items apply.
 *
 * The secondary weapon (uswapwep):
 * 1.  Is filled by the e(x)change command, which swaps this slot
 *     with the main weapon.  If the "pushweapon" option is set,
 *     the (w)ield command will also store the old weapon in the
 *     secondary slot.
 * 2.  Can be field with anything that will fit in the main weapon
 *     slot; that is, any type of item.
 * 3.  Is usually NOT considered to be carried in the hands.
 *     That would force too many checks among the main weapon,
 *     second weapon, shield, gloves, and rings; and it would
 *     further be complicated by bimanual weapons.  A special
 *     exception is made for two-weapon combat.
 * 4.  Is used as the second weapon for two-weapon combat, and as
 *     a convenience to swap with the main weapon.
 * 5.  Never conveys intrinsics.
 * 6.  Cursed items never weld (see #3 for reasons), but they also
 *     prevent two-weapon combat.
 *
 * The quiver (uquiver):
 * 1.  Is filled by the (Q)uiver command.
 * 2.  Can be filled with any type of item.
 * 3.  Is considered to be carried in a special part of the pack.
 * 4.  Is used as the item to throw with the (f)ire command.
 *     This is a convenience over the normal (t)hrow command.
 * 5.  Never conveys intrinsics.
 * 6.  Cursed items never weld; their effect is handled by the normal
 *     throwing code.
 *
 * No item may be in more than one of these slots.
 */


STATIC_DCL int FDECL(ready_weapon, (struct obj *));

/* used by will_weld() */
/* probably should be renamed */
#define erodeable_wep(optr)	((optr)->oclass == WEAPON_CLASS \
				|| is_weptool(optr) \
				|| (optr)->otyp == HEAVY_IRON_BALL \
				|| (optr)->otyp == IRON_CHAIN)

/* used by welded(), and also while wielding */
#define will_weld(optr)		((optr)->cursed \
				&& (erodeable_wep(optr) \
				   || (optr)->otyp == TIN_OPENER))


/*** Functions that place a given item in a slot ***/
/* Proper usage includes:
 * 1.  Initializing the slot during character generation or a
 *     restore.
 * 2.  Setting the slot due to a player's actions.
 * 3.  If one of the objects in the slot are split off, these
 *     functions can be used to put the remainder back in the slot.
 * 4.  Putting an item that was thrown and returned back into the slot.
 * 5.  Emptying the slot, by passing a null object.  NEVER pass
 *     zeroobj!
 *
 * If the item is being moved from another slot, it is the caller's
 * responsibility to handle that.  It's also the caller's responsibility
 * to print the appropriate messages.
 */
void
setuwep(obj)
register struct obj *obj;
{
	struct obj *olduwep = uwep;

	if (obj == uwep) return; /* necessary to not set unweapon */
	/* This message isn't printed in the caller because it happens
	 * *whenever* Sunsword is unwielded, from whatever cause.
	 */
	setworn(obj, W_WEP);
	if (uwep == obj && artifact_light(olduwep) && olduwep->lamplit) {
	    end_burn(olduwep, FALSE);
	    if (!Blind) pline("%s de brillar.", Tobjnam(olduwep, "deja"));
	}
	/* Note: Explicitly wielding a pick-axe will not give a "bashing"
	 * message.  Wielding one via 'a'pplying it will.
	 * 3.2.2:  Wielding arbitrary objects will give bashing message too.
	 */
	if (obj) {
		unweapon = (obj->oclass == WEAPON_CLASS) ?
				is_launcher(obj) || is_ammo(obj) ||
				is_missile(obj) || (is_pole(obj)
#ifdef STEED
				&& !u.usteed
#endif
				) : !is_weptool(obj);
	} else
		unweapon = TRUE;	/* for "bare hands" message */
	update_inventory();
}

STATIC_OVL int
ready_weapon(wep)
struct obj *wep;
{
	/* Separated function so swapping works easily */
	int res = 0;

	if (!wep) {
	    /* No weapon */
	    if (uwep) {
		const char *mano = body_part(HAND);
		pline("Tienes %s vac�%cs.", makeplural(mano),
			isfeminine(mano)? 'a' : 'o');
		setuwep((struct obj *) 0);
		res++;
	    } else {
		const char *mano = body_part(HAND);
		pline("Ya tienes %s vac�%cs.", makeplural(mano),
			isfeminine(mano)? 'a' : 'o');
	    }
	} else if (!uarmg && !Stone_resistance && wep->otyp == CORPSE
				&& touch_petrifies(&mons[wep->corpsenm])) {
	    /* Prevent wielding cockatrice when not wearing gloves --KAA */
	    char kbuf[BUFSZ];
	    const char *mano = body_part(HAND);

	    pline("Blandes el cad�ver de %s en tus %s desnud%cs.",
		  mons[wep->corpsenm].mname, makeplural(mano),
		  isfeminine(mano)? 'a' : 'o');
	    Sprintf(kbuf, "cad�ver de %s", mons[wep->corpsenm].mname);
	    instapetrify(kbuf);
	} else if (uarms && bimanual(wep))
	    No_puedes("blandir una %s de dos manos mientras lleves escudo.",
		      is_sword(wep) ? "espada" :
		          wep->otyp == BATTLE_AXE ? "hacha" : "arma");
	else if (wep->oartifact && !touch_artifact(wep, &youmonst)) {
	    res++;	/* takes a turn even though it doesn't get wielded */
	} else {
	    /* Weapon WILL be wielded after this point */
	    res++;
	    if (will_weld(wep)) {
		const char *tmp = xname(wep),
			   *elstr  = "El ",
			   *lastr  = "La ",
			   *losstr = "Los ",
			   *lasstr = "Las ";
		const char *thetmp = The(tmp);
		if (strncmp(tmp, elstr, 3) && !strncmp(thetmp, elstr, 3))
		    tmp = elstr;
		else if (strncmp(tmp, lastr, 3) && !strncmp(thetmp, lastr, 3))
		    tmp = lastr;
		else if (strncmp(tmp, losstr, 4) && !strncmp(thetmp, losstr, 4))
		    tmp = losstr;
		else if (strncmp(tmp, lasstr, 4) && !strncmp(thetmp, lasstr, 4))
		    tmp = lasstr;
		else tmp = "";
		pline("�%s%s a tu%s %s!", tmp, aobjnam(wep, "se suelda"),
			bimanual(wep) ? "s" : "",
			bimanual(wep) ?
				(const char *)makeplural(body_part(HAND))
				: body_part(HAND));
		wep->bknown = TRUE;
	    } else {
		/* The message must be printed before setuwep (since
		 * you might die and be revived from changing weapons),
		 * and the message must be before the death message and
		 * Lifesaved rewielding.  Yet we want the message to
		 * say "weapon in hand", thus this kludge.
		 */
		long dummy = wep->owornmask;
		wep->owornmask |= W_WEP;
		prinv((char *)0, wep, 0L);
		wep->owornmask = dummy;
	    }
	    setuwep(wep);

	    /* KMH -- Talking artifacts are finally implemented */
	    arti_speak(wep);

	    if (artifact_light(wep) && !wep->lamplit) {
		begin_burn(wep, FALSE);
		if (!Blind)
		    pline("�%s a brillar fuerte!", Tobjnam(wep, "empieza"));
	    }

#if 0
	    /* we'll get back to this someday, but it's not balanced yet */
	    if (Race_if(PM_ELF) && !wep->oartifact &&
			    objects[wep->otyp].oc_material == IRON) {
		/* Elves are averse to wielding cold iron */
		pline("Tienes un sentido inc�modo sobre blandir hierro fr�o.");
		change_luck(-1);
	    }
#endif

	    if (wep->unpaid) {
		struct monst *this_shkp;

		if ((this_shkp = shop_keeper(inside_shop(u.ux, u.uy))) !=
		    (struct monst *)0) {
		    pline("%s dice \"�Tenga cuidado con mi %s!\"",
			  shkname(this_shkp),
			  xname(wep));
		}
	    }
	}
	return(res);
}

void
setuqwep(obj)
register struct obj *obj;
{
	setworn(obj, W_QUIVER);
	update_inventory();
}

void
setuswapwep(obj)
register struct obj *obj;
{
	setworn(obj, W_SWAPWEP);
	update_inventory();
}


/*** Commands to change particular slot(s) ***/

static NEARDATA const char wield_objs[] =
	{ ALL_CLASSES, ALLOW_NONE, WEAPON_CLASS, TOOL_CLASS, 0 };
static NEARDATA const char ready_objs[] =
	{ ALL_CLASSES, ALLOW_NONE, WEAPON_CLASS, 0 };
static NEARDATA const char bullets[] =	/* (note: different from dothrow.c) */
	{ ALL_CLASSES, ALLOW_NONE, GEM_CLASS, WEAPON_CLASS, 0 };

int
dowield()
{
	register struct obj *wep, *oldwep;
	int result;

	/* May we attempt this? */
	multi = 0;
	if (cantwield(youmonst.data)) {
		pline("�No seas ridiculos%c!",
			poly_gender()==1? 'a' : 'o');
		return(0);
	}

	/* Prompt for a new weapon */
	if (!(wep = getobj(wield_objs, "", "blandir")))
		/* Cancelled */
		return (0);
	else if (wep == uwep) {
	    pline("�Ya est�s blandiendo esto!");
	    if (is_weptool(wep)) unweapon = FALSE;	/* [see setuwep()] */
		return (0);
	} else if (welded(uwep)) {
		weldmsg(uwep);
		/* previously interrupted armor removal mustn't be resumed */
		reset_remarm();
		return (0);
	}

	/* Handle no object, or object in other slot */
	if (wep == &zeroobj)
		wep = (struct obj *) 0;
	else if (wep == uswapwep)
		return (doswapweapon());
	else if (wep == uquiver)
		setuqwep((struct obj *) 0);
	else if (wep->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL
#ifdef STEED
			| W_SADDLE
#endif
			)) {
		pline("�No puedes blandir esto!");
		return (0);
	}

	/* Set your new primary weapon */
	oldwep = uwep;
	result = ready_weapon(wep);
	if (flags.pushweapon && oldwep && uwep != oldwep)
		setuswapwep(oldwep);
	untwoweapon();

	return (result);
}

int
doswapweapon()
{
	register struct obj *oldwep, *oldswap;
	int result = 0;


	/* May we attempt this? */
	multi = 0;
	if (cantwield(youmonst.data)) {
		pline("�No seas ridiculos%c!",
			poly_gender()==1? 'a' : 'o');
		return(0);
	}
	if (welded(uwep)) {
		weldmsg(uwep);
		return (0);
	}

	/* Unwield your current secondary weapon */
	oldwep = uwep;
	oldswap = uswapwep;
	setuswapwep((struct obj *) 0);

	/* Set your new primary weapon */
	result = ready_weapon(oldswap);

	/* Set your new secondary weapon */
	if (uwep == oldwep)
		/* Wield failed for some reason */
		setuswapwep(oldswap);
	else {
		setuswapwep(oldwep);
		if (uswapwep)
			prinv((char *)0, uswapwep, 0L);
		else
			pline("No tienes arma segunda lista.");
	}

	if (u.twoweap && !can_twoweapon())
		untwoweapon();

	return (result);
}

int
dowieldquiver()
{
	register struct obj *newquiver;
	const char *quivee_types = (uslinging() ||
		  (uswapwep && objects[uswapwep->otyp].oc_skill == P_SLING)) ?
				  bullets : ready_objs;

	/* Since the quiver isn't in your hands, don't check cantwield(), */
	/* will_weld(), touch_petrifies(), etc. */
	multi = 0;

	/* Because 'Q' used to be quit... */
	/* LENGUA:  "salir" doesn't start with Q... */
	if (flags.suppress_alert < FEATURE_NOTICE_VER(3,3,0))
		pline("Nota: Por favor usa #salir si quieres salir del juego.");

	/* Prompt for a new quiver */
	if (!(newquiver = getobj(quivee_types, "", "listar")))
		/* Cancelled */
		return (0);

	/* Handle no object, or object in other slot */
	/* Any type is okay, since we give no intrinsics anyways */
	if (newquiver == &zeroobj) {
		/* Explicitly nothing */
		if (uquiver) {
			pline("Ya no tienes amunici�n lista.");
			setuqwep(newquiver = (struct obj *) 0);
		} else {
			pline("�Ya no tienes amunici�n lista!");
			return(0);
		}
	} else if (newquiver == uquiver) {
		pline("�Esta amunici�n ya est� lista!");
		return(0);
	} else if (newquiver == uwep) {
		/* Prevent accidentally readying the main weapon */
		boolean plu = is_plural(uwep);
		/* FIXME: fix the gender */
		pline("��st%s ya est�%s en uso como una arma!",
		      plu ? "os" : "e",
		      plu ? "n" : "");
		return(0);
	} else if (newquiver->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL
#ifdef STEED
			| W_SADDLE
#endif
			)) {
		No_puedes("�listar esto!");
		return (0);
	} else {
		long dummy;


		/* Check if it's the secondary weapon */
		if (newquiver == uswapwep) {
			setuswapwep((struct obj *) 0);
			untwoweapon();
		}

		/* Okay to put in quiver; print it */
		dummy = newquiver->owornmask;
		newquiver->owornmask |= W_QUIVER;
		prinv((char *)0, newquiver, 0L);
		newquiver->owornmask = dummy;
	}

	/* Finally, place it in the quiver */
	setuqwep(newquiver);
	/* Take no time since this is a convenience slot */
	return (0);
}

/* used for #rub and for applying pick-axe, whip, grappling hook, or polearm */
/* (moved from apply.c) */
boolean
wield_tool(obj, verb_inf, verb_pres)
struct obj *obj;
const char *verb_inf;	/* "frotar",&c */
const char *verb_pres;	/* "frotas",&c */
{
    const char *what;
    boolean more_than_1, fem;

    if (obj == uwep) return TRUE;   /* nothing to do if already wielding it */

    if (!verb_inf) {
	verb_inf  = "esgrimir";
	verb_pres = "esgrimes";
    }
    what = xname(obj);
    more_than_1 = (obj->quan > 1L ||
		   strstri(what, "par de ") != 0 ||
		   strstri(what, "s de ") != 0);
    fem = isfeminine(what);

    if (obj->owornmask & (W_ARMOR|W_RING|W_AMUL|W_TOOL)) {
	char yourbuf[BUFSZ];

	No_puedes("%s %s %s mientras %s%s llevas.",
		 verb_inf, shk_your(yourbuf, obj, (char *)0), what,
		 fem? "la" : "lo",
		 more_than_1 ? "s" : "");
	return FALSE;
    }
    if (welded(uwep)) {
	if (flags.verbose) {
	    const char *hand = body_part(HAND);

	    if (bimanual(uwep)) hand = makeplural(hand);
	    if (strstri(what, "par de ") != 0) more_than_1 = FALSE;
	    pline(
	     "Porque tu arma est� soldada a tu%s %s, no puedes %s %s %s.",
		  more_than_1 ? "s" : "", hand, verb_inf,
		  more_than_1 ? fem? "estas" : "estos"
			      : fem? "esta"  : "este",
		  xname(obj));
	} else {
	    No_puedes("hacer eso.");
	}
	return FALSE;
    }
    if (cantwield(youmonst.data)) {
	No_puedes("sujetarl%c%s con suficiente fuerza.",
		fem? 'a' : 'o', more_than_1 ? "s" : "");
	return FALSE;
    }
    /* check shield */
    if (uarms && bimanual(obj)) {
	No_puedes("%s %s de dos manos mientras llevas un escudo puesto.",
		verb_inf, (obj->oclass == WEAPON_CLASS) ? "un arma" : "una herramienta");
	return FALSE;
    }
    if (uquiver == obj) setuqwep((struct obj *)0);
    if (uswapwep == obj) {
	(void) doswapweapon();
	/* doswapweapon might fail */
	if (uswapwep == obj) return FALSE;
    } else {
	pline("Ahora %s %s.", verb_pres, doname(obj));
	setuwep(obj);
    }
    if (uwep != obj) return FALSE;	/* rewielded old object after dying */
    /* applying weapon or tool that gets wielded ends two-weapon combat */
    if (u.twoweap)
	untwoweapon();
    if (obj->oclass != WEAPON_CLASS)
	unweapon = TRUE;
    return TRUE;
}

int
can_twoweapon()
{
	struct obj *otmp;

#define NOT_WEAPON(obj) (!is_weptool(obj) && obj->oclass != WEAPON_CLASS)
	if (!could_twoweap(youmonst.data)) {
		if (Upolyd)
		    No_puedes("usar dos armas en tu forma actual.");
		else
		    pline("%s no pueden usar dos armas a la vez.",
			  makeplural((flags.female && urole.name.f) ?
				     urole.name.f : urole.name.m));
	} else if (!uwep || !uswapwep) {
		const char *mano = body_part(HAND);
		boolean dosmanos = !uwep && !uswapwep;
		boolean fem = isfeminine(mano);
		pline("Tu%s %s%s%s est�%s vac�%c%s.",
			dosmanos? "s" : "",
			mano,
			uwep? " izquierd" : uswapwep ? " derech" : "",
			dosmanos? "" : fem? "a" : "o",
			dosmanos? "n" : "",
			fem? 'a' : 'o',
			dosmanos? "s" : "");
	} else if (NOT_WEAPON(uwep) || NOT_WEAPON(uswapwep)) {
		otmp = NOT_WEAPON(uwep) ? uwep : uswapwep;
		pline("%s no %s.", Yname2(otmp),
		    is_plural(otmp) ? "son armas" : "es una arma");
	} else if (bimanual(uwep) || bimanual(uswapwep)) {
		otmp = bimanual(uwep) ? uwep : uswapwep;
		pline("%s no es de una mano.", Yname2(otmp));
	} else if (uarms)
		No_puedes("usar dos armas mientras traigas escudo.");
	else if (uswapwep->oartifact)
		pline("�%s %s ser tenido segundo a otra arma!",
			Yname2(uswapwep), otense(uswapwep, "resiste"));
	else if (!uarmg && !Stone_resistance && (uswapwep->otyp == CORPSE &&
		    touch_petrifies(&mons[uswapwep->corpsenm]))) {
		char kbuf[BUFSZ];
		const char *mano = body_part(HAND);

		pline("Blandes el cad�ver de %s en tu %s desnud%c.",
		    mons[uswapwep->corpsenm].mname, mano,
		    isfeminine(mano)? 'a' : 'o');
		Sprintf(kbuf, "cad�ver de %s", mons[uswapwep->corpsenm].mname);
		instapetrify(kbuf);
	} else if (Glib || uswapwep->cursed) {
		if (!Glib)
			uswapwep->bknown = TRUE;
		drop_uswapwep();
	} else
		return (TRUE);
	return (FALSE);
}

void
drop_uswapwep()
{
	char str[BUFSZ];
	struct obj *obj = uswapwep;

	/* Avoid trashing makeplural's static buffer */
	Strcpy(str, makeplural(body_part(HAND)));
	pline("�%s de tus %s!",  upstart(aobjnam(obj, "se resbala")), str);
	dropx(obj);
}

int
dotwoweapon()
{
	/* You can always toggle it off */
	if (u.twoweap) {
		pline("Cambias a tu arma primera.");
		u.twoweap = 0;
		update_inventory();
		return (0);
	}

	/* May we use two weapons? */
	if (can_twoweapon()) {
		/* Success! */
		pline("Empiezas a combatir con dos armas.");
		u.twoweap = 1;
		update_inventory();
		return (rnd(20) > ACURR(A_DEX));
	}
	return (0);
}

/*** Functions to empty a given slot ***/
/* These should be used only when the item can't be put back in
 * the slot by life saving.  Proper usage includes:
 * 1.  The item has been eaten, stolen, burned away, or rotted away.
 * 2.  Making an item disappear for a bones pile.
 */
void
uwepgone()
{
	if (uwep) {
		if (artifact_light(uwep) && uwep->lamplit) {
		    end_burn(uwep, FALSE);
		    if (!Blind) pline("%s de brillar.", Tobjnam(uwep, "deja"));
		}
		setworn((struct obj *)0, W_WEP);
		unweapon = TRUE;
		update_inventory();
	}
}

void
uswapwepgone()
{
	if (uswapwep) {
		setworn((struct obj *)0, W_SWAPWEP);
		update_inventory();
	}
}

void
uqwepgone()
{
	if (uquiver) {
		setworn((struct obj *)0, W_QUIVER);
		update_inventory();
	}
}

void
untwoweapon()
{
	if (u.twoweap) {
		pline("No puedes usar dos armas m�s.");
		u.twoweap = FALSE;
		update_inventory();
	}
	return;
}

/* Maybe rust object, or corrode it if acid damage is called for */
void
erode_obj(target, acid_dmg, fade_scrolls)
struct obj *target;		/* object (e.g. weapon or armor) to erode */
boolean acid_dmg;
boolean fade_scrolls;
{
	int erosion;
	struct monst *victim;
	boolean vismon;
	boolean visobj;

	if (!target)
	    return;
	victim = carried(target) ? &youmonst :
	    mcarried(target) ? target->ocarry : (struct monst *)0;
	vismon = victim && (victim != &youmonst) && canseemon(victim);
	visobj = !victim && cansee(bhitpos.x, bhitpos.y); /* assume thrown */

	erosion = acid_dmg ? target->oeroded2 : target->oeroded;

	if (target->greased) {
	    grease_protect(target,(char *)0,victim);
	} else if (target->oclass == SCROLL_CLASS) {
	    if(fade_scrolls && target->otyp != SCR_BLANK_PAPER
#ifdef MAIL
	    && target->otyp != SCR_MAIL
#endif
					)
	    {
		if (!Blind) {
		    if (victim == &youmonst)
			pline("Tu%s %s.", plur(target->quan),
			      aobjnam(target, "se descolora"));
		    else if (vismon)
			pline("%s %s %s.",
			      The(xname(target)), del(mon_nam(victim)),
			      otense(target, "se descolora"));
		    else if (visobj)
			pline("%s %s.",
			      The(xname(target)),
			      otense(target, "se descolora"));
		}
		target->otyp = SCR_BLANK_PAPER;
		target->spe = 0;
	    }
	} else if (target->oerodeproof ||
		(acid_dmg ? !is_corrodeable(target) : !is_rustprone(target))) {
	    if (flags.verbose || !(target->oerodeproof && target->rknown)) {
		const char *que = xname(target);
		if (victim == &youmonst) {
		    pline("Tu%s %s no %s afectad%c%s.",
			  plur(target->quan), que,
			  target->quan>1L? "son" : "es",
			  isfeminine(que)? 'a' : 'o',
			  plur(target->quan));
		} else if (vismon) {
		    pline("%s %s no %s afectad%c%s.",
			  The(que), del(mon_nam(victim)),
			  target->quan>1L? "son" : "es",
			  isfeminine(que)? 'a' : 'o',
			  plur(target->quan));
		}
		/* no message if not carried */
	    }
	    if (target->oerodeproof) target->rknown = TRUE;
	} else if (erosion < MAX_ERODE) {
	    if (victim == &youmonst)
		pline("�Tu%s %s%s!",
		    plur(target->quan),
		    aobjnam(target, acid_dmg ? "se corroe" : "se oxida"),
		    erosion+1 == MAX_ERODE ? " completamente" :
		    erosion ? " m�s" : "");
	    else if (vismon)
		pline("�%s %s %s%s!",
		    The(xname(target)),
		    del(mon_nam(victim)),
		    otense(target, acid_dmg ? "se corroe" : "se oxida"),
		    erosion+1 == MAX_ERODE ? " completamente" :
		    erosion ? " m�s" : "");
	    else if (visobj)
		pline("�%s %s%s!",
		    The(xname(target)),
		    otense(target, acid_dmg ? "se corroe" : "se oxida"),
		    erosion+1 == MAX_ERODE ? " completamente" :
		    erosion ? " m�s" : "");
	    if (acid_dmg)
		target->oeroded2++;
	    else
		target->oeroded++;
	} else {
	    if (flags.verbose) {
		const char *que = xname(target);
		if (victim == &youmonst)
		    pline("Tu%s %s %s%s completamente %s%c%s.",
			plur(target->quan),
			que,
			Blind ? "siente" : "parece",
			target->quan>1L ? "n" : "",
			acid_dmg ? "corro�d" : "oxidad",
			isfeminine(que)? 'a' : 'o',
			plur(target->quan));
		else if (vismon)
		    pline("%s %s %s completamente %s%c%s.",
			The(que),
			del(mon_nam(victim)),
			otense(target, "parece"),
			acid_dmg ? "corro�d" : "oxidad",
			isfeminine(que)? 'a' : 'o',
			plur(target->quan));
		else if (visobj)
		    pline("%s %s completamente %s%c%s.",
			The(que),
			otense(target, "parece"),
			acid_dmg ? "corro�d" : "oxidad",
			isfeminine(que)? 'a' : 'o',
			plur(target->quan));
	    }
	}
}

int
chwepon(otmp, amount)
register struct obj *otmp;
register int amount;
{
	const char *color = hcolor((amount < 0) ? NH_BLACK : NH_BLUE);
	const char *xtime;
	int otyp = STRANGE_OBJECT;
	const char *arma;

	if(!uwep || (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))) {
		char buf[BUFSZ];

		Sprintf(buf, "Tus %s %s.", makeplural(body_part(HAND)),
			(amount >= 0) ? "temblan" : "pican");
		strange_feeling(otmp, buf);
		exercise(A_DEX, (boolean) (amount >= 0));
		return(0);
	}

	if (otmp && otmp->oclass == SCROLL_CLASS) otyp = otmp->otyp;

	if(uwep->otyp == WORM_TOOTH && amount >= 0) {
		uwep->otyp = CRYSKNIFE;
		uwep->oerodeproof = 0;
		pline("Tu arma parece m�s aguda ahora.");
		uwep->cursed = 0;
		if (otyp != STRANGE_OBJECT) makeknown(otyp);
		return(1);
	}

	if(uwep->otyp == CRYSKNIFE && amount < 0) {
		uwep->otyp = WORM_TOOTH;
		uwep->oerodeproof = 0;
		pline("Tu arma parece m�s embotada ahora.");
		if (otyp != STRANGE_OBJECT && otmp->bknown) makeknown(otyp);
		return(1);
	}

	arma = xname(uwep);
	color = feminize(color, isfeminine(arma));
	if (amount < 0 && uwep->oartifact && restrict_name(uwep, ONAME(uwep))) {
	    if (!Blind)
		pline("Tu%s %s suavemente %s %s.", plur(uwep->quan),
		      arma, otense(uwep, "brilla"), color);
	    return(1);
	}
	/* there is a (soft) upper and lower limit to uwep->spe */
	if(((uwep->spe > 5 && amount >= 0) || (uwep->spe < -5 && amount < 0))
								&& rn2(3)) {
	    if (!Blind)
	    pline("Tu%s %s violentamente %s %s por un rato y entonces %s.",
		 plur(uwep->quan),
		 arma, otense(uwep, "brilla"), color,
		 otense(uwep, "evapora"));
	    else
		pline("Tu%s %s.", is_plural(uwep)? "s" : "",
		      aobjnam(uwep, "evapora"));

	    useupall(uwep);	/* let all of them disappear */
	    return(1);
	}
	if (!Blind) {
	    xtime = (amount*amount == 1) ? "momento" : "rato";
	    pline("Tu%s %s %salumbra%s %s por un %s.",
		 plur(uwep->quan),
		 arma,
		 amount == 0 ? "violentemente " : "",
		 uwep->quan > 1L ? "n" : "",
		 color, xtime);
	    if (otyp != STRANGE_OBJECT && uwep->known &&
		    (amount > 0 || (amount < 0 && otmp->bknown)))
		makeknown(otyp);
	}
	uwep->spe += amount;
	if(amount > 0) uwep->cursed = 0;

	/*
	 * Enchantment, which normally improves a weapon, has an
	 * addition adverse reaction on Magicbane whose effects are
	 * spe dependent.  Give an obscure clue here.
	 */
	if (uwep->oartifact == ART_MAGICBANE && uwep->spe >= 0) {
		const char *mano = body_part(HAND);
		pline("�Tu %s derech%c %s!",
			mano, isfeminine(mano)? 'a' : 'o',
			(((amount > 1) && (uwep->spe > 1)) ? "convulsa" : "pica"));
	}

	/* an elven magic clue, cookie@keebler */
	/* elven weapons vibrate warningly when enchanted beyond a limit */
	if ((uwep->spe > 5)
		&& (is_elven_weapon(uwep) || uwep->oartifact || !rn2(7)))
	    pline("Tu%s %s de repente %s inesperadamente.",
		plur(uwep->quan),
		arma,
		otense(uwep, "vibra"));

	return(1);
}

int
welded(obj)
register struct obj *obj;
{
	if (obj && obj == uwep && will_weld(obj)) {
		obj->bknown = TRUE;
		return 1;
	}
	return 0;
}

void
weldmsg(obj)
register struct obj *obj;
{
	long savewornmask;
	const char *arma = xname(obj);

	savewornmask = obj->owornmask;
	pline("�Tu%s %s %s soldad%c%s a tu%s %s!",
		plur(obj->quan),
		arma, otense(obj, "est�"),
		isfeminine(arma)? 'a' : 'o',
		plur(obj->quan),
		bimanual(obj) ? "s" : "",
		bimanual(obj) ? (const char *)makeplural(body_part(HAND))
				: body_part(HAND));
	obj->owornmask = savewornmask;
}

/*wield.c*/
