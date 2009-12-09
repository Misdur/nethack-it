/*	SCCS Id: @(#)lock.c	3.4	2000/02/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_PTR int NDECL(picklock);
STATIC_PTR int NDECL(forcelock);

/* at most one of `door' and `box' should be non-null at any given time */
STATIC_VAR NEARDATA struct xlock_s {
	struct rm  *door;
	struct obj *box;
	int picktyp, chance, usedtime;
} xlock;

#ifdef OVLB

STATIC_DCL const char *NDECL(lock_action);
STATIC_DCL boolean FDECL(obstructed,(int,int));
STATIC_DCL void FDECL(chest_shatter_msg, (struct obj *));

boolean
picking_lock(x, y)
	int *x, *y;
{
	if (occupation == picklock) {
	    *x = u.ux + u.dx;
	    *y = u.uy + u.dy;
	    return TRUE;
	} else {
	    *x = *y = 0;
	    return FALSE;
	}
}

boolean
picking_at(x, y)
int x, y;
{
	return (boolean)(occupation == picklock && xlock.door == &levl[x][y]);
}

/* produce an occupation string appropriate for the current activity */
STATIC_OVL const char *
lock_action()
{
	static const char *lock_actions[] = {
		/* [0] */	"cerrar con llave la puerta",
		/* [1] */	"cerrar con llave el cofre",
		/* [2] */	"cerrar con llave el cajón",
		/* [3] */	"hacer lo imposible con una tarjeta de crédito"
	};
	static const char *unlock_actions[] = {
		/* [0] */	"abrir con llave la puerta",
		/* [1] */	"abrir con llave el cofre",
		/* [2] */	"abrir con llave el cajón",
		/* [3] */	"violentar la cerradura"
	};

	/* if the target is currently unlocked, we're trying to lock it now */
	if (xlock.door && !(xlock.door->doormask & D_LOCKED))
		return lock_actions[0];	/* "locking the door" */
	else if (xlock.box && !xlock.box->olocked)
		return xlock.box->otyp == CHEST ? lock_actions[1] : lock_actions[2];
	/* otherwise we're trying to unlock it */
	else if (xlock.picktyp == LOCK_PICK)
		return unlock_actions[3];	/* "picking the lock" */
#ifdef TOURIST
	else if (xlock.picktyp == CREDIT_CARD)
		return unlock_actions[3];	/* same as lock_pick */
#endif
	else if (xlock.door)
		return unlock_actions[0];	/* "unlocking the door" */
	else
		return xlock.box->otyp == CHEST ? unlock_actions[1] : unlock_actions[2];
}

STATIC_PTR
int
picklock()	/* try to open/close a lock */
{

	if (xlock.box) {
	    if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy)) {
		return((xlock.usedtime = 0));		/* you or it moved */
	    }
	} else {		/* door */
	    if(xlock.door != &(levl[u.ux+u.dx][u.uy+u.dy])) {
		return((xlock.usedtime = 0));		/* you moved */
	    }
	    switch (xlock.door->doormask) {
		case D_NODOOR:
		    pline("Esta entrada no tiene puerta.");
		    return((xlock.usedtime = 0));
		case D_ISOPEN:
		    No_puedes("cerrar con llave una puerta abierta.");
		    return((xlock.usedtime = 0));
		case D_BROKEN:
		    pline("Esta puerta está rota.");
		    return((xlock.usedtime = 0));
	    }
	}

	if (xlock.usedtime++ >= 50 || nohands(youmonst.data)) {
	    pline("Desistes de tu intento de %s.", lock_action());
	    exercise(A_DEX, TRUE);	/* even if you don't succeed */
	    return((xlock.usedtime = 0));
	}

	if(rn2(100) >= xlock.chance) return(1);		/* still busy */

	pline("Tienes éxito al %s.", lock_action());
	if (xlock.door) {
	    if(xlock.door->doormask & D_TRAPPED) {
		    b_trapped("puerta", FINGER);
		    xlock.door->doormask = D_NODOOR;
		    unblock_point(u.ux+u.dx, u.uy+u.dy);
		    if (*in_rooms(u.ux+u.dx, u.uy+u.dy, SHOPBASE))
			add_damage(u.ux+u.dx, u.uy+u.dy, 0L);
		    newsym(u.ux+u.dx, u.uy+u.dy);
	    } else if (xlock.door->doormask & D_LOCKED)
		xlock.door->doormask = D_CLOSED;
	    else xlock.door->doormask = D_LOCKED;
	} else {
	    xlock.box->olocked = !xlock.box->olocked;
	    if(xlock.box->otrapped)	
		(void) chest_trap(xlock.box, FINGER, FALSE);
	}
	exercise(A_DEX, TRUE);
	return((xlock.usedtime = 0));
}

