/*	SCCS Id: @(#)polyself.c	3.4	2003/01/08	*/
/*	Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Polymorph self routine.
 *
 * Note:  the light source handling code assumes that both youmonst.m_id
 * and youmonst.mx will always remain 0 when it handles the case of the
 * player polymorphed into a light-emitting monster.
 */

#include "hack.h"

#ifdef OVLB
STATIC_DCL void FDECL(polyman, (const char *,const char *));
STATIC_DCL void NDECL(break_armor);
STATIC_DCL void FDECL(drop_weapon,(int));
STATIC_DCL void NDECL(uunstick);
STATIC_DCL int FDECL(armor_to_dragon,(int));
STATIC_DCL void NDECL(newman);

/* update the youmonst.data structure pointer */
void
set_uasmon()
{
	set_mon_data(&youmonst, &mons[u.umonnum], 0);
}

/* make a (new) human out of the player */
STATIC_OVL void
polyman(fmt, arg)
const char *fmt, *arg;
{
	boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
		was_mimicking = (youmonst.m_ap_type == M_AP_OBJECT);
	boolean could_pass_walls = Passes_walls;
	boolean was_blind = !!Blind;

	if (Upolyd) {
		u.acurr = u.macurr;	/* restore old attribs */
		u.amax = u.mamax;
		u.umonnum = u.umonster;
		flags.female = u.mfemale;
	}
	set_uasmon();

	u.mh = u.mhmax = 0;
	u.mtimedone = 0;
	skinback(FALSE);
	u.uundetected = 0;

	if (sticky) uunstick();
	find_ac();
	if (was_mimicking) {
	    if (multi < 0) unmul("");
	    youmonst.m_ap_type = M_AP_NOTHING;
	}

	newsym(u.ux,u.uy);

	pline(fmt, arg);
	/* check whether player foolishly genocided self while poly'd */
	if ((mvitals[urole.malenum].mvflags & G_GENOD) ||
			(urole.femalenum != NON_PM &&
			(mvitals[urole.femalenum].mvflags & G_GENOD)) ||
			(mvitals[urace.malenum].mvflags & G_GENOD) ||
			(urace.femalenum != NON_PM &&
			(mvitals[urace.femalenum].mvflags & G_GENOD))) {
	    /* intervening activity might have clobbered genocide info */
	    killer = delayed_killer;
	    if (!killer || !strstri(killer, "genocid")) {
		killer_format = KILLED_BY;
		killer = "genocidio de sí mismo";
	    }
	    done(GENOCIDED);
	}

	if (u.twoweap && !could_twoweap(youmonst.data))
	    untwoweapon();

	if (u.utraptype == TT_PIT) {
	    if (could_pass_walls) {	/* player forms cannot pass walls */
		u.utrap = rn1(6,2);
	    }
	}
	if (was_blind && !Blind) {	/* reverting from eyeless */
	    Blinded = 1L;
	    make_blinded(0L, TRUE);	/* remove blindness */
	}

	if(!Levitation && !u.ustuck &&
	   (is_pool(u.ux,u.uy) || is_lava(u.ux,u.uy)))
		spoteffects(TRUE);

	see_monsters();
}

void
change_sex()
{
	/* setting u.umonster for caveman/cavewoman or priest/priestess
	   swap unintentionally makes `Upolyd' appear to be true */
	boolean already_polyd = (boolean) Upolyd;

	/* Some monsters are always of one sex and their sex can't be changed */
	/* succubi/incubi can change, but are handled below */
	/* !already_polyd check necessary because is_male() and is_female()
           are true if the player is a priest/priestess */
	if (!already_polyd || (!is_male(youmonst.data) && !is_female(youmonst.data) && !is_neuter(youmonst.data)))
	    flags.female = !flags.female;
	if (already_polyd)	/* poly'd: also change saved sex */
	    u.mfemale = !u.mfemale;
	max_rank_sz();		/* [this appears to be superfluous] */
	if ((already_polyd ? u.mfemale : flags.female) && urole.name.f)
	    Strcpy(pl_character, urole.name.f);
	else
	    Strcpy(pl_character, urole.name.m);
	u.umonster = ((already_polyd ? u.mfemale : flags.female) && urole.femalenum != NON_PM) ?
			urole.femalenum : urole.malenum;
	if (!already_polyd) {
	    u.umonnum = u.umonster;
	} else if (u.umonnum == PM_SUCCUBUS || u.umonnum == PM_INCUBUS) {
	    flags.female = !flags.female;
	    /* change monster type to match new sex */
	    u.umonnum = (u.umonnum == PM_SUCCUBUS) ? PM_INCUBUS : PM_SUCCUBUS;
	    set_uasmon();
	}
}

STATIC_OVL void
newman()
{
	int tmp, oldlvl;

	tmp = u.uhpmax;
	oldlvl = u.ulevel;
	u.ulevel = u.ulevel + rn1(5, -2);
	if (u.ulevel > 127 || u.ulevel < 1) { /* level went below 0? */
	    u.ulevel = oldlvl; /* restore old level in case they lifesave */
	    goto dead;
	}
	if (u.ulevel > MAXULEV) u.ulevel = MAXULEV;
	/* If your level goes down, your peak level goes down by
	   the same amount so that you can't simply use blessed
	   full healing to undo the decrease.  But if your level
	   goes up, your peak level does *not* undergo the same
	   adjustment; you might end up losing out on the chance
	   to regain some levels previously lost to other causes. */
	if (u.ulevel < oldlvl) u.ulevelmax -= (oldlvl - u.ulevel);
	if (u.ulevelmax < u.ulevel) u.ulevelmax = u.ulevel;

	if (!rn2(10)) change_sex();

	adjabil(oldlvl, (int)u.ulevel);
	reset_rndmonst(NON_PM);	/* new monster generation criteria */

	/* random experience points for the new experience level */
	u.uexp = rndexp(FALSE);

	/* u.uhpmax * u.ulevel / oldlvl: proportionate hit points to new level
	 * -10 and +10: don't apply proportionate HP to 10 of a starting
	 *   character's hit points (since a starting character's hit points
	 *   are not on the same scale with hit points obtained through level
	 *   gain)
	 * 9 - rn2(19): random change of -9 to +9 hit points
	 */
#ifndef LINT
	u.uhpmax = ((u.uhpmax - 10) * (long)u.ulevel / oldlvl + 10) +
		(9 - rn2(19));
#endif

#ifdef LINT
	u.uhp = u.uhp + tmp;
#else
	u.uhp = u.uhp * (long)u.uhpmax/tmp;
#endif

	tmp = u.uenmax;
#ifndef LINT
	u.uenmax = u.uenmax * (long)u.ulevel / oldlvl + 9 - rn2(19);
#endif
	if (u.uenmax < 0) u.uenmax = 0;
#ifndef LINT
	u.uen = (tmp ? u.uen * (long)u.uenmax / tmp : u.uenmax);
#endif

	redist_attr();
	u.uhunger = rn1(500,500);
	if (Sick) make_sick(0L, (char *) 0, FALSE, SICK_ALL);
	Stoned = 0;
	delayed_killer = 0;
	if (u.uhp <= 0 || u.uhpmax <= 0) {
		if (Polymorph_control) {
		    if (u.uhp <= 0) u.uhp = 1;
		    if (u.uhpmax <= 0) u.uhpmax = 1;
		} else {
dead: /* we come directly here if their experience level went to 0 or less */
		    pline("Tu nueva forma no parece bastante sana para sobrevivir.");
		    killer_format = KILLED_BY_AN;
		    killer="polímorfo no exitoso";
		    done(DIED);
		    newuhs(FALSE);
		    return; /* lifesaved */
		}
	}
	newuhs(FALSE);
	polyman(flags.female? "¡Te sientes como una nueva %s!"
			    : "¡Te sientes como un nuevo %s!",
		(flags.female && urace.individual.f) ? urace.individual.f :
		(urace.individual.m) ? urace.individual.m : urace.noun);
	if (Slimed) {
		pline("Tu cuerpo se transforma, pero todavía hay limo sobre ti.");
		Slimed = 10L;
	}
	flags.botl = 1;
	see_monsters();
	(void) encumber_msg();
}

