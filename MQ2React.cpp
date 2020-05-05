// MQ2React.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.

#include "yaml/Yaml.hpp"
#include "../MQ2Plugin.h"

PLUGIN_VERSION(0.1);
PreSetup("MQ2React");

// Constants
constexpr auto REACT_SLEEP = 50;

// Global Declarations
static Yaml::Node root;

/*
@brief Determines if a react exists based on its nickname.
@param nickname Name of the react to look for.
@return -1 if the item wasn't found, otherwise its index in the list.
*/
int GetReactIdx(const std::string& nickname) {
	PCHARINFO pCharInfo = GetCharInfo();
	if (!pCharInfo) return -1;

	Yaml::Node& Reacts = root[pCharInfo->Name]["reacts"];

	int react_idx = 0;
	for (auto itr = Reacts.Begin(); itr != Reacts.End(); itr++) {
		Yaml::Node& react = (*itr).second;

		if (react["nickname"].As<std::string>() == nickname) {
			return react_idx;
		}
		react_idx++;
	}
	// Default return
	return -1;
}


void PrintReacts() {
	PCHARINFO pCharInfo = GetCharInfo();
	if (!pCharInfo) return;

	Yaml::Node& Reacts = root[pCharInfo->Name]["reacts"];

	for (auto itr = Reacts.Begin(); itr != Reacts.End(); itr++) {
		Yaml::Node& react = (*itr).second;
		WriteChatfSafe("%s",react["nickname"].As<std::string>().c_str());
	}
}

void PrintHelp()
{
	WriteChatf("MQ2React");
	WriteChatf("Ineract with MQ2React via /react <verb> <optarg> <optarg> <optarg>");
	WriteChatf("Reaction configuration is per character name per character name in mq2react.ini.");
	WriteChatf("/react help - display this message.");
	WriteChatf("/noparse /react add <nickname> <condition> <action> - add a new reaction to your config with <condition> and <action> labeled <nickname>. Disabled by default.");
	WriteChatf("/react remove <nickname> - remove a reaction from your config file that has the label <nickname> .");
	WriteChatf("/react enable <nickname> - enable a reaction with label <nickname>.");
	WriteChatf("/react disable <nickname> - disable a reaction with label <nickname>.");
	WriteChatf("/react list - Display all condition nicknames.");
}

// Parse /react <verb> <opt:nickname> <opt:condition> <opt:action>
// /help - print command help
VOID ReactCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	char Verb[MAX_STRING] = { 0 };
	char Nickname[MAX_STRING] = { 0 };
	char Condition[MAX_STRING] = { 0 };
	char Action[MAX_STRING] = { 0 };
	PCHARINFO pCharInfo = GetCharInfo();

	GetArg(Verb, szLine, 1);

	if (!_stricmp(Verb, "help")) {
		PrintHelp();
	}
	if (!_stricmp(Verb, "add")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		GetArg(Condition, szLine, 3);
		if (!strlen(Condition)) PrintHelp();

		GetArg(Action, szLine, 4);
		if (!strlen(Action)) PrintHelp();

		if (GetReactIdx(Nickname) == -1) {
			Yaml::Node& NewReact = root[pCharInfo->Name]["reacts"].PushBack();
			NewReact["nickname"] = Nickname;
			NewReact["condition"] = Condition;
			NewReact["action"] = Action;
			NewReact["enabled"] = "0";
		}
		Yaml::Serialize(root, INIFileName);
	}
	if (!_stricmp(Verb, "remove")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		int react_idx = GetReactIdx(Nickname);
		if (react_idx != -1) {
			root[pCharInfo->Name]["reacts"].Erase(react_idx);
		}
	}
	if (!_stricmp(Verb, "enable")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		int react_idx = GetReactIdx(Nickname);
		if (react_idx != -1) {
			root[pCharInfo->Name]["reacts"][react_idx]["enabled"] = "1";
		}
	}
	if (!_stricmp(Verb, "disable")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		int react_idx = GetReactIdx(Nickname);
		if (react_idx != -1) {
			root[pCharInfo->Name]["reacts"][react_idx]["enabled"] = "0";
		}
	}
	if (!_stricmp(Verb, "list"))
		PrintReacts();
}

class MQ2ReactType : public MQ2Type {
public:
	enum Members {
		Action,
		Condition,
		Enabled
	};

	MQ2ReactType() :MQ2Type("React")
	{
		TypeMember(Action);
		TypeMember(Condition);
		TypeMember(Enabled);
	}
	~MQ2ReactType() {}