STATIC_PTR
int
forcelock()	/* try to force a locked chest */
{

	register struct obj *otmp;

	if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy))
		return((xlock.usedtime = 0));		/* you or it moved */

	if (xlock.usedtime++ >= 50 || !uwep || nohands(youmonst.data)) {
	    pline("Desistes de tu intento de forzar la cerradura.");
	    if(xlock.usedtime >= 50)		/* you made the effort */
	      exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
	    return((xlock.usedtime = 0));
	}

	if(xlock.picktyp) {	/* blade */

	    if(rn2(1000-(int)uwep->spe) > (992-greatest_erosion(uwep)*10) &&
	       !uwep->cursed && !obj_resists(uwep, 0, 99)) {
		/* for a +0 weapon, probability that it survives an unsuccessful
		 * attempt to force the lock is (.992)^50 = .67
		 */
		const char *arma = xname(uwep);
		if (uwep->quan > 1L)
			pline("¡Un%c de tus %s se ha roto!",
				isfeminine(arma)? 'a' : 'o',
				arma);
		else
			pline("¡Tu %s se ha roto!", arma);
		useup(uwep);
		pline("Desistes de tu intento de forzar la cerradura.");
		exercise(A_DEX, TRUE);
		return((xlock.usedtime = 0));
	    }
	} else			/* blunt */
	    wake_nearby();	/* due to hammering on the container */

	if(rn2(100) >= xlock.chance) return(1);		/* still busy */

	pline("Tienes éxito al forzar la cerradura.");
	xlock.box->olocked = 0;
	xlock.box->obroken = 1;
	if(!xlock.picktyp && !rn2(3)) {
	    struct monst *shkp;
	    boolean costly;
	    long loss = 0L;

	    costly = (*u.ushops && costly_spot(u.ux, u.uy));
	    shkp = costly ? shop_keeper(*u.ushops) : 0;

	    pline("Es más, has destruido totalmente %s.",
		  the(xname(xlock.box)));

	    /* Put the contents on ground at the hero's feet. */
	    while ((otmp = xlock.box->cobj) != 0) {
		obj_extract_self(otmp);
		if(!rn2(3) || otmp->oclass == POTION_CLASS) {
		    chest_shatter_msg(otmp);
		    if (costly)
		        loss += stolen_value(otmp, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE);
		    if (otmp->quan == 1L) {
			obfree(otmp, (struct obj *) 0);
			continue;
		    }
		    useup(otmp);
		}
		if (xlock.box->otyp == ICE_BOX && otmp->otyp == CORPSE) {
		    otmp->age = monstermoves - otmp->age; /* actual age */
		    start_corpse_timeout(otmp);
		}
		place_object(otmp, u.ux, u.uy);
		stackobj(otmp);
	    }

	    if (costly)
		loss += stolen_value(xlock.box, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE);
	    if(loss) pline("Debes %ld %s por objetos destruidos.", loss, currency(loss));
	    delobj(xlock.box);
	}
	exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
	return((xlock.usedtime = 0));
}

#endif /* OVLB */
#ifdef OVL0

void
reset_pick()
{
	xlock.usedtime = xlock.chance = xlock.picktyp = 0;
	xlock.door = 0;
	xlock.box = 0;
}

#endif /* OVL0 */
#ifdef OVLB