void
polyself(forcecontrol)
boolean forcecontrol;     
{
	char buf[BUFSZ];
	int old_light, new_light;
	int mntmp = NON_PM;
	int tries=0;
	boolean draconian = (uarm &&
				uarm->otyp >= GRAY_DRAGON_SCALE_MAIL &&
				uarm->otyp <= YELLOW_DRAGON_SCALES);
	boolean iswere = (u.ulycn >= LOW_PM || is_were(youmonst.data));
	boolean isvamp = (youmonst.data->mlet == S_VAMPIRE || u.umonnum == PM_VAMPIRE_BAT);
	boolean was_floating = (Levitation || Flying);

        if(!Polymorph_control && !forcecontrol && !draconian && !iswere && !isvamp) {
	    if (rn2(20) > ACURR(A_CON)) {
		pline(shudder_for_moment);
		losehp(rnd(30), "choque de sistema", KILLED_BY_AN);
		exercise(A_CON, FALSE);
		return;
	    }
	}
	old_light = Upolyd ? emits_light(youmonst.data) : 0;

	if (Polymorph_control || forcecontrol) {
		do {
			getlin("¿Hacerte qué tipo de monstruo? [escribe el nombre]",
				buf);
			mntmp = name_to_mon(buf);
			if (mntmp < LOW_PM)
				pline("Nunca he oído de tales monstruos.");
			/* Note:  humans are illegal as monsters, but an
			 * illegal monster forces newman(), which is what we
			 * want if they specified a human.... */
			else if (!polyok(&mons[mntmp]) && !your_race(&mons[mntmp]))
				No_puedes("polimorfar a eso.");
			else break;
		} while(++tries < 5);
		if (tries==5) pline(thats_enough_tries);
		/* allow skin merging, even when polymorph is controlled */
		if (draconian &&
		    (mntmp == armor_to_dragon(uarm->otyp) || tries == 5))
		    goto do_merge;
	} else if (draconian || iswere || isvamp) {
		/* special changes that don't require polyok() */
		if (draconian) {
		    do_merge:
			mntmp = armor_to_dragon(uarm->otyp);
			if (!(mvitals[mntmp].mvflags & G_GENOD)) {
				/* allow G_EXTINCT */
				pline("Te unes con tu armadura escamosa.");
				uskin = uarm;
				uarm = (struct obj *)0;
				/* save/restore hack */
				uskin->owornmask |= I_SPECIAL;
			}
		} else if (iswere) {
			if (is_were(youmonst.data))
				mntmp = PM_HUMAN; /* Illegal; force newman() */
			else
				mntmp = u.ulycn;
		} else {
			if (youmonst.data->mlet == S_VAMPIRE)
				mntmp = PM_VAMPIRE_BAT;
			else
				mntmp = PM_VAMPIRE;
		}
		/* if polymon fails, "you feel" message has been given
		   so don't follow up with another polymon or newman */
		if (mntmp == PM_HUMAN) newman();	/* werecritter */
		else (void) polymon(mntmp);
		goto made_change;    /* maybe not, but this is right anyway */
	}

	if (mntmp < LOW_PM) {
		tries = 0;
		do {
			/* randomly pick an "ordinary" monster */
			mntmp = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
		} while((!polyok(&mons[mntmp]) || is_placeholder(&mons[mntmp]))
				&& tries++ < 200);
	}

	/* The below polyok() fails either if everything is genocided, or if
	 * we deliberately chose something illegal to force newman().
	 */
	if (!polyok(&mons[mntmp]) || !rn2(5) || your_race(&mons[mntmp]))
		newman();
	else if(!polymon(mntmp)) return;

	if (!uarmg) selftouch("No más resistente a la petrificación, ");

 made_change:
	new_light = Upolyd ? emits_light(youmonst.data) : 0;
	if (old_light != new_light) {
	    if (old_light)
		del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
	    if (new_light == 1) ++new_light;  /* otherwise it's undetectable */
	    if (new_light)
		new_light_source(u.ux, u.uy, new_light,
				 LS_MONSTER, (genericptr_t)&youmonst);
	}
	if (is_pool(u.ux,u.uy) && was_floating && !(Levitation || Flying) &&
		!breathless(youmonst.data) && !amphibious(youmonst.data) &&
		!Swimming) drown();
}

