/*	SCCS Id: @(#)apply.c	3.4	2003/11/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

#ifdef OVLB

static const char tools[] = { TOOL_CLASS, WEAPON_CLASS, WAND_CLASS, 0 };
static const char tools_too[] = { ALL_CLASSES, TOOL_CLASS, POTION_CLASS,
				  WEAPON_CLASS, WAND_CLASS, GEM_CLASS, 0 };

#ifdef TOURIST
STATIC_DCL int FDECL(use_camera, (struct obj *));
#endif
STATIC_DCL int FDECL(use_towel, (struct obj *));
STATIC_DCL boolean FDECL(its_dead, (int,int,int *));
STATIC_DCL int FDECL(use_stethoscope, (struct obj *));
STATIC_DCL void FDECL(use_whistle, (struct obj *));
STATIC_DCL void FDECL(use_magic_whistle, (struct obj *));
STATIC_DCL void FDECL(use_leash, (struct obj *));
STATIC_DCL int FDECL(use_mirror, (struct obj *));
STATIC_DCL void FDECL(use_bell, (struct obj **));
STATIC_DCL void FDECL(use_candelabrum, (struct obj *));
STATIC_DCL void FDECL(use_candle, (struct obj **));
STATIC_DCL void FDECL(use_lamp, (struct obj *));
STATIC_DCL void FDECL(light_cocktail, (struct obj *));
STATIC_DCL void FDECL(use_tinning_kit, (struct obj *));
STATIC_DCL void FDECL(use_figurine, (struct obj **));
STATIC_DCL void FDECL(use_grease, (struct obj *));
STATIC_DCL void FDECL(use_trap, (struct obj *));
STATIC_DCL void FDECL(use_stone, (struct obj *));
STATIC_PTR int NDECL(set_trap);		/* occupation callback */
STATIC_DCL int FDECL(use_whip, (struct obj *));
STATIC_DCL int FDECL(use_pole, (struct obj *));
STATIC_DCL int FDECL(use_cream_pie, (struct obj *));
STATIC_DCL int FDECL(use_grapple, (struct obj *));
STATIC_DCL int FDECL(do_break_wand, (struct obj *));
STATIC_DCL boolean FDECL(figurine_location_checks,
				(struct obj *, coord *, BOOLEAN_P));
STATIC_DCL boolean NDECL(uhave_graystone);
STATIC_DCL void FDECL(add_class, (char *, CHAR_P));

#ifdef	AMIGA
void FDECL( amii_speaker, ( struct obj *, char *, int ) );
#endif

static const char no_elbow_room[] = "No tienes bastante espacio para maniobrar.";

#ifdef TOURIST
STATIC_OVL int
use_camera(obj)
	struct obj *obj;
{
	register struct monst *mtmp;

	if(Underwater) {
		pline("Usar tu cámara bajo el agua invalidaría la garantía.");
		return(0);
	}
	if(!getdir((char *)0)) return(0);

	if (obj->spe <= 0) {
		pline(nothing_happens);
		return (1);
	}
	consume_obj_charge(obj, TRUE);

	if (obj->cursed && !rn2(2)) {
		(void) zapyourself(obj, TRUE);
	} else if (u.uswallow) {
		pline("Sacas una fotografía %s %s.",
		    del(the(mbodypart(u.ustuck, STOMACH))),
		    del(mon_nam(u.ustuck)));
	} else if (u.dz) {
		pline("Sacas una fotografía %s.",
		      del((u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy)));
	} else if (!u.dx && !u.dy) {
		(void) zapyourself(obj, TRUE);
	} else if ((mtmp = bhit(u.dx, u.dy, COLNO, FLASHED_LIGHT,
				(int FDECL((*),(MONST_P,OBJ_P)))0,
				(int FDECL((*),(OBJ_P,OBJ_P)))0,
				obj)) != 0) {
		obj->ox = u.ux,  obj->oy = u.uy;
		(void) flash_hits_mon(mtmp, obj);
	}
	return 1;
}
#endif

STATIC_OVL int
use_towel(obj)
	struct obj *obj;
{
	const char *cara = body_part(FACE);
	const char *manos = makeplural(body_part(HAND));

	if(!freehand()) {
		pline("¡No tienes %s libres!", manos);
		return 0;
	} else if (obj->owornmask) {
		No_puedes("usarla mientras la lleves puesta!");
		return 0;
	} else if (obj->cursed) {
		long old;
		switch (rn2(3)) {
		case 2:
		    old = Glib;
		    Glib += rn1(10, 3);
		    if (old)
			pline("¡Tus %s están más suci%cs que nunca!",
			      manos, 
			      isfeminine(manos)? 'a' : 'o');
		    else
			pline("¡Tus %s se embarran!", manos);
		    return 1;
		case 1:
		    if (!ublindf) {
			old = u.ucreamed;
			u.ucreamed += rn1(10, 3);
			pline("¡Puaj! ¡Ahora %s fango en %s!",
			      (old ? "tienes más" : "tienes"), the(cara));
			make_blinded(Blinded + (long)u.ucreamed - old, TRUE);
		    } else {
			if (ublindf->cursed) {
			    /* LENGUA:  ¿Cómo se dice "cock-eyed"? */
			    /* Andrés:  'Cock-eyed' es 'bizco'. Pero eso se
			       refiere a ti, no a tus anteojos.  La frase "Te
			       colocas los anteojos bizcos." no me suena muy
			       bien, pero es mejor que dejar la traducción
			       incompleta. */
			    const char *como = rn2(2)? "bizc" : "torcid";
			    if (ublindf->otyp == LENSES)
				pline("Te colocas los anteojos %sos.", como);
			    else
				pline("Te colocas la venda %sa.", como);
			} else {
			    const char *what = (ublindf->otyp == LENSES) ?
						"os anteojos" : "a venda";
			    struct obj *saved_ublindf = ublindf;
			    pline("Te remueves l%s.", what);
			    Blindf_off(ublindf);
			    dropx(saved_ublindf);
			}
		    }
		    return 1;
		case 0:
		    break;
		}
	}

	if (Glib) {
		Glib = 0;
		pline("Te limpias %s.", the(makeplural(body_part(HAND))));
		return 1;
	} else if(u.ucreamed) {
		Blinded -= u.ucreamed;
		u.ucreamed = 0;

		if (!Blinded) {
			pline("Te has quitado el fango.");
			Blinded = 1;
			make_blinded(0L,TRUE);
		} else {
			Sientes("que tu %s está limpi%c ahora.",
			      cara,
			      isfeminine(cara)? 'a' : 'o');
		}
		return 1;
	}

	pline("Tu %s y tu %s ya están limpi%cs.",
	      cara, manos,
	      (isfeminine(cara) && isfeminine(manos))? 'a' : 'o');

	return 0;
}

/* maybe give a stethoscope message based on floor objects */
STATIC_OVL boolean
its_dead(rx, ry, resp)
int rx, ry, *resp;
{
	struct obj *otmp;
	struct trap *ttmp;

	if (!can_reach_floor()) return FALSE;

	/* additional stethoscope messages from jyoung@apanix.apana.org.au */
	if (Hallucination && sobj_at(CORPSE, rx, ry)) {
	    Oyes("una voz diciendo, \"Está muerto, Jim.\"");
	    *resp = 1;
	    return TRUE;
	} else if (Role_if(PM_HEALER) && ((otmp = sobj_at(CORPSE, rx, ry)) != 0 ||
				    (otmp = sobj_at(STATUE, rx, ry)) != 0)) {
	    /* possibly should check uppermost {corpse,statue} in the pile
	       if both types are present, but it's not worth the effort */
	    if (vobj_at(rx, ry)->otyp == STATUE) otmp = vobj_at(rx, ry);
	    if (otmp->otyp == CORPSE) {
		pline("Determinas que %s pobre criatura está muerta.",
		    (rx == u.ux && ry == u.uy) ? "esta" : "esa");
	    } else {
		ttmp = t_at(rx, ry);
		pline("%s parece gozar de una %s salud para ser una estatua.",
		      The(mons[otmp->corpsenm].mname),
		      (ttmp && ttmp->ttyp == STATUE_TRAP) ?
			"extraordinaria" : "excelente");
	    }
	    return TRUE;
	}
	return FALSE;
}

static const char hollow_str[] = "un sonido hueco.  ¡Esto debe de ser un%s %s secret%s!";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
STATIC_OVL int
use_stethoscope(obj)
	register struct obj *obj;
{
	static long last_used_move = -1;
	static short last_used_movement = 0;
	struct monst *mtmp;
	struct rm *lev;
	int rx, ry, res;
	boolean interference = (u.uswallow && is_whirly(u.ustuck->data) &&
				!rn2(Role_if(PM_HEALER) ? 10 : 3));

	if (nohands(youmonst.data)) {	/* should also check for no ears and/or deaf */
		pline("¡No tienes manos!");	/* not `body_part(HAND)' */
		return 0;
	} else if (!freehand()) {
		pline("No tienes %s libres.", makeplural(body_part(HAND)));
		return 0;
	}
	if (!getdir((char *)0)) return 0;

	res = (moves == last_used_move) &&
	      (youmonst.movement == last_used_movement);
	last_used_move = moves;
	last_used_movement = youmonst.movement;

#ifdef STEED
	if (u.usteed && u.dz > 0) {
		if (interference) {
			pline("%s interfiere.", Monnam(u.ustuck));
			mstatusline(u.ustuck);
		} else
			mstatusline(u.usteed);
		return res;
	} else
#endif
	if (u.uswallow && (u.dx || u.dy || u.dz)) {
		mstatusline(u.ustuck);
		return res;
	} else if (u.uswallow && interference) {
		pline("%s interfiere.", Monnam(u.ustuck));
		mstatusline(u.ustuck);
		return res;
	} else if (u.dz) {
		if (Underwater)
		    Oyes("leves chapoteos.");
		else if (u.dz < 0 || !can_reach_floor())
		    No_puedes("alcanzar %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		else if (its_dead(u.ux, u.uy, &res))
		    ;	/* message already given */
		else if (Is_stronghold(&u.uz))
		    Oyes("el crepitar de los fuegos de infierno.");
		else {
		    const char *que = surface(u.ux,u.uy);
		    boolean fem = isfeminine(que);
		    pline("%s parece bastante san%c.",
			  capitalize(que), fem? 'a' : 'o');
		}
		return res;
	} else if (obj->cursed && !rn2(2)) {
		Oyes("el latido de tu corazón.");
		return res;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy) {
		ustatusline();
		return res;
	}
	rx = u.ux + u.dx; ry = u.uy + u.dy;
	if (!isok(rx,ry)) {
		Oyes("el débil sonido de un teclado.");
		return 0;
	}
	if ((mtmp = m_at(rx,ry)) != 0) {
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx,ry)) newsym(mtmp->mx,mtmp->my);
		}
		if (!canspotmon(mtmp))
			map_invisible(rx,ry);
		return res;
	}
	if (glyph_is_invisible(levl[rx][ry].glyph)) {
		unmap_object(rx, ry);
		newsym(rx, ry);
		pline("El monstruo invisible debe de haberse movido.");
	}
	lev = &levl[rx][ry];
	switch(lev->typ) {
	case SDOOR:
		Oyes(hollow_str, "a", "puerta", "a");
		cvt_sdoor_to_door(lev);		/* ->typ = DOOR */
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	case SCORR:
		Oyes(hollow_str, "", "pasadizo", "o");
		lev->typ = CORR;
		unblock_point(rx,ry);
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	}

	if (!its_dead(rx, ry, &res))
	    pline("No oyes nada en especial.");
	return res;
}

static const char whistle_str[] = "Produces un silbido %s.";

STATIC_OVL void
use_whistle(obj)
struct obj *obj;
{
	pline(whistle_str, obj->cursed? "estridente" : "alto");
	wake_nearby();
}

STATIC_OVL void
use_magic_whistle(obj)
struct obj *obj;
{
	register struct monst *mtmp, *nextmon;

	if(obj->cursed && !rn2(2)) {
		pline("Produces un zumbido agudo.");
		wake_nearby();
	} else {
		int pet_cnt = 0;
		pline(whistle_str, Hallucination ? "normal" : "extraño");
		for(mtmp = fmon; mtmp; mtmp = nextmon) {
		    nextmon = mtmp->nmon; /* trap might kill mon */
		    if (DEADMONSTER(mtmp)) continue;
		    if (mtmp->mtame) {
			if (mtmp->mtrapped) {
			    /* no longer in previous trap (affects mintrap) */
			    mtmp->mtrapped = 0;
			    fill_pit(mtmp->mx, mtmp->my);
			}
			mnexto(mtmp);
			if (canspotmon(mtmp)) ++pet_cnt;
			if (mintrap(mtmp) == 2) change_luck(-1);
		    }
		}
		if (pet_cnt > 0) makeknown(obj->otyp);
	}
}

boolean
um_dist(x,y,n)
register xchar x, y, n;
{
	return((boolean)(abs(u.ux - x) > n  || abs(u.uy - y) > n));
}

