/*	SCCS Id: @(#)sit.c	3.4	2002/09/21	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

void
take_gold()
{
#ifndef GOLDOBJ
	if (u.ugold <= 0)  {
		Sientes("una sensación extraña.");
	} else {
		pline("¡Noticias que no tienes oro!");
		u.ugold = 0;
		flags.botl = 1;
	}
#else
        struct obj *otmp, *nobj;
	int lost_money = 0;
	for (otmp = invent; otmp; otmp = nobj) {
		nobj = otmp->nobj;
		if (otmp->oclass == COIN_CLASS) {
			lost_money = 1;
			delobj(otmp);
		}
	}
	if (!lost_money)  {
		Sientes("una sensación extraña.");
	} else {
		pline("¡Noticias que no tienes dinero!");
		flags.botl = 1;
	}
#endif
}

int
dosit()
{
	static const char sit_message[] = "Te sientas en %s.";
	register struct trap *trap;
	register int typ = levl[u.ux][u.uy].typ;


#ifdef STEED
	if (u.usteed) {
	    pline("Ya te sientas en %s.", mon_nam(u.usteed));
	    return (0);
	}
#endif

	if(!can_reach_floor())	{
	    if (Levitation)
		pline("Volteas en lugar.");
	    else
		pline("Te sientas en aire.");
	    return 0;
	} else if (is_pool(u.ux, u.uy) && !Underwater) {  /* water walking */
	    goto in_water;
	}

	if(OBJ_AT(u.ux, u.uy)) {
	    register struct obj *obj;

	    obj = level.objects[u.ux][u.uy];
	    pline(sit_message, the(xname(obj)));
	    if (!(Is_box(obj) || objects[obj->otyp].oc_material == CLOTH))
		pline("No es muy cómod%c...",
			isfeminine(xname(obj))? 'a' : 'o');

	} else if ((trap = t_at(u.ux, u.uy)) != 0 ||
		   (u.utrap && (u.utraptype >= TT_LAVA))) {

	    if (u.utrap) {
		exercise(A_WIS, FALSE);	/* you're getting stuck longer */
		if(u.utraptype == TT_BEARTRAP) {
		    No_puedes("sentarte cuando tu %s está en la trampa para osos.", body_part(FOOT));
		    u.utrap++;
	        } else if(u.utraptype == TT_PIT) {
		    if(trap->ttyp == SPIKED_PIT) {
			pline("Te sientas sobre una púa.  ¡Ouch!");
			losehp(1, "sentarse sobre una púa de hierro", KILLED_BY);
			exercise(A_STR, FALSE);
		    } else
			pline("Te sientas en el hoyo.");
		    u.utrap += rn2(5);
		} else if(u.utraptype == TT_WEB) {
		    pline("¡Te sientas en la telaraña y te enredas más!");
		    u.utrap += rn1(10, 5);
		} else if(u.utraptype == TT_LAVA) {
		    /* Must have fire resistance or they'd be dead already */
		    pline("¡Te sientas en la lava!");
		    u.utrap += rnd(4);
		    losehp(d(2,10), "sentarse en lava", KILLED_BY);
		} else if(u.utraptype == TT_INFLOOR) {
		    No_puedes("maniobrar para sentarte!");
		    u.utrap++;
		}
	    } else {
	        pline("Te sientas.");
		dotrap(trap, 0);
	    }
	} else if(Underwater || Is_waterlevel(&u.uz)) {
	    if (Is_waterlevel(&u.uz))
		pline("No hay ningunos cojínes flotando acerca.");
	    else
		pline("Te sientas en el fondo fangoso.");
	} else if(is_pool(u.ux, u.uy)) {
 in_water:
	    pline("Te sientas en el agua.");
	    if (!rn2(10) && uarm)
		(void) rust_dmg(uarm, "armadura", 1, TRUE, &youmonst);
	    if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
		(void) rust_dmg(uarm, "armadura", 1, TRUE, &youmonst);
#ifdef SINKS
	} else if(IS_SINK(typ)) {

	    pline(sit_message, the(defsyms[S_sink].explanation));
	    pline("Se moja tu %s.", humanoid(youmonst.data) ? "trasera" : "barriga");
#endif
	} else if(IS_ALTAR(typ)) {

	    pline(sit_message, the(defsyms[S_altar].explanation));
	    altar_wrath(u.ux, u.uy);

	} else if(IS_GRAVE(typ)) {

	    pline(sit_message, the(defsyms[S_grave].explanation));

	} else if(typ == STAIRS) {

	    pline(sit_message, "la escalera");

	} else if(typ == LADDER) {

	    pline(sit_message, "la escalera de mano");

	} else if (is_lava(u.ux, u.uy)) {

	    /* must be WWalking */
	    pline(sit_message, "la lava");
	    burn_away_slime();
	    if (likes_lava(youmonst.data)) {
		pline("La lava siente cálida.");
		return 1;
	    }
	    pline("¡La lava te quema!");
	    losehp(d((Fire_resistance ? 2 : 10), 10),
		   "sentarse en lava", KILLED_BY);

	} else if (is_ice(u.ux, u.uy)) {

	    pline(sit_message, the(defsyms[S_ice].explanation));
	    if (!Cold_resistance) pline("El hielo siente frío.");

	} else if (typ == DRAWBRIDGE_DOWN) {

	    pline(sit_message, "puente levadizo");

	} else if(IS_THRONE(typ)) {

	    pline(sit_message, the(defsyms[S_throne].explanation));
	    if (rnd(6) > 4)  {
		switch (rnd(13))  {
		    case 1:
			(void) adjattrib(rn2(A_MAX), -rn1(4,3), FALSE);
			losehp(rnd(10), "trono maldito", KILLED_BY_AN);
			break;
		    case 2:
			(void) adjattrib(rn2(A_MAX), 1, FALSE);
			break;
		    case 3:
			pline("¡Un choque eléctrico%s dispara través de tu cuerpo!",
			      (Shock_resistance) ? "" : " masivo");
			losehp(Shock_resistance ? rnd(6) : rnd(30),
			       "silla eléctrica", KILLED_BY_AN);
			exercise(A_CON, FALSE);
			break;
		    case 4:
			Te_sientes("mucho, mucho mejor!");
			if (Upolyd) {
			    if (u.mh >= (u.mhmax - 5))  u.mhmax += 4;
			    u.mh = u.mhmax;
			}
			if(u.uhp >= (u.uhpmax - 5))  u.uhpmax += 4;
			u.uhp = u.uhpmax;
			make_blinded(0L,TRUE);
			make_sick(0L, (char *) 0, FALSE, SICK_ALL);
			heal_legs();
			flags.botl = 1;
			break;
		    case 5:
			take_gold();
			break;
		    case 6:
			if(u.uluck + rn2(5) < 0) {
			    Te_sientes("que tu suerte se cambia.");
			    change_luck(1);
			} else	    makewish(FALSE);
			break;
		    case 7:
			{
			register int cnt = rnd(10);

			pline("Una voz resona:");
			verbalize("¡Se convoca vuestra audiencia, Señor%s!",
				  flags.female ? "a" : "");
			while(cnt--)
			    (void) makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
			break;
			}
		    case 8:
			pline("Una voz resona:");
			verbalize("Por vuestra orden imperiosa, Señor%s...",
				  flags.female ? "a" : "");
			do_genocide(5);	/* REALLY|ONTHRONE, see do_genocide() */
			break;
		    case 9:
			pline("Una voz resona:");
	verbalize("¡Que maldit%c seas tú por sentarte sobre este trono santísimo!",
				flags.female ? 'a' : 'o');
			if (Luck > 0)  {
			    make_blinded(Blinded + rn1(100,250),TRUE);
			} else	    rndcurse();
			break;
		    case 10:
			if (Luck < 0 || (HSee_invisible & INTRINSIC))  {
				if (level.flags.nommap) {
					pline(
					"¡Un zumbido terrible te llena la cabeza!");
					make_confused(HConfusion + rnd(30),
									FALSE);
				} else {
					pline("Una imagen se forma en tu mente.");
					do_mapping();
				}
			} else  {
				pline("Tu visión se hace clara.");
				HSee_invisible |= FROMOUTSIDE;
				newsym(u.ux, u.uy);
			}
			break;
		    case 11:
			if (Luck < 0)  {
			    Te_sientes("amenazad%c.", poly_gender()==1? 'a' : 'o');
			    aggravate();
			} else  {

			    Sientes("una sensación retorcida.");
			    tele();		/* teleport him */
			}
			break;
		    case 12:
			pline("¡Obtienes un discernimiento!");
			if (invent) {
			    /* rn2(5) agrees w/seffects() */
			    identify_pack(rn2(5));
			}
			break;
		    case 13:
			pline("¡Tu mente se convierte en un pretzel!");
			make_confused(HConfusion + rn1(7,16),FALSE);
			break;
		    default:	impossible("efecto de trono");
				break;
		}
	    } else {
		if (is_prince(youmonst.data))
	    	    Te_sientes("muy cómod%c aquí.",
			    poly_gender()==1? 'a' : 'o');
		else
	    	    Te_sientes("extrañamente afuera de tu lugar...");
	    }

	    if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ)) {
		/* may have teleported */
		levl[u.ux][u.uy].typ = ROOM;
		pline("El trono se desvanece en una bocanada de lógica.");
		newsym(u.ux,u.uy);
	    }

	} else if (lays_eggs(youmonst.data)) {
		struct obj *uegg;

		if (!flags.female) {
			pline("¡Machos no pueden poner huevos!");
			return 0;
		}

		if (u.uhunger < (int)objects[EGG].oc_nutrition) {
			pline("No tienes bastante energía para poner un huevo.");
			return 0;
		}

		uegg = mksobj(EGG, FALSE, FALSE);
		uegg->spe = 1;
		uegg->quan = 1;
		uegg->owt = weight(uegg);
		uegg->corpsenm = egg_type_from_parent(u.umonnum, FALSE);
		uegg->known = uegg->dknown = 1;
		attach_egg_hatch_timeout(uegg);
		pline("Pones un huevo.");
		dropy(uegg);
		stackobj(uegg);
		morehungry((int)objects[EGG].oc_nutrition);
	} else if (u.uswallow)
		pline("¡No hay sillas aquí!");
	else
		pline("¿Disfrutas sentarte en %s?", surface(u.ux,u.uy));
	return(1);
}