/* (try to) make a mntmp monster out of the player */
int
polymon(mntmp)	/* returns 1 if polymorph successful */
int	mntmp;
{
	boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
		was_blind = !!Blind, dochange = FALSE;
	boolean could_pass_walls = Passes_walls;
	int mlvl;

	if (mvitals[mntmp].mvflags & G_GENOD) {	/* allow G_EXTINCT */
		Te_sientes("bastante %s-esc%c.",mons[mntmp].mname,
			flags.female? 'a' : 'o');
		exercise(A_WIS, TRUE);
		return(0);
	}

	/* KMH, conduct */
	u.uconduct.polyselfs++;

	if (!Upolyd) {
		/* Human to monster; save human stats */
		u.macurr = u.acurr;
		u.mamax = u.amax;
		u.mfemale = flags.female;
	} else {
		/* Monster to monster; restore human stats, to be
		 * immediately changed to provide stats for the new monster
		 */
		u.acurr = u.macurr;
		u.amax = u.mamax;
		flags.female = u.mfemale;
	}

	if (youmonst.m_ap_type) {
	    /* stop mimicking immediately */
	    if (multi < 0) unmul("");
	} else if (mons[mntmp].mlet != S_MIMIC) {
	    /* as in polyman() */
	    youmonst.m_ap_type = M_AP_NOTHING;
	}
	if (is_male(&mons[mntmp])) {
		if(flags.female) dochange = TRUE;
	} else if (is_female(&mons[mntmp])) {
		if(!flags.female) dochange = TRUE;
	} else if (!is_neuter(&mons[mntmp]) && mntmp != u.ulycn) {
		if(!rn2(10)) dochange = TRUE;
	}
	if (dochange) {
		flags.female = !flags.female;
		if (flags.female)
		    pline("¡%s %s%s!",
			(u.umonnum != mntmp) ? "Te haces una" : "Te sientes como una nueva",
			mons[mntmp].mname,
			(is_male(&mons[mntmp]) || is_female(&mons[mntmp]))
			    ? "" : " hembra");
		else
		    pline("¡%s %s%s!",
			(u.umonnum != mntmp) ? "Te haces un" : "Te sientes como un nuevo",
			mons[mntmp].mname,
			(is_male(&mons[mntmp]) || is_female(&mons[mntmp]))
			    ? "" : " macho");
	} else {
		if (u.umonnum != mntmp)
			pline("¡Te haces %s!", an(mons[mntmp].mname));
		else
			Te_sientes("como un nuevo %s!", mons[mntmp].mname);
	}
	if (Stoned && poly_when_stoned(&mons[mntmp])) {
		/* poly_when_stoned already checked stone golem genocide */
		pline("¡Te vuelves piedra!");
		mntmp = PM_STONE_GOLEM;
		Stoned = 0;
		delayed_killer = 0;
	}

	u.mtimedone = rn1(500, 500);
	u.umonnum = mntmp;
	set_uasmon();

	/* New stats for monster, to last only as long as polymorphed.
	 * Currently only strength gets changed.
	 */
	if(strongmonst(&mons[mntmp])) ABASE(A_STR) = AMAX(A_STR) = STR18(100);

	if (Stone_resistance && Stoned) { /* parnes@eniac.seas.upenn.edu */
		Stoned = 0;
		delayed_killer = 0;
		pline("Ya no pareces estar petrificándote.");
	}
	if (Sick_resistance && Sick) {
		make_sick(0L, (char *) 0, FALSE, SICK_ALL);
		pline("Ya no sientes enferm%c.",
			poly_gender()==1? 'a' : 'o');
	}
	if (Slimed) {
	    if (flaming(youmonst.data)) {
		pline("¡El limo se quema!");
		Slimed = 0L;
		flags.botl = 1;
	    } else if (mntmp == PM_GREEN_SLIME) {
		/* do it silently */
		Slimed = 0L;
		flags.botl = 1;
	    }
	}
	if (nohands(youmonst.data)) Glib = 0;

	/*
	mlvl = adj_lev(&mons[mntmp]);
	 * We can't do the above, since there's no such thing as an
	 * "experience level of you as a monster" for a polymorphed character.
	 */
	mlvl = (int)mons[mntmp].mlevel;
	if (youmonst.data->mlet == S_DRAGON && mntmp >= PM_GRAY_DRAGON) {
		u.mhmax = In_endgame(&u.uz) ? (8*mlvl) : (4*mlvl + d(mlvl,4));
	} else if (is_golem(youmonst.data)) {
		u.mhmax = golemhp(mntmp);
	} else {
		if (!mlvl) u.mhmax = rnd(4);
		else u.mhmax = d(mlvl, 8);
		if (is_home_elemental(&mons[mntmp])) u.mhmax *= 3;
	}
	u.mh = u.mhmax;

	if (u.ulevel < mlvl) {
	/* Low level characters can't become high level monsters for long */
#ifdef DUMB
		/* DRS/NS 2.2.6 messes up -- Peter Kendell */
		int mtd = u.mtimedone, ulv = u.ulevel;

		u.mtimedone = mtd * ulv / mlvl;
#else
		u.mtimedone = u.mtimedone * u.ulevel / mlvl;
#endif
	}

	if (uskin && mntmp != armor_to_dragon(uskin->otyp))
		skinback(FALSE);
	break_armor();
	drop_weapon(1);
	if (hides_under(youmonst.data))
		u.uundetected = OBJ_AT(u.ux, u.uy);
	else if (youmonst.data->mlet == S_EEL)
		u.uundetected = is_pool(u.ux, u.uy);
	else
		u.uundetected = 0;

	if (u.utraptype == TT_PIT) {
	    if (could_pass_walls && !Passes_walls) {
		u.utrap = rn1(6,2);
	    } else if (!could_pass_walls && Passes_walls) {
		u.utrap = 0;
	    }
	}
	if (was_blind && !Blind) {	/* previous form was eyeless */
	    Blinded = 1L;
	    make_blinded(0L, TRUE);	/* remove blindness */
	}
	newsym(u.ux,u.uy);		/* Change symbol */

	if (!sticky && !u.uswallow && u.ustuck && sticks(youmonst.data)) u.ustuck = 0;
	else if (sticky && !sticks(youmonst.data)) uunstick();
#ifdef STEED
	if (u.usteed) {
	    if (touch_petrifies(u.usteed->data) &&
	    		!Stone_resistance && rnl(3)) {
	    	char buf[BUFSZ];

	    	pline("No más resistente a la petrificación, %s.",
	    			mon_nam_acc(u.usteed, "tocas"));
	    	Sprintf(buf, "cabalgar %s", an(u.usteed->data->mname));
	    	instapetrify(buf);
 	    }
	    if (!can_ride(u.usteed)) dismount_steed(DISMOUNT_POLY);
	}
#endif

	if (flags.verbose) {
	    static const char use_thec[] = "Usa el mando #%s para %s.";
	    static const char monsterc[] = "monstruo";
	    if (can_breathe(youmonst.data))
		pline(use_thec,monsterc,"usar tu arma de respira");
	    if (attacktype(youmonst.data, AT_SPIT))
		pline(use_thec,monsterc,"escupir veneno");
	    if (youmonst.data->mlet == S_NYMPH)
		pline(use_thec,monsterc,"quitarte una bola de hierro");
	    if (attacktype(youmonst.data, AT_GAZE))
		pline(use_thec,monsterc,"mirar los monstruos");
	    if (is_hider(youmonst.data))
		pline(use_thec,monsterc,"esconderte");
	    if (is_were(youmonst.data))
		pline(use_thec,monsterc,"llamar ayuda");
	    if (webmaker(youmonst.data))
		pline(use_thec,monsterc,"hilar una telaraña");
	    if (u.umonnum == PM_GREMLIN)
		pline(use_thec,monsterc,"multiplicar en una fuente");
	    if (is_unicorn(youmonst.data))
		pline(use_thec,monsterc,"usar tu cuerno");
	    if (is_mind_flayer(youmonst.data))
		pline(use_thec,monsterc,"emitir una ráfaga mental");
	    if (youmonst.data->msound == MS_SHRIEK) /* worthless, actually */
		pline(use_thec,monsterc,"gritar");
	    if (lays_eggs(youmonst.data) && flags.female)
		pline(use_thec,"sentar","poner un huevo");
	}
	/* you now know what an egg of your type looks like */
	if (lays_eggs(youmonst.data)) {
	    learn_egg_type(u.umonnum);
	    /* make queen bees recognize killer bee eggs */
	    learn_egg_type(egg_type_from_parent(u.umonnum, TRUE));
	}
	find_ac();
	if((!Levitation && !u.ustuck && !Flying &&
	    (is_pool(u.ux,u.uy) || is_lava(u.ux,u.uy))) ||
	   (Underwater && !Swimming))
	    spoteffects(TRUE);
	if (Passes_walls && u.utrap && u.utraptype == TT_INFLOOR) {
	    u.utrap = 0;
	    pline("La roca ya no parece tramparte.");
	} else if (likes_lava(youmonst.data) && u.utrap && u.utraptype == TT_LAVA) {
	    u.utrap = 0;
	    pline("La lava ahora siente sosegante.");
	}
	if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
	    if (Punished) {
		pline("Te resbalas afuera de la cadena de hierro.");
		unpunish();
	    }
	}
	if (u.utrap && (u.utraptype == TT_WEB || u.utraptype == TT_BEARTRAP) &&
		(amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data) ||
		  (youmonst.data->msize <= MZ_SMALL && u.utraptype == TT_BEARTRAP))) {
	    pline("Ya no estás pegad%c a la %s.",
		    poly_gender()==1? 'a' : 'o',
		    u.utraptype == TT_WEB ? "telaraña" : "trampa para osos");
	    /* probably should burn webs too if PM_FIRE_ELEMENTAL */
	    u.utrap = 0;
	}
	if (webmaker(youmonst.data) && u.utrap && u.utraptype == TT_WEB) {
	    pline("Te orientes en la telaraña.");
	    u.utrap = 0;
	}
	flags.botl = 1;
	vision_full_recalc = 1;
	see_monsters();
	exercise(A_CON, FALSE);
	exercise(A_WIS, TRUE);
	(void) encumber_msg();
	return(1);
}