int
number_leashed()
{
	register int i = 0;
	register struct obj *obj;

	for(obj = invent; obj; obj = obj->nobj)
		if(obj->otyp == LEASH && obj->leashmon != 0) i++;
	return(i);
}

void
o_unleash(otmp)		/* otmp is about to be destroyed or stolen */
register struct obj *otmp;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->m_id == (unsigned)otmp->leashmon)
			mtmp->mleashed = 0;
	otmp->leashmon = 0;
}

void
m_unleash(mtmp, feedback)	/* mtmp is about to die, or become untame */
register struct monst *mtmp;
boolean feedback;
{
	register struct obj *otmp;

	if (feedback) {
	    if (canseemon(mtmp))
		pline("¡%s se suelta de su correa!", Monnam(mtmp));
	    else
		pline("Tu correa cae suelta.");
	}
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH &&
				otmp->leashmon == (int)mtmp->m_id)
			otmp->leashmon = 0;
	mtmp->mleashed = 0;
}

void
unleash_all()		/* player is about to die (for bones) */
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH) otmp->leashmon = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		mtmp->mleashed = 0;
}

#define MAXLEASHED	2

/* ARGSUSED */
STATIC_OVL void
use_leash(obj)
struct obj *obj;
{
	coord cc;
	register struct monst *mtmp;
	int spotmon;

	if(!obj->leashmon && number_leashed() >= MAXLEASHED) {
		No_puedes("usar ninguna correa más.");
		return;
	}

	if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return;

	if((cc.x == u.ux) && (cc.y == u.uy)) {
#ifdef STEED
		if (u.usteed && u.dz > 0) {
		    mtmp = u.usteed;
		    spotmon = 1;
		    goto got_target;
		}
#endif
		pline("¿Quieres ponerte la correa?  Muy divertido...");
		return;
	}

	if(!(mtmp = m_at(cc.x, cc.y))) {
		pline("No hay ninguna criatura ahí.");
		return;
	}

	spotmon = canspotmon(mtmp);
#ifdef STEED
 got_target:
#endif

	if(!mtmp->mtame) {
	    if(!spotmon)
		pline("No hay ninguna criatura ahí.");
	    else
		if (obj->leashmon)
			pline("¡No tienes una correa atada %s!", al(mon_nam(mtmp)));
		else
			No_puedes("ponerle la correa %s!", al(mon_nam(mtmp)));
	    return;
	}
	if(!obj->leashmon) {
		if(mtmp->mleashed) {
			const char *que = spotmon ? l_monnam(mtmp) : "monstruo";
			pline("Ya tienes una correa atada a est%c %s.",
				isfeminine(que)? 'a' : 'e', que);
			return;
		}
		pline("Le pones la correa a %s%s.",
		    spotmon ? "tu " : "", l_monnam(mtmp));
		mtmp->mleashed = 1;
		obj->leashmon = (int)mtmp->m_id;
		mtmp->msleeping = 0;
		return;
	}
	if(obj->leashmon != (int)mtmp->m_id) {
		pline("Esta correa no esta atada a esa criatura.");
		return;
	} else {
		if(obj->cursed) {
			pline("¡La correa no se suelta!");
			obj->bknown = TRUE;
			return;
		}
		mtmp->mleashed = 0;
		obj->leashmon = 0;
		pline("Le quitas la correa a %s%s.",
		    spotmon ? "tu " : "", l_monnam(mtmp));
	}
	return;
}

struct obj *
get_mleash(mtmp)	/* assuming mtmp->mleashed has been checked */
register struct monst *mtmp;
{
	register struct obj *otmp;

	otmp = invent;
	while(otmp) {
		if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
			return(otmp);
		otmp = otmp->nobj;
	}
	return((struct obj *)0);
}

#endif /* OVLB */
#ifdef OVL1

boolean
next_to_u()
{
	register struct monst *mtmp;
	register struct obj *otmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if(mtmp->mleashed) {
			if (distu(mtmp->mx,mtmp->my) > 2) mnexto(mtmp);
			if (distu(mtmp->mx,mtmp->my) > 2) {
			    for(otmp = invent; otmp; otmp = otmp->nobj)
				if(otmp->otyp == LEASH &&
					otmp->leashmon == (int)mtmp->m_id) {
				    if(otmp->cursed) return(FALSE);
				    Sientes("que %s correa se afloja.",
					(number_leashed() > 1) ? "una" : "la");
				    mtmp->mleashed = 0;
				    otmp->leashmon = 0;
				}
			}
		}
	}
#ifdef STEED
	/* no pack mules for the Amulet */
	if (u.usteed && mon_has_amulet(u.usteed)) return FALSE;
#endif
	return(TRUE);
}

#endif /* OVL1 */
#ifdef OVL0

void
check_leash(x, y)
register xchar x, y;
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for (otmp = invent; otmp; otmp = otmp->nobj) {
	    if (otmp->otyp != LEASH || otmp->leashmon == 0) continue;
	    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if ((int)mtmp->m_id == otmp->leashmon) break; 
	    }
	    if (!mtmp) {
		impossible("¿La correa en uso no está unida a nada?");
		otmp->leashmon = 0;
		continue;
	    }
	    if (dist2(u.ux,u.uy,mtmp->mx,mtmp->my) >
		    dist2(x,y,mtmp->mx,mtmp->my)) {
		if (!um_dist(mtmp->mx, mtmp->my, 3)) {
		    ;	/* still close enough */
		} else if (otmp->cursed && !breathless(mtmp->data)) {
		    if (um_dist(mtmp->mx, mtmp->my, 5) ||
			    (mtmp->mhp -= rnd(2)) <= 0) {
			long save_pacifism = u.uconduct.killer;

			pline("¡Tu correa mata %s estrangulándol%c!",
				al(mon_nam(mtmp)), mon_gender(mtmp)? 'a' : 'o');
			/* hero might not have intended to kill pet, but
			   that's the result of his actions; gain experience,
			   lose pacifism, take alignment and luck hit, make
			   corpse less likely to remain tame after revival */
			xkilled(mtmp, 0);	/* no "you kill it" message */
			/* life-saving doesn't ordinarily reset this */
			if (mtmp->mhp > 0) u.uconduct.killer = save_pacifism;
		    } else {
			pline("¡%s se esta estrangulando con la correa!",
				Monnam(mtmp));
			/* tameness eventually drops to 1 here (never 0) */
			if (mtmp->mtame && rn2(mtmp->mtame)) mtmp->mtame--;
		    }
		} else {
		    if (um_dist(mtmp->mx, mtmp->my, 5)) {
			pline("¡La correa %s se suelta con un chasquido!",
					del(mon_nam(mtmp)));
			m_unleash(mtmp, FALSE);
		    } else {
			pline("Tiras de la correa.");
			if (mtmp->data->msound != MS_SILENT)
			    switch (rn2(3)) {
			    case 0:  growl(mtmp);   break;
			    case 1:  yelp(mtmp);    break;
			    default: whimper(mtmp); break;
			    }
		    }
		}
	    }
	}
}

#endif /* OVL0 */
#ifdef OVLB

#define WEAK	3	/* from eat.c */

static const char look_str[] = "Pareces %s.";

STATIC_OVL int
use_mirror(obj)
struct obj *obj;
{
	register struct monst *mtmp;
	register char mlet;
	boolean vis;

	if(!getdir((char *)0)) return 0;
	if(obj->cursed && !rn2(2)) {
		if (!Blind)
			pline("¡El espejo se nubla y no refleja nada!");
		return 1;
	}
	if(!u.dx && !u.dy && !u.dz) {
		if(!Blind && !Invisible) {
		    if (u.umonnum == PM_FLOATING_EYE) {
			if (!Free_action) {
			pline(Hallucination ?
			      "¡Guau!  ¡El espejo te devuelve la mirada!" :
			      "¡Ay!  ¡Te has paralizado!");
			nomul(-rnd((MAXULEV+6) - u.ulevel));
			} else pline("Te tensas por un momento bajo tu mirada.");
		    } else if (youmonst.data->mlet == S_VAMPIRE)
			pline("No tienes reflejo.");
		    else if (u.umonnum == PM_UMBER_HULK) {
			pline("¿Oh?  ¡Es%c no se parece a ti!",
			      poly_gender()==1? 'a' : 'e');
			make_confused(HConfusion + d(3,4),FALSE);
		    } else if (Hallucination)
			pline(look_str, feminize(hcolor((char *)0), poly_gender()==1));
		    else if (Sick)
			pline(look_str, feminize("enfermo", poly_gender()==1));
		    else if (u.uhs >= WEAK)
			pline(look_str, feminize("malnutrido", poly_gender()==1));
		    else pline("Pareces tan %s%c como siempre.",
			       (ACURR(A_CHA) > 14) ? "hermos" : "fe",
			       poly_gender()==1? 'a' : 'o');
		} else {
			const char *cara = body_part(FACE);
			No_puedes("ver tu %s %s%c.", cara,
				  (ACURR(A_CHA) > 14) ? "hermos" : "fe",
				  isfeminine(cara)? 'a' : 'o');
		}
		return 1;
	}
	if(u.uswallow) {
		if (!Blind) pline("El espejo refleja %s %s.",
		    the(mbodypart(u.ustuck, STOMACH)),
		    del(mon_nam(u.ustuck)));
		return 1;
	}
	if(Underwater) {
		pline(Hallucination ?
		      "Das a los peces la oportunidad de arreglarse el maquillaje" :
		      "El agua turbia se refleja en el espejo.");
		return 1;
	}
	if(u.dz) {
		if (!Blind) {
		    pline("El espejo refleja %s.",
        		  (u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		}
		return 1;
	}
	mtmp = bhit(u.dx, u.dy, COLNO, INVIS_BEAM,
		    (int FDECL((*),(MONST_P,OBJ_P)))0,
		    (int FDECL((*),(OBJ_P,OBJ_P)))0,
		    obj);
	if (!mtmp || !haseyes(mtmp->data))
		return 1;

	vis = canseemon(mtmp);
	mlet = mtmp->data->mlet;
	if (mtmp->msleeping) {
		if (vis) {
		    const char *quien = Monnam(mtmp);
		    pline("%s está demasiado cansad%c para mirarse en tu espejo.",
			  quien, isfeminine(quien)? 'a' : 'o');
		}
	} else if (!mtmp->mcansee) {
	    if (vis)
		pline("Ahora %s no puede ver nada.", Monnam(mtmp));
	/* some monsters do special things */
	} else if (mlet == S_VAMPIRE || mlet == S_GHOST) {
	    if (vis)
		pline ("%s no tiene reflejo.", Monnam(mtmp));
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_MEDUSA]) {
		if (mon_reflects(mtmp, "¡La mirada se refleja en %s %s!"))
			return 1;
		if (vis)
			pline("¡%s se petrifica!", Monnam(mtmp));
		stoned = TRUE;
		killed(mtmp);
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_FLOATING_EYE]) {
		int tmp = d((int)mtmp->m_lev, (int)mtmp->data->mattk[0].damd);
		if (!rn2(4)) tmp = 120;
		if (vis) {
			const char *quien = Monnam(mtmp);
			pline("%s es paralizad%c por su reflejo.",
			      quien, isfeminine(quien)? 'a' : 'o');
		} else Oyes("%s dejar de moverse.",something);
		mtmp->mcanmove = 0;
		if ( (int) mtmp->mfrozen + tmp > 127)
			mtmp->mfrozen = 127;
		else mtmp->mfrozen += tmp;
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_UMBER_HULK]) {
		if (vis)
			pline ("¡%s se confunde!", Monnam(mtmp));
		mtmp->mconf = 1;
	} else if(!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH
				     || mtmp->data==&mons[PM_SUCCUBUS])) {
		if (vis) {
		    pline ("%s se admira en tu espejo.", Monnam(mtmp));
		    pline ("¡Lo toma!");
		} else pline ("¡Te roba el espejo!");
		setnotworn(obj); /* in case mirror was wielded */
		freeinv(obj);
		(void) mpickobj(mtmp,obj);
		if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
	} else if (!is_unicorn(mtmp->data) && !humanoid(mtmp->data) &&
			(!mtmp->minvis || perceives(mtmp->data)) && rn2(5)) {
		if (vis)
		    pline ("%s se asusta de su reflejo.",
			   Monnam(mtmp));
		monflee(mtmp, d(2,4), FALSE, FALSE);
	} else if (!Blind) {
		if (mtmp->minvis && !See_invisible)
		    ;
		else if ((mtmp->minvis && !perceives(mtmp->data))
			 || !haseyes(mtmp->data))
		    pline("No parece que %s se dé cuenta de su reflejo.",
			Monnam(mtmp));
		else
		    pline("%s no presta atención a su reflejo.",
			  Monnam(mtmp));
	}
	return 1;
}

