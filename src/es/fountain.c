/*	SCCS Id: @(#)fountain.c	3.4	2003/03/23	*/
/*	Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

#include "hack.h"

STATIC_DCL void NDECL(dowatersnakes);
STATIC_DCL void NDECL(dowaterdemon);
STATIC_DCL void NDECL(dowaternymph);
STATIC_PTR void FDECL(gush, (int,int,genericptr_t));
STATIC_DCL void NDECL(dofindgem);

void
floating_above(what)
const char *what;
{
    pline("Estás flotando muy encima %s.", del(what));
}

STATIC_OVL void
dowatersnakes() /* Fountain of snakes! */
{
    register int num = rn1(5,2);
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_MOCCASIN].mvflags & G_GONE)) {
	if (!Blind)
	    pline("¡Se derrama una corriente interminable de %s!",
		  Hallucination ? makeplural(rndmonnam()) : "serpientes");
	else
	    Oyes("siseos!");
	while(num-- > 0)
	    if((mtmp = makemon(&mons[PM_WATER_MOCCASIN],
			u.ux, u.uy, NO_MM_FLAGS)) && t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
    } else
	pline("La fuente burbujea furiosamente por un momento, luego se calma.");
}

STATIC_OVL
void
dowaterdemon() /* Water demon */
{
    register struct monst *mtmp;

    if(!(mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
	if((mtmp = makemon(&mons[PM_WATER_DEMON],u.ux,u.uy, NO_MM_FLAGS))) {
	    if (!Blind)
		pline("¡Liberas %s!", al(a_monnam(mtmp)));
	    else
		Sientes("la presencia de mal.");

	/* Give those on low levels a (slightly) better chance of survival */
	    if (rnd(100) > (80 + level_difficulty())) {
		pline("¡Agradecid%c por su liberación, te concede un deseo!",
		      mon_gender(mtmp)? 'a' : 'o');
		makewish(FALSE);
		mongone(mtmp);
	    } else if (t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
	}
    } else
	pline("La fuente se burbujea furiosamente por un momento, luego se calma.");
}

STATIC_OVL void
dowaternymph() /* Water Nymph */
{
	register struct monst *mtmp;

	if(!(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
	   (mtmp = makemon(&mons[PM_WATER_NYMPH],u.ux,u.uy, NO_MM_FLAGS))) {
		if (!Blind)
		   pline("¡Atraes %s!", a_monnam(mtmp));
		else
		   Oyes("una voz seductiva.");
		mtmp->msleeping = 0;
		if (t_at(mtmp->mx, mtmp->my))
		    (void) mintrap(mtmp);
	} else
		if (!Blind)
		   pline("Una gran burbuja sube a la superficie y se revienta.");
		else
		   Oyes("un fuerte pop.");
}

void
dogushforth(drinking) /* Gushing forth along LOS from (u.ux, u.uy) */
int drinking;
{
	int madepool = 0;

	do_clear_area(u.ux, u.uy, 7, gush, (genericptr_t)&madepool);
	if (!madepool) {
	    if (drinking)
		pline("Tu sed se calma.");
	    else
		pline("El agua te rocía por todos lados.");
	}
}

STATIC_PTR void
gush(x, y, poolcnt)
int x, y;
genericptr_t poolcnt;
{
	register struct monst *mtmp;
	register struct trap *ttmp;

	if (((x+y)%2) || (x == u.ux && y == u.uy) ||
	    (rn2(1 + distmin(u.ux, u.uy, x, y)))  ||
	    (levl[x][y].typ != ROOM) ||
	    (sobj_at(BOULDER, x, y)) || nexttodoor(x, y))
		return;

	if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
		return;

	if (!((*(int *)poolcnt)++))
	    pline("¡El agua se chorrea y desborda de la fuente!");

	/* Put a pool at x, y */
	levl[x][y].typ = POOL;
	/* No kelp! */
	del_engr_at(x, y);
	water_damage(level.objects[x][y], FALSE, TRUE);

	if ((mtmp = m_at(x, y)) != 0)
		(void) minliquid(mtmp);
	else
		newsym(x,y);
}

STATIC_OVL void
dofindgem() /* Find a gem in the sparkling waters. */
{
	if (!Blind) pline("¡Avistas una gema en las aguas relucientes!");
	else Sientes("una joya aquí!");
	(void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE-1),
						u.ux, u.uy, FALSE, FALSE);
	SET_FOUNTAIN_LOOTED(u.ux,u.uy);
	newsym(u.ux, u.uy);
	exercise(A_WIS, TRUE);			/* a discovery! */
}

void
dryup(x, y, isyou)
xchar x, y;
boolean isyou;
{
	if (IS_FOUNTAIN(levl[x][y].typ) &&
	    (!rn2(3) || FOUNTAIN_IS_WARNED(x,y))) {
		if(isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x,y)) {
			struct monst *mtmp;
			SET_FOUNTAIN_WARNED(x,y);
			/* Warn about future fountain use. */
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			    if (DEADMONSTER(mtmp)) continue;
			    if ((mtmp->data == &mons[PM_WATCHMAN] ||
				mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
			       couldsee(mtmp->mx, mtmp->my) &&
			       mtmp->mpeaceful) {
				pline("%s grita:", Amonnam(mtmp));
				verbalize("¡Hey, deja de usar esa fuente!");
				break;
			    }
			}
			/* You can see or hear this effect */
			if(!mtmp) pline("El chorro se reduce a un goteo.");
			return;
		}
#ifdef WIZARD
		if (isyou && wizard) {
			if (yn("¿Secar la fuente?") == 'n')
				return;
		}
#endif
		/* replace the fountain with ordinary floor */
		levl[x][y].typ = ROOM;
		levl[x][y].looted = 0;
		levl[x][y].blessedftn = 0;
		if (cansee(x,y)) pline("¡La fuente se seca!");
		/* The location is seen if the hero/monster is invisible */
		/* or felt if the hero is blind.			 */
		newsym(x, y);
		level.flags.nfountains--;
		if(isyou && in_town(x, y))
		    (void) angry_guards(FALSE);
	}
}

void
drinkfountain()
{
	/* What happens when you drink from a fountain? */
	register boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
	register int fate = rnd(30);

	if (Levitation) {
		floating_above("la fuente");
		return;
	}

	if (mgkftn && u.uluck >= 0 && fate >= 10) {
		int i, ii, littleluck = (u.uluck < 4);

		pline("¡Vaya!  ¡Esto te hace sentirte genial!");
		/* blessed restore ability */
		for (ii = 0; ii < A_MAX; ii++)
		    if (ABASE(ii) < AMAX(ii)) {
			ABASE(ii) = AMAX(ii);
			flags.botl = 1;
		    }
		/* gain ability, blessed if "natural" luck is high */
		i = rn2(A_MAX);		/* start at a random attribute */
		for (ii = 0; ii < A_MAX; ii++) {
		    if (adjattrib(i, 1, littleluck ? -1 : 0) && littleluck)
			break;
		    if (++i >= A_MAX) i = 0;
		}
		display_nhwindow(WIN_MESSAGE, FALSE);
		pline("Una bocanada de vapor escapa de la fuente...");
		exercise(A_WIS, TRUE);
		levl[u.ux][u.uy].blessedftn = 0;
		return;
	}

	if (fate < 10) {
		pline("El trago fresco te refresca.");
		u.uhunger += rnd(10); /* don't choke on water */
		newuhs(FALSE);
		if(mgkftn) return;
	} else {
	    switch (fate) {

		case 19: /* Self-knowledge */

			Te_sientes("conciente de ti mism%c...", poly_gender()==1? 'a' : 'o');
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0);
			exercise(A_WIS, TRUE);
			pline("El sentido se disminuye.");
			break;

		case 20: /* Foul water */

			pline("¡El agua está sucia!  Basqueas y vomitas.");
			morehungry(rn1(20, 11));
			vomit();
			break;

		case 21: /* Poisonous */

			pline("¡El agua está contaminada!");
			if (Poison_resistance) {
			   pline(
			      "Quizás es el desaguadero de la granja de %s cercana.",
				 fruitname(FALSE));
			   losehp(rnd(4),"sorbo de jugo no refrigerado",
				KILLED_BY_AN);
			   break;
			}
			losestr(rn1(4,3));
			losehp(rnd(10),"agua contaminada", KILLED_BY);
			exercise(A_CON, FALSE);
			break;

		case 22: /* Fountain of snakes! */

			dowatersnakes();
			break;

		case 23: /* Water demon */
			dowaterdemon();
			break;

		case 24: /* Curse an item */ {
			register struct obj *obj;

			pline("¡Esta agua no es buena!");
			morehungry(rn1(20, 11));
			exercise(A_CON, FALSE);
			for(obj = invent; obj ; obj = obj->nobj)
				if (!rn2(5))	curse(obj);
			break;
			}

		case 25: /* See invisible */

			if (Blind) {
			    if (Invisible) {
				Te_sientes("transparente.");
			    } else {
				Te_sientes("muy conciente de ti mism%c.", poly_gender()==1? 'a' : 'o');
				pline("Entonces se pasa.");
			    }
			} else {
			   pline("Ves una imagen de alguien acechándo.");
			   pline("Pero desaparece.");
			}
			HSee_invisible |= FROMOUTSIDE;
			newsym(u.ux,u.uy);
			exercise(A_WIS, TRUE);
			break;

		case 26: /* See Monsters */

			(void) monster_detect((struct obj *)0, 0);
			exercise(A_WIS, TRUE);
			break;

		case 27: /* Find a gem in the sparkling waters. */

			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}

		case 28: /* Water Nymph */

			dowaternymph();
			break;

		case 29: /* Scare */ {
			register struct monst *mtmp;

			pline("¡Esta agua te da mal aliento!");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			    if(!DEADMONSTER(mtmp))
				monflee(mtmp, 0, FALSE, FALSE);
			}
			break;

		case 30: /* Gushing forth in this room */

			dogushforth(TRUE);
			break;

		default:

			pline("Esta agua tibia es insípida.");
			break;
	    }
	}
	dryup(u.ux, u.uy, TRUE);
}