STATIC_OVL void
break_armor()
{
    register struct obj *otmp;

    if (breakarm(youmonst.data)) {
	if ((otmp = uarm) != 0) {
		if (donning(otmp)) cancel_don();
		pline("¡Te rompes afuera de tu armadura!");
		exercise(A_STR, FALSE);
		(void) Armor_gone();
		useup(otmp);
	}
	if ((otmp = uarmc) != 0) {
	    if(otmp->oartifact) {
		pline("¡Tu %s se cae!", cloak_simple_name(otmp));
		(void) Cloak_off();
		dropx(otmp);
	    } else {
		pline("¡Tu %s se rasga!", cloak_simple_name(otmp));
		(void) Cloak_off();
		useup(otmp);
	    }
	}
#ifdef TOURIST
	if (uarmu) {
		pline("¡Tu camisa se rasga a trisas!");
		useup(uarmu);
	}
#endif
    } else if (sliparm(youmonst.data)) {
	if (((otmp = uarm) != 0) && (racial_exception(&youmonst, otmp) < 1)) {
		if (donning(otmp)) cancel_don();
		pline("¡Tu armadura se cae a tu alrededor!");
		(void) Armor_gone();
		dropx(otmp);
	}
	if ((otmp = uarmc) != 0) {
		if (is_whirly(youmonst.data)) {
			const char *que = cloak_simple_name(otmp);
			pline("¡Tu %s se cae, insostenid%c!",
				que, isfeminine(que)? 'a' : 'o');
		}
		else pline("¡Contraes afuera de tu %s!", cloak_simple_name(otmp));
		(void) Cloak_off();
		dropx(otmp);
	}
#ifdef TOURIST
	if ((otmp = uarmu) != 0) {
		if (is_whirly(youmonst.data))
			pline("¡Te filtras través de tu camisa!");
		else pline("¡Te haces demasiadísimo pequeñ%c para tu camisa!",
				poly_gender()==1? 'a' : 'o');
		setworn((struct obj *)0, otmp->owornmask & W_ARMU);
		dropx(otmp);
	}
#endif
    }
    if (has_horns(youmonst.data)) {
	if ((otmp = uarmh) != 0) {
	    if (is_flimsy(otmp) && !donning(otmp)) {
		char yourbuf[BUFSZ];
		char *npl = plur(num_horns(youmonst.data));
		char *vpl = (num_horns(youmonst.data)>1)? "n" : "";

		/* Future possiblities: This could damage/destroy helmet */
		pline("Tu%s cuerno%s atraviesa%s %s.",
			npl, npl, vpl, shk_your(yourbuf, otmp, (char *)0));
	    } else {
		if (donning(otmp)) cancel_don();
		pline("¡Tu yelmo se cae %s!", al(surface(u.ux, u.uy)));
		(void) Helmet_off();
		dropx(otmp);
	    }
	}
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data)) {
	if ((otmp = uarmg) != 0) {
	    if (donning(otmp)) cancel_don();
	    /* Drop weapon along with gloves */
	    pline("¡Dejas caer tus guantes%s!", uwep ? " y arma" : "");
	    drop_weapon(0);
	    (void) Gloves_off();
	    dropx(otmp);
	}
	if ((otmp = uarms) != 0) {
	    pline("¡Ya no puedes retener tu escudo!");
	    (void) Shield_off();
	    dropx(otmp);
	}
	if ((otmp = uarmh) != 0) {
	    if (donning(otmp)) cancel_don();
	    pline("¡Tu yelmo se cae %s!", al(surface(u.ux, u.uy)));
	    (void) Helmet_off();
	    dropx(otmp);
	}
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data) ||
		slithy(youmonst.data) || youmonst.data->mlet == S_CENTAUR) {
	if ((otmp = uarmf) != 0) {
	    if (donning(otmp)) cancel_don();
	    if (is_whirly(youmonst.data))
		pline("¡Tus botas se caen!");
	    else pline("¡Tus botas se %s afuera de tus pies!",
			verysmall(youmonst.data) ? "resbalan" : "empujan");
	    (void) Boots_off();
	    dropx(otmp);
	}
    }
}

