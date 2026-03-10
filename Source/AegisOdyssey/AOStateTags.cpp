#include "AOStateTags.h"

namespace AOStateTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Equip_Normal, "State.Equip.Normal", "角色正常状态（不装备武器）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_LightAttack, "State.Combat.LightAttack", "角色轻攻击状态");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_CombatWindow, "State.Combat.CombatWindow", "角色攻击窗口");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Combating, "State.Combat.Combating", "角色连招中");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Recovery, "State.Combat.Recovery", "角色攻击动作后摇");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Preparation, "State.Combat.Preparation", "角色攻击动作前摇");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Roll, "State.Combat.Roll", "角色翻滚状态");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Sprint, "State.Combat.Sprint", "角色疾跑状态");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Jump, "State.Combat.Jump", "角色跳跃状态");

}