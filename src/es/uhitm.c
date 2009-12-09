/*	SCCS Id: @(#)uhitm.c	3.4	2003/02/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL boolean FDECL(known_hitum, (struct monst *,int *,struct attack *));
STATIC_DCL void FDECL(steal_it, (struct monst *, struct attack *));
STATIC_DCL boolean FDECL(hitum, (struct monst *,int,struct attack *));
STATIC_DCL boolean FDECL(hmon_hitmon, (struct monst *,struct obj *,int));
#ifdef STEED
STATIC_DCL int FDECL(joust, (struct monst *,struct obj *));
#endif
STATIC_DCL void NDECL(demonpet);
STATIC_DCL boolean FDECL(m_slips_free, (struct monst *mtmp,struct attack *mattk));
STATIC_DCL int FDECL(explum, (struct monst *,struct attack *));
STATIC_DCL void FDECL(start_engulf, (struct monst *));
STATIC_DCL void NDECL(end_engulf);
STATIC_DCL int FDECL(gulpum, (struct monst *,struct attack *));
STATIC_DCL boolean FDECL(hmonas, (struct monst *,int));
STATIC_DCL void FDECL(nohandglow, (struct monst *));
STATIC_DCL boolean FDECL(shade_aware, (struct obj *));

extern boolean notonhead;	/* for long worms */
/* The below might become a parameter instead if we use it a lot */
static int dieroll;
/* Used to flag attacks caused by Stormbringer's maliciousness. */
static boolean override_confirmation = FALSE;

#define PROJECTILE(obj)	((obj) && is_ammo(obj))

/* modified from hurtarmor() in mhitu.c */
/* This is not static because it is also used for monsters rusting monsters */
void
hurtmarmor(mdef, attk)
struct monst *mdef;
int attk;
{
	int	hurt;
	struct obj *target;

	switch(attk) {
	    /* 0 is burning, which we should never be called with */
	    case AD_RUST: hurt = 1; break;
	    case AD_CORR: hurt = 3; break;
	    default: hurt = 2; break;
	}
	/* What the following code does: it keeps looping until it
	 * finds a target for the rust monster.
	 * Head, feet, etc... not covered by metal, or covered by
	 * rusty metal, are not targets.  However, your body always
	 * is, no matter what covers it.
	 */
	while (1) {
	    switch(rn2(5)) {
	    case 0:
		target = which_armor(mdef, W_ARMH);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    case 1:
		target = which_armor(mdef, W_ARMC);
		if (target) {
		    (void)rust_dmg(target, xname(target), hurt, TRUE, mdef);
		    break;
		}
		if ((target = which_armor(mdef, W_ARM)) != (struct obj *)0) {
		    (void)rust_dmg(target, xname(target), hurt, TRUE, mdef);
#ifdef TOURIST
		} else if ((target = which_armor(mdef, W_ARMU)) != (struct obj *)0) {
		    (void)rust_dmg(target, xname(target), hurt, TRUE, mdef);
#endif
		}
		break;
	    case 2:
		target = which_armor(mdef, W_ARMS);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    case 3:
		target = which_armor(mdef, W_ARMG);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    case 4:
		target = which_armor(mdef, W_ARMF);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    }
	    break; /* Out of while loop */
	}
}

/* FALSE means it's OK to attack */
boolean
attack_checks(mtmp, wep)
register struct monst *mtmp;
struct obj *wep;	/* uwep for attack(), null for kick_monster() */
{
	char qbuf[QBUFSZ];

	/* if you're close enough to attack, alert any waiting monster */
	mtmp->mstrategy &= ~STRAT_WAITMASK;

	if (u.uswallow && mtmp == u.ustuck) return FALSE;

	if (flags.forcefight) {
		/* Do this in the caller, after we checked that the monster
		 * didn't die from the blow.  Reason: putting the 'I' there
		 * causes the hero to forget the square's contents since
		 * both 'I' and remembered contents are stored in .glyph.
		 * If the monster dies immediately from the blow, the 'I' will
		 * not stay there, so the player will have suddenly forgotten
		 * the square's contents for no apparent reason.
		if (!canspotmon(mtmp) &&
		    !glyph_is_invisible(levl[u.ux+u.dx][u.uy+u.dy].glyph))
			map_invisible(u.ux+u.dx, u.uy+u.dy);
		 */
		return FALSE;
	}