STATIC_OVL void
drop_weapon(alone)
int alone;
{
    struct obj *otmp;
    struct obj *otmp2;

    if ((otmp = uwep) != 0) {
	/* !alone check below is currently superfluous but in the
	 * future it might not be so if there are monsters which cannot
	 * wear gloves but can wield weapons
	 */
	if (!alone || cantwield(youmonst.data)) {
	    struct obj *wep = uwep;

	    if (alone) pline("¡Descubres que debes dejar caer tu%s arma%s!",
			   	u.twoweap ? "s" : "",
			   	u.twoweap ? "s" : "");
	    otmp2 = u.twoweap ? uswapwep : 0;
	    uwepgone();
	    if (!wep->cursed || wep->otyp != LOADSTONE)
		dropx(otmp);
	    if (otmp2 != 0) {
		uswapwepgone();
		if (!otmp2->cursed || otmp2->otyp != LOADSTONE)
		    dropx(otmp2);
	    }
	    untwoweapon();
	} else if (!could_twoweap(youmonst.data)) {
	    untwoweapon();
	}
    }
}

void
rehumanize()
{
	char buf[80];
	char fem = flags.female? 'a' : 'o';
	/* You can't revert back while unchanging */
	if (Unchanging && (u.mh < 1)) {
		killer_format = NO_KILLER_PREFIX;
		Sprintf(buf, "matad%c mientras trampad%c en forma de criatura",
			fem, fem);
		killer = buf;
		done(DIED);
	}

	if (emits_light(youmonst.data))
	    del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
	polyman("¡Vuelves a forma %s!", feminize(urace.adj, TRUE));

	if (u.uhp < 1) {
	    char kbuf[256];

	    Sprintf(kbuf, "revertir a forma %s insaludable", urace.adj);
	    killer_format = KILLED_BY;
	    killer = kbuf;
	    done(DIED);
	}
	if (!uarmg) selftouch("No más resistente a la petrificación, ");
	nomul(0);

	flags.botl = 1;
	vision_full_recalc = 1;
	(void) encumber_msg();
}

int
dobreathe()
{
	struct attack *mattk;

	if (Strangled) {
	    No_puedes("respirar.  Lo siento.");
	    return(0);
	}
	if (u.uen < 15) {
	    pline("¡No tienes bastante energía para respirar!");
	    return(0);
	}
	u.uen -= 15;
	flags.botl = 1;

	if (!getdir((char *)0)) return(0);

	mattk = attacktype_fordmg(youmonst.data, AT_BREA, AD_ANY);
	if (!mattk)
	    impossible("¿mal ataque de respiración?");
	else
	    buzz((int) (20 + mattk->adtyp-1), (int)mattk->damn,
		u.ux, u.uy, u.dx, u.dy);
	return(1);
}

int
dospit()
{
	struct obj *otmp;

	if (!getdir((char *)0)) return(0);
	otmp = mksobj(u.umonnum==PM_COBRA ? BLINDING_VENOM : ACID_VENOM,
			TRUE, FALSE);
	otmp->spe = 1; /* to indicate it's yours */
	throwit(otmp, 0L, FALSE);
	return(1);
}

int
doremove()
{
	if (!Punished) {
		pline("¡No estás encadenad%c a nada!",
			poly_gender()==1? 'a' : 'o');
		return(0);
	}
	unpunish();
	return(1);
}

int
dospinweb()
{
	register struct trap *ttmp = t_at(u.ux,u.uy);

	if (Levitation || Is_airlevel(&u.uz)
	    || Underwater || Is_waterlevel(&u.uz)) {
		pline("Debes estar en el suelo para hilar una telaraña.");
		return(0);
	}
	if (u.uswallow) {
		pline("Libres flúido de telaraña dentro %s.", del(mon_nam(u.ustuck)));
		if (is_animal(u.ustuck->data)) {
			expels(u.ustuck, u.ustuck->data, TRUE);
			return(0);
		}
		if (is_whirly(u.ustuck->data)) {
			int i;

			for (i = 0; i < NATTK; i++)
				if (u.ustuck->data->mattk[i].aatyp == AT_ENGL)
					break;
			if (i == NATTK)
			       impossible("¿Tragador no tiene ataque de engolfar?");
			else {
				char sweep[30];

				sweep[0] = '\0';
				switch(u.ustuck->data->mattk[i].adtyp) {
					case AD_FIRE:
						Strcpy(sweep, "se enciende y ");
						break;
					case AD_ELEC:
						Strcpy(sweep, "se fríe y ");
						break;
					case AD_COLD:
						Strcpy(sweep,
						      "se congiela, se rompe y ");
						break;
				}
				pline("¡La telaraña %sse barre!", sweep);
			}
			return(0);
		}		     /* default: a nasty jelly-like creature */
		pline("La telaraña se disuelve en %s.", mon_nam(u.ustuck));
		return(0);
	}
	if (u.utrap) {
		No_puedes("hilar telarañas mientras pegad%c en una trampa.",
			poly_gender()==1? 'a' : 'o');
		return(0);
	}
	exercise(A_DEX, TRUE);
	if (ttmp) switch (ttmp->ttyp) {
		case PIT:
		case SPIKED_PIT: pline("Hilas una telaraña, cubriendo el hoyo.");
			deltrap(ttmp);
			bury_objs(u.ux, u.uy);
			newsym(u.ux, u.uy);
			return(1);
		case SQKY_BOARD: pline("Se amortigua el tablero rechinante");
			deltrap(ttmp);
			newsym(u.ux, u.uy);
			return(1);
		case TELEP_TRAP:
		case LEVEL_TELEP:
		case MAGIC_PORTAL:
			pline("¡Tu telaraña se desvanece!");
			return(0);
		case WEB: pline("Haces más densa la telaraña.");
			return(1);
		case HOLE:
		case TRAPDOOR:
			pline("Hilas una telaraña, cubriendo la %s.",
			    (ttmp->ttyp == TRAPDOOR) ? "trampa" : "abertura");
			deltrap(ttmp);
			newsym(u.ux, u.uy);
			return 1;
		case ROLLING_BOULDER_TRAP:
			pline("Hilas una telaraña, obstruyendo el gatillo.");
			deltrap(ttmp);
			newsym(u.ux, u.uy);
			return(1);
		case ARROW_TRAP:
		case DART_TRAP:
		case BEAR_TRAP:
		case ROCKTRAP:
		case FIRE_TRAP:
		case LANDMINE:
		case SLP_GAS_TRAP:
		case RUST_TRAP:
		case MAGIC_TRAP:
		case ANTI_MAGIC:
		case POLY_TRAP:
			pline("¡Has disparado una trampa!");
			dotrap(ttmp, 0);
			return(1);
		default:
			impossible("¿Telaraña sobre trampa de tipo %d?", ttmp->ttyp);
			return(0);
		}
	else if (On_stairs(u.ux, u.uy)) {
	    /* cop out: don't let them hide the stairs */
	    pline("Tu telaraña falla de impedir acceso a la escalera%s.",
		 (levl[u.ux][u.uy].typ == STAIRS) ? "" : " de mano");
	    return(1);
		 
	}
	ttmp = maketrap(u.ux, u.uy, WEB);
	if (ttmp) {
		ttmp->tseen = 1;
		ttmp->madeby_u = 1;
	}
	newsym(u.ux, u.uy);
	return(1);
}