STATIC_OVL void
use_bell(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	struct monst *mtmp;
	boolean wakem = FALSE, learno = FALSE,
		ordinary = (obj->otyp != BELL_OF_OPENING || !obj->spe),
		invoking = (obj->otyp == BELL_OF_OPENING &&
			 invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy));

	pline("Tocas %s.", the(xname(obj)));

	if (Underwater || (u.uswallow && ordinary)) {
#ifdef	AMIGA
	    amii_speaker( obj, "AhDhGqEqDhEhAqDqFhGw", AMII_MUFFLED_VOLUME );
#endif
	    pline("Pero el sonido está amortiguado.");

	} else if (invoking && ordinary) {
	    /* needs to be recharged... */
	    pline("Pero no hace ningún sonido.");
	    learno = TRUE;	/* help player figure out why */

	} else if (ordinary) {
#ifdef	AMIGA
	    amii_speaker( obj, "ahdhgqeqdhehaqdqfhgw", AMII_MUFFLED_VOLUME );
#endif
	    if (obj->cursed && !rn2(4) &&
		    /* note: once any of them are gone, we stop all of them */
		    !(mvitals[PM_WOOD_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_MOUNTAIN_NYMPH].mvflags & G_GONE) &&
		    (mtmp = makemon(mkclass(S_NYMPH, 0),
					u.ux, u.uy, NO_MINVENT)) != 0) {
		pline("¡Llamas a %s!", a_monnam(mtmp));
		if (!obj_resists(obj, 93, 100)) {
		    pline("¡%s hecho añicos!", Tobjnam(obj, "se ha"));
		    useup(obj);
		    *optr = 0;
		} else switch (rn2(3)) {
			default:
				break;
			case 1:
				mon_adjust_speed(mtmp, 2, (struct obj *)0);
				break;
			case 2: /* no explanation; it just happens... */
				nomovemsg = "";
				nomul(-rnd(2));
				break;
		}
	    }
	    wakem = TRUE;

	} else {
	    /* charged Bell of Opening */
	    consume_obj_charge(obj, TRUE);

	    if (u.uswallow) {
		if (!obj->cursed)
		    (void) openit();
		else
		    pline(nothing_happens);

	    } else if (obj->cursed) {
		coord mm;

		mm.x = u.ux;
		mm.y = u.uy;
		mkundead(&mm, FALSE, NO_MINVENT);
		wakem = TRUE;

	    } else  if (invoking) {
		pline("%s un chillido desquiciante...",
		      Tobjnam(obj, "produce"));
#ifdef	AMIGA
		amii_speaker( obj, "aefeaefeaefeaefeaefe", AMII_LOUDER_VOLUME );
#endif
		obj->age = moves;
		learno = TRUE;
		wakem = TRUE;

	    } else if (obj->blessed) {
		int res = 0;

#ifdef	AMIGA
		amii_speaker( obj, "ahahahDhEhCw", AMII_SOFT_VOLUME );
#endif
		if (uchain) {
		    unpunish();
		    res = 1;
		}
		res += openit();
		switch (res) {
		  case 0:  pline(nothing_happens); break;
		  case 1:  pline("%s abre...", Something);
			   learno = TRUE; break;
		  default: pline("Se abren cosas a tu alrededor...");
			   learno = TRUE; break;
		}

	    } else {  /* uncursed */
#ifdef	AMIGA
		amii_speaker( obj, "AeFeaeFeAefegw", AMII_OKAY_VOLUME );
#endif
		if (findit() != 0) learno = TRUE;
		else pline(nothing_happens);
	    }

	}	/* charged BofO */

	if (learno) {
	    makeknown(BELL_OF_OPENING);
	    obj->known = 1;
	}
	if (wakem) wake_nearby();
}

STATIC_OVL void
use_candelabrum(obj)
register struct obj *obj;
{
	const char *s = obj->spe != 1 ? "velas" : "vela";

	if(Underwater) {
		No_puedes("encender fuego en el agua.");
		return;
	}
	if(obj->lamplit) {
		pline("Apagas la%s %s.", plur(obj->spe), s);
		end_burn(obj, TRUE);
		return;
	}
	if(obj->spe <= 0) {
		pline("Este %s no tiene velas.", xname(obj));
		return;
	}
	if(u.uswallow || obj->cursed) {
		if (!Blind)
		    pline("La%s %s %s por un momento, y entonces se %s.",
			  plur(obj->spe), s, vtense(s, "titila"), vtense(s, "apaga"));
		return;
	}
	if(obj->spe < 7) {
		pline("Sólo hay %d %s en %s.",
		    obj->spe, s, the(xname(obj)));
		if (!Blind)
		    pline("Está%s encendida%s.  %s con una luz mortecina.",
			  obj->spe == 1 ? "" : "n",
			  plur(obj->spe),
			  Tobjnam(obj, "alumbra"));
	} else {
		pline("%sLa%s %s %s arden%s",
			(Blind ? "" : "¡"),
			plur(obj->spe), s,
			del(the(xname(obj))),
			(Blind ? "." : " con fuerza!"));
	}
	if (!invocation_pos(u.ux, u.uy)) {
		pline("¡La%s %s se %s rápidamente!", plur(obj->spe), s, vtense(s, "consume"));
		obj->age /= 2;
	} else {
		if(obj->spe == 7) {
		    if (Blind)
		      pline("¡%s un calor extraño!", Tobjnam(obj, "irradia"));
		    else
		      pline("¡%s con una luz extraña!", Tobjnam(obj, "brilla"));
		}
		obj->known = 1;
	}
	begin_burn(obj, FALSE);
}

STATIC_OVL void
use_candle(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	register struct obj *otmp;
	const char *s = obj->quan != 1 ? "velas" : "vela";
	char qbuf[QBUFSZ];

	if(u.uswallow) {
		pline(no_elbow_room);
		return;
	}
	if(Underwater) {
		pline("Lo siento, el fuego y el agua no se pueden mezclar.");
		return;
	}

	otmp = carrying(CANDELABRUM_OF_INVOCATION);
	if(!otmp || otmp->spe == 7) {
		use_lamp(obj);
		return;
	}

	Sprintf(qbuf, "¿Quieres poner %s en", the(xname(obj)));
	Sprintf(eos(qbuf), " %s?",
		safe_qbuf(qbuf, sizeof(" ?"), the(xname(otmp)),
			the(simple_typename(otmp->otyp)), "esto"));
	if(yn(qbuf) == 'n') {
		if (!obj->lamplit)
		    pline("Intentas encender %s...", the(xname(obj)));
		use_lamp(obj);
		return;
	} else {
		if ((long)otmp->spe + obj->quan > 7L)
		    obj = splitobj(obj, 7L - (long)otmp->spe);
		else *optr = 0;
		pline("Pones %ld%s %s %s.",
		    obj->quan, !otmp->spe ? "" : " más",
		    s, al(the(xname(otmp))));
		if (!otmp->spe || otmp->age > obj->age)
		    otmp->age = obj->age;
		otmp->spe += (int)obj->quan;
		if (otmp->lamplit && !obj->lamplit)
		    pline("¡La%s %s nueva%s se enciende%s mágicamente!",
			  plur(obj->quan),
			  s,
			  plur(obj->quan),
			  vtense(s, "encienda"));
		else if (!otmp->lamplit && obj->lamplit)
		    pline("Se apaga%s.", (obj->quan > 1L) ? "n" : "");
		if (obj->unpaid)
		    verbalize("¡Usted la%s %s, usted la%s compra!",
			      plur(obj->quan),
			      otmp->lamplit ? "quema" : "usa",
			      plur(obj->quan));
		if (obj->quan < 7L && otmp->spe == 7)
		    pline("%s ahora tiene siete velas%s.",
			  The(xname(otmp)), otmp->lamplit ? " encendidas" : "");
		/* candelabrum's light range might increase */
		if (otmp->lamplit) obj_merge_light_sources(otmp, otmp);
		/* candles are no longer a separate light source */
		if (obj->lamplit) end_burn(obj, TRUE);
		/* candles are now gone */
		useupall(obj);
	}
}

boolean
snuff_candle(otmp)  /* call in drop, throw, and put in box, etc. */
register struct obj *otmp;
{
	register boolean candle = Is_candle(otmp);

	if ((candle || otmp->otyp == CANDELABRUM_OF_INVOCATION) &&
		otmp->lamplit) {
	    xchar x, y;
	    boolean many = candle ? otmp->quan > 1L : otmp->spe > 1;
	    register const char *plu = many? "s" : "";

	    (void) get_obj_location(otmp, &x, &y, 0);
	    if (otmp->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		pline("La%s llama%s de %s se extingue%s.",
		      plu, plu, yname(otmp), many? "n" : "");
	   end_burn(otmp, TRUE);
	   return(TRUE);
	}
	return(FALSE);
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
boolean
snuff_lit(obj)
struct obj *obj;
{
	xchar x, y;

	if (obj->lamplit) {
	    if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		    obj->otyp == BRASS_LANTERN || obj->otyp == POT_OIL) {
		(void) get_obj_location(obj, &x, &y, 0);
		if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		    pline("¡%s se %s!", Yname2(obj), otense(obj, "apaga"));
		end_burn(obj, TRUE);
		return TRUE;
	    }
	    if (snuff_candle(obj)) return TRUE;
	}
	return FALSE;
}

/* Called when potentially lightable object is affected by fire_damage().
   Return TRUE if object was lit and FALSE otherwise --ALI */
boolean
catch_lit(obj)
struct obj *obj;
{
	xchar x, y;

	if (!obj->lamplit && (obj->otyp == MAGIC_LAMP || ignitable(obj))) {
	    if ((obj->otyp == MAGIC_LAMP ||
		 obj->otyp == CANDELABRUM_OF_INVOCATION) &&
		obj->spe == 0)
		return FALSE;
	    else if (obj->otyp != MAGIC_LAMP && obj->age == 0)
		return FALSE;
	    if (!get_obj_location(obj, &x, &y, 0))
		return FALSE;
	    if (obj->otyp == CANDELABRUM_OF_INVOCATION && obj->cursed)
		return FALSE;
	    if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		 obj->otyp == BRASS_LANTERN) && obj->cursed && !rn2(2))
		return FALSE;
	    if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		pline("%s %s light!", Yname2(obj), otense(obj, "catch"));
	    if (obj->otyp == POT_OIL) makeknown(obj->otyp);
	    if (obj->unpaid && costly_spot(u.ux, u.uy) && (obj->where == OBJ_INVENT)) {
	        /* if it catches while you have it, then it's your tough luck */
		char *que = yname(obj);
		check_unpaid(obj);
		verbalize("Esto es además del precio %s mism%c%s, por supuesto.",
			del(que), isfeminine(que)? 'a' : 'o',
			obj->quan > 1? "s" : "");
		bill_dummy_object(obj);
	    }
	    begin_burn(obj, FALSE);
	    return TRUE;
	}
	return FALSE;
}

STATIC_OVL void
use_lamp(obj)
struct obj *obj;
{
	if(Underwater) {
		pline("Esta no es una lámpara de buceo.");
		return;
	}
	if(obj->lamplit) {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN) {
		    const char *que = Yname2(obj);
		    pline("%s ahora está apagad%c.", que,
			  isfeminine(que)? 'a' : 'o');
		} else
		    pline("Apagas %s.", yname(obj));
		end_burn(obj, TRUE);
		return;
	}
	/* magic lamps with an spe == 0 (wished for) cannot be lit */
	if ((!Is_candle(obj) && obj->age == 0)
			|| (obj->otyp == MAGIC_LAMP && obj->spe == 0)) {
		if (obj->otyp == BRASS_LANTERN)
			pline("Tu lámpara ha perdido su poder.");
		else {
			const char *que = xname(obj);
			pline("Est%c %s no tiene aceite.",
			      isfeminine(que)? 'a' : 'o', que);
		}
		return;
	}
	if (obj->cursed && !rn2(2)) {
		pline("%s por un momento, y se %s.",
		      Tobjnam(obj, "titila"), otense(obj, "apaga"));
	} else {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN) {
		    const char *que = Yname2(obj);
		    check_unpaid(obj);
		    pline("%s ahora está encendid%c.",
			  que,
			  isfeminine(que)? 'a' : 'o');
		} else {	/* candle(s) */
		    pline("%sLa%s llama%s %s se %s%s",
			Blind ? "" : "¡",
			plur(obj->quan),
			plur(obj->quan),
			del(yname(obj)), otense(obj, "arde"),
			Blind ? "." : " con fuerza!");
		    if (obj->unpaid && costly_spot(u.ux, u.uy) &&
			  obj->age == 20L * (long)objects[obj->otyp].oc_cost) {
			const char *ithem = obj->quan > 1L ? "s" : "";
			verbalize("¡Usted la%s quema, usted la%s compra!",
				ithem, ithem);
			bill_dummy_object(obj);
		    }
		}
		begin_burn(obj, FALSE);
	}
}

STATIC_OVL void
light_cocktail(obj)
	struct obj *obj;	/* obj is a potion of oil */
{
	if (u.uswallow) {
	    pline(no_elbow_room);
	    return;
	}