	/*
	* React.Action[nickname] -- Returns the action associated with the react 'nickname'
	* React.Condition[nickname] -- Returns the condition associated with the react 'nickname'
	* React.Enabled[nickname] -- Returns if the react 'nickname' is enabled or not
	*/
	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR& Dest)
	{
		PMQ2TYPEMEMBER pMember = MQ2ReactType::FindMember(Member);
		if (!pMember)
			return false;
		if (!pLocalPlayer)
			return false;

		PCHARINFO pCharInfo = GetCharInfo();
		switch ((Members)pMember->ID) {
			DebugSpewAlways("%d -- %s\n", (Members)pMember->ID, pCharInfo->Name);
			case Action:
				if (Index && Index[0] != '\0') {
					int idx = GetReactIdx(Index);
					if (idx != -1) {
						strcpy_s(_buf, root[pCharInfo->Name]["reacts"][idx]["action"].As<std::string>().c_str());
						Dest.Ptr = &_buf[0];
						Dest.Type = pStringType;
					}
				}
				return true;
			case Condition:
				if (Index && Index[0] != '\0') {
					int idx = GetReactIdx(Index);
					if (idx != -1) {
						strcpy_s(_buf, root[pCharInfo->Name]["reacts"][idx]["condition"].As<std::string>().c_str());
						Dest.Ptr = &_buf[0];
						Dest.Type = pStringType;
					}
				}
				return true;
			case Enabled:
				if (Index && Index[0] != '\0') {
					int idx = GetReactIdx(Index);
					if (idx != -1) {
						strcpy_s(_buf, root[pCharInfo->Name]["reacts"][idx]["enabled"].As<std::string>().c_str());
						Dest.Ptr = &_buf[0];
						Dest.Type = pStringType;
					}
				}
				return true;
			default:
				return false;
				break;
		}
	}

	bool FromData(MQ2VARPTR& VarPtr, MQ2TYPEVAR& Source)
	{
		return false;
	}
	bool FromString(MQ2VARPTR& VarPtr, PCHAR Source)
	{
		return false;
	}
private:
	CHAR _buf[MAX_STRING];
};
class MQ2ReactType* pReactType = nullptr;

BOOL TLOReact(char* szIndex, MQ2TYPEVAR& Dest)
{
	Dest.DWord = 1;
	Dest.Type = pReactType;
	return true;
}

void LoadConfig(const char* configname)
{
	PCHARINFO pCharInfo = GetCharInfo();
	if (!pCharInfo) return;

	if (!_FileExists(configname))
		Yaml::Serialize(root, configname);

	// Must call after the file check or you could error out.
	Yaml::Parse(root, configname);

	// Try and load the current character's reacts, if they aren't in the config yet, then add
	// them with a blank list of reacts.
	Yaml::Node& char_config = root[pCharInfo->Name];

	// If the character doesn't exist we'll create a configuration file entry for them and provide
	// a disabled default react entry in case they want to edit the config file instead of use ingame commands.
	if (char_config.IsNone()) {
		if (EQADDR_SERVERNAME[0]) {
			root[pCharInfo->Name]["Server"] = std::string(EQADDR_SERVERNAME);
		}
		else {
			root[pCharInfo->Name]["Server"] = "Unknown";
		}
		Yaml::Node& DefaultReact = root[pCharInfo->Name]["reacts"].PushFront();
		DefaultReact["nickname"] = "DefaultExample";
		DefaultReact["condition"] = "${Me.PctHPs} == 100 && ${Me.CombatState.NotEqual[COMBAT]}";
		DefaultReact["action"] = "/echo If you enable this it will spam you with echos when you're at full health out of combat.";
		DefaultReact["enabled"] = "0";
		Yaml::Serialize(root, configname);
	}
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2React");
	AddCommand("/react", ReactCommand);
	AddMQ2Data("React", TLOReact);
	pReactType = new MQ2ReactType;
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2React");
	RemoveCommand("/react");
	RemoveMQ2Data("React");
}

// Called after entering a new zone
PLUGIN_API VOID OnZoned(VOID)
{
	DebugSpewAlways("MQ2React::OnZoned()");
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
	DebugSpewAlways("MQ2React::SetGameState()");
	if (GameState == GAMESTATE_INGAME)
		LoadConfig(INIFileName);
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
	//DebugSpewAlways("MQ2React::OnPulse()");
	static int pulse = 0;
	static auto action_queue = std::queue<std::string>();

	if (GetGameState() != GAMESTATE_INGAME)
		return;

	PCHARINFO pCharInfo = GetCharInfo();
	if (!pCharInfo) return;

	// We've not yet loaded our configuration if mini-yaml finds root node is None
	if (root.IsNone()) return;

	if (++pulse < REACT_SLEEP)
		return;

	// Time to wake-up
	pulse = 0;

	// Loop through every react we have and then add any ready-to-go reacts to our action_queue
	Yaml::Node& Reacts = root[pCharInfo->Name]["reacts"];
	for (auto itr = Reacts.Begin(); itr != Reacts.End(); itr++) {
		Yaml::Node& react = (*itr).second;
		
		double result = 0;
		std::string condition = react["condition"].As<std::string>();
		// ParseMacroData will resolve any TLOs in our action string
		ParseMacroData(&condition[0], condition.size());
		// Calculate will return a DWORD result, if the result is non-zero it's true and we'll add to our queue
		Calculate(&condition[0], result);

		if (result != 0) {
			action_queue.push(react["action"].As<std::string>());
		}
	}

	// Pop off an element from our reaction queue, if any, and execute it with EzCommand


}

