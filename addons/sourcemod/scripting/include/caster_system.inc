#if defined _caster_system_included
 #endinput
#endif
#define _caster_system_included

/**
 * AccountID API supplied by the L4D2-Competitive-Rework-Fix provider.
 * The original caster_system.sp only implements the legacy natives below.
 */
enum CasterSystemAction
{
    CasterSystemAction_Get = 0,
    CasterSystemAction_Set,
    CasterSystemAction_Remove
}

/**
 * Checks, adds or removes a Steam AccountID from the caster list.
 * Requires the caster_system provider from L4D2-Competitive-Rework-Fix.
 */
native bool bCaster(int accountId, CasterSystemAction action);

/**
 * If this in-game and fully connected client is a caster or not
 *
 * @return          True if this is a registered caster
 */
native bool IsClientCaster(int client);

/**
 * If this Steam ID is a registered caster or not
 *
 * @param           String containing the user's auth id
 * @return          True if this is a registered caster
 */
native bool IsIDCaster(const char[] AuthID);

/**
 * Called when a client is registered as a caster
 * 
 * @param		Client index
 * 
 * @noreturn
*/
forward void OnCasterRegistered(int client);

/**
 * Called when a client is unregistered as a caster
 * 
 * @param		Client index
 * 
 * @noreturn
*/
forward void OnCasterUnregistered(int client);

public SharedPlugin __pl_caster_system =
{
	name = "caster_system",
	file = "caster_system.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_caster_system_SetNTVOptional()
{
	MarkNativeAsOptional("bCaster");
	MarkNativeAsOptional("IsClientCaster");
	MarkNativeAsOptional("IsIDCaster");
}
#endif