int
pick_lock(pick) /* pick a lock with a given object */
	register struct	obj	*pick;
{
	int picktyp, c, ch;
	coord cc;
	struct rm	*door;
	struct obj	*otmp;
	char qbuf[QBUFSZ];

	picktyp = pick->otyp;

	/* check whether we're resuming an interrupted previous attempt */
	if (xlock.usedtime && picktyp == xlock.picktyp) {
	    static char no_longer[] = "Desafortunadamente, ya no puedes %s %s.";

	    if (nohands(youmonst.data)) {
		const char *what = (picktyp == LOCK_PICK) ? "el pico" : "la llave";
#ifdef TOURIST
		if (picktyp == CREDIT_CARD) what = "la tarjeta";
#endif
		pline(no_longer, "sostener", what);
		reset_pick();
		return 0;
	    } else if (xlock.box && !can_reach_floor()) {
		pline(no_longer, "alcanzar", "la cerradura");
		reset_pick();
		return 0;
	    } else {
		const char *action = lock_action();
		pline("Continúas tu intento de %s.", action);
		set_occupation(picklock, action, 0);
		return(1);
	    }
	}

	if(nohands(youmonst.data)) {
		No_puedes("sostener %s -- no tienes manos!", doname(pick));
		return(0);
	}

	if((picktyp != LOCK_PICK &&
#ifdef TOURIST
	    picktyp != CREDIT_CARD &&
#endif
	    picktyp != SKELETON_KEY)) {
		impossible("¿violentar la cerradura con objeto %d?", picktyp);
		return(0);
	}
	ch = 0;		/* lint suppression */

	if(!get_adjacent_loc((char *)0, "¡Ubicación inválida!", u.ux, u.uy, &cc)) return 0;
	if (cc.x == u.ux && cc.y == u.uy) {	/* pick lock on a container */
	    const char *verb;
	    boolean it;
	    int count;

	    if (u.dz < 0) {
		pline("No hay ninguna clase de cerradura %s.",
		      Levitation ? "aquí" : "ahí");
		return 0;
	    } else if (is_lava(u.ux, u.uy)) {
		pline("Hacer eso probablemente fundiría tu %s.",
		      xname(pick));
		return 0;
	    } else if (is_pool(u.ux, u.uy) && !Underwater) {
		pline("El agua no tiene cerradura.");
		return 0;
	    }

	    count = 0;
	    c = 'n';			/* in case there are no boxes here */
	    for(otmp = level.objects[cc.x][cc.y]; otmp; otmp = otmp->nexthere)
		if (Is_box(otmp)) {
		    ++count;
		    if (!can_reach_floor()) {
			No_puedes("%s desde aquí arriba.", the(xname(otmp)));
			return 0;
		    }
		    it = 0;
		    if (otmp->obroken) verb = "fixar";
		    else if (!otmp->olocked) verb = "trabar", it = 1;
		    else if (picktyp != LOCK_PICK) verb = "destrabar", it = 1;
		    else verb = "violentar";
		    {
			/* Do this first so we get the right pronoun even if
			   safe_qbuf substitutes "una caja" and the proper
			   name is masculine */
			const char *noun = 
				safe_qbuf("", sizeof("Hay  aquí, ¿destrabar su cerradura?"),
				    doname(otmp), an(simple_typename(otmp->otyp)), "una caja");
			Sprintf(qbuf, "Hay %s aquí, ¿%s%s?",
				noun, verb,
				it ? (isfeminine(noun)? "la" : "lo") : " su cerradura");
		    }
		    c = ynq(qbuf);
		    if(c == 'q') return(0);
		    if(c == 'n') continue;

		    if (otmp->obroken) {
			No_puedes("reparar su cerradura rota con %s.", doname(pick));
			return 0;
		    }
#ifdef TOURIST
		    else if (picktyp == CREDIT_CARD && !otmp->olocked) {
			/* credit cards are only good for unlocking */
			No_puedes("hacer eso con %s.", doname(pick));
			return 0;
		    }
#endif
		    switch(picktyp) {
#ifdef TOURIST
			case CREDIT_CARD:
			    ch = ACURR(A_DEX) + 20*Role_if(PM_ROGUE);
			    break;
#endif
			case LOCK_PICK:
			    ch = 4*ACURR(A_DEX) + 25*Role_if(PM_ROGUE);
			    break;
			case SKELETON_KEY:
			    ch = 75 + ACURR(A_DEX);
			    break;
			default:	ch = 0;
		    }
		    if(otmp->cursed) ch /= 2;

		    xlock.picktyp = picktyp;
		    xlock.box = otmp;
		    xlock.door = 0;
		    break;
		}
	    if (c != 'y') {
		if (!count)
		    pline("No parece haber ninguna clase de cerradura aquí.");
		return(0);		/* decided against all boxes */
	    }
	} else {			/* pick the lock in a door */
	    struct monst *mtmp;

	    if (u.utrap && u.utraptype == TT_PIT) {
		No_puedes("alcanzar más allá de la orilla del hoyo.");
		return(0);
	    }

	    door = &levl[cc.x][cc.y];
	    if ((mtmp = m_at(cc.x, cc.y)) && canseemon(mtmp)
			&& mtmp->m_ap_type != M_AP_FURNITURE
			&& mtmp->m_ap_type != M_AP_OBJECT) {
#ifdef TOURIST
		if (picktyp == CREDIT_CARD &&
		    (mtmp->isshk || mtmp->data == &mons[PM_ORACLE]))
		    verbalize("Nada de cheques, nada de crédito, nada de problemas.");
		else
#endif
		    pline("No pienso que %s vaya a apreciar eso.", mon_nam(mtmp));
		return(0);
	    }
	    if(!IS_DOOR(door->typ)) {
		if (is_drawbridge_wall(cc.x,cc.y) >= 0)
		    pline("No %s ninguna cerradura en el puente levantizo.",
				Blind ? "sientes" : "ves");
		else
		    pline("No %s ninguna puerta ahí.",
				Blind ? "sientes" : "ves");
		return(0);
	    }
	    switch (door->doormask) {
		case D_NODOOR:
		    pline("Esta entrada no tiene puerta.");
		    return(0);
		case D_ISOPEN:
		    No_puedes("cerrar con llave una puerta abierta.");
		    return(0);
		case D_BROKEN:
		    pline("Esta puerta está rota.");
		    return(0);
		default:
#ifdef TOURIST
		    /* credit cards are only good for unlocking */
		    if(picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
			No_puedes("cerrar con llave una puerta con una tarjeta de crédito.");
			return(0);
		    }
#endif

		    Sprintf(qbuf,"¿%s con llave?",
			(door->doormask & D_LOCKED) ? "Abrir" : "Cerrar" );

		    c = yn(qbuf);
		    if(c == 'n') return(0);

		    switch(picktyp) {
#ifdef TOURIST
			case CREDIT_CARD:
			    ch = 2*ACURR(A_DEX) + 20*Role_if(PM_ROGUE);
			    break;
#endif
			case LOCK_PICK:
			    ch = 3*ACURR(A_DEX) + 30*Role_if(PM_ROGUE);
			    break;
			case SKELETON_KEY:
			    ch = 70 + ACURR(A_DEX);
			    break;
			default:    ch = 0;
		    }
		    xlock.door = door;
		    xlock.box = 0;
	    }
	}
	flags.move = 0;
	xlock.chance = ch;
	xlock.picktyp = picktyp;
	xlock.usedtime = 0;
	set_occupation(picklock, lock_action(), 0);
	return(1);
}