	if (obj->lamplit) {
	    pline("Apagas la poción encendida.");
	    end_burn(obj, TRUE);
	    /*
	     * Free & add to re-merge potion.  This will average the
	     * age of the potions.  Not exactly the best solution,
	     * but its easy.
	     */
	    freeinv(obj);
	    (void) addinv(obj);
	    return;
	} else if (Underwater) {
	    pline("No hay bastante oxígeno para sostener un fuego.");
	    return;
	}

	pline("Alumbras %s.%s", yname(obj),
	    Blind ? "" : "  Emite luz mortecina.");
	if (obj->unpaid && costly_spot(u.ux, u.uy)) {
	    /* Normally, we shouldn't both partially and fully charge
	     * for an item, but (Yendorian Fuel) Taxes are inevitable...
	     */
	    check_unpaid(obj);
	    verbalize("Esto es además del precio de la poción, por supuesto.");
	    bill_dummy_object(obj);
	}
	makeknown(obj->otyp);

	if (obj->quan > 1L) {
	    obj = splitobj(obj, 1L);
	    begin_burn(obj, FALSE);	/* burn before free to get position */
	    obj_extract_self(obj);	/* free from inv */

	    /* shouldn't merge */
	    obj = hold_another_object(obj, "¡Dejas caer %s!",
				      doname(obj), (const char *)0);
	} else
	    begin_burn(obj, FALSE);
}

static NEARDATA const char cuddly[] = { TOOL_CLASS, GEM_CLASS, 0 };

int
dorub()
{
	struct obj *obj = getobj(cuddly, "", "frotar");

	if (obj && obj->oclass == GEM_CLASS) {
	    if (is_graystone(obj)) {
		use_stone(obj);
		return 1;
	    } else {
		pline("Lo siento, no sé como se usa esto.");
		return 0;
	    }
	}

	if (!obj || !wield_tool(obj, "frotar", "frotas")) return 0;

	/* now uwep is obj */
	if (uwep->otyp == MAGIC_LAMP) {
	    if (uwep->spe > 0 && !rn2(3)) {
		check_unpaid_usage(uwep, TRUE);		/* unusual item use */
		djinni_from_bottle(uwep);
		makeknown(MAGIC_LAMP);
		uwep->otyp = OIL_LAMP;
		uwep->spe = 0; /* for safety */
		uwep->age = rn1(500,1000);
		if (uwep->lamplit) begin_burn(uwep, TRUE);
		update_inventory();
	    } else if (rn2(2) && !Blind)
		pline("Ves una bocanada de humo.");
	    else pline(nothing_happens);
	} else if (obj->otyp == BRASS_LANTERN) {
	    /* message from Adventure */
	    pline("Frotar la lámpara eléctrica no resulta demasiado gratificante.");
	    pline("De todos modos, no ocurre nada excitante.");
	} else pline(nothing_happens);
	return 1;
}

int
dojump()
{
	/* Physical jump */
	return jump(0);
}

int
jump(magic)
int magic; /* 0=Physical, otherwise skill level */
{
	coord cc;

	if (!magic && (nolimbs(youmonst.data) || slithy(youmonst.data))) {
		/* normally (nolimbs || slithy) implies !Jumping,
		   but that isn't necessarily the case for knights */
		No_puedes("saltar; no tienes piernas!");
		return 0;
	} else if (!magic && !Jumping) {
		No_puedes("saltar muy lejos.");
		return 0;
	} else if (u.uswallow) {
		if (magic) {
			pline("Brincas un poco.");
			return 1;
		}
		pline("¡Debes de estar bromeando!");
		return 0;
	} else if (u.uinwater) {
		if (magic) {
			pline("Te sacudes un poco en el aire.");
			return 1;
		}
		pline("¡Es momento de nadar, no de saltar!");
		return 0;
	} else if (u.ustuck) {
		if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf) {
		    pline("Te liberas %s.", del(mon_nam(u.ustuck)));
		    u.ustuck = 0;
		    return 1;
		}
		if (magic) {
			pline("!Te retuerces un poco como presa %s!", del(mon_nam(u.ustuck)));
			return 1;
		}
		No_puedes("escapar %s!", del(mon_nam(u.ustuck)));
		return 0;
	} else if (Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		if (magic) {
			pline("Te agitas un poco.");
			return 1;
		}
		pline("No tienes suficiente tracción para saltar.");
		return 0;
	} else if (!magic && near_capacity() > UNENCUMBERED) {
		pline("¡Estás demasiado cargad%c para saltar!",
		      poly_gender()==1? 'a' : 'o');
		return 0;
	} else if (!magic && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
		pline("¡No tienes fuerzas para saltar!");
		return 0;
	} else if (Wounded_legs) {
		long wl = (Wounded_legs & BOTH_SIDES);
		const char *bp = body_part(LEG);

		if (wl == BOTH_SIDES) bp = makeplural(bp);
#ifdef STEED
		if (u.usteed)
		    pline("%s no está en condiciónes para saltar.", Monnam(u.usteed));
		else
#endif
		pline("Tu%s %s%s no está%s en condiciónes para saltar.",
		     (wl == BOTH_SIDES) ? "s" : "",
		     bp,
		     (wl == BOTH_SIDES) ? "" :
			feminize((wl == LEFT_SIDE)? " izquierdo" : " derecho",
				 isfeminine(bp)),
		     (wl == BOTH_SIDES) ? "n" : "");
		return 0;
	}
#ifdef STEED
	else if (u.usteed && u.utrap) {
		pline("%s está atascad%c en una trampa.",
		      Monnam(u.usteed), mon_gender(u.usteed)? 'a' : 'o');
		return (0);
	}
#endif

	pline("¿A dónde quieres saltar?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "la posición deseada") < 0)
		return 0;	/* user pressed ESC */
	if (!magic && !(HJumping & ~INTRINSIC) && !EJumping &&
			distu(cc.x, cc.y) != 5) {
		/* The Knight jumping restriction still applies when riding a
		 * horse.  After all, what shape is the knight piece in chess?
		 */
		pline("¡Movimiento ilegal!");
		return 0;
	} else if (distu(cc.x, cc.y) > (magic ? 6+magic*3 : 9)) {
		pline("¡Demasiado lejos!");
		return 0;
	} else if (!cansee(cc.x, cc.y)) {
		No_puedes("ver dónde aterizar!");
		return 0;
	} else if (!isok(cc.x, cc.y)) {
		No_puedes("saltar ahí!");
		return 0;
	} else {
	    coord uc;
	    int range, temp;

	    if(u.utrap)
		switch(u.utraptype) {
		case TT_BEARTRAP: {
		    register long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
		    pline("¡Te lastimas al liberarte de la trampa para osos!  ¡Ay!");
		    losehp(rnd(10), "saltar desde una trampa para osos", KILLED_BY);
		    set_wounded_legs(side, rn1(1000,500));
		    break;
		  }
		case TT_PIT:
		    pline("¡Saltas desde el hoyo!");
		    break;
		case TT_WEB:
		    pline("¡Rasgas la telaraña al liberarte de ella!");
		    deltrap(t_at(u.ux,u.uy));
		    break;
		case TT_LAVA:
		    pline("¡Te arrastras fuera de la lava!");
		    u.utrap = 0;
		    return 1;
		case TT_INFLOOR:
		    pline("Fuerzas tus %s, pero todavía estás atascad%c en el suelo.",
			makeplural(body_part(LEG)),
			poly_gender()==1? 'a' : 'o');
		    set_wounded_legs(LEFT_SIDE, rn1(10, 11));
		    set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
		    return 1;
		}

	    /*
	     * Check the path from uc to cc, calling hurtle_step at each
	     * location.  The final position actually reached will be
	     * in cc.
	     */
	    uc.x = u.ux;
	    uc.y = u.uy;
	    /* calculate max(abs(dx), abs(dy)) as the range */
	    range = cc.x - uc.x;
	    if (range < 0) range = -range;
	    temp = cc.y - uc.y;
	    if (temp < 0) temp = -temp;
	    if (range < temp)
		range = temp;
	    (void) walk_path(&uc, &cc, hurtle_step, (genericptr_t)&range);

	    /* A little Sokoban guilt... */
	    if (In_sokoban(&u.uz))
		change_luck(-1);

	    teleds(cc.x, cc.y, TRUE);
	    nomul(-1);
	    nomovemsg = "";
	    morehungry(rnd(25));
	    return 1;
	}
}

boolean
tinnable(corpse)
struct obj *corpse;
{
	if (corpse->oeaten) return 0;
	if (!mons[corpse->corpsenm].cnutrit) return 0;
	return 1;
}

STATIC_OVL void
use_tinning_kit(obj)
register struct obj *obj;
{
	register struct obj *corpse, *can;

	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (obj->spe <= 0) {
		pline("Parece que no tienes latas.");
		return;
	}
	if (!(corpse = floorfood("enlatar", 2))) return;
	if (corpse->oeaten) {
		No_puedes("enlatar %s que está a medio comer.", something);
		return;
	}
	if (touch_petrifies(&mons[corpse->corpsenm])
		&& !Stone_resistance && !uarmg) {
	    char kbuf[BUFSZ];

	    if (poly_when_stoned(youmonst.data))
		pline("Enlatas %s sin llevar guantes.",
			an(mons[corpse->corpsenm].mname));
	    else {
		pline("Enlatar %s sin llevar guantes es un error fatal...",
			an(mons[corpse->corpsenm].mname));
		Sprintf(kbuf, "tratar de enlatar %s sin guantes",
			an(mons[corpse->corpsenm].mname));
	    }
	    instapetrify(kbuf);
	}
	if (is_rider(&mons[corpse->corpsenm])) {
		(void) revive_corpse(corpse);
		verbalize("Sí...  Pero Guerra no conserva sus enemigos...");
		return;
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
		pline("Esto es demasiado insustancial para enlatarlo.");
		return;
	}
	consume_obj_charge(obj, TRUE);

	if ((can = mksobj(TIN, FALSE, FALSE)) != 0) {
	    static const char you_buy_it[] = "¡Usted lo enlata, usted lo compra!";

	    can->corpsenm = corpse->corpsenm;
	    can->cursed = obj->cursed;
	    can->blessed = obj->blessed;
	    can->owt = weight(can);
	    can->known = 1;
	    can->spe = -1;  /* Mark tinned tins. No spinach allowed... */
	    if (carried(corpse)) {
		if (corpse->unpaid)
		    verbalize(you_buy_it);
		useup(corpse);
	    } else {
		if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
		    verbalize(you_buy_it);
		useupf(corpse, 1L);
	    }
	    can = hold_another_object(can, "Haces, pero no puedes recoger, %s.",
				      doname(can), (const char *)0);
	} else impossible("Falló el enlatado.");
}

void
use_unicorn_horn(obj)
struct obj *obj;
{
#define PROP_COUNT 6		/* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX*3)	/* number of attribute points we might fix */
	int idx, val, val_limit,
	    trouble_count, unfixable_trbl, did_prop, did_attr;
	int trouble_list[PROP_COUNT + ATTR_COUNT];

	if (obj && obj->cursed) {
	    long lcount = (long) rnd(100);

	    switch (rn2(6)) {
	    case 0: make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON),20),
			xname(obj), TRUE, SICK_NONVOMITABLE);
		    break;
	    case 1: make_blinded(Blinded + lcount, TRUE);
		    break;
	    case 2: if (!Confusion) {
			if (Hallucination)
			    /* LENGUA:  How does one speak of an acid trip in Spanish? */
			    pline("De repente sientes que viajas.");
			else
			    Te_sientes("confundid%c de repente.",
				       poly_gender()==1? 'a' : 'o');
		    }
		    make_confused(HConfusion + lcount, TRUE);
		    break;
	    case 3: make_stunned(HStun + lcount, TRUE);
		    break;
	    case 4: (void) adjattrib(rn2(A_MAX), -1, FALSE);
		    break;
	    case 5: (void) make_hallucinated(HHallucination + lcount, TRUE, 0L);
		    break;
	    }
	    return;
	}

/*
 * Entries in the trouble list use a very simple encoding scheme.
 */
