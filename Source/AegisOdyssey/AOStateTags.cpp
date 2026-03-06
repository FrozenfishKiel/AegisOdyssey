#include "AOStateTags.h"

namespace AOStateTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Equip_Normal, "State.Equip.Normal", "角色正常状态（不装备武器）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_LightAttack, "State.Combat.LightAttack", "角色轻攻击状态");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_CombatWindow, "State.Combat.CombatWindow", "角色攻击窗口");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Combating, "State.Combat.Combating", "角色连招中");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Recovery, "State.Combat.Recovery", "角色攻击动作后摇");

}