int
doforce()		/* try to force a chest with your weapon */
{
	register struct obj *otmp;
	register int c, picktyp;
	char qbuf[QBUFSZ];

	if(!uwep ||	/* proper type test */
	   (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep) &&
	    uwep->oclass != ROCK_CLASS) ||
	   (objects[uwep->otyp].oc_skill < P_DAGGER) ||
	   (objects[uwep->otyp].oc_skill > P_LANCE) ||
	   uwep->otyp == FLAIL || uwep->otyp == AKLYS
#ifdef KOPS
	   || uwep->otyp == RUBBER_HOSE
#endif
	  ) {
	    No_puedes("forzar nada sin un arma%s.",
		  (uwep) ? " apropiada" : "");
	    return(0);
	}

	picktyp = is_blade(uwep);
	if(xlock.usedtime && xlock.box && picktyp == xlock.picktyp) {
	    pline("Resumes tu intento de forzar la cerradura.");
	    set_occupation(forcelock, "forzar la cerradura", 0);
	    return(1);
	}

	/* A lock is made only for the honest man, the thief will break it. */
	xlock.box = (struct obj *)0;
	for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
	    if(Is_box(otmp)) {
		if (otmp->obroken || !otmp->olocked) {
		    Hay("%s aquí, pero su cerradura ya está %s.",
			  doname(otmp), otmp->obroken ? "rota" : "abierta");
		    continue;
		}
		Sprintf(qbuf,"Hay %s aquí, ¿forzar su cerradura?",
			safe_qbuf("", sizeof("Hay  aquí, ¿forzar su cerradura?"),
				doname(otmp), an(simple_typename(otmp->otyp)),
				"una caja"));

		c = ynq(qbuf);
		if(c == 'q') return(0);
		if(c == 'n') continue;

		if(picktyp)
		    pline("Fuerzas tu %s en una raja y atisba.", xname(uwep));
		else
		    pline("Empiezas a golpearlo con tu %s.", xname(uwep));
		xlock.box = otmp;
		xlock.chance = objects[uwep->otyp].oc_wldam * 2;
		xlock.picktyp = picktyp;
		xlock.usedtime = 0;
		break;
	    }

	if(xlock.box)	set_occupation(forcelock, "forzar la cerradura", 0);
	/* LENGUA:  Does this pun work in Spanish? */
	else		pline("Decidas no forzar la cuestión.");
	return(1);
}