#define prop2trbl(X)	((X) + A_MAX)
#define attr2trbl(Y)	(Y)
#define prop_trouble(X) trouble_list[trouble_count++] = prop2trbl(X)
#define attr_trouble(Y) trouble_list[trouble_count++] = attr2trbl(Y)

	trouble_count = unfixable_trbl = did_prop = did_attr = 0;

	/* collect property troubles */
	if (Sick) prop_trouble(SICK);
	if (Blinded > (long)u.ucreamed) prop_trouble(BLINDED);
	if (HHallucination) prop_trouble(HALLUC);
	if (Vomiting) prop_trouble(VOMITING);
	if (HConfusion) prop_trouble(CONFUSION);
	if (HStun) prop_trouble(STUNNED);

	unfixable_trbl = unfixable_trouble_count(TRUE);

	/* collect attribute troubles */
	for (idx = 0; idx < A_MAX; idx++) {
	    val_limit = AMAX(idx);
	    /* don't recover strength lost from hunger */
	    if (idx == A_STR && u.uhs >= WEAK) val_limit--;
	    /* don't recover more than 3 points worth of any attribute */
	    if (val_limit > ABASE(idx) + 3) val_limit = ABASE(idx) + 3;

	    for (val = ABASE(idx); val < val_limit; val++)
		attr_trouble(idx);
	    /* keep track of unfixed trouble, for message adjustment below */
	    unfixable_trbl += (AMAX(idx) - val_limit);
	}

	if (trouble_count == 0) {
	    pline(nothing_happens);
	    return;
	} else if (trouble_count > 1) {		/* shuffle */
	    int i, j, k;

	    for (i = trouble_count - 1; i > 0; i--)
		if ((j = rn2(i + 1)) != i) {
		    k = trouble_list[j];
		    trouble_list[j] = trouble_list[i];
		    trouble_list[i] = k;
		}
	}

	/*
	 *		Chances for number of troubles to be fixed
	 *		 0	1      2      3      4	    5	   6	  7
	 *   blessed:  22.7%  22.7%  19.5%  15.4%  10.7%   5.7%   2.6%	 0.8%
	 *  uncursed:  35.4%  35.4%  22.9%   6.3%    0	    0	   0	  0
	 */
	val_limit = rn2( d(2, (obj && obj->blessed) ? 4 : 2) );
	if (val_limit > trouble_count) val_limit = trouble_count;

	/* fix [some of] the troubles */
	for (val = 0; val < val_limit; val++) {
	    idx = trouble_list[val];

	    switch (idx) {
	    case prop2trbl(SICK):
		make_sick(0L, (char *) 0, TRUE, SICK_ALL);
		did_prop++;
		break;
	    case prop2trbl(BLINDED):
		make_blinded((long)u.ucreamed, TRUE);
		did_prop++;
		break;
	    case prop2trbl(HALLUC):
		(void) make_hallucinated(0L, TRUE, 0L);
		did_prop++;
		break;
	    case prop2trbl(VOMITING):
		make_vomiting(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(CONFUSION):
		make_confused(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(STUNNED):
		make_stunned(0L, TRUE);
		did_prop++;
		break;
	    default:
		if (idx >= 0 && idx < A_MAX) {
		    ABASE(idx) += 1;
		    did_attr++;
		} else
		    panic("use_unicorn_horn: ¿problemas graves? (%d)", idx);
		break;
	    }
	}

	if (did_attr)
	    pline("¡Esto hace que te sientas %s!",
		  (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
		  "genial" : "mejor");
	else if (!did_prop)
	    pline("Parece que no pasa nada.");

	flags.botl = (did_attr || did_prop);
#undef PROP_COUNT
#undef ATTR_COUNT
#undef prop2trbl
#undef attr2trbl
#undef prop_trouble
#undef attr_trouble
}

/*
 * Timer callback routine: turn figurine into monster
 */
void
fig_transform(arg, timeout)
genericptr_t arg;
long timeout;
{
	struct obj *figurine = (struct obj *)arg;
	struct monst *mtmp;
	coord cc;
	boolean cansee_spot, silent, okay_spot;
	boolean redraw = FALSE;
	char monnambuf[BUFSZ], carriedby[BUFSZ];

	if (!figurine) {
#ifdef DEBUG
	    pline("null figurine in fig_transform()");
#endif
	    return;
	}
	silent = (timeout != monstermoves); /* happened while away */
	okay_spot = get_obj_location(figurine, &cc.x, &cc.y, 0);
	if (figurine->where == OBJ_INVENT ||
	    figurine->where == OBJ_MINVENT)
		okay_spot = enexto(&cc, cc.x, cc.y,
				   &mons[figurine->corpsenm]);
	if (!okay_spot ||
	    !figurine_location_checks(figurine,&cc, TRUE)) {
		/* reset the timer to try again later */
		(void) start_timer((long)rnd(5000), TIMER_OBJECT,
				FIG_TRANSFORM, (genericptr_t)figurine);
		return;
	}

	cansee_spot = cansee(cc.x, cc.y);
	mtmp = make_familiar(figurine, cc.x, cc.y, TRUE);
	if (mtmp) {
	    Sprintf(monnambuf, "%s",an(m_monnam(mtmp)));
	    switch (figurine->where) {
		case OBJ_INVENT:
		    if (Blind)
			pline("¡Sientes %s %s de tu paquete!", something,
			    locomotion(mtmp->data,"cayendo"));
		    else
			pline("¡Ves %s %s de tu paquete!", something,
			    locomotion(mtmp->data,"cayendo"));
		    break;

		case OBJ_FLOOR:
		    if (cansee_spot && !silent) {
			pline("¡De repente, ves que una estatuilla se transforma en %s!",
				monnambuf);
			redraw = TRUE;	/* update figurine's map location */
		    }
		    break;

		case OBJ_MINVENT:
		    if (cansee_spot && !silent) {
			struct monst *mon;
			mon = figurine->ocarry;
			/* figurine carring monster might be invisible */
			if (canseemon(figurine->ocarry)) {
			    Sprintf(carriedby, "l paquete de %s", a_monnam(mon));
			}
			else if (is_pool(mon->mx, mon->my))
			    Strcpy(carriedby, "l agua");
			else
			    Strcpy(carriedby, "l aire");
			pline("¡Ves %s %s de%s!", monnambuf,
			    locomotion(mtmp->data, "caer"), carriedby);
		    }
		    break;
#if 0
		case OBJ_MIGRATING:
		    break;
#endif

		default:
		    impossible("estatuilla se volvió viva dónde? (%d)",
				(int)figurine->where);
		break;
	    }
	}
	/* free figurine now */
	obj_extract_self(figurine);
	obfree(figurine, (struct obj *)0);
	if (redraw) newsym(cc.x, cc.y);
}

STATIC_OVL boolean
figurine_location_checks(obj, cc, quietly)
struct obj *obj;
coord *cc;
boolean quietly;
{
	xchar x,y;

	if (carried(obj) && u.uswallow) {
		if (!quietly)
			pline("No tienes bastante espacio aquí.");
		return FALSE;
	}
	x = cc->x; y = cc->y;
	if (!isok(x,y)) {
		if (!quietly)
			No_puedes("poner la estatuilla ahí.");
		return FALSE;
	}
	if (IS_ROCK(levl[x][y].typ) &&
	    !(passes_walls(&mons[obj->corpsenm]) && may_passwall(x,y))) {
		if (!quietly)
			No_puedes("poner una estatuilla en %s!",
				  IS_TREE(levl[x][y].typ) ? "un árbol" : "piedra");
		return FALSE;
	}
	if (sobj_at(BOULDER,x,y) && !passes_walls(&mons[obj->corpsenm])
			&& !throws_rocks(&mons[obj->corpsenm])) {
		if (!quietly)
			No_puedes("poner la estatuilla sobre la roca.");
		return FALSE;
	}
	return TRUE;
}

STATIC_OVL void
use_figurine(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	xchar x, y;
	coord cc;
	
	if (u.uswallow) {
		/* can't activate a figurine while swallowed */
		if (!figurine_location_checks(obj, (coord *)0, FALSE))
			return;
	}
	if(!getdir((char *)0)) {
		flags.move = multi = 0;
		return;
	}
	x = u.ux + u.dx; y = u.uy + u.dy;
	cc.x = x; cc.y = y;
	/* Passing FALSE arg here will result in messages displayed */
	if (!figurine_location_checks(obj, &cc, FALSE)) return;
	pline("%s y se transforma.",
	      (u.dx||u.dy) ? "Pones la estatuilla a tu lado" :
	      (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ||
	       is_pool(cc.x, cc.y)) ?
		  "Liberas la estatuilla" :
	      (u.dz < 0 ?
		  "Lanzas la estatuilla al aire" :
		  "Pones la estatuilla en el suelo"));
	(void) make_familiar(obj, cc.x, cc.y, FALSE);
	(void) stop_timer(FIG_TRANSFORM, (genericptr_t)obj);
	useup(obj);
	*optr = 0;
}

static NEARDATA const char lubricables[] = { ALL_CLASSES, ALLOW_NONE, 0 };
static NEARDATA const char que_quitar_armadura_exterior[] =
			"que quitarte %s para poder engrasar tu %s.";

STATIC_OVL void
use_grease(obj)
struct obj *obj;
{
	struct obj *otmp;
	char buf[BUFSZ];

	if (Glib) {
	    pline("%s entre tus %s.", Tobjnam(obj, "se resbala"),
		  makeplural(body_part(FINGER)));
	    dropx(obj);
	    return;
	}

	if (obj->spe > 0) {
		if ((obj->cursed || Fumbling) && !rn2(2)) {
			consume_obj_charge(obj, TRUE);

			pline("%s entre tus %s.", Tobjnam(obj, "se resbala"),
			      makeplural(body_part(FINGER)));
			dropx(obj);
			return;
		}
		otmp = getobj(lubricables, "", "engrasar");
		if (!otmp) return;
		if ((otmp->owornmask & WORN_ARMOR) && uarmc) {
			Strcpy(buf, the(xname(uarmc)));
			Hay(que_quitar_armadura_exterior, buf, xname(otmp));
			return;
		}
#ifdef TOURIST
		if ((otmp->owornmask & WORN_SHIRT) && (uarmc || uarm)) {
			Strcpy(buf, uarmc ? the(xname(uarmc)) : "");
			if (uarmc && uarm) Strcat(buf, " y ");
			Strcat(buf, uarm ? the(xname(uarm)) : "");
			Hay(que_quitar_armadura_exterior, buf, xname(otmp));
			return;
		}
#endif
		consume_obj_charge(obj, TRUE);

		if (otmp != &zeroobj) {
			pline("Cubres %s con una gruesa capa de grasa.",
			    yname(otmp));
			otmp->greased = 1;
			if (obj->cursed && !nohands(youmonst.data)) {
			    incr_itimeout(&Glib, rnd(15));
			    pline("Algo de grasa se pega en tus %s.",
				makeplural(body_part(HAND)));
			}
		} else {
			Glib += rnd(15);
			pline("Cubres tus %s de grasa.",
			    makeplural(body_part(FINGER)));
		}
	} else {
	    pline("%s vací%c.",
		  Tobjnam(obj, obj->known? "está" : "parece estar"),
		  isfeminine(xname(obj))? 'a' : 'o');
	}
	update_inventory();
}

static struct trapinfo {
	struct obj *tobj;
	xchar tx, ty;
	int time_needed;
	boolean force_bungle;
} trapinfo;

void
reset_trapset()
{
	trapinfo.tobj = 0;
	trapinfo.force_bungle = 0;
}

/* touchstones - by Ken Arnold */
STATIC_OVL void
use_stone(tstone)
struct obj *tstone;
{
    struct obj *obj;
    boolean do_scratch;
    const char *streak_color, *choices;
    char stonebuf[QBUFSZ];
    static const char scritch[] = "\"scritch, scritch\"";
    static const char allowall[3] = { COIN_CLASS, ALL_CLASSES, 0 };
    static const char justgems[3] = { ALLOW_NONE, GEM_CLASS, 0 };
#ifndef GOLDOBJ
    struct obj goldobj;
#endif

    /* in case it was acquired while blinded */
    if (!Blind) tstone->dknown = 1;
    /* when the touchstone is fully known, don't bother listing extra
       junk as likely candidates for rubbing */
    choices = (tstone->otyp == TOUCHSTONE && tstone->dknown &&
		objects[TOUCHSTONE].oc_name_known) ? justgems : allowall;
    Sprintf(stonebuf, "frotar con la%s piedra%s",
	    plur(tstone->quan), plur(tstone->quan));
    if ((obj = getobj(choices, "", stonebuf)) == 0)
	return;
#ifndef GOLDOBJ
    if (obj->oclass == COIN_CLASS) {
	u.ugold += obj->quan;	/* keep botl up to date */
	goldobj = *obj;
	dealloc_obj(obj);
	obj = &goldobj;
    }
#endif

    if (obj == tstone && obj->quan == 1) {
	No_puedes("frotar %s consigo.", the(xname(obj)));
	return;
    }

    if (tstone->otyp == TOUCHSTONE && tstone->cursed &&
	    obj->oclass == GEM_CLASS && !is_graystone(obj) &&
	    !obj_resists(obj, 80, 100)) {
	if (Blind)
	    Sientes("algo estrellándose.");
	else if (Hallucination)
	    pline("Oh, vaya, mira que bonitos pedazos.");
	else {
            const char *que = xname(obj);
	    pline("Una quebradura rompe %s%s.",
		  (obj->quan > 1) ? feminize("uno de ", isfeminine(que)) : "",
                  the(que));
        }
#ifndef GOLDOBJ
     /* assert(obj != &goldobj); */
#endif
	useup(obj);
	return;
    }

    if (Blind) {
	pline(scritch);
	return;
    } else if (Hallucination) {
	pline("Oh vaya, hombre: ¡Fractales!");
	return;
    }

    do_scratch = FALSE;
    streak_color = 0;

    switch (obj->oclass) {
    case GEM_CLASS:	/* these have class-specific handling below */
    case RING_CLASS:
	if (tstone->otyp != TOUCHSTONE) {
	    do_scratch = TRUE;
	} else if (obj->oclass == GEM_CLASS && (tstone->blessed ||
		(!tstone->cursed &&
		    (Role_if(PM_ARCHEOLOGIST) || Race_if(PM_GNOME))))) {
	    makeknown(TOUCHSTONE);
	    makeknown(obj->otyp);
	    prinv((char *)0, obj, 0L);
	    return;
	} else {
	    /* either a ring or the touchstone was not effective */
	    if (objects[obj->otyp].oc_material == GLASS) {
		do_scratch = TRUE;
		break;
	    }
	}
	streak_color = c_obj_colors[objects[obj->otyp].oc_color];
	break;		/* gem or ring */

    default:
	switch (objects[obj->otyp].oc_material) {
	case CLOTH:
	    pline("%s un poco más pulid%c ahora.", Tobjnam(tstone, "parece"),
		  isfeminine(xname(tstone))? 'a' : 'o');
	    return;
	case LIQUID:
	    if (!obj->known)		/* note: not "whetstone" */
		/* LENGUA:  This pun doesn't work in Spanish */
		/*You("must think this is a wetstone, do you?")*/;
	    else
		pline("%s un poco más mojad%c ahora.", Tobjnam(tstone, "está"),
		      isfeminine(xname(tstone))? 'a' : 'o');
	    return;
	case WAX:
	    streak_color = "encerado";
	    break;		/* okay even if not touchstone */
	case WOOD:
	    streak_color = "de madera";
	    break;		/* okay even if not touchstone */
	case GOLD:
	    do_scratch = TRUE;	/* scratching and streaks */
	    streak_color = "dorado";
	    break;
	case SILVER:
	    do_scratch = TRUE;	/* scratching and streaks */
	    streak_color = "plateado";
	    break;
	default:
	    /* Objects passing the is_flimsy() test will not
	       scratch a stone.  They will leave streaks on
	       non-touchstones and touchstones alike. */
	    if (is_flimsy(obj))
		streak_color = c_obj_colors[objects[obj->otyp].oc_color];
	    else
		do_scratch = (tstone->otyp != TOUCHSTONE);
	    break;
	}
	break;		/* default oclass */
    }

    Sprintf(stonebuf, "la%s piedra%s", plur(tstone->quan), plur(tstone->quan));
    if (streak_color)
	streak_color = makeplural(feminize(streak_color, TRUE));
    if (do_scratch)
	pline("Haces marcas%s%s en %s.",
	      streak_color ? " " : "",
	      streak_color ? streak_color : (const char *)"",
	      stonebuf);
    else if (streak_color)
	pline("Ves rayas %s en %s.", streak_color, stonebuf);
    else
	pline(scritch);
    return;
}

/* Place a landmine/bear trap.  Helge Hafting */
STATIC_OVL void
use_trap(otmp)
struct obj *otmp;
{
	int ttyp, tmp;
	const char *what = (char *)0;
	char buf[BUFSZ];
	const char *occutext = "armar la trampa";

	if (nohands(youmonst.data))
	    what = "sin manos";
	else if (Stunned)
	    what = feminize("estando aturdido", poly_gender()==1);
	else if (u.uswallow) {
	    what = is_animal(u.ustuck->data) ? "estando tragado" :
			"estando sumergido";
	    what = feminize(what, poly_gender()==1);
	}
	else if (Underwater)
	    what = "bajo el agua";
	else if (Levitation)
	    what = "levitando";
	else if (is_pool(u.ux, u.uy))
	    what = "en el agua";
	else if (is_lava(u.ux, u.uy))
	    what = "en la lava";
	else if (On_stairs(u.ux, u.uy))
	    what = (u.ux == xdnladder || u.ux == xupladder) ?
			"en la escalerilla" : "en la escalera";
	else if (IS_FURNITURE(levl[u.ux][u.uy].typ) ||
		IS_ROCK(levl[u.ux][u.uy].typ) ||
		closed_door(u.ux, u.uy) || t_at(u.ux, u.uy))
	    what = "aquí";
	if (what) {
	    No_puedes("armar una trampa %s!",what);
	    reset_trapset();
	    return;
	}
	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	if (otmp == trapinfo.tobj &&
		u.ux == trapinfo.tx && u.uy == trapinfo.ty) {
	    shk_your(buf, otmp, (char *)0);
	    pline("Continúas armando %s.", buf);
	    set_occupation(set_trap, occutext, 0);
	    return;
	}
	trapinfo.tobj = otmp;
	trapinfo.tx = u.ux,  trapinfo.ty = u.uy;
	tmp = ACURR(A_DEX);
	trapinfo.time_needed = (tmp > 17) ? 2 : (tmp > 12) ? 3 :
				(tmp > 7) ? 4 : 5;
	if (Blind) trapinfo.time_needed *= 2;
	tmp = ACURR(A_STR);
	if (ttyp == BEAR_TRAP && tmp < 18)
	    trapinfo.time_needed += (tmp > 12) ? 1 : (tmp > 7) ? 2 : 4;
	/*[fumbling and/or confusion and/or cursed object check(s)
	   should be incorporated here instead of in set_trap]*/
#ifdef STEED
	if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
	    boolean chance;

	    if (Fumbling || otmp->cursed) chance = (rnl(10) > 3);
	    else  chance = (rnl(10) > 5);
	    pline("No tienes mucha destreza para alcanzar desde %s.",
		mon_nam(u.usteed));
	    Sprintf(buf, "¿Continuar tu intento para armar %s?",
		the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
	    if(yn(buf) == 'y') {
		if (chance) {
			switch(ttyp) {
			    case LANDMINE:	/* set it off */
			    	trapinfo.time_needed = 0;
			    	trapinfo.force_bungle = TRUE;
				break;
			    case BEAR_TRAP:	/* drop it without arming it */
				reset_trapset();
				pline("¡Dejas caer %s!",
			  the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
				dropx(otmp);
				return;
			}
		}
	    } else {
	    	reset_trapset();
		return;
	    }
	}
#endif
	shk_your(buf, otmp, (char *)0);
	pline("Empiezas a armar %s.", buf);
	set_occupation(set_trap, occutext, 0);
	return;
}

STATIC_PTR
int
set_trap()
{
	struct obj *otmp = trapinfo.tobj;
	struct trap *ttmp;
	int ttyp;

	if (!otmp || !carried(otmp) ||
		u.ux != trapinfo.tx || u.uy != trapinfo.ty) {
	    /* ?? */
	    reset_trapset();
	    return 0;
	}

	if (--trapinfo.time_needed > 0) return 1;	/* still busy */

	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	ttmp = maketrap(u.ux, u.uy, ttyp);
	if (ttmp) {
	    ttmp->tseen = 1;
	    ttmp->madeby_u = 1;
	    newsym(u.ux, u.uy); /* if our hero happens to be invisible */
	    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
		add_damage(u.ux, u.uy, 0L);		/* schedule removal */
	    }
	    if (!trapinfo.force_bungle)
		pline("Acabas de armar %s.",
			the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
	    if (((otmp->cursed || Fumbling) && (rnl(10) > 5)) || trapinfo.force_bungle)
		dotrap(ttmp,
			(unsigned)(trapinfo.force_bungle ? FORCEBUNGLE : 0));
	} else {
	    /* this shouldn't happen */
	    pline("Tu intento de armar una trampa falla.");
	}
	useup(otmp);
	reset_trapset();
	return 0;
}

STATIC_OVL int
use_whip(obj)
struct obj *obj;
{
    char buf[BUFSZ];
    struct monst *mtmp;
    struct obj *otmp;
    int rx, ry, proficient, res = 0;
    const char *msg_slipsfree = "El látigo se escurre.";
    const char *msg_snap = "¡Snap!";

    if (obj != uwep) {
	if (!wield_tool(obj, "latiguear", "latigueas")) return 0;
	else res = 1;
    }
    if (!getdir((char *)0)) return res;

    if (Stunned || (Confusion && !rn2(5))) confdir();
    rx = u.ux + u.dx;
    ry = u.uy + u.dy;
    mtmp = m_at(rx, ry);

    /* fake some proficiency checks */
    proficient = 0;
    if (Role_if(PM_ARCHEOLOGIST)) ++proficient;
    if (ACURR(A_DEX) < 6) proficient--;
    else if (ACURR(A_DEX) >= 14) proficient += (ACURR(A_DEX) - 14);
    if (Fumbling) --proficient;
    if (proficient > 3) proficient = 3;
    if (proficient < 0) proficient = 0;

    if (u.uswallow && attack(u.ustuck)) {
	pline("No hay bastante espacio para chasquear tu látigo.");

    } else if (Underwater) {
	Hay("demasiada resistencia para chasquear tu látigo.");

    } else if (u.dz < 0) {
	pline("Quitas un insecto %s.",del(ceiling(u.ux,u.uy)));

    } else if ((!u.dx && !u.dy) || (u.dz > 0)) {
	int dam;

#ifdef STEED
	/* Sometimes you hit your steed by mistake */
	if (u.usteed && !rn2(proficient + 2)) {
	    pline("¡Azotas %s!", al(mon_nam(u.usteed)));
	    kick_steed();
	    return 1;
	}
#endif
	if (Levitation
#ifdef STEED
			|| u.usteed
#endif
		) {
	    /* Have a shot at snaring something on the floor */
	    otmp = level.objects[u.ux][u.uy];
	    if (otmp && otmp->otyp == CORPSE && otmp->corpsenm == PM_HORSE) {
		/* LENGUA:  Does this pun work in Spanish? */
		pline("¿Por qué azotar a un caballo muerto?");
		return 1;
	    }
	    if (otmp && proficient) {
		pline("Envuelves tu látigo alrededor de %s en %s.",
		    an(singular(otmp, xname)), surface(u.ux, u.uy));
		if (rnl(6) || pickup_object(otmp, 1L, TRUE) < 1)
		    pline(msg_slipsfree);
		return 1;
	    }
	}
	dam = rnd(2) + dbon() + obj->spe;
	if (dam <= 0) dam = 1;
	pline("Golpeas tu %s con tu látigo.", body_part(FOOT));
	losehp(dam, "se mató con su propio látigo", NO_KILLER_PREFIX);
	flags.botl = 1;
	return 1;

    } else if ((Fumbling || Glib) && !rn2(5)) {
	pline("El látigo se escurre de tu %s.", body_part(HAND));
	dropx(obj);

    } else if (u.utrap && u.utraptype == TT_PIT) {
	/*
	 *     Assumptions:
	 *
	 *	if you're in a pit
	 *		- you are attempting to get out of the pit
	 *		- or, if you are applying it towards a small
	 *		  monster then it is assumed that you are
	 *		  trying to hit it.
	 *	else if the monster is wielding a weapon
	 *		- you are attempting to disarm a monster
	 *	else
	 *		- you are attempting to hit the monster
	 *
	 *	if you're confused (and thus off the mark)
	 *		- you only end up hitting.
	 *
	 */
	const char *wrapped_what = (char *)0;

	if (mtmp) {
	    if (bigmonst(mtmp->data)) {
		wrapped_what = strcpy(buf, mon_nam(mtmp));
	    } else if (proficient) {
		if (attack(mtmp)) return 1;
		else pline(msg_snap);
	    }
	}
	if (!wrapped_what) {
	    if (IS_FURNITURE(levl[rx][ry].typ))
		wrapped_what = something;
	    else if (sobj_at(BOULDER, rx, ry))
		wrapped_what = "una roca";
	}
	if (wrapped_what) {
	    coord cc;

	    cc.x = rx; cc.y = ry;
	    pline("Envuelves tu látigo alrededor %s.", del(wrapped_what));
	    if (proficient && rn2(proficient + 2)) {
		if (!mtmp || enexto(&cc, rx, ry, youmonst.data)) {
		    pline("¡Te sacas del hoyo!");
		    teleds(cc.x, cc.y, TRUE);
		    u.utrap = 0;
		    vision_full_recalc = 1;
		}
	    } else {
		pline(msg_slipsfree);
	    }
	    if (mtmp) wakeup(mtmp);
	} else pline(msg_snap);

    } else if (mtmp) {
	if (!canspotmon(mtmp) &&
		!glyph_is_invisible(levl[rx][ry].glyph)) {
	   Hay("un monstruo ahí que no podías ver.");
	   map_invisible(rx, ry);
	}
	otmp = MON_WEP(mtmp);	/* can be null */
	if (otmp) {
	    char onambuf[BUFSZ];
	    const char *mon_hand;
	    boolean gotit = proficient && (!Fumbling || !rn2(10));

	    Strcpy(onambuf, cxname(otmp));
	    if (gotit) {
		mon_hand = mbodypart(mtmp, HAND);
		if (bimanual(otmp)) mon_hand = makeplural(mon_hand);
	    } else
		mon_hand = 0;	/* lint suppression */

	    pline("Envuelves tu látigo alrededor de%s %s %s.",
		  isfeminine(onambuf)? " la" : "l", onambuf, del(mon_nam(mtmp)));
	    if (gotit && otmp->cursed) {
		pline("%sEstá%s soldad%c%s a su%s %s%c",
		      !otmp->bknown ? "¡" : "",
		      (otmp->quan == 1L) ? "" : "n",
		      isfeminine(onambuf)? 'a' : 'o',
		      plur(otmp->quan),
		      (bimanual(otmp)) ? "" : "s",
		      mon_hand,
		      !otmp->bknown ? '!' : '.');
		otmp->bknown = 1;
		gotit = FALSE;	/* can't pull it free */
	    }
	    if (gotit) {
		obj_extract_self(otmp);
		possibly_unwield(mtmp, FALSE);
		setmnotwielded(mtmp,otmp);

		switch (rn2(proficient + 1)) {
		case 2:
		    /* to floor near you */
		    pline("¡Tiras %s %s %s!",
			  the(onambuf),
			  del(mon_nam(mtmp)),
			  al(surface(u.ux, u.uy)));
		    place_object(otmp, u.ux, u.uy);
		    stackobj(otmp);
		    break;
		case 3:
		    /* right to you */
#if 0
		    if (!rn2(25)) {
			/* proficient with whip, but maybe not
			   so proficient at catching weapons */
			int hitu, hitvalu;

			hitvalu = 8 + otmp->spe;
			hitu = thitu(hitvalu,
				     dmgval(otmp, &youmonst),
				     otmp, (char *)0);
			if (hitu) {
			    pline("¡%s te golpea al tratar de agarrarl%c!",
				  The(onambuf),
				  isfeminine(onambuf)? 'a' : 'o');
			}
			place_object(otmp, u.ux, u.uy);
			stackobj(otmp);
			break;
		    }
#endif /* 0 */
		    /* right into your inventory */
		    pline("¡Agarras %s %s!", onambuf, del(mon_nam(mtmp)));
		    if (otmp->otyp == CORPSE &&
			    touch_petrifies(&mons[otmp->corpsenm]) &&
			    !uarmg && !Stone_resistance &&
			    !(poly_when_stoned(youmonst.data) &&
				polymon(PM_STONE_GOLEM))) {
			char kbuf[BUFSZ];

			Sprintf(kbuf, "cadáver de %s",
			        mons[otmp->corpsenm].mname);
			pline("Agarrar el %s es un error fatal.", kbuf);
			instapetrify(kbuf);
		    }
		    otmp = hold_another_object(otmp, "¡Dejas caer %s!",
					       doname(otmp), (const char *)0);
		    break;
		default:
		    /* to floor beneath mon */
		    pline("¡Tiras %s de%s %s %s!", the(onambuf),
			isfeminine(mon_hand)? " la" : "l", mon_hand,
			del(mon_nam(mtmp)));
		    obj_no_longer_held(otmp);
		    place_object(otmp, mtmp->mx, mtmp->my);
		    stackobj(otmp);
		    break;
		}
	    } else {
		pline(msg_slipsfree);
	    }
	    wakeup(mtmp);
	} else {
	    if (mtmp->m_ap_type &&
		!Protection_from_shape_changers && !sensemon(mtmp))
		stumble_onto_mimic(mtmp);
	    else pline("Chasqueas tu látigo hacia %s.", mon_nam(mtmp));
	    if (proficient) {
		if (attack(mtmp)) return 1;
		else pline(msg_snap);
	    }
	}

    } else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
	    /* it must be air -- water checked above */
	    pline("Chasqueas tu látigo por el aire.");

    } else {
	pline(msg_snap);

    }
    return 1;
}


static const char
	not_enough_room[] = "No hay bastante espacio aquí para usarlo.",
	where_to_hit[] = "¿Dónde quieres golpear?",
	cant_see_spot[] = "No golpearás nada si no puedes ver ese lugar.",
	cant_reach[] = "No puedes alcanzar este lugar desde aquí.";

/* Distance attacks by pole-weapons */
STATIC_OVL int
use_pole (obj)
	struct obj *obj;
{
	int res = 0, typ, max_range = 4, min_range = 4;
	coord cc;
	struct monst *mtmp;


	/* Are you allowed to use the pole? */
	if (u.uswallow) {
	    pline(not_enough_room);
	    return (0);
	}
	if (obj != uwep) {
	    if (!wield_tool(obj, "esgrimir", "esgrimes")) return(0);
	    else res = 1;
	}
     /* assert(obj == uwep); */

	/* Prompt for a location */
	pline(where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "el lugar donde golpear") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
	else if (P_SKILL(typ) == P_SKILLED) max_range = 5;
	else max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
	    pline("¡Demasiado lejos!");
	    return (res);
	} else if (distu(cc.x, cc.y) < min_range) {
	    pline("¡Demasiado cerca!");
	    return (res);
	} else if (!cansee(cc.x, cc.y) &&
		   ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0 ||
		    !canseemon(mtmp))) {
	    pline(cant_see_spot);
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    pline(cant_reach);
	    return res;
	}

	/* Attack the monster there */
	if ((mtmp = m_at(cc.x, cc.y)) != (struct monst *)0) {
	    int oldhp = mtmp->mhp;

	    bhitpos = cc;
	    check_caitiff(mtmp);
	    (void) thitmonst(mtmp, uwep);
	    /* check the monster's HP because thitmonst() doesn't return
	     * an indication of whether it hit.  Not perfect (what if it's a
	     * non-silver weapon on a shade?)
	     */
	    if (mtmp->mhp < oldhp)
		u.uconduct.weaphit++;
	} else
	    /* Now you know that nothing is there... */
	    pline(nothing_happens);
	return (1);
}