	/* Put up an invisible monster marker, but with exceptions for
	 * monsters that hide and monsters you've been warned about.
	 * The former already prints a warning message and
	 * prevents you from hitting the monster just via the hidden monster
	 * code below; if we also did that here, similar behavior would be
	 * happening two turns in a row.  The latter shows a glyph on
	 * the screen, so you know something is there.
	 */
	if (!canspotmon(mtmp) &&
		    !glyph_is_warning(glyph_at(u.ux+u.dx,u.uy+u.dy)) &&
		    !glyph_is_invisible(levl[u.ux+u.dx][u.uy+u.dy].glyph) &&
		    !(!Blind && mtmp->mundetected && hides_under(mtmp->data))) {
		pline("¡Espera!  ¡Hay %s allí que no puedes ver!",
			something);
		map_invisible(u.ux+u.dx, u.uy+u.dy);
		/* if it was an invisible mimic, treat it as if we stumbled
		 * onto a visible mimic
		 */
		if(mtmp->m_ap_type && !Protection_from_shape_changers) {
		    if(!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data,AD_STCK))
			u.ustuck = mtmp;
		}
		wakeup(mtmp); /* always necessary; also un-mimics mimics */
		return TRUE;
	}

	if (mtmp->m_ap_type && !Protection_from_shape_changers &&
	   !sensemon(mtmp) &&
	   !glyph_is_warning(glyph_at(u.ux+u.dx,u.uy+u.dy))) {
		/* If a hidden mimic was in a square where a player remembers
		 * some (probably different) unseen monster, the player is in
		 * luck--he attacks it even though it's hidden.
		 */
		if (glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph)) {
		    seemimic(mtmp);
		    return(FALSE);
		}
		stumble_onto_mimic(mtmp);
		return TRUE;
	}

	if (mtmp->mundetected && !canseemon(mtmp) &&
		!glyph_is_warning(glyph_at(u.ux+u.dx,u.uy+u.dy)) &&
		(hides_under(mtmp->data) || mtmp->data->mlet == S_EEL)) {
	    mtmp->mundetected = mtmp->msleeping = 0;
	    newsym(mtmp->mx, mtmp->my);
	    if (glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph)) {
		seemimic(mtmp);
		return(FALSE);
	    }
	    if (!(Blind ? Blind_telepat : Unblind_telepat)) {
		struct obj *obj;

		if (Blind || (is_pool(mtmp->mx,mtmp->my) && !Underwater))
		    pline("¡Espera!  ¡Hay un monstruo escondido allí!");
		else if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0)
		    pline("¡Espera!  ¡Hay %s escondiéndose bajo %s!",
			  an(l_monnam(mtmp)), doname(obj));
		return TRUE;
	    }
	}

	/*
	 * make sure to wake up a monster from the above cases if the
	 * hero can sense that the monster is there.
	 */
	if ((mtmp->mundetected || mtmp->m_ap_type) && sensemon(mtmp)) {
	    mtmp->mundetected = 0;
	    wakeup(mtmp);
	}

	if (flags.confirm && mtmp->mpeaceful
	    && !Confusion && !Hallucination && !Stunned) {
		/* Intelligent chaotic weapons (Stormbringer) want blood */
		if (wep && wep->oartifact == ART_STORMBRINGER) {
			override_confirmation = TRUE;
			return(FALSE);
		}
		if (canspotmon(mtmp)) {
			Sprintf(qbuf, "¿De verdad %s?", mon_nam_acc(mtmp, "atacar"));
			if (yn(qbuf) != 'y') {
				flags.move = 0;
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/*
 * It is unchivalrous for a knight to attack the defenseless or from behind.
 */
void
check_caitiff(mtmp)
struct monst *mtmp;
{
	if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL &&
	    (!mtmp->mcanmove || mtmp->msleeping ||
	     (mtmp->mflee && !mtmp->mavenge)) &&
	    u.ualign.record > -10) {
	    pline("¡Tú bellac%c!", poly_gender()==1? 'a' : 'o');
	    adjalign(-1);
	}
}

schar
find_roll_to_hit(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	int tmp2;

	tmp = 1 + Luck + abon() + find_mac(mtmp) + u.uhitinc +
		maybe_polyd(youmonst.data->mlevel, u.ulevel);

	check_caitiff(mtmp);

/*	attacking peaceful creatures is bad for the samurai's giri */
	if (Role_if(PM_SAMURAI) && mtmp->mpeaceful &&
	    u.ualign.record > -10) {
	    pline("¡Atacas sin honor a un inocente!");
	    adjalign(-1);
	}

/*	Adjust vs. (and possibly modify) monster state.		*/

	if(mtmp->mstun) tmp += 2;
	if(mtmp->mflee) tmp += 2;

	if (mtmp->msleeping) {
		mtmp->msleeping = 0;
		tmp += 2;
	}
	if(!mtmp->mcanmove) {
		tmp += 4;
		if(!rn2(10)) {
			mtmp->mcanmove = 1;
			mtmp->mfrozen = 0;
		}
	}
	if (is_orc(mtmp->data) && maybe_polyd(is_elf(youmonst.data),
			Race_if(PM_ELF)))
	    tmp++;
	if(Role_if(PM_MONK) && !Upolyd) {
	    if (uarm) {
		pline("Tu armadura es bastante engorrosa...");
		tmp -= urole.spelarmr;
	    } else if (!uwep && !uarms) {
		tmp += (u.ulevel / 3) + 2;
	    }
	}

/*	with a lot of luggage, your agility diminishes */
	if ((tmp2 = near_capacity()) != 0) tmp -= (tmp2*2) - 1;
	if (u.utrap) tmp -= 3;
/*	Some monsters have a combination of weapon attacks and non-weapon
 *	attacks.  It is therefore wrong to add hitval to tmp; we must add
 *	it only for the specific attack (in hmonas()).
 */
	if (uwep && !Upolyd) {
		tmp += hitval(uwep, mtmp);
		tmp += weapon_hit_bonus(uwep);
	}
	return tmp;
}

/* try to attack; return FALSE if monster evaded */
/* u.dx and u.dy must be set */
boolean
attack(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	register struct permonst *mdat = mtmp->data;
	char buf[BUFSZ];

	/* This section of code provides protection against accidentally
	 * hitting peaceful (like '@') and tame (like 'd') monsters.
	 * Protection is provided as long as player is not: blind, confused,
	 * hallucinating or stunned.
	 * changes by wwp 5/16/85
	 * More changes 12/90, -dkh-. if its tame and safepet, (and protected
	 * 07/92) then we assume that you're not trying to attack. Instead,
	 * you'll usually just swap places if this is a movement command
	 */
	/* Intelligent chaotic weapons (Stormbringer) want blood */
	if (is_safepet(mtmp) && !flags.forcefight) {
	    if (!uwep || uwep->oartifact != ART_STORMBRINGER) {
		/* there are some additional considerations: this won't work
		 * if in a shop or Punished or you miss a random roll or
		 * if you can walk thru walls and your pet cannot (KAA) or
		 * if your pet is a long worm (unless someone does better).
		 * there's also a chance of displacing a "frozen" monster.
		 * sleeping monsters might magically walk in their sleep.
		 */
		boolean foo = (Punished || !rn2(7) || is_longworm(mtmp->data)),
			inshop = FALSE;
		char *p;

		for (p = in_rooms(mtmp->mx, mtmp->my, SHOPBASE); *p; p++)
		    if (tended_shop(&rooms[*p - ROOMOFFSET])) {
			inshop = TRUE;
			break;
		    }

		if (inshop || foo ||
			(IS_ROCK(levl[u.ux][u.uy].typ) &&
					!passes_walls(mtmp->data))) {
		    char buf[BUFSZ];

		    monflee(mtmp, rnd(6), FALSE, FALSE);
		    Strcpy(buf, y_monnam(mtmp));
		    buf[0] = toupper2(buf[0]);
		    pline("Paras.  ¡%s te obstrue!", buf);
		    return(TRUE);
		} else if ((mtmp->mfrozen || (! mtmp->mcanmove)
				|| (mtmp->data->mmove == 0)) && rn2(6)) {
		    pline("¡%s no parece mover!", Monnam(mtmp));
		    return(TRUE);
		} else return(FALSE);
	    }
	}

	/* possibly set in attack_checks;
	   examined in known_hitum, called via hitum or hmonas below */
	override_confirmation = FALSE;
	if (attack_checks(mtmp, uwep)) return(TRUE);

	if (Upolyd) {
		/* certain "pacifist" monsters don't attack */
		if(noattacks(youmonst.data)) {
			pline("No tienes ninguna manera de atacar físicamente a los monstruos.");
			mtmp->mstrategy &= ~STRAT_WAITMASK;
			goto atk_done;
		}
	}

	sprintf(buf, "No puedes luchar mientras seas tan cargad%c.",
		poly_gender()==1? 'a' : 'o');
	if(check_capacity(buf))
	    goto atk_done;

	if (u.twoweap && !can_twoweapon())
		untwoweapon();

	if(unweapon) {
	    unweapon = FALSE;
	    if(flags.verbose) {
		if(uwep)
		    pline("Empiezas a pegar a los monstruos con tu %s.",
			aobjnam(uwep, (char *)0));
		else if (!cantwield(youmonst.data)) {
		    const char *mano = body_part(HAND);
		    pline("Empiezas a %s a los monstruos con tus %s %s%cs.",
			Role_if(PM_MONK) ? "golpear" : "pegar",
			makeplural(mano),
			uarmg ? "enguantad" : "desnud",	/* Del Lamb */
			isfeminine(mano)? 'a' : 'o');
		}
	    }
	}
	exercise(A_STR, TRUE);		/* you're exercising muscles */
	/* andrew@orca: prevent unlimited pick-axe attacks */
	u_wipe_engr(3);

	/* Is the "it died" check actually correct? */
	if(mdat->mlet == S_LEPRECHAUN && !mtmp->mfrozen && !mtmp->msleeping &&
	   !mtmp->mconf && mtmp->mcansee && !rn2(7) &&
	   (m_move(mtmp, 0) == 2 ||			    /* it died */
	   mtmp->mx != u.ux+u.dx || mtmp->my != u.uy+u.dy)) /* it moved */
		return(FALSE);

	tmp = find_roll_to_hit(mtmp);
	if (Upolyd)
		(void) hmonas(mtmp, tmp);
	else
		(void) hitum(mtmp, tmp, youmonst.data->mattk);
	mtmp->mstrategy &= ~STRAT_WAITMASK;

atk_done:
	/* see comment in attack_checks() */
	/* we only need to check for this if we did an attack_checks()
	 * and it returned 0 (it's okay to attack), and the monster didn't
	 * evade.
	 */
	if (flags.forcefight && mtmp->mhp > 0 && !canspotmon(mtmp) &&
	    !glyph_is_invisible(levl[u.ux+u.dx][u.uy+u.dy].glyph) &&
	    !(u.uswallow && mtmp == u.ustuck))
		map_invisible(u.ux+u.dx, u.uy+u.dy);

	return(TRUE);
}

STATIC_OVL boolean
known_hitum(mon, mhit, uattk)	/* returns TRUE if monster still lives */
register struct monst *mon;
register int *mhit;
struct attack *uattk;
{
	register boolean malive = TRUE;

	if (override_confirmation) {
	    /* this may need to be generalized if weapons other than
	       Stormbringer acquire similar anti-social behavior... */
	    if (flags.verbose) pline("¡Tu hoja sanguinaria ataca!");
	}

	if(!*mhit) {
	    missum(mon, uattk);
	} else {
	    int oldhp = mon->mhp,
		x = u.ux + u.dx, y = u.uy + u.dy;

	    /* KMH, conduct */
	    if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
		u.uconduct.weaphit++;

	    /* we hit the monster; be careful: it might die or
	       be knocked into a different location */
	    notonhead = (mon->mx != x || mon->my != y);
	    malive = hmon(mon, uwep, 0);
	    /* this assumes that Stormbringer was uwep not uswapwep */ 
	    if (malive && u.twoweap && !override_confirmation &&
		    m_at(x, y) == mon)
		malive = hmon(mon, uswapwep, 0);
	    if (malive) {
		/* monster still alive */
		if(!rn2(25) && mon->mhp < mon->mhpmax/2
			    && !(u.uswallow && mon == u.ustuck)) {
		    /* maybe should regurgitate if swallowed? */
		    if(!rn2(3)) {
			monflee(mon, rnd(100), FALSE, TRUE);
		    } else monflee(mon, 0, FALSE, TRUE);

		    if(u.ustuck == mon && !u.uswallow && !sticks(youmonst.data))
			u.ustuck = 0;
		}
		/* Vorpal Blade hit converted to miss */
		/* could be headless monster or worm tail */
		if (mon->mhp == oldhp) {
		    *mhit = 0;
		    /* a miss does not break conduct */
		    if (uwep &&
			(uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
			--u.uconduct.weaphit;
		}
		if (mon->wormno && *mhit)
		    cutworm(mon, x, y, uwep);
	    }
	}
	return(malive);
}

STATIC_OVL boolean
hitum(mon, tmp, uattk)		/* returns TRUE if monster still lives */
struct monst *mon;
int tmp;
struct attack *uattk;
{
	boolean malive;
	int mhit = (tmp > (dieroll = rnd(20)) || u.uswallow);

	if(tmp > dieroll) exercise(A_DEX, TRUE);
	malive = known_hitum(mon, &mhit, uattk);
	(void) passive(mon, mhit, malive, AT_WEAP);
	return(malive);
}

boolean			/* general "damage monster" routine */
hmon(mon, obj, thrown)		/* return TRUE if mon still alive */
struct monst *mon;
struct obj *obj;
int thrown;
{
	boolean result, anger_guards;

	anger_guards = (mon->mpeaceful &&
			    (mon->ispriest || mon->isshk ||
			     mon->data == &mons[PM_WATCHMAN] ||
			     mon->data == &mons[PM_WATCH_CAPTAIN]));
	result = hmon_hitmon(mon, obj, thrown);
	if (mon->ispriest && !rn2(2)) ghod_hitsu(mon);
	if (anger_guards) (void)angry_guards(!flags.soundok);
	return result;
}

/* guts of hmon() */
STATIC_OVL boolean
hmon_hitmon(mon, obj, thrown)
struct monst *mon;
struct obj *obj;
int thrown;
{
	int tmp;
	struct permonst *mdat = mon->data;
	int barehand_silver_rings = 0;
	/* The basic reason we need all these booleans is that we don't want
	 * a "hit" message when a monster dies, so we have to know how much
	 * damage it did _before_ outputting a hit message, but any messages
	 * associated with the damage don't come out until _after_ outputting
	 * a hit message.
	 */
	boolean hittxt = FALSE, destroyed = FALSE, already_killed = FALSE;
	boolean get_dmg_bonus = TRUE;
	boolean ispoisoned = FALSE, needpoismsg = FALSE, poiskilled = FALSE;
	boolean silvermsg = FALSE, silverobj = FALSE;
	boolean valid_weapon_attack = FALSE;
	boolean unarmed = !uwep && !uarm && !uarms;
#ifdef STEED
	int jousting = 0;
#endif
	int wtype;
	struct obj *monwep;
	char yourbuf[BUFSZ];
	char unconventional[BUFSZ];	/* substituted for word "attack" in msg */
	char saved_oname[BUFSZ];

	unconventional[0] = '\0';
	saved_oname[0] = '\0';

	wakeup(mon);
	if(!obj) {	/* attack with bare hands */
	    if (mdat == &mons[PM_SHADE])
		tmp = 0;
	    else if (martial_bonus())
		tmp = rnd(4);	/* bonus for martial arts */
	    else
		tmp = rnd(2);
	    valid_weapon_attack = (tmp > 1);
	    /* blessed gloves give bonuses when fighting 'bare-handed' */
	    if (uarmg && uarmg->blessed && (is_undead(mdat) || is_demon(mdat)))
		tmp += rnd(4);
	    /* So do silver rings.  Note: rings are worn under gloves, so you
	     * don't get both bonuses.
	     */
	    if (!uarmg) {
		if (uleft && objects[uleft->otyp].oc_material == SILVER)
		    barehand_silver_rings++;
		if (uright && objects[uright->otyp].oc_material == SILVER)
		    barehand_silver_rings++;
		if (barehand_silver_rings && hates_silver(mdat)) {
		    tmp += rnd(20);
		    silvermsg = TRUE;
		}
	    }
	} else {
	    Strcpy(saved_oname, cxname(obj));
	    if(obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
	       obj->oclass == GEM_CLASS) {

		/* is it not a melee weapon? */
		if (/* if you strike with a bow... */
		    is_launcher(obj) ||
		    /* or strike with a missile in your hand... */
		    (!thrown && (is_missile(obj) || is_ammo(obj))) ||
		    /* or use a pole at short range and not mounted... */
		    (!thrown &&
#ifdef STEED
		     !u.usteed &&
#endif
		     is_pole(obj)) ||
		    /* or throw a missile without the proper bow... */
		    (is_ammo(obj) && !ammo_and_launcher(obj, uwep))) {
		    /* then do only 1-2 points of damage */
		    if (mdat == &mons[PM_SHADE] && obj->otyp != SILVER_ARROW)
			tmp = 0;
		    else
			tmp = rnd(2);
		    if (!thrown && obj == uwep && obj->otyp == BOOMERANG &&
			    rnl(4) == 4-1) {
			boolean more_than_1 = (obj->quan > 1L);
			const char *que = shk_your(yourbuf, obj, (char *)0);

			pline("Al %s, %s%s se rompe en pedazos.",
			      mon_nam_acc(mon, "golpear"),
			      more_than_1 ? (isfeminine(que)? "una de " : "uno de ")
					  : "",
			      que);
			if (!more_than_1) uwepgone();	/* set unweapon */
			useup(obj);
			if (!more_than_1) obj = (struct obj *) 0;
			hittxt = TRUE;
			if (mdat != &mons[PM_SHADE])
			    tmp++;
		    }
		} else {
		    tmp = dmgval(obj, mon);
		    /* a minimal hit doesn't exercise proficiency */
		    valid_weapon_attack = (tmp > 1);
		    if (!valid_weapon_attack || mon == u.ustuck || u.twoweap) {
			;	/* no special bonuses */
		    } else if (mon->mflee && Role_if(PM_ROGUE) && !Upolyd) {
			pline("¡%s de atrás!", mon_nam_acc(mon, "Golpeas"));
			tmp += rnd(u.ulevel);
			hittxt = TRUE;
		    } else if (dieroll == 2 && obj == uwep &&
			  obj->oclass == WEAPON_CLASS &&
			  (bimanual(obj) ||
			    (Role_if(PM_SAMURAI) && obj->otyp == KATANA && !uarms)) &&
			  ((wtype = uwep_skill_type()) != P_NONE &&
			    P_SKILL(wtype) >= P_SKILLED) &&
			  ((monwep = MON_WEP(mon)) != 0 &&
			   !is_flimsy(monwep) &&
			   !obj_resists(monwep,
				 50 + 15 * greatest_erosion(obj), 100))) {
			/*
			 * 2.5% chance of shattering defender's weapon when
			 * using a two-handed weapon; less if uwep is rusted.
			 * [dieroll == 2 is most successful non-beheading or
			 * -bisecting hit, in case of special artifact damage;
			 * the percentage chance is (1/20)*(50/100).]
			 */
			setmnotwielded(mon,monwep);
			MON_NOWEP(mon);
			mon->weapon_check = NEED_WEAPON;
			pline("¡%s %s se %s por la fuerza de tu golpeazo!",
			      The(xname(monwep)), del(mon_nam(mon)),
			      otense(monwep, "rompe"));
			m_useup(mon, monwep);
			/* If someone just shattered MY weapon, I'd flee! */
			if (rn2(4)) {
			    monflee(mon, d(2,3), TRUE, TRUE);
			}
			hittxt = TRUE;
		    }

		    if (obj->oartifact &&
			artifact_hit(&youmonst, mon, obj, &tmp, dieroll)) {
			if(mon->mhp <= 0) /* artifact killed monster */
			    return FALSE;
			if (tmp == 0) return TRUE;
			hittxt = TRUE;
		    }
		    if (objects[obj->otyp].oc_material == SILVER
				&& hates_silver(mdat)) {
			silvermsg = TRUE; silverobj = TRUE;
		    }
#ifdef STEED
		    if (u.usteed && !thrown && tmp > 0 &&
			    weapon_type(obj) == P_LANCE && mon != u.ustuck) {
			jousting = joust(mon, obj);
			/* exercise skill even for minimal damage hits */
			if (jousting) valid_weapon_attack = TRUE;
		    }
#endif
		    if (thrown && (is_ammo(obj) || is_missile(obj))) {
			if (ammo_and_launcher(obj, uwep)) {
			    /* Elves and Samurai do extra damage using
			     * their bows&arrows; they're highly trained.
			     */
			    if (Role_if(PM_SAMURAI) &&
				obj->otyp == YA && uwep->otyp == YUMI)
				tmp++;
			    else if (Race_if(PM_ELF) &&
				     obj->otyp == ELVEN_ARROW &&
				     uwep->otyp == ELVEN_BOW)
				tmp++;
			}
			if(obj->opoisoned && is_poisonable(obj))
			    ispoisoned = TRUE;
		    }
		}
	    } else if(obj->oclass == POTION_CLASS) {
		if (obj->quan > 1L)
		    obj = splitobj(obj, 1L);
		else
		    setuwep((struct obj *)0);
		freeinv(obj);
		potionhit(mon, obj, TRUE);
		if (mon->mhp <= 0) return FALSE;	/* killed */
		hittxt = TRUE;
		/* in case potion effect causes transformation */
		mdat = mon->data;
		tmp = (mdat == &mons[PM_SHADE]) ? 0 : 1;
	    } else {
		if (mdat == &mons[PM_SHADE] && !shade_aware(obj)) {
		    tmp = 0;
		    Strcpy(unconventional, cxname(obj));
		} else {
		    switch(obj->otyp) {
		    case BOULDER:		/* 1d20 */
		    case HEAVY_IRON_BALL:	/* 1d25 */
		    case IRON_CHAIN:		/* 1d4+1 */
			tmp = dmgval(obj, mon);
			break;
		    case MIRROR:
			if (breaktest(obj)) {
			    pline("Rompes %s.  ¡Eso es mala suerte!",
				shk_your(yourbuf, obj, (char *)0));
			    change_luck(-2);
			    useup(obj);
			    obj = (struct obj *) 0;
			    unarmed = FALSE;	/* avoid obj==0 confusion */
			    get_dmg_bonus = FALSE;
			    hittxt = TRUE;
			}
			tmp = 1;
			break;
#ifdef TOURIST
		    case EXPENSIVE_CAMERA:
			pline("Tienes éxito de destruir %s.  ¡Felicidades!",
			        shk_your(yourbuf, obj, (char *)0));
			useup(obj);
			return(TRUE);
			/*NOTREACHED*/
			break;
#endif
		    case CORPSE:		/* fixed by polder@cs.vu.nl */
			if (touch_petrifies(&mons[obj->corpsenm])) {
			    static const char withwhat[] = "el cadáver";
			    tmp = 1;
			    hittxt = TRUE;
			    pline("%s con %s %s.",
				  mon_nam_acc(mon, "Golpeas"),
				  (obj->quan > 1) ? makeplural(withwhat) : withwhat,
				  del(mons[obj->corpsenm].mname));
			    if (!munstone(mon, TRUE))
				minstapetrify(mon, TRUE);
			    if (resists_ston(mon)) break;
			    /* note: hp may be <= 0 even if munstoned==TRUE */
			    return (boolean) (mon->mhp > 0);
#if 0
			} else if (touch_petrifies(mdat)) {
			    /* maybe turn the corpse into a statue? */
#endif
			}
			tmp = (obj->corpsenm >= LOW_PM ?
					mons[obj->corpsenm].msize : 0) + 1;
			break;
		    case EGG:
		      {
#define useup_eggs(o)	{ if (thrown) obfree(o,(struct obj *)0); \
			  else useupall(o); \
			  o = (struct obj *)0; }	/* now gone */
			long cnt = obj->quan;

			tmp = 1;		/* nominal physical damage */
			get_dmg_bonus = FALSE;
			hittxt = TRUE;		/* message always given */
			/* egg is always either used up or transformed, so next
			   hand-to-hand attack should yield a "bashing" mesg */
			if (obj == uwep) unweapon = TRUE;
			if (obj->spe && obj->corpsenm >= LOW_PM) {
			    if (obj->quan < 5)
				change_luck((schar) -(obj->quan));
			    else
				change_luck(-5);
			}

			if (touch_petrifies(&mons[obj->corpsenm])) {
			    /*learn_egg_type(obj->corpsenm);*/
			    if (obj->known)
				pline("¡Esplat! %s con %s huevo%s de %s.",
					mon_nam_acc(mon, "Golpeas"),
					cnt > 1L ? "los" : "el",
					plur(cnt),
					mons[obj->corpsenm].mname);
			    else
				pline("¡Esplat! %s con %s huevo%s petrificante%s.",
					mon_nam_acc(mon, "Golpeas"),
					cnt > 1L ? "unos" : "un",
					plur(cnt),
					plur(cnt));
			    obj->known = 1;	/* (not much point...) */
			    useup_eggs(obj);
			    if (!munstone(mon, TRUE))
				minstapetrify(mon, TRUE);
			    if (resists_ston(mon)) break;
			    return (boolean) (mon->mhp > 0);
			} else {	/* ordinary egg(s) */
			    const char *eggp =
				     (obj->corpsenm != NON_PM && obj->known) ?
					      the(mons[obj->corpsenm].mname) :
					      (cnt > 1L) ? "unos" : "un";
			    pline("%s con %s huevo%s.",
				mon_nam_acc(mon, "Golpeas"), eggp, plur(cnt));
			    if (touch_petrifies(mdat) && !stale_egg(obj)) {
				pline("%s huevo%s no está%s vivo%s más...",
				      (cnt == 1L) ? "Los" : "El",
				      plur(cnt),
				      (cnt == 1L) ? "" : "n",
				      plur(cnt));
				if (obj->timed) obj_stop_timers(obj);
				obj->otyp = ROCK;
				obj->oclass = GEM_CLASS;
				obj->oartifact = 0;
				obj->spe = 0;
				obj->known = obj->dknown = obj->bknown = 0;
				obj->owt = weight(obj);
				if (thrown) place_object(obj, mon->mx, mon->my);
			    } else {
				pline("¡Esplat!");
				useup_eggs(obj);
				exercise(A_WIS, FALSE);
			    }
			}
			break;
#undef useup_eggs
		      }
		    case CLOVE_OF_GARLIC:	/* no effect against demons */
			if (is_undead(mdat)) {
			    monflee(mon, d(2, 4), FALSE, TRUE);
			}
			tmp = 1;
			break;
		    case CREAM_PIE:
		    case BLINDING_VENOM:
			mon->msleeping = 0;
			if (can_blnd(&youmonst, mon, (uchar)
				    (obj->otyp == BLINDING_VENOM
				     ? AT_SPIT : AT_WEAP), obj)) {
			    if (Blind) {
				pline(obj->otyp == CREAM_PIE ?
				      "¡Esplat!" : "¡Esplash!");
			    } else if (obj->otyp == BLINDING_VENOM) {
				pline("¡El veneno %s%s!", mon_nam_acc(mon, "ciega"),
					  mon->mcansee ? "" : " más");
			    } else {
				char *what = The(xname(obj));
				if (!thrown && obj->quan > 1)
				    what = An(singular(obj, xname));
				if (haseyes(mdat)
				    && mdat != &mons[PM_FLOATING_EYE])
					pline("¡%s %s sobre %s %s!",
					      what, vtense(what, "chapotea"),
					      the(mbodypart(mon, FACE)),
					      del(mon_nam(mon)));
				else
					pline("¡%s %s sobre %s!",
					      what, vtense(what, "chapotea"),
					      mon_nam(mon));
			    }
			    setmangry(mon);
			    mon->mcansee = 0;
			    tmp = rn1(25, 21);
			    if(((int) mon->mblinded + tmp) > 127)
				mon->mblinded = 127;
			    else mon->mblinded += tmp;
			} else {
			    pline(obj->otyp==CREAM_PIE ? "¡Esplat!" : "¡Esplash!");
			    setmangry(mon);
			}
			if (thrown) obfree(obj, (struct obj *)0);
			else useup(obj);
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			tmp = 0;
			break;
		    case ACID_VENOM: /* thrown (or spit) */
			if (resists_acid(mon)) {
				pline("Tu veneno %s sin daño.",
					mon_nam_acc(mon, "golpea"));
				tmp = 0;
			} else {
				pline("¡Tu veneno %s!", mon_nam_acc(mon, "quema"));
				tmp = dmgval(obj, mon);
			}
			if (thrown) obfree(obj, (struct obj *)0);
			else useup(obj);
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			break;
		    default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/100;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
			/*
			 * Things like silver wands can arrive here so
			 * so we need another silver check.
			 */
			if (objects[obj->otyp].oc_material == SILVER
						&& hates_silver(mdat)) {
				tmp += rnd(20);
				silvermsg = TRUE; silverobj = TRUE;
			}
		    }
		}
	    }
	}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG)
	 *      *OR* if attacking bare-handed!! */

	if (get_dmg_bonus && tmp > 0) {
		tmp += u.udaminc;
		/* If you throw using a propellor, you don't get a strength
		 * bonus but you do get an increase-damage bonus.
		 */
		if(!thrown || !obj || !uwep || !ammo_and_launcher(obj, uwep))
		    tmp += dbon();
	}

	if (valid_weapon_attack) {
	    struct obj *wep;

	    /* to be valid a projectile must have had the correct projector */
	    wep = PROJECTILE(obj) ? uwep : obj;
	    tmp += weapon_dam_bonus(wep);
	    /* [this assumes that `!thrown' implies wielded...] */
	    wtype = thrown ? weapon_type(wep) : uwep_skill_type();
	    use_skill(wtype, 1);
	}

	if (ispoisoned) {
	    int nopoison = (10 - (obj->owt/10));            
	    if(nopoison < 2) nopoison = 2;
	    if Role_if(PM_SAMURAI) {
		pline("¡Usas una arma envenenada sin honor!");
		adjalign(-sgn(u.ualign.type));
	    } else if ((u.ualign.type == A_LAWFUL) && (u.ualign.record > -10)) {
		Te_sientes("como cobarde malvad%c por usar una arma envenenada.",
			poly_gender()==1? 'a' : 'o');
		adjalign(-1);
	    }
	    if (obj && !rn2(nopoison)) {
		const char *que = xname(obj);
		obj->opoisoned = FALSE;
		pline("Tu%s %s ya no %s envenenad%c%s.",
		     plur(obj->quan),
		     (obj->quan == 1L) ? que : makeplural(que),
		     otense(obj, "está"),
		     isfeminine(que)? 'a' : 'o',
		     plur(obj->quan));
	    }
	    if (resists_poison(mon))
		needpoismsg = TRUE;
	    else if (rn2(10))
		tmp += rnd(6);
	    else poiskilled = TRUE;
	}
	if (tmp < 1) {
	    /* make sure that negative damage adjustment can't result
	       in inadvertently boosting the victim's hit points */
	    tmp = 0;
	    if (mdat == &mons[PM_SHADE]) {
		if (!hittxt) {
		    const char *what = unconventional[0] ? unconventional : "ataque";
		    pline("Tu %s %s sin daño través %s.",
		    	what, vtense(what, "pasa"),
			del(mon_nam(mon)));
		    hittxt = TRUE;
		}
	    } else {
		if (get_dmg_bonus) tmp = 1;
	    }
	}

#ifdef STEED
	if (jousting) {
	    tmp += d(2, (obj == uwep) ? 10 : 2);	/* [was in dmgval()] */
	    pline("%s%s%s",
		    canseemon(mon) ? exclam2(tmp) : "",
		    mon_nam_acc(mon, "Justeas"),
		    canseemon(mon) ? exclam(tmp) : ".");
	    if (jousting < 0) {
		pline("¡Tu %s se rompe al impacto!", xname(obj));
		/* (must be either primary or secondary weapon to get here) */
		u.twoweap = FALSE;	/* untwoweapon() is too verbose here */
		if (obj == uwep) uwepgone();		/* set unweapon */
		/* minor side-effect: broken lance won't split puddings */
		useup(obj);
		obj = 0;
	    }
	    /* avoid migrating a dead monster */
	    if (mon->mhp > tmp) {
		mhurtle(mon, u.dx, u.dy, 1);
		mdat = mon->data; /* in case of a polymorph trap */
		if (DEADMONSTER(mon)) already_killed = TRUE;
	    }
	    hittxt = TRUE;
	} else
#endif

	/* VERY small chance of stunning opponent if unarmed. */
	if (unarmed && tmp > 1 && !thrown && !obj && !Upolyd) {
	    if (rnd(100) < P_SKILL(P_BARE_HANDED_COMBAT) &&
			!bigmonst(mdat) && !thick_skinned(mdat)) {
		if (canspotmon(mon))
		    pline("¡%s %s de tu pegado poderoso!",
			  Monnam(mon), stagger(mon->data, "se bambolea"));
		/* avoid migrating a dead monster */
		if (mon->mhp > tmp) {
		    mhurtle(mon, u.dx, u.dy, 1);
		    mdat = mon->data; /* in case of a polymorph trap */
		    if (DEADMONSTER(mon)) already_killed = TRUE;
		}
		hittxt = TRUE;
	    }
	}

	if (!already_killed) mon->mhp -= tmp;
	/* adjustments might have made tmp become less than what
	   a level draining artifact has already done to max HP */
	if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
	if (mon->mhp < 1)
		destroyed = TRUE;
	if (mon->mtame && (!mon->mflee || mon->mfleetim) && tmp > 0) {
		abuse_dog(mon);
		monflee(mon, 10 * rnd(tmp), FALSE, FALSE);
	}
	if((mdat == &mons[PM_BLACK_PUDDING] || mdat == &mons[PM_BROWN_PUDDING])
		   && obj && obj == uwep
		   && objects[obj->otyp].oc_material == IRON
		   && mon->mhp > 1 && !thrown && !mon->mcan
		   /* && !destroyed  -- guaranteed by mhp > 1 */ ) {
		if (clone_mon(mon, 0, 0)) {
			pline("¡%s se divide como lo golpeas!", Monnam(mon));
			hittxt = TRUE;
		}
	}

	if (!hittxt &&			/*( thrown => obj exists )*/
	  (!destroyed || (thrown && m_shot.n > 1 && m_shot.o == obj->otyp))) {
		if (thrown) hit(mshot_xname(obj), mon, exclam(tmp));
		else if (!flags.verbose) pline("Lo golpeas.");
		else
		    pline("%s%s%s",
			  canseemon(mon) ? exclam2(tmp) : "",
			  mon_nam_acc(mon,
			      Role_if(PM_BARBARIAN) ? "Pegas" : "Golpeas"),
			  canseemon(mon) ? exclam(tmp) : ".");
	}

	if (silvermsg) {
		const char *fmt;
		char silverobjbuf[BUFSZ];

		if (canspotmon(mon)) {
		    if (barehand_silver_rings == 1)
			fmt = "¡Tu anillo de plata quema %s%s!";
		    else if (barehand_silver_rings == 2)
			fmt = "¡Tus anillos de plata queman %s%s!";
		    else if (silverobj && saved_oname[0]) {
		    	Sprintf(silverobjbuf, "¡Tu %s%s %s %%s%%s!",
		    		saved_oname,
		    		strstri(saved_oname, "plata") ?
					"" : " de plata",
				vtense(saved_oname, "quema"));
		    	fmt = silverobjbuf;
		    } else
			fmt = "¡La plata quema %s%s!";
		    if (noncorporeal(mdat))
			pline(fmt, "", al(mon_nam(mon)));
		    else
			pline(fmt, "el carne ", del(mon_nam(mon)));
		} else
		    pline("¡Se quema!");
	}

	if (needpoismsg)
		pline("El veneno no parece %s.", mon_nam_acc(mon, "afectar"));
	if (poiskilled) {
		pline("El veneno fue mortal...");
		if (!already_killed) xkilled(mon, 0);
		return FALSE;
	} else if (destroyed) {
		if (!already_killed)
		    killed(mon);	/* takes care of most messages */
	} else if(u.umconf && !thrown) {
		nohandglow(mon);
		if (!mon->mconf && !resist(mon, SPBOOK_CLASS, 0, NOTELL)) {
			mon->mconf = 1;
			if (!mon->mstun && mon->mcanmove && !mon->msleeping &&
				canseemon(mon))
			    pline("%s parece confundid%c.", Monnam(mon),
				mon_gender(mon)? 'a' : 'o');
		}
	}

	return((boolean)(destroyed ? FALSE : TRUE));
}

STATIC_OVL boolean
shade_aware(obj)
struct obj *obj;
{
	if (!obj) return FALSE;
	/*
	 * The things in this list either
	 * 1) affect shades.
	 *  OR
	 * 2) are dealt with properly by other routines
	 *    when it comes to shades.
	 */
	if (obj->otyp == BOULDER || obj->otyp == HEAVY_IRON_BALL
	    || obj->otyp == IRON_CHAIN		/* dmgval handles those first three */
	    || obj->otyp == MIRROR		/* silver in the reflective surface */
	    || obj->otyp == CLOVE_OF_GARLIC	/* causes shades to flee */
	    || objects[obj->otyp].oc_material == SILVER)
		return TRUE;
	return FALSE;
}

/* check whether slippery clothing protects from hug or wrap attack */
/* [currently assumes that you are the attacker] */
STATIC_OVL boolean
m_slips_free(mdef, mattk)
struct monst *mdef;
struct attack *mattk;
{
	struct obj *obj;

	if (mattk->adtyp == AD_DRIN) {
	    /* intelligence drain attacks the head */
	    obj = which_armor(mdef, W_ARMH);
	} else {
	    /* grabbing attacks the body */
	    obj = which_armor(mdef, W_ARMC);		/* cloak */
	    if (!obj) obj = which_armor(mdef, W_ARM);	/* suit */
#ifdef TOURIST
	    if (!obj) obj = which_armor(mdef, W_ARMU);	/* shirt */
#endif
	}

	/* if your cloak/armor is greased, monster slips off; this
	   protection might fail (33% chance) when the armor is cursed */
	if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK) &&
		(!obj->cursed || rn2(3))) {
	    /* avoid "capa resbalosa resbalosa"
	       for undiscovered oilskin cloak */
	    const char *que =
		(obj->greased || objects[obj->otyp].oc_name_known) ?
		xname(obj) : cloak_simple_name(obj);
	    pline("¡%s %s %s%c %s!",
		mattk->adtyp == AD_WRAP ?
			"Resbalas" : "Agarras, pero no puedes mantener tu agarro sobre,",
		mattk->adtyp == AD_WRAP ? del(the(que)) : the(que),
		obj->greased ? "grasad" : "resbalos",
		isfeminine(que)? 'a' : 'o',
		del(mon_nam(mdef)));

	    if (obj->greased && !rn2(2)) {
		pline("La grasa se desgasta.");
		obj->greased = 0;
	    }
	    return TRUE;
	}
	return FALSE;
}