void
rndcurse()			/* curse a few inventory items at random! */
{
	int	nobj = 0;
	int	cnt, onum;
	struct	obj	*otmp;
	static const char mal_aura[] = "Sientes una aura malignante alrededor de %s.";

	if (uwep && (uwep->oartifact == ART_MAGICBANE) && rn2(20)) {
	    pline(mal_aura, "la hoja absorbente de magia");
	    return;
	}

	if(Antimagic) {
	    shieldeff(u.ux, u.uy);
	    pline(mal_aura, "ti");
	}

	for (otmp = invent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
	    /* gold isn't subject to being cursed or blessed */
	    if (otmp->oclass == COIN_CLASS) continue;
#endif
	    nobj++;
	}
	if (nobj) {
	    for (cnt = rnd(6/((!!Antimagic) + (!!Half_spell_damage) + 1));
		 cnt > 0; cnt--)  {
		onum = rnd(nobj);
		for (otmp = invent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
		    /* as above */
		    if (otmp->oclass == COIN_CLASS) continue;
#endif
		    if (--onum == 0) break;	/* found the target */
		}
		/* the !otmp case should never happen; picking an already
		   cursed item happens--avoid "resists" message in that case */
		if (!otmp || otmp->cursed) continue;	/* next target */

		if(otmp->oartifact && spec_ability(otmp, SPFX_INTEL) &&
		   rn2(10) < 8) {
		    pline("¡%s!", Tobjnam(otmp, "resiste"));
		    continue;
		}

		if(otmp->blessed)
			unbless(otmp);
		else
			curse(otmp);
	    }
	    update_inventory();
	}

#ifdef STEED
	/* treat steed's saddle as extended part of hero's inventory */
	if (u.usteed && !rn2(4) &&
		(otmp = which_armor(u.usteed, W_SADDLE)) != 0 &&
		!otmp->cursed) {	/* skip if already cursed */
	    if (otmp->blessed)
		unbless(otmp);
	    else
		curse(otmp);
	    if (!Blind) {
		const char *que = xname(otmp);
		pline("%s %s %s %s.",
			The(que),
			del(y_monnam(u.usteed)),
			otense(otmp, "brilla"),
		        feminize(hcolor(otmp->cursed ? NH_BLACK : (const char *)"marrón"),
				 isfeminine(que)));
		otmp->bknown = TRUE;
	    }
	}
#endif	/*STEED*/
}