STATIC_OVL int
use_cream_pie(obj)
struct obj *obj;
{
	boolean wasblind = Blind;
	boolean wascreamed = u.ucreamed;
	boolean several = FALSE;

	if (obj->quan > 1L) {
		several = TRUE;
		obj = splitobj(obj, 1L);
	}
	if (Hallucination)
		/* LENGUA:  facial == ? */
		/* Andrés: Supongo que es correcto hablar de 'un facial'. */
		pline("Te das un facial.");
	else {
		char *que = the(xname(obj));
		pline("Metes %s en %s%s.", the(body_part(FACE)),
			several ? isfeminine(que)? "una de " : "uno de "
				: "",
			several ? makeplural(que) : que);
	}
	if(can_blnd((struct monst*)0, &youmonst, AT_WEAP, obj)) {
		const char *cara = body_part(FACE);
		int blindinc = rnd(25);
		u.ucreamed += blindinc;
		make_blinded(Blinded + (long)blindinc, FALSE);
		if (!Blind || (Blind && wasblind))
			pline("Hay %sbaba pegajosa por tod%c tu %s.",
				wascreamed ? "más " : "",
				isfeminine(cara)? 'a' : 'o', cara);
		else /* Blind  && !wasblind */
			No_puedes("ver a través de toda la baba pegajosa sobre tu %s",
				cara);
	}
	if (obj->unpaid) {
		verbalize("¡Usted lo usa, usted lo compra!");
		bill_dummy_object(obj);
	}
	obj_extract_self(obj);
	delobj(obj);
	return(0);
}

