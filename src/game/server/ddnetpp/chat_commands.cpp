#include <game/server/gamecontext.h>

// used to be a ddnet command but got deleted there
// we kept it in ddnet++
void CGameContext::ConCredits(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(pSelf->DDPPCredits())
		return;

	static constexpr const char *CREDITS[] = {
		"DDNet is run by the DDNet staff (DDNet.org/staff)",
		"Great maps and many ideas from the great community",
		"Help and code by eeeee, HMH, east, CookieMichal, Learath2,",
		"Savander, laxa, Tobii, BeaR, Wohoo, nuborn, timakro, Shiki,",
		"trml, Soreu, hi_leute_gll, Lady Saavik, Chairn, heinrich5991,",
		"swick, oy, necropotame, Ryozuki, Redix, d3fault, marcelherd,",
		"BannZay, ACTom, SiuFuWong, PathosEthosLogos, TsFreddie,",
		"Jupeyy, noby, ChillerDragon, ZombieToad, weez15, z6zzz,",
		"Piepow, QingGo, RafaelFF, sctt, jao, daverck, fokkonaut,",
		"Bojidar, FallenKN, ardadem, archimede67, sirius1242, Aerll,",
		"trafilaw, Zwelf, Patiga, Konsti, ElXreno, MikiGamer,",
		"Fireball, Banana090, axblk, yangfl, Kaffeine, Zodiac,",
		"c0d3d3v, GiuCcc, Ravie, Robyt3, simpygirl, Tater, Cellegen,",
		"srdante, Nouaa, Voxel, luk51, Vy0x2, Avolicious, louis,",
		"Marmare314, hus3h, ArijanJ, tarunsamanta2k20, Possseidon,",
		"+KZ, Teero, furo, dobrykafe, Moiman, JSaurusRex,",
		"Steinchen, ewancg, gerdoe-jr, melon, KebsCS, bencie,",
		"DynamoFox, MilkeeyCat, iMilchshake, SchrodingerZhu,",
		"catseyenebulous, Rei-Tw, Matodor, Emilcha, art0007i, SollyBunny,",
		"0xfaulty & others",
		"Based on DDRace by the DDRace developers,",
		"which is a mod of Teeworlds by the Teeworlds developers.",
	};
	for(const char *pLine : CREDITS)
		log_info("chatresp", "%s", pLine);
}