void
dipfountain(obj)
register struct obj *obj;
{
	if (Levitation) {
		floating_above("la fuente");
		return;
	}

	/* Don't grant Excalibur when there's more than one object.  */
	/* (quantity could be > 1 if merged daggers got polymorphed) */
	if (obj->otyp == LONG_SWORD && obj->quan == 1L
	    && u.ulevel >= 5 && !rn2(6)
	    && !obj->oartifact
	    && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {

		if (u.ualign.type != A_LAWFUL) {
			/* Ha!  Trying to cheat her. */
			pline("Una neblina helada se levanta del agua y envuelve la espada.");
			pline("¡La fuente desaparece!");
			curse(obj);
			if (obj->spe > -6 && !rn2(3)) obj->spe--;
			obj->oerodeproof = FALSE;
			exercise(A_WIS, FALSE);
		} else {
			/* The lady of the lake acts! - Eric Backus */
			/* Be *REAL* nice */
	pline("De las turbias profundidades, se extiende una mano para bendecir la espada.");
			pline("¡Cuando la mano se retira, la fuente desaparece!");
			obj = oname(obj, artiname(ART_EXCALIBUR));
			discover_artifact(ART_EXCALIBUR);
			bless(obj);
			obj->oeroded = obj->oeroded2 = 0;
			obj->oerodeproof = TRUE;
			exercise(A_WIS, TRUE);
		}
		update_inventory();
		levl[u.ux][u.uy].typ = ROOM;
		levl[u.ux][u.uy].looted = 0;
		newsym(u.ux, u.uy);
		level.flags.nfountains--;
		if(in_town(u.ux, u.uy))
		    (void) angry_guards(FALSE);
		return;
	} else if (get_wet(obj) && !rn2(2))
		return;

	/* Acid and water don't mix */
	if (obj->otyp == POT_ACID) {
	    useup(obj);
	    return;
	}

	switch (rnd(30)) {
		case 16: /* Curse the item */
			curse(obj);
			break;
		case 17:
		case 18:
		case 19:
		case 20: /* Uncurse the item */
			if(obj->cursed) {
			    if (!Blind)
				pline("El agua brilla por un momento.");
			    uncurse(obj);
			} else {
			    pline("Un sentido de pérdida se apodera de ti.");
			}
			break;
		case 21: /* Water Demon */
			dowaterdemon();
			break;
		case 22: /* Water Nymph */
			dowaternymph();
			break;
		case 23: /* an Endless Stream of Snakes */
			dowatersnakes();
			break;
		case 24: /* Find a gem */
			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}
		case 25: /* Water gushes forth */
			dogushforth(FALSE);
			break;
		case 26: /* Strange feeling */
			pline("Un extraño hormigueo corre por tu %s.",
							body_part(ARM));
			break;
		case 27: /* Strange feeling */
			Sientes("un súbito escalofrío.");
			break;
		case 28: /* Strange feeling */
			pline("Te agobia un impulso de darte un baño.");
#ifndef GOLDOBJ
			if (u.ugold > 10) {
			    u.ugold -= somegold() / 10;
			    pline("¡Perdiste algo de tu oro en la fuente!");
			    CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			    exercise(A_WIS, FALSE);
			}
#else
			{
			    long money = money_cnt(invent);
			    struct obj *otmp;
                            if (money > 10) {
				/* Amount to loose.  Might get rounded up as fountains don't pay change... */
			        money = somegold(money) / 10; 
			        for (otmp = invent; otmp && money > 0; otmp = otmp->nobj) if (otmp->oclass == COIN_CLASS) {
				    int denomination = objects[otmp->otyp].oc_cost;
				    long coin_loss = (money + denomination - 1) / denomination;
                                    coin_loss = min(coin_loss, otmp->quan);
				    otmp->quan -= coin_loss;
				    money -= coin_loss * denomination;				  
				    if (!otmp->quan) delobj(otmp);
				}
				pline("¡Has perdido algo de tu dinero en la fuente!");
				CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			        exercise(A_WIS, FALSE);
                            }
			}
#endif
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.	Just like a shopping mall!  Chris Woodbury  */

		    if (FOUNTAIN_IS_LOOTED(u.ux,u.uy)) break;
		    SET_FOUNTAIN_LOOTED(u.ux,u.uy);
		    (void) mkgold((long)
			(rnd((dunlevs_in_dungeon(&u.uz)-dunlev(&u.uz)+1)*2)+5),
			u.ux, u.uy);
		    if (!Blind)
			pline("Muy abajo, ves monedas brillando en el agua.");
		    exercise(A_WIS, TRUE);
		    newsym(u.ux,u.uy);
		    break;
	}
	update_inventory();
	dryup(u.ux, u.uy, TRUE);
}