STATIC_OVL int
use_grapple (obj)
	struct obj *obj;
{
	int res = 0, typ, max_range = 4, tohit;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;

	/* Are you allowed to use the hook? */
	if (u.uswallow) {
	    pline(not_enough_room);
	    return (0);
	}
	if (obj != uwep) {
	    if (!wield_tool(obj, "tirar", "tiras")) return(0);
	    else res = 1;
	}
     /* assert(obj == uwep); */

	/* Prompt for a location */
	pline(where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "el lugar donde golpear") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
	else if (P_SKILL(typ) == P_SKILLED) max_range = 5;
	else max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
	    pline("¡Demasiado lejos!");
	    return (res);
	} else if (!cansee(cc.x, cc.y)) {
	    pline(cant_see_spot);
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    pline(cant_reach);
	    return res;
	}

	/* What do you want to hit? */
	tohit = rn2(5);
	if (typ != P_NONE && P_SKILL(typ) >= P_SKILLED) {
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    anything any;
	    char buf[BUFSZ];
	    menu_item *selected;

	    any.a_void = 0;	/* set all bits to zero */
	    any.a_int = 1;	/* use index+1 (cant use 0) as identifier */
	    start_menu(tmpwin);
	    any.a_int++;
	    Sprintf(buf, "un objecto sobre %s", surface(cc.x, cc.y));
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	    any.a_int++;
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			"un monstruo", MENU_UNSELECTED);
	    any.a_int++;
	    Sprintf(buf, "%s", surface(cc.x, cc.y));
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	    end_menu(tmpwin, "¿Dirigir a qué?");
	    tohit = rn2(4);
	    if (select_menu(tmpwin, PICK_ONE, &selected) > 0 &&
			rn2(P_SKILL(typ) > P_SKILLED ? 20 : 2))
		tohit = selected[0].item.a_int - 1;
	    free((genericptr_t)selected);
	    destroy_nhwindow(tmpwin);
	}

	/* What did you hit? */
	switch (tohit) {
	case 0:	/* Trap */
	    /* FIXME -- untrap needs to deal with non-adjacent traps */
	    break;
	case 1:	/* Object */
	    if ((otmp = level.objects[cc.x][cc.y]) != 0) {
		pline("¡Arrancas un objeto %s!", del(surface(cc.x, cc.y)));
		(void) pickup_object(otmp, 1L, FALSE);
		/* If pickup fails, leave it alone */
		newsym(cc.x, cc.y);
		return (1);
	    }
	    break;
	case 2:	/* Monster */
	    if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0) break;
	    if (verysmall(mtmp->data) && !rn2(4) &&
			enexto(&cc, u.ux, u.uy, (struct permonst *)0)) {
		pline("¡%s!", mon_nam_acc(mtmp, "Tiras"));
		mtmp->mundetected = 0;
		rloc_to(mtmp, cc.x, cc.y);
		return (1);
	    } else if ((!bigmonst(mtmp->data) && !strongmonst(mtmp->data)) ||
		       rn2(4)) {
		(void) thitmonst(mtmp, uwep);
		return (1);
	    }
	    /* FALL THROUGH */
	case 3:	/* Surface */
	    if (IS_AIR(levl[cc.x][cc.y].typ) || is_pool(cc.x, cc.y))
		pline("El gancho corta través %s.", del(surface(cc.x, cc.y)));
	    else {
		pline("¡Te tiras %s!", al(surface(cc.x, cc.y)));
		hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, FALSE);
		spoteffects(TRUE);
	    }
	    return (1);
	default:	/* Yourself (oops!) */
	    if (P_SKILL(typ) <= P_BASIC) {
		pline("¡Te enganchas!");
		/* LENGUA:  English, "a grappling hook" */
		losehp(rn1(10,10), "un gancho de aferrar", KILLED_BY);
		return (1);
	    }
	    break;
	}
	pline(nothing_happens);
	return (1);
}


#define BY_OBJECT	((struct monst *)0)

/* return 1 if the wand is broken, hence some time elapsed */
STATIC_OVL int
do_break_wand(obj)
    struct obj *obj;
{
    static const char nothing_else_happens[] = "Pero no pasa nada más...";
    register int i, x, y;
    register struct monst *mon;
    int dmg, damage;
    boolean affects_objects;
    boolean shop_damage = FALSE;
    int expltype = EXPL_MAGICAL;
    char confirm[QBUFSZ], the_wand[BUFSZ];

    Strcpy(the_wand, yname(obj));
    Sprintf(confirm, "¿Estás muy segur%c que quieres romper %s?",
	    poly_gender()==1? 'a' : 'o',
	safe_qbuf("", sizeof("¿Estás muy seguro que quieres romper ?"),
				the_wand, ysimple_name(obj), "la varita"));
    if (yn(confirm) == 'n' ) return 0;

    if (nohands(youmonst.data)) {
	No_puedes("romper %s sin manos!", the_wand);
	return 0;
    } else if (ACURR(A_STR) < 10) {
	pline("¡No tienes fuerza para romper %s!", the_wand);
	return 0;
    }
    pline("¡Levantando %s por encima de tu %s, la rompes en dos pedazos!",
	  the_wand, body_part(HEAD));

    /* [ALI] Do this first so that wand is removed from bill. Otherwise,
     * the freeinv() below also hides it from setpaid() which causes problems.
     */
    if (obj->unpaid) {
	check_unpaid(obj);		/* Extra charge for use */
	bill_dummy_object(obj);
    }

    current_wand = obj;		/* destroy_item might reset this */
    freeinv(obj);		/* hide it from destroy_item instead... */
    setnotworn(obj);		/* so we need to do this ourselves */

    if (obj->spe <= 0) {
	pline(nothing_else_happens);
	goto discard_broken_wand;
    }
    obj->ox = u.ux;
    obj->oy = u.uy;
    dmg = obj->spe * 4;
    affects_objects = FALSE;