int
doopen()		/* try to open a door */
{
	coord cc;
	register struct rm *door;
	struct monst *mtmp;

	if (nohands(youmonst.data)) {
	    No_puedes("abrir nada -- no tienes manos!");
	    return 0;
	}

	if (u.utrap && u.utraptype == TT_PIT) {
	    No_puedes("alcanzar la orilla del hoyo.");
	    return 0;
	}

	if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return(0);

	if((cc.x == u.ux) && (cc.y == u.uy)) return(0);

	if ((mtmp = m_at(cc.x,cc.y))			&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	door = &levl[cc.x][cc.y];

	if(!IS_DOOR(door->typ)) {
		if (is_db_wall(cc.x,cc.y)) {
		    pline("No hay manera obvia de abrir el puente levantizo.");
		    return(0);
		}
		pline("No %s ninguna puerta ahí.",
				Blind ? "sientes" : "ves");
		return(0);
	}

	if (!(door->doormask & D_CLOSED)) {
	    const char *mesg;

	    switch (door->doormask) {
	    case D_BROKEN: mesg = "puerta está rota"; break;
	    case D_NODOOR: mesg = "entrada no tiene puerta"; break;
	    case D_ISOPEN: mesg = "puerta ya está abierta"; break;
	    default:	   mesg = "puerta está cerrada con llave"; break;
	    }
	    pline("Esta %s.", mesg);
	    if (Blind) feel_location(cc.x,cc.y);
	    return(0);
	}

	if(verysmall(youmonst.data)) {
	    pline("Eres demasiado pequeñ%c para abrir la puerta.",
		  poly_gender()==1? 'a' : 'o');
	    return(0);
	}

	/* door is known to be CLOSED */
	if (rnl(20) < (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3) {
	    pline("La puerta se abre.");
	    if(door->doormask & D_TRAPPED) {
		b_trapped("puerta", FINGER);
		door->doormask = D_NODOOR;
		if (*in_rooms(cc.x, cc.y, SHOPBASE)) add_damage(cc.x, cc.y, 0L);
	    } else
		door->doormask = D_ISOPEN;
	    if (Blind)
		feel_location(cc.x,cc.y);	/* the hero knows she opened it  */
	    else
		newsym(cc.x,cc.y);
	    unblock_point(cc.x,cc.y);		/* vision: new see through there */
	} else {
	    exercise(A_STR, TRUE);
	    pline("¡La puerta resiste!");
	}

	return(1);
}

STATIC_OVL
boolean
obstructed(x,y)
register int x, y;
{
	register struct monst *mtmp = m_at(x, y);

	if(mtmp && mtmp->m_ap_type != M_AP_FURNITURE) {
		if (mtmp->m_ap_type == M_AP_OBJECT) goto objhere;
		pline("¡%s obstruye la puerta!", !canspotmon(mtmp) ?
			"Alguna criatura" : Monnam(mtmp));
		if (!canspotmon(mtmp))
		    map_invisible(mtmp->mx, mtmp->my);
		return(TRUE);
	}
	if (OBJ_AT(x, y)) {
objhere:	pline("%s obstruye la puerta.", Something);
		return(TRUE);
	}
	return(FALSE);
}

int
doclose()		/* try to close a door */
{
	register int x, y;
	register struct rm *door;
	struct monst *mtmp;

	if (nohands(youmonst.data)) {
	    No_puedes("cerrar nada -- no tienes manos!");
	    return 0;
	}

	if (u.utrap && u.utraptype == TT_PIT) {
	    No_puedes("alcanzar la orilla del hoyo.");
	    return 0;
	}

	if(!getdir((char *)0)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if((x == u.ux) && (y == u.uy)) {
		pline("¡Obstrues la puerta!");
		return(1);
	}

	if ((mtmp = m_at(x,y))				&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	door = &levl[x][y];

	if(!IS_DOOR(door->typ)) {
		if (door->typ == DRAWBRIDGE_DOWN)
		    pline("No hay manera obvia de abrir el puente levantizo.");
		else
		    pline("No %s ninguna puerta ahí.",
				Blind ? "sientes" : "ves");
		return(0);
	}

	if(door->doormask == D_NODOOR) {
	    pline("Esta entrada no tiene puerta.");
	    return(0);
	}

	if(obstructed(x, y)) return(0);

	if(door->doormask == D_BROKEN) {
	    pline("Esta puerta está rota.");
	    return(0);
	}

	if(door->doormask & (D_CLOSED | D_LOCKED)) {
	    pline("Esta puerta ya está cerrada.");
	    return(0);
	}

	if(door->doormask == D_ISOPEN) {
	    if(verysmall(youmonst.data)
#ifdef STEED
		&& !u.usteed
#endif
		) {
		 pline("Eres demasiado pequeñ%c para cerrar la puerta.",
			poly_gender()==1? 'a' : 'o');
		 return(0);
	    }
	    if (
#ifdef STEED
		 u.usteed ||
#endif
		rn2(25) < (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3) {
		pline("La puerta se cierra.");
		door->doormask = D_CLOSED;
		if (Blind)
		    feel_location(x,y);	/* the hero knows she closed it */
		else
		    newsym(x,y);
		block_point(x,y);	/* vision:  no longer see there */
	    }
	    else {
	        exercise(A_STR, TRUE);
	        pline("¡La puerta resiste!");
	    }
	}

	return(1);
}

boolean			/* box obj was hit with spell effect otmp */
boxlock(obj, otmp)	/* returns true if something happened */
register struct obj *obj, *otmp;	/* obj *is* a box */
{
	register boolean res = 0;

	switch(otmp->otyp) {
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
	    if (!obj->olocked) {	/* lock it; fix if broken */
		pline("¡Klunk!");
		obj->olocked = 1;
		obj->obroken = 0;
		res = 1;
	    } /* else already closed and locked */
	    break;
	case WAN_OPENING:
	case SPE_KNOCK:
	    if (obj->olocked) {		/* unlock; couldn't be broken */
		pline("¡Klick!");
		obj->olocked = 0;
		res = 1;
	    } else			/* silently fix if broken */
		obj->obroken = 0;
	    break;
	case WAN_POLYMORPH:
	case SPE_POLYMORPH:
	    /* maybe start unlocking chest, get interrupted, then zap it;
	       we must avoid any attempt to resume unlocking it */
	    if (xlock.box == obj)
		reset_pick();
	    break;
	}
	return res;
}

boolean			/* Door/secret door was hit with spell effect otmp */
doorlock(otmp,x,y)	/* returns true if something happened */
struct obj *otmp;
int x, y;
{
	register struct rm *door = &levl[x][y];
	boolean res = TRUE;
	int loudness = 0;
	const char *msg = (const char *)0;
	const char *dustcloud = "Una nube de polvo";
	const char *quickly_dissipates = "pronto se disipa";
	
	if (door->typ == SDOOR) {
	    switch (otmp->otyp) {
	    case WAN_OPENING:
	    case SPE_KNOCK:
	    case WAN_STRIKING:
	    case SPE_FORCE_BOLT:
		door->typ = DOOR;
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
		newsym(x,y);
		if (cansee(x,y)) pline("¡Una puerta se aparece en la pared!");
		if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK)
		    return TRUE;
		break;		/* striking: continue door handling below */
	    case WAN_LOCKING:
	    case SPE_WIZARD_LOCK:
	    default:
		return FALSE;
	    }
	}

	switch(otmp->otyp) {
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
#ifdef REINCARNATION
	    if (Is_rogue_level(&u.uz)) {
	    	boolean vis = cansee(x,y);
		/* Can't have real locking in Rogue, so just hide doorway */
		if (vis) pline("%s se salta en la más vieja, más primitiva entrada.",
			dustcloud);
		else
			Oyes("un swoosh.");
		if (obstructed(x,y)) {
			if (vis) pline("La nube %s.",quickly_dissipates);
			return FALSE;
		}
		block_point(x, y);
		door->typ = SDOOR;
		if (vis) pline("¡La entrada se desvanece!");
		newsym(x,y);
		return TRUE;
	    }
#endif
	    if (obstructed(x,y)) return FALSE;
	    /* Don't allow doors to close over traps.  This is for pits */
	    /* & trap doors, but is it ever OK for anything else? */
	    if (t_at(x,y)) {
		/* maketrap() clears doormask, so it should be NODOOR */
		pline(
		"%s se salta en la entrada, pero %s.",
		dustcloud, quickly_dissipates);
		return FALSE;
	    }

	    switch (door->doormask & ~D_TRAPPED) {
	    case D_CLOSED:
		msg = "¡La entrada se traba!";
		break;
	    case D_ISOPEN:
		msg = "¡La puerta se columpia cerrada, y se traba!";
		break;
	    case D_BROKEN:
		msg = "¡La puerta rota se reune y se traba!";
		break;
	    case D_NODOOR:
		msg =
		"¡Una nube de polvo se salta y se reune, formando una puerta!";
		break;
	    default:
		res = FALSE;
		break;
	    }
	    block_point(x, y);
	    door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
	    newsym(x,y);
	    break;
	case WAN_OPENING:
	case SPE_KNOCK:
	    if (door->doormask & D_LOCKED) {
		msg = "¡La puerta de destraba!";
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
	    } else res = FALSE;
	    break;
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
	    if (door->doormask & (D_LOCKED | D_CLOSED)) {
		if (door->doormask & D_TRAPPED) {
		    if (MON_AT(x, y))
			(void) mb_trapped(m_at(x,y));
		    else if (flags.verbose) {
			if (cansee(x,y))
			    pline("¡¡BUM!!  Ves una puerta explotando.");
			else if (flags.soundok)
			    Oyes("una explosión distante.");
		    }
		    door->doormask = D_NODOOR;
		    unblock_point(x,y);
		    newsym(x,y);
		    loudness = 40;
		    break;
		}
		door->doormask = D_BROKEN;
		if (flags.verbose) {
		    if (cansee(x,y))
			pline("¡La puerta se abre de golpe!");
		    else if (flags.soundok)
			Oyes("un sonido de golpazo.");
		}
		unblock_point(x,y);
		newsym(x,y);
		/* force vision recalc before printing more messages */
		if (vision_full_recalc) vision_recalc(0);
		loudness = 20;
	    } else res = FALSE;
	    break;
	default: impossible("magia (%d) atentada en una puerta.", otmp->otyp);
	    break;
	}
	if (msg && cansee(x,y)) pline(msg);
	if (loudness > 0) {
	    /* door was destroyed */
	    wake_nearto(x, y, loudness);
	    if (*in_rooms(x, y, SHOPBASE)) add_damage(x, y, 0L);
	}

	if (res && picking_at(x, y)) {
	    /* maybe unseen monster zaps door you're unlocking */
	    stop_occupation();
	    reset_pick();
	}
	return res;
}

STATIC_OVL void
chest_shatter_msg(otmp)
struct obj *otmp;
{
	const char *disposition;
	const char *thing;
	long save_Blinded;

	if (otmp->oclass == POTION_CLASS) {
		pline("¡%s %s estrellando!", Blind? "Oyes" : "Ves", an(bottlename()));
		if (!breathless(youmonst.data) || haseyes(youmonst.data))
			potionbreathe(otmp);
		return;
	}
	/* We have functions for distant and singular names, but not one */
	/* which does _both_... */
	save_Blinded = Blinded;
	Blinded = 1;
	thing = singular(otmp, xname);
	Blinded = save_Blinded;
	switch (objects[otmp->otyp].oc_material) {
	case PAPER:	disposition = "se hace trizas";
		break;
	case WAX:	disposition = "se estruja";
		break;
	case VEGGY:	disposition = "se hace pulpa";
		break;
	case FLESH:	disposition = "se maja";
		break;
	case GLASS:	disposition = "se estrella";
		break;
	case WOOD:	disposition = "se rompe en astillas";
		break;
	default:	disposition = "se destruye";
		break;
	}
	pline("¡%s %s!", An(thing), disposition);
}

#endif /* OVLB */

/*lock.c*/
