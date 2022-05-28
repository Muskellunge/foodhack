#include "common.h"

#define resolve_npc ((Npc* (*)(ScriptInstance* script, NpcId npcIdOrPtr))0x802CDAC0)
#define create_standard_npc ((u32 (*)(NpcBlueprint* bp, u32* animList))0x80038864)
Npc* get_npc_by_index(u32 index);
Shadow* get_shadow_by_index(u32 index);
Enemy* get_enemy_safe(NpcId npcId);

typedef struct NpcBlueprint {
	/* 0x00 */ u32 flags;
	/* 0x04 */ u32 initialAnim;
	/* 0x08 */ UNK_FUN_PTR(onUpdate);
	/* 0x0C */ UNK_FUN_PTR(onRender);
} NpcBlueprint;

Npc* _disguise = NULL;
u32* _disguiseAnimList = NULL;

u8 is_disguised(void) {
	return _disguise != NULL;
}

void update_disguise_npc(void) {
	if (is_disguised()) {
		_disguise->yaw = gPlayerStatus.currentYaw;
		_disguise->pos = gPlayerStatus.position;

		_disguise->currentAnim = _disguiseAnimList[0]; // Idle
		switch ((u16) gPlayerStatus.anim) {
			case 4: _disguise->currentAnim = _disguiseAnimList[1]; break; // Walk
			case 5: _disguise->currentAnim = _disguiseAnimList[2]; break; // Run
			case 7: _disguise->currentAnim = _disguiseAnimList[4]; break; // Jump
			case 8: _disguise->currentAnim = _disguiseAnimList[5]; break; // Fall
		}
	}
}

void exit_disguise(void) {
	if (is_disguised()) {
		free_npc(_disguise);
		_disguise = NULL;
	}
}

// Call $DisguiseAs ( npcId )
ApiStatus DisguiseAs(ScriptInstance* script, s32 isInitialCall) {
	Bytecode* args = script->ptrReadPos;

	Npc* targetNpc = resolve_npc(script, get_variable(script, args[0]));
	NpcBlueprint bp;
	Shadow* shadow;

	_disguiseAnimList = get_enemy_safe(targetNpc->npcID)->animList;

	exit_disguise(); // just in case

	// TODO: require that enemy has tattleString != NULL
	// TODO: cute animation

	bp.flags = 0x00000148;
	bp.initialAnim = _disguiseAnimList[0];
	bp.onUpdate = NULL;
	bp.onRender = NULL;
	_disguise = get_npc_by_index(create_standard_npc(&bp, _disguiseAnimList));

	disable_npc_shadow(_disguise);
	/*
	// doesn't seem to work
	shadow = get_shadow_by_index(gPlayerStatus.shadowID);
	shadow->scale.x = targetNpc->collisionRadius / 25.0f;
	shadow->scale.z = targetNpc->collisionRadius / 25.0f;
	*/

	gPlayerStatus.colliderHeight   = targetNpc->collisionHeight;
	gPlayerStatus.colliderDiameter = targetNpc->collisionRadius;

	update_disguise_npc();

	return ApiStatus_DONE2;
}

u8 disguise_is_action_state_allowed(ActionState actionState) {
	if (is_disguised()) {
		// TODO: disable jump depending on npc you are
		return actionState != ActionState_HAMMER && actionState != ActionState_SPIN;
	}

	return TRUE;
}

// Call $ExitDisguise ( )
ApiStatus ExitDisguise(ScriptInstance* script, s32 isInitialCall) {
	exit_disguise();
	// TODO: cute animation
	return ApiStatus_DONE2;
}