    switch (obj->otyp) {
    case WAN_WISHING:
    case WAN_NOTHING:
    case WAN_LOCKING:
    case WAN_PROBING:
    case WAN_ENLIGHTENMENT:
    case WAN_OPENING:
    case WAN_SECRET_DOOR_DETECTION:
	pline(nothing_else_happens);
	goto discard_broken_wand;
    case WAN_DEATH:
    case WAN_LIGHTNING:
	dmg *= 4;
	goto wanexpl;
    case WAN_FIRE:
	expltype = EXPL_FIERY;
    case WAN_COLD:
	if (expltype == EXPL_MAGICAL) expltype = EXPL_FROSTY;
	dmg *= 2;
    case WAN_MAGIC_MISSILE:
    wanexpl:
	explode(u.ux, u.uy,
		(obj->otyp - WAN_MAGIC_MISSILE), dmg, WAND_CLASS, expltype);
	makeknown(obj->otyp);	/* explode described the effect */
	goto discard_broken_wand;
    case WAN_STRIKING:
	/* we want this before the explosion instead of at the very end */
	pline("¡Un campo de fuerza te aplasta!");
	dmg = d(1 + obj->spe,6);	/* normally 2d12 */
    case WAN_CANCELLATION:
    case WAN_POLYMORPH:
    case WAN_TELEPORTATION:
    case WAN_UNDEAD_TURNING:
	affects_objects = TRUE;
	break;
    default:
	break;
    }

    /* magical explosion and its visual effect occur before specific effects */
    explode(obj->ox, obj->oy, 0, rnd(dmg), WAND_CLASS, EXPL_MAGICAL);

    /* this makes it hit us last, so that we can see the action first */
    for (i = 0; i <= 8; i++) {
	bhitpos.x = x = obj->ox + xdir[i];
	bhitpos.y = y = obj->oy + ydir[i];
	if (!isok(x,y)) continue;

	if (obj->otyp == WAN_DIGGING) {
	    if(dig_check(BY_OBJECT, FALSE, x, y)) {
		if (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)) {
		    /* normally, pits and holes don't anger guards, but they
		     * do if it's a wall or door that's being dug */
		    watch_dig((struct monst *)0, x, y, TRUE);
		    if (*in_rooms(x,y,SHOPBASE)) shop_damage = TRUE;
		}		    
		digactualhole(x, y, BY_OBJECT,
			      (rn2(obj->spe) < 3 || !Can_dig_down(&u.uz)) ?
			       PIT : HOLE);
	    }
	    continue;
	} else if(obj->otyp == WAN_CREATE_MONSTER) {
	    /* u.ux,u.uy creates it near you--x,y might create it in rock */
	    (void) makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS);
	    continue;
	} else {
	    if (x == u.ux && y == u.uy) {
		/* teleport objects first to avoid race with tele control and
		   autopickup.  Other wand/object effects handled after
		   possible wand damage is assessed */
		if (obj->otyp == WAN_TELEPORTATION &&
		    affects_objects && level.objects[x][y]) {
		    (void) bhitpile(obj, bhito, x, y);
		    if (flags.botl) bot();		/* potion effects */
		}
		damage = zapyourself(obj, FALSE);
		if (damage)
		    losehp(damage, "se mató rompiendo una varita",
			   NO_KILLER_PREFIX);
		if (flags.botl) bot();		/* blindness */
	    } else if ((mon = m_at(x, y)) != 0) {
		(void) bhitm(mon, obj);
	     /* if (flags.botl) bot(); */
	    }
	    if (affects_objects && level.objects[x][y]) {
		(void) bhitpile(obj, bhito, x, y);
		if (flags.botl) bot();		/* potion effects */
	    }
	}
    }

    /* Note: if player fell thru, this call is a no-op.
       Damage is handled in digactualhole in that case */
    if (shop_damage) pay_for_damage("cavar en", FALSE);

    if (obj->otyp == WAN_LIGHT)
	litroom(TRUE, obj);	/* only needs to be done once */

 discard_broken_wand:
    obj = current_wand;		/* [see dozap() and destroy_item()] */
    current_wand = 0;
    if (obj)
	delobj(obj);
    nomul(0);
    return 1;
}

STATIC_OVL boolean
uhave_graystone()
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(is_graystone(otmp))
			return TRUE;
	return FALSE;
}

STATIC_OVL void
add_class(cl, class)
char *cl;
char class;
{
	char tmp[2];
	tmp[0] = class;
	tmp[1] = '\0';
	Strcat(cl, tmp);
}

int
doapply()
{
	struct obj *obj;
	register int res = 1;
	char class_list[MAXOCLASSES+2];

	if(check_capacity((char *)0)) return (0);

	if (carrying(POT_OIL) || uhave_graystone())
		Strcpy(class_list, tools_too);
	else
		Strcpy(class_list, tools);
	if (carrying(CREAM_PIE) || carrying(EUCALYPTUS_LEAF))
		add_class(class_list, FOOD_CLASS);

	obj = getobj(class_list, "", "usar o aplicar");
	if(!obj) return 0;

	if (obj->oartifact && !touch_artifact(obj, &youmonst))
	    return 1;	/* evading your grasp costs a turn; just be
			   grateful that you don't drop it as well */

	if (obj->oclass == WAND_CLASS)
	    return do_break_wand(obj);

	switch(obj->otyp){
	case BLINDFOLD:
	case LENSES:
		if (obj == ublindf) {
		    if (!cursed(obj)) Blindf_off(obj);
		} else if (!ublindf)
		    Blindf_on(obj);
		else pline("Ya %s.",
			ublindf->otyp == TOWEL ?     feminize("estás cubierto por una toalla", poly_gender()==1) :
			ublindf->otyp == BLINDFOLD ? "tienes los ojos vendados" :
						     "estás llevando anteojos");
		break;
	case CREAM_PIE:
		res = use_cream_pie(obj);
		break;
	case BULLWHIP:
		res = use_whip(obj);
		break;
	case GRAPPLING_HOOK:
		res = use_grapple(obj);
		break;
	case LARGE_BOX:
	case CHEST:
	case ICE_BOX:
	case SACK:
	case BAG_OF_HOLDING:
	case OILSKIN_SACK:
		res = use_container(obj, 1);
		break;
	case BAG_OF_TRICKS:
		bagotricks(obj);
		break;
	case CAN_OF_GREASE:
		use_grease(obj);
		break;
	case LOCK_PICK:
#ifdef TOURIST
	case CREDIT_CARD:
#endif
	case SKELETON_KEY:
		(void) pick_lock(obj);
		break;
	case PICK_AXE:
	case DWARVISH_MATTOCK:
		res = use_pick_axe(obj);
		break;
	case TINNING_KIT:
		use_tinning_kit(obj);
		break;
	case LEASH:
		use_leash(obj);
		break;
#ifdef STEED
	case SADDLE:
		res = use_saddle(obj);
		break;
#endif
	case MAGIC_WHISTLE:
		use_magic_whistle(obj);
		break;
	case TIN_WHISTLE:
		use_whistle(obj);
		break;
	case EUCALYPTUS_LEAF:
		/* MRKR: Every Australian knows that a gum leaf makes an */
		/*	 excellent whistle, especially if your pet is a  */
		/*	 tame kangaroo named Skippy.			 */
		if (obj->blessed) {
		    use_magic_whistle(obj);
		    /* sometimes the blessing will be worn off */
		    if (!rn2(49)) {
			if (!Blind) {
			    char buf[BUFSZ];

			    pline("%s %s %s.", Shk_Your(buf, obj, (char *)0),
				  aobjnam(obj, "alumbra"), hcolor("marrón"));
			    obj->bknown = 1;
			}
			unbless(obj);
		    }
		} else {
		    use_whistle(obj);
		}
		break;
	case STETHOSCOPE:
		res = use_stethoscope(obj);
		break;
	case MIRROR:
		res = use_mirror(obj);
		break;
	case BELL:
	case BELL_OF_OPENING:
		use_bell(&obj);
		break;
	case CANDELABRUM_OF_INVOCATION:
		use_candelabrum(obj);
		break;
	case WAX_CANDLE:
	case TALLOW_CANDLE:
		use_candle(&obj);
		break;
	case OIL_LAMP:
	case MAGIC_LAMP:
	case BRASS_LANTERN:
		use_lamp(obj);
		break;
	case POT_OIL:
		light_cocktail(obj);
		break;
#ifdef TOURIST
	case EXPENSIVE_CAMERA:
		res = use_camera(obj);
		break;
#endif
	case TOWEL:
		res = use_towel(obj);
		break;
	case CRYSTAL_BALL:
		use_crystal_ball(obj);
		break;
	case MAGIC_MARKER:
		res = dowrite(obj);
		break;
	case TIN_OPENER:
		if(!carrying(TIN)) {
			pline("No tienes una lata que abrir.");
			goto xit;
		}
		No_puedes("abrir una lata sin comer o desechar sus contenidas.");
		if(flags.verbose)
			pline("Si quieres comerla, usa el comando 'e'.");
		if(obj != uwep)
	pline("Será mucho más fácil abrir la lata si blandes el abrelatas.");
		goto xit;

	case FIGURINE:
		use_figurine(&obj);
		break;
	case UNICORN_HORN:
		use_unicorn_horn(obj);
		break;
	case WOODEN_FLUTE:
	case MAGIC_FLUTE:
	case TOOLED_HORN:
	case FROST_HORN:
	case FIRE_HORN:
	case WOODEN_HARP:
	case MAGIC_HARP:
	case BUGLE:
	case LEATHER_DRUM:
	case DRUM_OF_EARTHQUAKE:
		res = do_play_instrument(obj);
		break;
	case HORN_OF_PLENTY:	/* not a musical instrument */
		if (obj->spe > 0) {
		    struct obj *otmp;
		    const char *what;

		    consume_obj_charge(obj, TRUE);
		    if (!rn2(13)) {
			otmp = mkobj(POTION_CLASS, FALSE);
			if (objects[otmp->otyp].oc_magic) do {
			    otmp->otyp = rnd_class(POT_BOOZE, POT_WATER);
			} while (otmp->otyp == POT_SICKNESS);
			what = "Una poción";
		    } else {
			otmp = mkobj(FOOD_CLASS, FALSE);
			if (otmp->otyp == FOOD_RATION && !rn2(7))
			    otmp->otyp = LUMP_OF_ROYAL_JELLY;
			what = "Algo de comida";
		    }
		    pline("%s se derrama.", what);
		    otmp->blessed = obj->blessed;
		    otmp->cursed = obj->cursed;
		    otmp->owt = weight(otmp);
		    otmp = hold_another_object(otmp, u.uswallow ?
				       "¡Oops!  ¡%s fuera de tu alcance!" :
					(Is_airlevel(&u.uz) ||
					 Is_waterlevel(&u.uz) ||
					 levl[u.ux][u.uy].typ < IRONBARS ||
					 levl[u.ux][u.uy].typ >= ICE) ?
					       "¡Oops!  ¡%s lejos de ti!" :
					       "¡Oops!  ¡%s al suelo!",
					       The(aobjnam(otmp, "se resbala")),
					       (const char *)0);
		    makeknown(HORN_OF_PLENTY);
		} else
		    pline(nothing_happens);
		break;
	case LAND_MINE:
	case BEARTRAP:
		use_trap(obj);
		break;
	case FLINT:
	case LUCKSTONE:
	case LOADSTONE:
	case TOUCHSTONE:
		use_stone(obj);
		break;
	default:
		/* Pole-weapons can strike at a distance */
		if (is_pole(obj)) {
			res = use_pole(obj);
			break;
		} else if (is_pick(obj) || is_axe(obj)) {
			res = use_pick_axe(obj);
			break;
		}
		pline("Lo siento, no sé cómo se usa eso.");
	xit:
		nomul(0);
		return 0;
	}
	if (res && obj && obj->oartifact) arti_speak(obj);
	nomul(0);
	return res;
}

/* Keep track of unfixable troubles for purposes of messages saying you feel
 * great.
 */
int
unfixable_trouble_count(is_horn)
	boolean is_horn;
{
	int unfixable_trbl = 0;

	if (Stoned) unfixable_trbl++;
	if (Strangled) unfixable_trbl++;
	if (Wounded_legs
#ifdef STEED
		    && !u.usteed
#endif
				) unfixable_trbl++;
	if (Slimed) unfixable_trbl++;
	/* lycanthropy is not desirable, but it doesn't actually make you feel
	   bad */

	/* we'll assume that intrinsic stunning from being a bat/stalker
	   doesn't make you feel bad */
	if (!is_horn) {
	    if (Confusion) unfixable_trbl++;
	    if (Sick) unfixable_trbl++;
	    if (HHallucination) unfixable_trbl++;
	    if (Vomiting) unfixable_trbl++;
	    if (HStun) unfixable_trbl++;
	}
	return unfixable_trbl;
}

#endif /* OVLB */

/*apply.c*/