int
dosummon()
{
	int placeholder;
	if (u.uen < 10) {
	    pline("¡Te falta la energía para llamar ayuda!");
	    return(0);
	}
	u.uen -= 10;
	flags.botl = 1;

	pline("¡Pides a todos tus hermanos para ayuda!");
	exercise(A_WIS, TRUE);
	if (!were_summon(youmonst.data, TRUE, &placeholder, (char *)0))
		pline("Pero ningunos vienen.");
	return(1);
}

int
dogaze()
{
	register struct monst *mtmp;
	int looked = 0;
	char qbuf[QBUFSZ];
	int i;
	uchar adtyp = 0;

	for (i = 0; i < NATTK; i++) {
	    if(youmonst.data->mattk[i].aatyp == AT_GAZE) {
		adtyp = youmonst.data->mattk[i].adtyp;
		break;
	    }
	}
	if (adtyp != AD_CONF && adtyp != AD_FIRE) {
	    impossible("¿ataque de mirada %d?", adtyp);
	    return 0;
	}


	if (Blind) {
	    No_puedes("ver nada que puedes mirar.");
	    return 0;
	}
	if (u.uen < 15) {
	    pline("¡Te falta la energía para usar tu mirada especial!");
	    return(0);
	}
	u.uen -= 15;
	flags.botl = 1;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
		looked++;
		if (Invis && !perceives(mtmp->data))
		    pline("Parece que %s no noticie tu mirada.", mon_nam(mtmp));
		else if (mtmp->minvis && !See_invisible)
		    No_puedes("ver donde mirar %s.", mon_nam(mtmp));
		else if (mtmp->m_ap_type == M_AP_FURNITURE
			|| mtmp->m_ap_type == M_AP_OBJECT) {
		    looked--;
		    continue;
		} else if (flags.safe_dog && !Confusion && !Hallucination
		  && mtmp->mtame) {
		    pline("Evitas mirar %s.", y_monnam(mtmp));
		} else {
		    if (flags.confirm && mtmp->mpeaceful && !Confusion
							&& !Hallucination) {
 			Sprintf(qbuf, "¿Realmente %s?",
				mon_nam_acc(mtmp,
				(adtyp == AD_CONF) ? "confundir" : "atacar"));
			if (yn(qbuf) != 'y') continue;
			setmangry(mtmp);
		    }
		    if (!mtmp->mcanmove || mtmp->mstun || mtmp->msleeping ||
				    !mtmp->mcansee || !haseyes(mtmp->data)) {
			looked--;
			continue;
		    }
		    /* No reflection check for consistency with when a monster
		     * gazes at *you*--only medusa gaze gets reflected then.
		     */
		    if (adtyp == AD_CONF) {
			if (!mtmp->mconf)
			    pline("¡Tu mirada %s!", mon_nam_acc(mtmp, "confunde"));
			else
			    pline("%s se pone cada vez más confundid%c.",
				    Monnam(mtmp), mon_gender(mtmp)? 'a' : 'o');
			mtmp->mconf = 1;
		    } else if (adtyp == AD_FIRE) {
			int dmg = d(2,6);
			pline("¡%s con una mirada fogosa!", mon_nam_acc(mtmp, "Ataques"));
			if (resists_fire(mtmp)) {
			    pline("¡El fuego no %s!", mon_nam_acc(mtmp, "quema"));
			    dmg = 0;
			}
			if((int) u.ulevel > rn2(20))
			    (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
			if((int) u.ulevel > rn2(20))
			    (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
			if((int) u.ulevel > rn2(25))
			    (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
			if (dmg && !DEADMONSTER(mtmp)) mtmp->mhp -= dmg;
			if (mtmp->mhp <= 0) killed(mtmp);
		    }
		    /* For consistency with passive() in uhitm.c, this only
		     * affects you if the monster is still alive.
		     */
		    if (!DEADMONSTER(mtmp) &&
			  (mtmp->data==&mons[PM_FLOATING_EYE]) && !mtmp->mcan) {
			if (!Free_action) {
			    pline("¡Eres paralizad%c por la mirada %s!",
					poly_gender()==1? 'a' : 'o',
					del(mon_nam(mtmp)));
			    nomul((u.ulevel > 6 || rn2(4)) ?
				    -d((int)mtmp->m_lev+1,
					    (int)mtmp->data->mattk[0].damd)
				    : -200);
			    return 1;
			} else
			    pline("Te atiesas por un momento bajo la mirada %s.",
				    del(mon_nam(mtmp)));
		    }
		    /* Technically this one shouldn't affect you at all because
		     * the Medusa gaze is an active monster attack that only
		     * works on the monster's turn, but for it to *not* have an
		     * effect would be too weird.
		     */
		    if (!DEADMONSTER(mtmp) &&
			    (mtmp->data == &mons[PM_MEDUSA]) && !mtmp->mcan) {
			pline(
			 "Mirar a la %s despierta no es una idea muy buena.",
			    l_monnam(mtmp));
			/* as if gazing at a sleeping anything is fruitful... */
			pline("Te vuelves piedra...");
			killer_format = KILLED_BY;
			killer = "encontrar deliberadamente la mirada de Medusa";
			done(STONING);
		    }
		}
	    }
	}
	if (!looked) pline("Miras ningún lugar de particular.");
	return 1;
}

int
dohide()
{
	boolean ismimic = youmonst.data->mlet == S_MIMIC;

	if (u.uundetected || (ismimic && youmonst.m_ap_type != M_AP_NOTHING)) {
		pline("Ya estás escondiéndote.");
		return(0);
	}
	if (ismimic) {
		/* should bring up a dialog "what would you like to imitate?" */
		youmonst.m_ap_type = M_AP_OBJECT;
		youmonst.mappearance = STRANGE_OBJECT;
	} else
		u.uundetected = 1;
	newsym(u.ux,u.uy);
	return(1);
}

int
domindblast()
{
	struct monst *mtmp, *nmon;

	if (u.uen < 10) {
	    pline("Concentras, pero te falta la energía para mantenerlo.");
	    return(0);
	}
	u.uen -= 10;
	flags.botl = 1;

	pline("Concentras.");
	pline("Una onda de energía psíquica se flúe.");
	for(mtmp=fmon; mtmp; mtmp = nmon) {
		int u_sen;

		nmon = mtmp->nmon;
		if (DEADMONSTER(mtmp))
			continue;
		if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM)
			continue;
		if(mtmp->mpeaceful)
			continue;
		u_sen = telepathic(mtmp->data) && !mtmp->mcansee;
		if (u_sen || (telepathic(mtmp->data) && rn2(2)) || !rn2(10)) {
			pline("Te agarras a la %s %s.",
				u_sen ? "telepatía" :
				telepathic(mtmp->data) ? "telepatía latente" :
				"mente",
				del(mon_nam(mtmp)));
			mtmp->mhp -= rnd(15);
			if (mtmp->mhp <= 0)
				killed(mtmp);
		}
	}
	return 1;
}