/* used when hitting a monster with a lance while mounted */
STATIC_OVL int	/* 1: joust hit; 0: ordinary hit; -1: joust but break lance */
joust(mon, obj)
struct monst *mon;	/* target */
struct obj *obj;	/* weapon */
{
    int skill_rating, joust_dieroll;

    if (Fumbling || Stunned) return 0;
    /* sanity check; lance must be wielded in order to joust */
    if (obj != uwep && (obj != uswapwep || !u.twoweap)) return 0;

    /* if using two weapons, use worse of lance and two-weapon skills */
    skill_rating = P_SKILL(weapon_type(obj));	/* lance skill */
    if (u.twoweap && P_SKILL(P_TWO_WEAPON_COMBAT) < skill_rating)
	skill_rating = P_SKILL(P_TWO_WEAPON_COMBAT);
    if (skill_rating == P_ISRESTRICTED) skill_rating = P_UNSKILLED; /* 0=>1 */

    /* odds to joust are expert:80%, skilled:60%, basic:40%, unskilled:20% */
    if ((joust_dieroll = rn2(5)) < skill_rating) {
	if (joust_dieroll == 0 && rnl(50) == (50-1) &&
		!unsolid(mon->data) && !obj_resists(obj, 0, 100))
	    return -1;	/* hit that breaks lance */
	return 1;	/* successful joust */
    }
    return 0;	/* no joust bonus; revert to ordinary attack */
}