#ifdef SINKS
void
breaksink(x,y)
int x, y;
{
    if(cansee(x,y) || (x == u.ux && y == u.uy))
	pline("¡Las cañerías se rompen!  ¡El agua sale a chorros!");
    level.flags.nsinks--;
    levl[x][y].doormask = 0;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x,y);
}

void
drinksink()
{
	struct obj *otmp;
	struct monst *mtmp;

	if (Levitation) {
		floating_above("el fregadero");
		return;
	}
	switch(rn2(20)) {
		case 0: pline("Tomas un sorbo de agua muy fría.");
			break;
		case 1: pline("Tomas un sorbo de agua muy cálida.");
			break;
		case 2: pline("Tomas un sorbo de agua hirviendo.");
			if (Fire_resistance)
				pline("Parece bastante sabrosa.");
			else losehp(rnd(6), "sorber agua hirviendo", KILLED_BY);
			break;
		case 3: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
				pline("El fregadero parece bastante sucio.");
			else {
				mtmp = makemon(&mons[PM_SEWER_RAT],
						u.ux, u.uy, NO_MM_FLAGS);
				if (mtmp) pline("¡Eek!  ¡Hay %s en el fregadero!",
					(Blind || !canspotmon(mtmp)) ?
					"algo retorciente" :
					a_monnam(mtmp));
			}
			break;
		case 4: do {
				otmp = mkobj(POTION_CLASS,FALSE);
				if (otmp->otyp == POT_WATER) {
					obfree(otmp, (struct obj *)0);
					otmp = (struct obj *) 0;
				}
			} while(!otmp);
			otmp->cursed = otmp->blessed = 0;
			pline("Algún líquido %s fluye del grifo.",
			      Blind ? "extraño" :
			      hcolor(OBJ_DESCR(objects[otmp->otyp])));
			otmp->dknown = !(Blind || Hallucination);
			otmp->quan++; /* Avoid panic upon useup() */
			otmp->fromsink = 1; /* kludge for docall() */
			(void) dopotion(otmp);
			obfree(otmp, (struct obj *)0);
			break;
		case 5: if (!(levl[u.ux][u.uy].looted & S_LRING)) {
			    pline("¡Encuentras un anillo en el fregadero!");
			    (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
			    levl[u.ux][u.uy].looted |= S_LRING;
			    exercise(A_WIS, TRUE);
			    newsym(u.ux,u.uy);
			} else pline("Un poco de agua sucia retrocede por el desagüe.");
			break;
		case 6: breaksink(u.ux,u.uy);
			break;
		case 7: pline("¡El agua se mueve como si tuviera voluntad propia!");
			if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
			    || !makemon(&mons[PM_WATER_ELEMENTAL],
					u.ux, u.uy, NO_MM_FLAGS))
				pline("Pero se calma.");
			break;
		case 8: pline("Yuk, esta agua sabe horrible.");
			more_experienced(1,0);
			newexplevel();
			break;
		case 9: pline("Gaggg... ¡Esto sabe a agua de alcantarilla!  Vomitas.");
			morehungry(rn1(30-ACURR(A_CON), 11));
			vomit();
			break;
		case 10: pline("¡Esta agua contiene desechos tóxicos!");
			if (!Unchanging) {
				pline("¡Sufres una metamorfosis monstruosa!");
				polyself(FALSE);
			}
			break;
		/* more odd messages --JJB */
		case 11: Oyes("el resonar de los tubos...");
			break;
		case 12: Oyes("las estrofas de canción de entre las cloacas...");
			break;
		case 19: if (Hallucination) {
		   pline("Del desagüe lóbrego, se extiende una mano... --oops--");
				break;
			}
		default: pline("Tomas un sorbo de agua %s.",
			rn2(3) ? (rn2(2) ? "fría" : "tibia") : "caliente");
	}
}
#endif /* SINKS */

/*fountain.c*/