void
attrcurse()			/* remove a random INTRINSIC ability */
{
	switch(rnd(11)) {
	case 1 : if (HFire_resistance & INTRINSIC) {
			HFire_resistance &= ~INTRINSIC;
			Te_sientes("más caliente.");
			break;
		}
	case 2 : if (HTeleportation & INTRINSIC) {
			HTeleportation &= ~INTRINSIC;
			Te_sientes("menos salt%s.",
				poly_gender()==1? "ona" : "ón");
			break;
		}
	case 3 : if (HPoison_resistance & INTRINSIC) {
			HPoison_resistance &= ~INTRINSIC;
			Te_sientes("un poco mal%c!",
				poly_gender()==1? 'a' : 'o');
			break;
		}
	case 4 : if (HTelepat & INTRINSIC) {
			HTelepat &= ~INTRINSIC;
			if (Blind && !Blind_telepat)
			    see_monsters();	/* Can't sense mons anymore! */
			pline("¡Tus sentidos se fallan!");
			break;
		}
	case 5 : if (HCold_resistance & INTRINSIC) {
			HCold_resistance &= ~INTRINSIC;
			Te_sientes("más frí%c.",
				poly_gender()==1? 'a' : 'o');
			break;
		}
	case 6 : if (HInvis & INTRINSIC) {
			HInvis &= ~INTRINSIC;
			Te_sientes("paranoide.");
			break;
		}
	case 7 : if (HSee_invisible & INTRINSIC) {
			HSee_invisible &= ~INTRINSIC;
			/* LENGUA:  yet another hallucination message that I
			   can't translate */
			pline("¡%s!", Hallucination ? "You tawt you taw a puttie tat"
						: "Pensaste que vieras alguno");
			break;
		}
	case 8 : if (HFast & INTRINSIC) {
			HFast &= ~INTRINSIC;
			Te_sientes("más despacio.");
			break;
		}
	case 9 : if (HStealth & INTRINSIC) {
			HStealth &= ~INTRINSIC;
			Te_sientes("torpe.");
			break;
		}
	case 10: if (HProtection & INTRINSIC) {
			HProtection &= ~INTRINSIC;
			Te_sientes("vulnerable.");
			break;
		}
	case 11: if (HAggravate_monster & INTRINSIC) {
			HAggravate_monster &= ~INTRINSIC;
			Te_sientes("menos atractiv%c.",
				poly_gender()==1? 'a' : 'o');
			break;
		}
	default: break;
	}
}

/*sit.c*/