/*
 * Send in a demon pet for the hero.  Exercise wisdom.
 *
 * This function used to be inline to damageum(), but the Metrowerks compiler
 * (DR4 and DR4.5) screws up with an internal error 5 "Expression Too Complex."
 * Pulling it out makes it work.
 */
STATIC_OVL void
demonpet()
{
	int i;
	struct permonst *pm;
	struct monst *dtmp;

	/* LENGUA:  English, "Some hell-p has arrived!"  The pun doesn't
	   translate */
	pline("¡Ha llegado alguna ayuda!");
	i = !rn2(6) ? ndemon(u.ualign.type) : NON_PM;
	pm = i != NON_PM ? &mons[i] : youmonst.data;
	if ((dtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS)) != 0)
	    (void)tamedog(dtmp, (struct obj *)0);
	exercise(A_WIS, TRUE);
}

/*
 * Player uses theft attack against monster.
 *
 * If the target is wearing body armor, take all of its possesions;
 * otherwise, take one object.  [Is this really the behavior we want?]
 *
 * This routine implicitly assumes that there is no way to be able to
 * resist petfication (ie, be polymorphed into a xorn or golem) at the
 * same time as being able to steal (poly'd into nymph or succubus).
 * If that ever changes, the check for touching a cockatrice corpse
 * will need to be smarter about whether to break out of the theft loop.
 */