STATIC_OVL void
uunstick()
{
	pline("%s ya no está más en tus garras.", Monnam(u.ustuck));
	u.ustuck = 0;
}

void
skinback(silently)
boolean silently;
{
	if (uskin) {
		pline("Tu piel vuelve a su forma original.");
		uarm = uskin;
		uskin = (struct obj *)0;
		/* undo save/restore hack */
		uarm->owornmask &= ~I_SPECIAL;
	}
}

#endif /* OVLB */
#ifdef OVL1

/* Formerly mbodypart; that function now wraps around this one to implement
   the gender inflection */
static const char *
mbodypart_core(mon, part)
struct monst *mon;
int part;
{
	/* LENGUA:  The entries for HANDED and LIGHT_HEADED may contain an @,
	   to be replaced with 'o' or 'a' according to gender. */
	static NEARDATA const char
	*humanoid_parts[] = { "brazo", "ojo", "cara", "dedo", "punta de dedo",
		"pie", "mano", "de mano", "cabeza", "pierna",
		"liger@ de cascos", "cuello", "espinazo", "dedo del pie",
		"cabello", "sangre", "pulmón", "nariz", "estómago" },
	*jelly_parts[] = { "seudópodo", "mancha oscura", "frente",
		"extensión de seudópodo", "extremidad de seudópodo",
		"raíz de seudópodo", "agarro", "de agarro", "área cerebral",
		"seudópodo bajo", "viscos@", "medio", "superficie",
		"extremidad de seudópodo", "ondas", "jugos", "superficie",
		"sensor", "estómago" },
	*animal_parts[] = { "miembro delantero", "ojo", "cara", "uña delantera",
		"punta de uña", "uña trasera", "uña delantera", "uñad@",
		"cabeza", "miembro trasero", "liger@ de cascos", "cerviz",
		"espinazo", "punta de uña trasera", "piel", "sangre", "pulmón",
		"nariz", "estómago" },
	*bird_parts[] = { "ala", "ojo", "cara", "ala", "punta de ala",
		"pata", "ala", "alad@", "cabeza", "pierna",
		"liger@ de cascos", "cuello", "espinazo", "dedo del pie",
		"plumas", "sangre", "pulmón", "nariz", "estómago" },
	*horse_parts[] = { "pierna delantera", "ojo", "cara", "pata delantera",
		"punta de pata", "pata trasera", "uña delantera", "de pata",
		"cabeza", "pierna trasera", "liger@ de cascos", "cerviz",
		"espinazo", "punto de pata trasera", "crin", "sangre", "pulmón",
		"nariz", "estómago" },
	*sphere_parts[] = { "apendaje", "nervio óptico", "cuerpo", "tentáculo",
		"punta de tentáculo", "apendaje bajo", "tentáculo",
		"de tentáculo", "cuerpo", "tentáculo bajo", "rotacional",
		"ecuador", "cuerpo", "punta de tentáculo bajo", "cilia",
		"fuerza de vida", "rétina", "nervio olfactorio", "interior" },
	*fungus_parts[] = { "mycelio", "área visual", "frente", "hifa",
		"hifa", "raíz", "hiebra", "de hiebra", "área de gorra",
		"rizoma", "esporulad@", "tallo", "raíz", "punta de rizoma",
		"esporos", "jugos", "agalla", "agalla", "interior" },
	*vortex_parts[] = { "región", "ojo", "frente", "corriente menor",
		"corriente menor", "corriente baja", "vuelta", "de vuelta",
		"centro", "corriente baja", "turbulent@", "centro",
		"corrientes", "borde", "corrientes", "fuerza de vida", "centro",
		"borde adelante", "interior" },
	*snake_parts[] = { "miembro vestigial", "ojo", "cara", "escama grande",
		"punto de escama grande", "región trasera", "hueco de escama",
		"de hueco de escama", "cabeza", "región trasera",
		"liger@ de cascos", "cerviz", "longitud", "escama trasera",
		"escamas", "sangre", "pulmón", "lengua bifurcada", "estómago" },
	*fish_parts[] = { "aleta", "ojo", "premaxilaria", "axilaria pélvica",
		"aleta pélvica", "aleta anal", "aleta pectoral", "de aleta",
		"cabeza", "pedúnculo", "extendid@", "agallas", "aleta dorsal",
		"aleta caudal", "escamas", "sangre", "agalla", "nariz",
		"estómago" };
	/* claw attacks are overloaded in mons[]; most humanoids with
	   such attacks should still reference hands rather than claws */
	static const char not_claws[] = {
		S_HUMAN, S_MUMMY, S_ZOMBIE, S_ANGEL,
		S_NYMPH, S_LEPRECHAUN, S_QUANTMECH, S_VAMPIRE,
		S_ORC, S_GIANT,		/* quest nemeses */
		'\0'		/* string terminator; assert( S_xxx != 0 ); */
	};
	struct permonst *mptr = mon->data;

	if (part == HAND || part == HANDED) {	/* some special cases */
	    if (mptr->mlet == S_DOG || mptr->mlet == S_FELINE ||
		    mptr->mlet == S_YETI)
		return part == HAND ? "pata" : "de pata";
	    if (humanoid(mptr) && attacktype(mptr, AT_CLAW) &&
		    !index(not_claws, mptr->mlet) &&
		    mptr != &mons[PM_STONE_GOLEM] &&
		    mptr != &mons[PM_INCUBUS] && mptr != &mons[PM_SUCCUBUS])
		return part == HAND ? "uña" : "de uña";
	}
	if ((mptr == &mons[PM_MUMAK] || mptr == &mons[PM_MASTODON]) &&
		part == NOSE)
	    return "trompa";
	if (mptr == &mons[PM_SHARK] && part == HAIR)
	    return "piel";	/* sharks don't have scales */
	if (mptr == &mons[PM_JELLYFISH] && (part == ARM || part == FINGER ||
	    part == HAND || part == FOOT || part == TOE))
	    return "tentáculo";
	if (mptr == &mons[PM_FLOATING_EYE] && part == EYE)
	    return "córnea";
	if (humanoid(mptr) &&
		(part == ARM || part == FINGER || part == FINGERTIP ||
		    part == HAND || part == HANDED))
	    return humanoid_parts[part];
	if (mptr == &mons[PM_RAVEN])
	    return bird_parts[part];
	if (mptr->mlet == S_CENTAUR || mptr->mlet == S_UNICORN ||
		(mptr == &mons[PM_ROTHE] && part != HAIR))
	    return horse_parts[part];
	if (mptr->mlet == S_LIGHT) {
		if (part == HANDED) return "rayad@";
/*
LENGUA: This was "ray", and the default case "beam"
		else if (part == ARM || part == FINGER ||
				part == FINGERTIP || part == HAND) return "ray";
*/
		else return "rayo";
	}
	if (mptr->mlet == S_EEL && mptr != &mons[PM_JELLYFISH])
	    return fish_parts[part];
	if (slithy(mptr) || (mptr->mlet == S_DRAGON && part == HAIR))
	    return snake_parts[part];
	if (mptr->mlet == S_EYE)
	    return sphere_parts[part];
	if (mptr->mlet == S_JELLY || mptr->mlet == S_PUDDING ||
		mptr->mlet == S_BLOB || mptr == &mons[PM_JELLYFISH])
	    return jelly_parts[part];
	if (mptr->mlet == S_VORTEX || mptr->mlet == S_ELEMENTAL)
	    return vortex_parts[part];
	if (mptr->mlet == S_FUNGUS)
	    return fungus_parts[part];
	if (humanoid(mptr))
	    return humanoid_parts[part];
	return animal_parts[part];
}