STATIC_OVL void
steal_it(mdef, mattk)
struct monst *mdef;
struct attack *mattk;
{
	struct obj *otmp, *stealoid, **minvent_ptr;
	long unwornmask;

	if (!mdef->minvent) return;		/* nothing to take */

	/* look for worn body armor */
	stealoid = (struct obj *)0;
	if (could_seduce(&youmonst, mdef, mattk)) {
	    /* find armor, and move it to end of inventory in the process */
	    minvent_ptr = &mdef->minvent;
	    while ((otmp = *minvent_ptr) != 0)
		if (otmp->owornmask & W_ARM) {
		    if (stealoid) panic("steal_it: multiple worn suits");
		    *minvent_ptr = otmp->nobj;	/* take armor out of minvent */
		    stealoid = otmp;
		    stealoid->nobj = (struct obj *)0;
		} else {
		    minvent_ptr = &otmp->nobj;
		}
	    *minvent_ptr = stealoid;	/* put armor back into minvent */
	}

	if (stealoid) {		/* we will be taking everything */
	    if (gender(mdef) == (int) u.mfemale &&
			youmonst.data->mlet == S_NYMPH)
		pline("%s.  Alegramente te da sus posesiones.",
		    mon_nam_acc(mdef, "Cautivas"));
	    else
		pline("%s y empieza a desvestir.",
		    mon_nam_acc(mdef, "Seduces"));
	}

	while ((otmp = mdef->minvent) != 0) {
	    if (!Upolyd) break;		/* no longer have ability to steal */
	    /* take the object away from the monster */
	    obj_extract_self(otmp);
	    if ((unwornmask = otmp->owornmask) != 0L) {
		mdef->misc_worn_check &= ~unwornmask;
		if (otmp->owornmask & W_WEP) {
		    setmnotwielded(mdef,otmp);
		    MON_NOWEP(mdef);
		}
		otmp->owornmask = 0L;
		update_mon_intrinsics(mdef, otmp, FALSE, FALSE);

		if (otmp == stealoid)	/* special message for final item */
		    pline("%s acaba de quitarse el traje.",
			  Monnam(mdef));
	    }
	    /* give the object to the character */
	    otmp = hold_another_object(otmp, "Agarraste pero dejaste caer %s.",
				       doname(otmp), "Robas: ");
	    if (otmp->where != OBJ_INVENT) continue;
	    if (otmp->otyp == CORPSE &&
		    touch_petrifies(&mons[otmp->corpsenm]) && !uarmg) {
		char kbuf[BUFSZ];

		Sprintf(kbuf, "cadaver de %s robado", mons[otmp->corpsenm].mname);
		instapetrify(kbuf);
		break;		/* stop the theft even if hero survives */
	    }
	    /* more take-away handling, after theft message */
	    if (unwornmask & W_WEP) {		/* stole wielded weapon */
		possibly_unwield(mdef, FALSE);
	    } else if (unwornmask & W_ARMG) {	/* stole worn gloves */
		mselftouch(mdef, (const char *)0, TRUE);
		if (mdef->mhp <= 0)	/* it's now a statue */
		    return;		/* can't continue stealing */
	    }

	    if (!stealoid) break;	/* only taking one item */
	}
}

int
damageum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register struct permonst *pd = mdef->data;
	register int	tmp = d((int)mattk->damn, (int)mattk->damd);
	int armpro;
	boolean negated;

	armpro = magic_negation(mdef);
	/* since hero can't be cancelled, only defender's armor applies */
	negated = !((rn2(3) >= armpro) || !rn2(50));

	if (is_demon(youmonst.data) && !rn2(13) && !uwep
		&& u.umonnum != PM_SUCCUBUS && u.umonnum != PM_INCUBUS
		&& u.umonnum != PM_BALROG) {
	    demonpet();
	    return(0);
	}
	switch(mattk->adtyp) {
	    case AD_STUN:
		if(!Blind)
		    pline("%s %s por un momento.", Monnam(mdef),
				stagger(mdef->data, "se bambolea"));
		mdef->mstun = 1;
		goto physical;
	    case AD_LEGS:
	     /* if (u.ucancelled) { */
	     /*    tmp = 0;	    */
	     /*    break;	    */
	     /* }		    */
		goto physical;
	    case AD_WERE:	    /* no special effect on monsters */
	    case AD_HEAL:	    /* likewise */
	    case AD_PHYS:
 physical:
		if(mattk->aatyp == AT_WEAP) {
		    if(uwep) tmp = 0;
		} else if(mattk->aatyp == AT_KICK) {
		    if(thick_skinned(mdef->data)) tmp = 0;
		    if(mdef->data == &mons[PM_SHADE]) {
			if (!(uarmf && uarmf->blessed)) {
			    impossible("¿mal flujo de función de ataque de espíritu?");
			    tmp = 0;
			} else
			    tmp = rnd(4); /* bless damage */
		    }
		}
		break;
	    case AD_FIRE:
		if (negated) {
		    tmp = 0;
		    break;
		}
		if (!Blind)
		    pline("¡%s %s!", Monnam(mdef),
			  on_fire(mdef->data, mattk));
		if (pd == &mons[PM_STRAW_GOLEM] ||
		    pd == &mons[PM_PAPER_GOLEM]) {
		    if (!Blind)
			pline("¡%s se quema completamente!", Monnam(mdef));
		    xkilled(mdef,2);
		    tmp = 0;
		    break;
		    /* Don't return yet; keep hp<1 and tmp=0 for pet msg */
		}
		tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if (resists_fire(mdef)) {
		    if (!Blind)
			pline("¡El fuego no %s!", mon_nam_acc(mdef, "calienta"));
		    golemeffects(mdef, AD_FIRE, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
	    case AD_COLD:
		if (negated) {
		    tmp = 0;
		    break;
		}
		if (!Blind) pline("¡%s está cubiert%c de hielo!",
			Monnam(mdef), mon_gender(mdef)? 'a' : 'o');
		if (resists_cold(mdef)) {
		    shieldeff(mdef->mx, mdef->my);
		    if (!Blind)
			pline("¡El hielo no %s!", mon_nam_acc(mdef, "resfría"));
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
	    case AD_ELEC:
		if (negated) {
		    tmp = 0;
		    break;
		}
		if (!Blind) pline("¡%s es chocad%c!", Monnam(mdef),
			mon_gender(mdef)? 'a' : 'o');
		tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if (resists_elec(mdef)) {
		    if (!Blind)
			pline("¡El choque no %s!", mon_nam_acc(mdef, "daña"));
		    golemeffects(mdef, AD_ELEC, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
	    case AD_ACID:
		if (resists_acid(mdef)) tmp = 0;
		break;
	    case AD_STON:
		if (!munstone(mdef, TRUE))
		    minstapetrify(mdef, TRUE);
		tmp = 0;
		break;
#ifdef SEDUCE
	    case AD_SSEX:
#endif
	    case AD_SEDU:
	    case AD_SITM:
		steal_it(mdef, mattk);
		tmp = 0;
		break;
	    case AD_SGLD:
#ifndef GOLDOBJ
		if (mdef->mgold) {
		    u.ugold += mdef->mgold;
		    mdef->mgold = 0;
		    pline("Tu billetera siente más pesada.");
		}
#else
                /* This you as a leprechaun, so steal
                   real gold only, no lesser coins */
	        {
		    struct obj *mongold = findgold(mdef->minvent);
	            if (mongold) {
		        obj_extract_self(mongold);  
		        if (merge_choice(invent, mongold) || inv_cnt() < 52) {
			    addinv(mongold);
			    pline("Tu billetera siente más pesada.");
			} else {
			    pline("Agarras el oro %s, pero no encuentras espacio en tu mochila.",
				    del(mon_nam(mdef)));
			    dropy(mongold);
		        }
		    }
	        }
#endif
		exercise(A_DEX, TRUE);
		tmp = 0;
		break;
	    case AD_TLPT:
		if (tmp <= 0) tmp = 1;
		if (!negated && tmp < mdef->mhp) {
		    char nambuf[BUFSZ];
		    boolean u_saw_mon = canseemon(mdef) ||
					(u.uswallow && u.ustuck == mdef);
		    /* record the name before losing sight of monster */
		    Strcpy(nambuf, Monnam(mdef));
		    if (u_teleport_mon(mdef, FALSE) &&
			    u_saw_mon && !canseemon(mdef))
			pline("¡%s de repente desaparece!", nambuf);
		}
		break;
	    case AD_BLND:
		if (can_blnd(&youmonst, mdef, mattk->aatyp, (struct obj*)0)) {
		    if(!Blind && mdef->mcansee)
			pline("Se ciega %s.", Monnam(mdef));
		    mdef->mcansee = 0;
		    tmp += mdef->mblinded;
		    if (tmp > 127) tmp = 127;
		    mdef->mblinded = tmp;
		}
		tmp = 0;
		break;
	    case AD_CURS:
		if (night() && !rn2(10) && !mdef->mcan) {
		    if (mdef->data == &mons[PM_CLAY_GOLEM]) {
			if (!Blind)
			    pline("¡Unas letras se desvanecen de la cabeza %s!",
				del(mon_nam(mdef)));
			xkilled(mdef, 0);
			/* Don't return yet; keep hp<1 and tmp=0 for pet msg */
		    } else {
			mdef->mcan = 1;
			pline("Ríes.");
		    }
		}
		tmp = 0;
		break;
	    case AD_DRLI:
		if (!negated && !rn2(3) && !resists_drli(mdef)) {
			int xtmp = d(2,6);
			pline("¡%s de repente parece más débil!", Monnam(mdef));
			mdef->mhpmax -= xtmp;
			if ((mdef->mhp -= xtmp) <= 0 || !mdef->m_lev) {
				pline("¡Muere %s!", mon_nam(mdef));
				xkilled(mdef,0);
			} else
				mdef->m_lev--;
			tmp = 0;
		}
		break;
	    case AD_RUST:
		if (pd == &mons[PM_IRON_GOLEM]) {
			pline("¡%s se cae en pedazos!", Monnam(mdef));
			xkilled(mdef,0);
		}
		hurtmarmor(mdef, AD_RUST);
		tmp = 0;
		break;
	    case AD_CORR:
		hurtmarmor(mdef, AD_CORR);
		tmp = 0;
		break;
	    case AD_DCAY:
		if (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
			pline("¡%s cae en pedazos!", Monnam(mdef));
			xkilled(mdef,0);
		}
		hurtmarmor(mdef, AD_DCAY);
		tmp = 0;
		break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!negated && !rn2(8)) {
		    const char *que = mpoisons_subj(&youmonst, mattk);
		    pline("¡Tu %s fue envenenad%c!", que,
			  isfeminine(que)? 'a' : 'o');
		    if (resists_poison(mdef))
			pline("El veneno no parece %s.",
				mon_nam_acc(mdef, "afectar"));
		    else {
			if (!rn2(10)) {
			    pline("Tu veneno era mortal...");
			    tmp = mdef->mhp;
			} else tmp += rn1(10,6);
		    }
		}
		break;
	    case AD_DRIN:
		if (notonhead || !has_head(mdef->data)) {
		    pline("%s no parece dañad%c.", Monnam(mdef),
			  mon_gender(mdef)? 'a' : 'o');
		    tmp = 0;
		    if (!Unchanging && mdef->data == &mons[PM_GREEN_SLIME]) {
			if (!Slimed) {
			    pline("Chupas algo de limo y no sientes muy bien.");
			    Slimed = 10L;
			}
		    }
		    break;
		}
		if (m_slips_free(mdef, mattk)) break;

		if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
		    pline("El yelmo %s obstruye tu ataque a su cabeza.",
			  del(mon_nam(mdef)));
		    break;
		}

		pline("¡Comes el cerebro %s!", del(mon_nam(mdef)));
		u.uconduct.food++;
		if (touch_petrifies(mdef->data) && !Stone_resistance && !Stoned) {
		    Stoned = 5;
		    killer_format = KILLED_BY_AN;
		    delayed_killer = mdef->data->mname;
		}
		if (!vegan(mdef->data))
		    u.uconduct.unvegan++;
		if (!vegetarian(mdef->data))
		    violated_vegetarian();
		if (mindless(mdef->data)) {
		    pline("%s no noticia.", mon_nam(mdef));
		    break;
		}
		tmp += rnd(10);
		morehungry(-rnd(30)); /* cannot choke */
		if (ABASE(A_INT) < AMAX(A_INT)) {
			ABASE(A_INT) += rnd(4);
			if (ABASE(A_INT) > AMAX(A_INT))
				ABASE(A_INT) = AMAX(A_INT);
			flags.botl = 1;
		}
		exercise(A_WIS, TRUE);
		break;
	    case AD_STCK:
		if (!negated && !sticks(mdef->data))
		    u.ustuck = mdef; /* it's now stuck to you */
		break;
	    case AD_WRAP:
		if (!sticks(mdef->data)) {
		    if (!u.ustuck && !rn2(10)) {
			if (m_slips_free(mdef, mattk)) {
			    tmp = 0;
			} else {
			    pline("¡Te envuelves alrededor %s!",
				  del(mon_nam(mdef)));
			    u.ustuck = mdef;
			}
		    } else if(u.ustuck == mdef) {
			/* Monsters don't wear amulets of magical breathing */
			if (is_pool(u.ux,u.uy) && !is_swimmer(mdef->data) &&
			    !amphibious(mdef->data)) {
			    pline("%s...", mon_nam_acc(mdef, "Ahogas"));
			    tmp = mdef->mhp;
			} else if(mattk->aatyp == AT_HUGS)
			    pline("%s está siendo estrujad%c.",
				  Monnam(mdef), mon_gender(mdef)? 'a' : 'o');
		    } else {
			tmp = 0;
			if (flags.verbose)
			    pline("Acepillas %s %s.",
				the(mbodypart(mdef, LEG)), del(mon_nam(mdef)));
		    }
		} else tmp = 0;
		break;
	    case AD_PLYS:
		if (!negated && mdef->mcanmove && !rn2(3) && tmp < mdef->mhp) {
		    if (!Blind) pline("¡%s!", mon_nam_acc(mdef, "Paralizas"));
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_SLEE:
		if (!negated && !mdef->msleeping &&
			    sleep_monst(mdef, rnd(10), -1)) {
		    if (!Blind)
			pline("¡Se duerme %s!", mon_nam(mdef));
		    slept_monst(mdef);
		}
		break;
	    case AD_SLIM:
		if (negated) break;	/* physical damage only */
		if (!rn2(4) && !flaming(mdef->data) &&
				mdef->data != &mons[PM_GREEN_SLIME]) {
	    	    pline("%s en limo.", mon_nam_acc(mdef, "Conviertes"));
		    (void) newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, FALSE);
		    tmp = 0;
		}
	    	break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
		/* there's no msomearmor() function, so just do damage */
	     /* if (negated) break; */
		break;
	    case AD_SLOW:
		if (!negated && mdef->mspeed != MSLOW) {
		    unsigned int oldspeed = mdef->mspeed;

		    mon_adjust_speed(mdef, -1, (struct obj *)0);
		    if (mdef->mspeed != oldspeed && canseemon(mdef))
			pline("%s se retarda.", Monnam(mdef));
		}
		break;
	    case AD_CONF:
		if (!mdef->mconf) {
		    if (canseemon(mdef))
			pline("%s se ve confundid%c.", Monnam(mdef),
				mon_gender(mdef)? 'a' : 'o');
		    mdef->mconf = 1;
		}
		break;
	    default:	tmp = 0;
		break;
	}

	mdef->mstrategy &= ~STRAT_WAITFORU; /* in case player is very fast */
	if((mdef->mhp -= tmp) < 1) {
	    if (mdef->mtame && !cansee(mdef->mx,mdef->my)) {
		Te_sientes("avergonzad%c por un momento.",
			poly_gender()==1? 'a' : 'o');
		if (tmp) xkilled(mdef, 0); /* !tmp but hp<1: already killed */
	    } else if (!flags.verbose) {
		pline("¡Lo destrues!");
		if (tmp) xkilled(mdef, 0);
	    } else
		if (tmp) killed(mdef);
	    return(2);
	}
	return(1);
}

STATIC_OVL int
explum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register int tmp = d((int)mattk->damn, (int)mattk->damd);

	pline("¡explotas!");
	switch(mattk->adtyp) {
	    boolean resistance; /* only for cold/fire/elec */

	    case AD_BLND:
		if (!resists_blnd(mdef)) {
		    pline("%s es cegad%c por tu rayo de luz!",
			  Monnam(mdef), mon_gender(mdef)? 'a' : 'o');
		    mdef->mblinded = min((int)mdef->mblinded + tmp, 127);
		    mdef->mcansee = 0;
		}
		break;
	    case AD_HALU:
		if (haseyes(mdef->data) && mdef->mcansee) {
		    pline("%s es afectad%c por tu rayo de luz!",
			  Monnam(mdef), mon_gender(mdef)? 'a' : 'o');
		    mdef->mconf = 1;
		}
		break;
	    case AD_COLD:
		resistance = resists_cold(mdef);
		goto common;
	    case AD_FIRE:
		resistance = resists_fire(mdef);
		goto common;
	    case AD_ELEC:
		resistance = resists_elec(mdef);
common:
		if (!resistance) {
		    pline("¡Se choca %s!", mon_nam(mdef));
		    mdef->mhp -= tmp;
		    if (mdef->mhp <= 0) {
			 killed(mdef);
			 return(2);
		    }
		} else {
		    shieldeff(mdef->mx, mdef->my);
		    if (is_golem(mdef->data))
			golemeffects(mdef, (int)mattk->adtyp, tmp);
		    else
			pline("El choque no parece %s.",
				mon_nam_acc(mdef, "afectar"));
		}
		break;
	    default:
		break;
	}
	return(1);
}

STATIC_OVL void
start_engulf(mdef)
struct monst *mdef;
{
	if (!Invisible) {
		map_location(u.ux, u.uy, TRUE);
		tmp_at(DISP_ALWAYS, mon_to_glyph(&youmonst));
		tmp_at(mdef->mx, mdef->my);
	}
	pline("¡%s!", mon_nam_acc(mdef, "Tragas"));
	delay_output();
	delay_output();
}

STATIC_OVL void
end_engulf()
{
	if (!Invisible) {
		tmp_at(DISP_END, 0);
		newsym(u.ux, u.uy);
	}
}

STATIC_OVL int
gulpum(mdef,mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register int tmp;
	register int dam = d((int)mattk->damn, (int)mattk->damd);
	struct obj *otmp;
	/* Not totally the same as for real monsters.  Specifically, these
	 * don't take multiple moves.  (It's just too hard, for too little
	 * result, to program monsters which attack from inside you, which
	 * would be necessary if done accurately.)  Instead, we arbitrarily
	 * kill the monster immediately for AD_DGST and we regurgitate them
	 * after exactly 1 round of attack otherwise.  -KAA
	 */

	if(mdef->data->msize >= MZ_HUGE) return 0;

	if(u.uhunger < 1500 && !u.uswallow) {
	    for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
		(void) snuff_lit(otmp);

	    if(!touch_petrifies(mdef->data) || Stone_resistance) {
#ifdef LINT	/* static char msgbuf[BUFSZ]; */
		char msgbuf[BUFSZ];
#else
		static char msgbuf[BUFSZ];
#endif
		start_engulf(mdef);
		switch(mattk->adtyp) {
		    case AD_DGST:
			/* eating a Rider or its corpse is fatal */
			if (is_rider(mdef->data)) {
			 pline("Desgraciadamente, digerir alguno de esto es fatal.");
			    end_engulf();
			    Sprintf(msgbuf, "imprudentemente trató comer %s",
				    mdef->data->mname);
			    killer = msgbuf;
			    killer_format = NO_KILLER_PREFIX;
			    done(DIED);
			    return 0;		/* lifesaved */
			}

			if (Slow_digestion) {
			    dam = 0;
			    break;
			}

			/* KMH, conduct */
			u.uconduct.food++;
			if (!vegan(mdef->data))
			     u.uconduct.unvegan++;
			if (!vegetarian(mdef->data))
			     violated_vegetarian();

			/* Use up amulet of life saving */
			if (!!(otmp = mlifesaver(mdef))) m_useup(mdef, otmp);

			newuhs(FALSE);
			xkilled(mdef,2);
			if (mdef->mhp > 0) { /* monster lifesaved */
			    pline("Con mucho priso, regurgitas el fuego en tu %s.",
				body_part(STOMACH));
			} else {
			    tmp = 1 + (mdef->data->cwt >> 8);
			    if (corpse_chance(mdef, &youmonst, TRUE) &&
				!(mvitals[monsndx(mdef->data)].mvflags &
				  G_NOCORPSE)) {
				/* nutrition only if there can be a corpse */
				u.uhunger += (mdef->data->cnutrit+1) / 2;
			    } else tmp = 0;
			    Sprintf(msgbuf, "%s totalmente.",
					    mon_nam_acc(mdef, "Digieres"));
			    if (tmp != 0) {
				/* setting afternmv = end_engulf is tempting,
				 * but will cause problems if the player is
				 * attacked (which uses his real location) or
				 * if his See_invisible wears off
				 */
				pline("%s.", mon_nam_acc(mdef, "Digieres"));
				if (Slow_digestion) tmp *= 2;
				nomul(-tmp);
				nomovemsg = msgbuf;
			    } else pline("%s", msgbuf);
			    if (mdef->data == &mons[PM_GREEN_SLIME]) {
				Sprintf(msgbuf, "%s no se te queda bien.",
					The(mdef->data->mname));
				if (!Unchanging) {
					Slimed = 5L;
					flags.botl = 1;
				}
			    } else
			    exercise(A_CON, TRUE);
			}
			end_engulf();
			return(2);
		    case AD_PHYS:
			if (youmonst.data == &mons[PM_FOG_CLOUD]) {
			    pline("%s está cargad%c por tu humedad.",
				  Monnam(mdef),
				  mon_gender(mdef)? 'a' : 'o');
			    if (amphibious(mdef->data) &&
				!flaming(mdef->data)) {
				dam = 0;
				pline("%s no parece dañad%c.", Monnam(mdef),
				      mon_gender(mdef)? 'a' : 'o');
			    }
			} else
			    pline("¡%s es pegad%c por tus escumbros!",Monnam(mdef),
				  mon_gender(mdef)? 'a' : 'o');
			break;
		    case AD_ACID:
			pline("¡%s es cubrid%c de tu fango!", Monnam(mdef),
				mon_gender(mdef)? 'a' : 'o');
			if (resists_acid(mdef)) {
			    pline("Parece inocuo %s.", al(mon_nam(mdef)));
			    dam = 0;
			}
			break;
		    case AD_BLND:
			if (can_blnd(&youmonst, mdef, mattk->aatyp, (struct obj *)0)) {
			    if (mdef->mcansee)
				pline("¡%s no puede ver allí!", Monnam(mdef));
			    mdef->mcansee = 0;
			    dam += mdef->mblinded;
			    if (dam > 127) dam = 127;
			    mdef->mblinded = dam;
			}
			dam = 0;
			break;
		    case AD_ELEC:
			if (rn2(2)) {
			    pline("El aire alrededor %s crepita de la electricidad.", del(mon_nam(mdef)));
			    if (resists_elec(mdef)) {
				pline("%s no parece dañad%c.", Monnam(mdef),
					mon_gender(mdef)? 'a' : 'o');
				dam = 0;
			    }
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_COLD:
			if (rn2(2)) {
			    if (resists_cold(mdef)) {
				pline("%s parece un poco frí%c.", Monnam(mdef),
					mon_gender(mdef)? 'a' : 'o');
				dam = 0;
			    } else
				pline("¡%s está muriendo del frío!",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_FIRE:
			if (rn2(2)) {
			    if (resists_fire(mdef)) {
				pline("%s parece un poco caliente.", Monnam(mdef));
				dam = 0;
			    } else
				pline("¡%s está quemando al crespo!",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		}
		end_engulf();
		if ((mdef->mhp -= dam) <= 0) {
		    killed(mdef);
		    if (mdef->mhp <= 0)	/* not lifesaved */
			return(2);
		}
		pline("¡%s!", mon_nam_acc(mdef,
		      is_animal(youmonst.data) ? "regurgitas" : "expulsas"));
		if (Slow_digestion || is_animal(youmonst.data)) {
		    pline("Obviamente, no te gustaba el sabor %s.",
			  del(mon_nam(mdef)));
		}
	    } else {
		char kbuf[BUFSZ];

		pline("Muerdes en %s.", mon_nam(mdef));
		Sprintf(kbuf, "tragar %s en todo", an(mdef->data->mname));
		instapetrify(kbuf);
	    }
	}
	return(0);
}

void
missum(mdef,mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	if (could_seduce(&youmonst, mdef, mattk))
		pline("Pretiendes ser amistos%c %s.",
			poly_gender()==1? 'a' : 'o', al(mon_nam(mdef)));
	else if(canspotmon(mdef) && flags.verbose)
		pline("%s.", mon_nam_acc(mdef, "Fallas"));
	else
		pline("Lo fallas.");
	if (!mdef->msleeping && mdef->mcanmove)
		wakeup(mdef);
}

STATIC_OVL boolean
hmonas(mon, tmp)		/* attack monster as a monster. */
register struct monst *mon;
register int tmp;
{
	struct attack *mattk, alt_attk;
	int	i, sum[NATTK], hittmp = 0;
	int	nsum = 0;
	int	dhit = 0;

	for(i = 0; i < NATTK; i++) {

	    sum[i] = 0;
	    mattk = getmattk(youmonst.data, i, sum, &alt_attk);
	    switch(mattk->aatyp) {
		case AT_WEAP:
use_weapon:
	/* Certain monsters don't use weapons when encountered as enemies,
	 * but players who polymorph into them have hands or claws and thus
	 * should be able to use weapons.  This shouldn't prohibit the use
	 * of most special abilities, either.
	 */
	/* Potential problem: if the monster gets multiple weapon attacks,
	 * we currently allow the player to get each of these as a weapon
	 * attack.  Is this really desirable?
	 */
			if (uwep) {
			    hittmp = hitval(uwep, mon);
			    hittmp += weapon_hit_bonus(uwep);
			    tmp += hittmp;
			}
			dhit = (tmp > (dieroll = rnd(20)) || u.uswallow);
			/* KMH -- Don't accumulate to-hit bonuses */
			if (uwep) tmp -= hittmp;
			/* Enemy dead, before any special abilities used */
			if (!known_hitum(mon,&dhit,mattk)) {
			    sum[i] = 2;
			    break;
			} else sum[i] = dhit;
			/* might be a worm that gets cut in half */
			if (m_at(u.ux+u.dx, u.uy+u.dy) != mon) return((boolean)(nsum != 0));
			/* Do not print "You hit" message, since known_hitum
			 * already did it.
			 */
			if (dhit && mattk->adtyp != AD_SPEL
				&& mattk->adtyp != AD_PHYS)
				sum[i] = damageum(mon,mattk);
			break;
		case AT_CLAW:
			if (i==0 && uwep && !cantwield(youmonst.data)) goto use_weapon;
#ifdef SEDUCE
			/* succubi/incubi are humanoid, but their _second_
			 * attack is AT_CLAW, not their first...
			 */
			if (i==1 && uwep && (u.umonnum == PM_SUCCUBUS ||
				u.umonnum == PM_INCUBUS)) goto use_weapon;
#endif
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
		case AT_TENT:
			if (i==0 && uwep && (youmonst.data->mlet==S_LICH)) goto use_weapon;
			if ((dhit = (tmp > rnd(20) || u.uswallow)) != 0) {
			    int compat;

			    if (!u.uswallow &&
				(compat=could_seduce(&youmonst, mon, mattk))) {
				pline("%s %s %s.",
				    mon->mcansee && haseyes(mon->data)
				    ? "Sonríes" : "Hablas",
				    al(mon_nam(mon)),
				    compat == 2 ? "amistosamente":"seductivamente");
				/* doesn't anger it; no wakeup() */
				sum[i] = damageum(mon, mattk);
				break;
			    }
			    wakeup(mon);
			    /* maybe this check should be in damageum()? */
			    if (mon->data == &mons[PM_SHADE] &&
					!(mattk->aatyp == AT_KICK &&
					    uarmf && uarmf->blessed)) {
				pline("Tu ataque pasa sin daño través %s.",
				    del(mon_nam(mon)));
				break;
			    }
			    if (mattk->aatyp == AT_KICK)
				    pline("%s.", mon_nam_acc(mon, "Pateas"));
			    else if (mattk->aatyp == AT_BITE)
				    pline("%s.", mon_nam_acc(mon, "Muerdes"));
			    else if (mattk->aatyp == AT_STNG)
				    pline("%s.", mon_nam_acc(mon, "Pinchas"));
			    else if (mattk->aatyp == AT_BUTT)
				    pline("%s.", mon_nam_acc(mon, "Topeteas"));
			    else if (mattk->aatyp == AT_TUCH)
				    pline("%s.", mon_nam_acc(mon, "Tocas"));
			    else if (mattk->aatyp == AT_TENT)
				    pline("Tus tentáculos %s.", mon_nam_acc(mon, "chupan"));
			    else pline("%s.", mon_nam_acc(mon, "Golpeas"));
			    sum[i] = damageum(mon, mattk);
			} else
			    missum(mon, mattk);
			break;

		case AT_HUGS:
			/* automatic if prev two attacks succeed, or if
			 * already grabbed in a previous attack
			 */
			dhit = 1;
			wakeup(mon);
			if (mon->data == &mons[PM_SHADE])
			    pline("Tu abrazo pasa sin daño través %s.",
				del(mon_nam(mon)));
			else if (!sticks(mon->data) && !u.uswallow) {
			    if (mon==u.ustuck) {
				pline("%s está sendo %s%c.", Monnam(mon),
				    u.umonnum==PM_ROPE_GOLEM ? "estrangulad":
				    "estrujad",
				    mon_gender(mon)? 'a' : 'o');
				sum[i] = damageum(mon, mattk);
			    } else if(i >= 2 && sum[i-1] && sum[i-2]) {
				pline("¡%s!", mon_nam_acc(mon, "Agarras"));
				u.ustuck = mon;
				sum[i] = damageum(mon, mattk);
			    }
			}
			break;

		case AT_EXPL:	/* automatic hit if next to */
			dhit = -1;
			wakeup(mon);
			sum[i] = explum(mon, mattk);
			break;

		case AT_ENGL:
			if((dhit = (tmp > rnd(20+i)))) {
				wakeup(mon);
				if (mon->data == &mons[PM_SHADE])
				    pline("Tu intento de %s pasa sin daño.",
					mon_nam_acc(mon, "tragar"));
				else {
				    sum[i]= gulpum(mon,mattk);
				    if (sum[i] == 2 &&
					    (mon->data->mlet == S_ZOMBIE ||
						mon->data->mlet == S_MUMMY) &&
					    rn2(5) &&
					    !Sick_resistance) {
					Te_sientes("%senferm%c.",
					    (Sick) ? "muy " : "",
					    poly_gender()==1? 'a' : 'o');
					mdamageu(mon, rnd(8));
				    }
				}
			} else
				missum(mon, mattk);
			break;

		case AT_MAGC:
			/* No check for uwep; if wielding nothing we want to
			 * do the normal 1-2 points bare hand damage...
			 */
			if (i==0 && (youmonst.data->mlet==S_KOBOLD
				|| youmonst.data->mlet==S_ORC
				|| youmonst.data->mlet==S_GNOME
				)) goto use_weapon;

		case AT_NONE:
		case AT_BOOM:
			continue;
			/* Not break--avoid passive attacks from enemy */

		case AT_BREA:
		case AT_SPIT:
		case AT_GAZE:	/* all done using #monster command */
			dhit = 0;
			break;

		default: /* Strange... */
			impossible("Ataque extraño tuyo (%d)",
				 mattk->aatyp);
	    }
	    if (dhit == -1) {
		u.mh = -1;	/* dead in the current form */
		rehumanize();
	    }
	    if (sum[i] == 2)
		return((boolean)passive(mon, 1, 0, mattk->aatyp));
							/* defender dead */
	    else {
		(void) passive(mon, sum[i], 1, mattk->aatyp);
		nsum |= sum[i];
	    }
	    if (!Upolyd)
		break; /* No extra attacks if no longer a monster */
	    if (multi < 0)
		break; /* If paralyzed while attacking, i.e. floating eye */
	}
	return((boolean)(nsum != 0));
}

/*	Special (passive) attacks on you by monsters done here.		*/

int
passive(mon, mhit, malive, aatyp)
register struct monst *mon;
register boolean mhit;
register int malive;
uchar aatyp;
{
	register struct permonst *ptr = mon->data;
	register int i, tmp;

	for(i = 0; ; i++) {
	    if(i >= NATTK) return(malive | mhit);	/* no passive attacks */
	    if(ptr->mattk[i].aatyp == AT_NONE) break;	/* try this one */
	}
	/* Note: tmp not always used */
	if (ptr->mattk[i].damn)
	    tmp = d((int)ptr->mattk[i].damn, (int)ptr->mattk[i].damd);
	else if(ptr->mattk[i].damd)
	    tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
	else
	    tmp = 0;

/*	These affect you even if they just died */

	switch(ptr->mattk[i].adtyp) {

	  case AD_ACID:
	    if(mhit && rn2(2)) {
		if (Blind || !flags.verbose)
		    pline("¡Eres salpicad%c!", poly_gender()==1? 'a' : 'o');
		else
		    pline("¡Eres salpicad%c por el ácido %s!",
			poly_gender()==1? 'a' : 'o',
	                del(mon_nam(mon)));

		if (!Acid_resistance)
			mdamageu(mon, tmp);
		if(!rn2(30)) erode_armor(&youmonst, TRUE);
	    }
	    if (mhit) {
		if (aatyp == AT_KICK) {
		    if (uarmf && !rn2(6))
			(void)rust_dmg(uarmf, xname(uarmf), 3, TRUE, &youmonst);
		} else if (aatyp == AT_WEAP || aatyp == AT_CLAW ||
			   aatyp == AT_MAGC || aatyp == AT_TUCH)
		    passive_obj(mon, (struct obj*)0, &(ptr->mattk[i]));
	    }
	    exercise(A_STR, FALSE);
	    break;
	  case AD_STON:
	    if (mhit) {		/* successful attack */
		long protector = attk_protection((int)aatyp);

		/* hero using monsters' AT_MAGC attack is hitting hand to
		   hand rather than casting a spell */
		if (aatyp == AT_MAGC) protector = W_ARMG;

		if (protector == 0L ||		/* no protection */
			(protector == W_ARMG && !uarmg && !uwep) ||
			(protector == W_ARMF && !uarmf) ||
			(protector == W_ARMH && !uarmh) ||
			(protector == (W_ARMC|W_ARMG) && (!uarmc || !uarmg))) {
		    if (!Stone_resistance &&
			    !(poly_when_stoned(youmonst.data) &&
				polymon(PM_STONE_GOLEM))) {
			pline("Te vuelves piedra...");
			done_in_by(mon);
			return 2;
		    }
		}
	    }
	    break;
	  case AD_RUST:
	    if(mhit && !mon->mcan) {
		if (aatyp == AT_KICK) {
		    if (uarmf)
			(void)rust_dmg(uarmf, xname(uarmf), 1, TRUE, &youmonst);
		} else if (aatyp == AT_WEAP || aatyp == AT_CLAW ||
			   aatyp == AT_MAGC || aatyp == AT_TUCH)
		    passive_obj(mon, (struct obj*)0, &(ptr->mattk[i]));
	    }
	    break;
	  case AD_CORR:
	    if(mhit && !mon->mcan) {
		if (aatyp == AT_KICK) {
		    if (uarmf)
			(void)rust_dmg(uarmf, xname(uarmf), 3, TRUE, &youmonst);
		} else if (aatyp == AT_WEAP || aatyp == AT_CLAW ||
			   aatyp == AT_MAGC || aatyp == AT_TUCH)
		    passive_obj(mon, (struct obj*)0, &(ptr->mattk[i]));
	    }
	    break;
	  case AD_MAGM:
	    /* wrath of gods for attacking Oracle */
	    if(Antimagic) {
		shieldeff(u.ux, u.uy);
		pline("¡Un granizo de proyectiles mágicos te falla por poco!");
	    } else {
		pline("¡Proyectiles mágicos aparecen del aire y te golpean!");
		mdamageu(mon, tmp);
	    }
	    break;
	  case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
	    if (mhit) {
		struct obj *obj = (struct obj *)0;

		if (aatyp == AT_KICK) {
		    obj = uarmf;
		    if (!obj) break;
		} else if (aatyp == AT_BITE || aatyp == AT_BUTT ||
			   (aatyp >= AT_STNG && aatyp < AT_WEAP)) {
		    break;		/* no object involved */
		}
		passive_obj(mon, obj, &(ptr->mattk[i]));
	    }
	    break;
	  default:
	    break;
	}

/*	These only affect you if they still live */

	if(malive && !mon->mcan && rn2(3)) {

	    switch(ptr->mattk[i].adtyp) {

	      case AD_PLYS:
		if(ptr == &mons[PM_FLOATING_EYE]) {
		    if (!canseemon(mon)) {
			break;
		    }
		    if(mon->mcansee) {
			if (ureflects("La mirada %s es reflejada por tu%s.",
				    del(mon_nam(mon))))
			    ;
			else if (Free_action)
			    pline("¡Te haces inflexible por un momento bajo la mirada %s!",
				    del(mon_nam(mon)));
			else {
			    pline("¡Eres paralizad%c por la mirada %s!",
				  poly_gender()==1? 'a' : 'o',
				  del(mon_nam(mon)));
			    nomul((ACURR(A_WIS) > 12 || rn2(4)) ? -tmp : -127);
			}
		    } else {
			pline("%s no puede defenderse.",
				Adjmonnam(mon,"ciego"));
			if(!rn2(500)) change_luck(-1);
		    }
		} else if (Free_action) {
		    pline("Te haces inflexible por un momento.");
		} else { /* gelatinous cube */
		    pline("¡Eres paralizad%c por %s!",
				  poly_gender()==1? 'a' : 'o',
				  mon_nam(mon));
	    	    nomovemsg = 0;	/* default: "you can move again" */
		    nomul(-tmp);
		    exercise(A_DEX, FALSE);
		}
		break;
	      case AD_COLD:		/* brown mold or blue jelly */
		if(monnear(mon, u.ux, u.uy)) {
		    if(Cold_resistance) {
			shieldeff(u.ux, u.uy);
			Te_sientes("un poco frí%c.",
				  poly_gender()==1? 'a' : 'o');
			ugolemeffects(AD_COLD, tmp);
			break;
		    }
		    pline("¡Estás de repente muy frí%c!",
			  poly_gender()==1? 'a' : 'o');
		    mdamageu(mon, tmp);
		/* monster gets stronger with your heat! */
		    mon->mhp += tmp / 2;
		    if (mon->mhpmax < mon->mhp) mon->mhpmax = mon->mhp;
		/* at a certain point, the monster will reproduce! */
		    if(mon->mhpmax > ((int) (mon->m_lev+1) * 8))
			(void)split_mon(mon, &youmonst);
		}
		break;
	      case AD_STUN:		/* specifically yellow mold */
		if(!Stunned)
		    make_stunned((long)tmp, TRUE);
		break;
	      case AD_FIRE:
		if(monnear(mon, u.ux, u.uy)) {
		    if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			Te_sientes("un poco caliente.");
			ugolemeffects(AD_FIRE, tmp);
			break;
		    }
		    pline("¡Estás de repente muy caliente!");
		    mdamageu(mon, tmp);
		}
		break;
	      case AD_ELEC:
		if(Shock_resistance) {
		    shieldeff(u.ux, u.uy);
		    pline("Te hormigueas un poco.");
		    ugolemeffects(AD_ELEC, tmp);
		    break;
		}
		pline("¡Eres sacudid%c de electricicad!",
			  poly_gender()==1? 'a' : 'o');
		mdamageu(mon, tmp);
		break;
	      default:
		break;
	    }
	}
	return(malive | mhit);
}

/*
 * Special (passive) attacks on an attacking object by monsters done here.
 * Assumes the attack was successful.
 */
void
passive_obj(mon, obj, mattk)
register struct monst *mon;
register struct obj *obj;	/* null means pick uwep, uswapwep or uarmg */
struct attack *mattk;		/* null means we find one internally */
{
	register struct permonst *ptr = mon->data;
	register int i;

	/* if caller hasn't specified an object, use uwep, uswapwep or uarmg */
	if (!obj) {
	    obj = (u.twoweap && uswapwep && !rn2(2)) ? uswapwep : uwep;
	    if (!obj && mattk->adtyp == AD_ENCH)
		obj = uarmg;		/* no weapon? then must be gloves */
	    if (!obj) return;		/* no object to affect */
	}

	/* if caller hasn't specified an attack, find one */
	if (!mattk) {
	    for(i = 0; ; i++) {
		if(i >= NATTK) return;	/* no passive attacks */
		if(ptr->mattk[i].aatyp == AT_NONE) break; /* try this one */
	    }
	    mattk = &(ptr->mattk[i]);
	}

	switch(mattk->adtyp) {

	case AD_ACID:
	    if(!rn2(6)) {
		erode_obj(obj, TRUE, FALSE);
	    }
	    break;
	case AD_RUST:
	    if(!mon->mcan) {
		erode_obj(obj, FALSE, FALSE);
	    }
	    break;
	case AD_CORR:
	    if(!mon->mcan) {
		erode_obj(obj, TRUE, FALSE);
	    }
	    break;
	case AD_ENCH:
	    if (!mon->mcan) {
		if (drain_item(obj) && carried(obj) &&
		    (obj->known || obj->oclass == ARMOR_CLASS)) {
		    const char *que = xname(obj);
		    pline("Tu%s %s %s menos efectiv%c%s.",
			   plur(obj->quan),
			   que,
			   aobjnam(obj, "parece"),
			   isfeminine(que)? 'a' : 'o',
			   plur(obj->quan));
	    	}
	    	break;
	    }
	  default:
	    break;
	}

	if (carried(obj)) update_inventory();
}

/* Note: caller must ascertain mtmp is mimicking... */
void
stumble_onto_mimic(mtmp)
struct monst *mtmp;
{
	const char *fmt = "¡Espera!  ¡Eso es %s!",
		   *generic = "un monstruo",
		   *what = 0;

	if(!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data,AD_STCK))
	    u.ustuck = mtmp;

	if (Blind) {
	    if (!Blind_telepat)
		what = generic;		/* with default fmt */
	    else if (mtmp->m_ap_type == M_AP_MONSTER)
		what = a_monnam(mtmp);	/* differs from what was sensed */
	} else {
	    int glyph = levl[u.ux+u.dx][u.uy+u.dy].glyph;

	    if (glyph_is_cmap(glyph) &&
		    (glyph_to_cmap(glyph) == S_hcdoor ||
		     glyph_to_cmap(glyph) == S_vcdoor))
		fmt = "¡La puerta de verdad era %s!";
	    else if (glyph_is_object(glyph) &&
		    glyph_to_obj(glyph) == GOLD_PIECE)
		fmt = "¡Ese oro era %s!";

	    /* cloned Wiz starts out mimicking some other monster and
	       might make himself invisible before being revealed */
	    if (mtmp->minvis && !See_invisible)
		what = generic;
	    else
		what = a_monnam(mtmp);
	}
	if (what) pline(fmt, what);

	wakeup(mtmp);	/* clears mimicking */
}

STATIC_OVL void
nohandglow(mon)
struct monst *mon;
{
	char *hands=makeplural(body_part(HAND));

	if (!u.umconf || mon->mconf) return;
	if (u.umconf == 1) {
		if (Blind)
			pline("Tus %s acaban de hormiguear.", hands);
		else
			pline("Tus %s acaban de brillar %s.", hands,
                makeplural(feminize(hcolor(NH_RED), isfeminine(hands))));
	} else {
		if (Blind)
			pline("El hormiguear en tus %s se disminue.", hands);
		else
			pline("Tus %s ya no brillan tan claramente %s.", hands,
				makeplural(feminize(hcolor(NH_RED), isfeminine(hands))));
	}
	u.umconf--;
}

int
flash_hits_mon(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;	/* source of flash */
{
	int tmp, amt, res = 0, useeit = canseemon(mtmp);

	if (mtmp->msleeping) {
	    mtmp->msleeping = 0;
	    if (useeit) {
		pline("El rayo %s.", mon_nam_acc(mtmp, "despierta"));
		res = 1;
	    }
	} else if (mtmp->data->mlet != S_LIGHT) {
	    if (!resists_blnd(mtmp)) {
		tmp = dist2(otmp->ox, otmp->oy, mtmp->mx, mtmp->my);
		if (useeit) {
		    pline("¡%s es cegad%c por el rayo!", Monnam(mtmp),
			mon_gender(mtmp)? 'a' : 'o');
		    res = 1;
		}
		if (mtmp->data == &mons[PM_GREMLIN]) {
		    /* Rule #1: Keep them out of the light. */
		    amt = otmp->otyp == WAN_LIGHT ? d(1 + otmp->spe, 4) :
		          rn2(min(mtmp->mhp,4));
		    pline("¡%s %s!", Monnam(mtmp), amt > mtmp->mhp / 2 ?
			  "lamenta de agonía" : "se grita de dolor");
		    if ((mtmp->mhp -= amt) <= 0) {
			if (flags.mon_moving)
			    monkilled(mtmp, (char *)0, AD_BLND);
			else
			    killed(mtmp);
		    } else if (cansee(mtmp->mx,mtmp->my) && !canspotmon(mtmp)){
			map_invisible(mtmp->mx, mtmp->my);
		    }
		}
		if (mtmp->mhp > 0) {
		    if (!flags.mon_moving) setmangry(mtmp);
		    if (tmp < 9 && !mtmp->isshk && rn2(4)) {
			if (rn2(4))
			    monflee(mtmp, rnd(100), FALSE, TRUE);
			else
			    monflee(mtmp, 0, FALSE, TRUE);
		    }
		    mtmp->mcansee = 0;
		    mtmp->mblinded = (tmp < 3) ? 0 : rnd(1 + 50/tmp);
		}
	    }
	}
	return res;
}

/*uhitm.c*/