const char *
mbodypart(mon, part)
struct monst *mon;
int part;
{
	const char *name;

	name = mbodypart_core(mon, part);
	if (strchr(name,'@')) {
		char *buf = nextobuf();
		Strcpy(buf, name);
		*strchr(buf, '@') = (mon_gender(mon)? 'a' : 'o');
		name = buf;
	}
	return name;
}

const char *
body_part(part)
int part;
{
	const char *name;

	name = mbodypart_core(&youmonst, part);
	if (strchr(name,'@')) {
		char *buf = nextobuf();
		Strcpy(buf, name);
		*strchr(buf, '@') = (poly_gender()==1? 'a' : 'o');
		name = buf;
	}
	return name;
}

#endif /* OVL1 */
#ifdef OVL0

int
poly_gender()
{
/* Returns gender of polymorphed player; 0/1=same meaning as flags.female,
 * 2=none.
 */
	if (is_neuter(youmonst.data) || !humanoid(youmonst.data)) return 2;
	return flags.female;
}

#endif /* OVL0 */
#ifdef OVLB

void
ugolemeffects(damtype, dam)
int damtype, dam;
{
	int heal = 0;
	/* We won't bother with "slow"/"haste" since players do not
	 * have a monster-specific slow/haste so there is no way to
	 * restore the old velocity once they are back to human.
	 */
	if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
		return;
	switch (damtype) {
		case AD_ELEC: if (u.umonnum == PM_FLESH_GOLEM)
				heal = dam / 6; /* Approx 1 per die */
			break;
		case AD_FIRE: if (u.umonnum == PM_IRON_GOLEM)
				heal = dam;
			break;
	}
	if (heal && (u.mh < u.mhmax)) {
		u.mh += heal;
		if (u.mh > u.mhmax) u.mh = u.mhmax;
		flags.botl = 1;
		pline("Extrañamente, sientes mejor que antes.");
		exercise(A_STR, TRUE);
	}
}

STATIC_OVL int
armor_to_dragon(atyp)
int atyp;
{
	switch(atyp) {
	    case GRAY_DRAGON_SCALE_MAIL:
	    case GRAY_DRAGON_SCALES:
		return PM_GRAY_DRAGON;
	    case SILVER_DRAGON_SCALE_MAIL:
	    case SILVER_DRAGON_SCALES:
		return PM_SILVER_DRAGON;
#if 0	/* DEFERRED */
	    case SHIMMERING_DRAGON_SCALE_MAIL:
	    case SHIMMERING_DRAGON_SCALES:
		return PM_SHIMMERING_DRAGON;
#endif
	    case RED_DRAGON_SCALE_MAIL:
	    case RED_DRAGON_SCALES:
		return PM_RED_DRAGON;
	    case ORANGE_DRAGON_SCALE_MAIL:
	    case ORANGE_DRAGON_SCALES:
		return PM_ORANGE_DRAGON;
	    case WHITE_DRAGON_SCALE_MAIL:
	    case WHITE_DRAGON_SCALES:
		return PM_WHITE_DRAGON;
	    case BLACK_DRAGON_SCALE_MAIL:
	    case BLACK_DRAGON_SCALES:
		return PM_BLACK_DRAGON;
	    case BLUE_DRAGON_SCALE_MAIL:
	    case BLUE_DRAGON_SCALES:
		return PM_BLUE_DRAGON;
	    case GREEN_DRAGON_SCALE_MAIL:
	    case GREEN_DRAGON_SCALES:
		return PM_GREEN_DRAGON;
	    case YELLOW_DRAGON_SCALE_MAIL:
	    case YELLOW_DRAGON_SCALES:
		return PM_YELLOW_DRAGON;
	    default:
		return -1;
	}
}

#endif /* OVLB */

/*polyself.c*/
