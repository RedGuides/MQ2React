// MQ2React.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.

#include "yaml/Yaml.hpp"
#include "../MQ2Plugin.h"

PLUGIN_VERSION(0.1);
PreSetup("MQ2React");

// Function Declarations
VOID ReactCommand(PSPAWNINFO pChar, PCHAR szLine);
BOOL TLOReact(PCHAR szIndex, MQ2TYPEVAR& Ret);
VOID LoadConfig(const char* configname);
int GetReactIdx(const std::string& nickname);
void PrintHelp();

// Global Declarations
Yaml::Node root;

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2React");
	AddCommand("/react", ReactCommand);
	AddMQ2Data("React", TLOReact);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2React");
	RemoveCommand("/react");

	//Remove commands, MQ2Data items, hooks, etc.
	//RemoveMQ2Benchmark(bmMyBenchmark);
	//RemoveCommand("/mycommand");
	//RemoveXMLFile("MQUI_MyXMLFile.xml");
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
	LoadConfig(INIFileName);
	//if (GameState==GAMESTATE_INGAME)
	// create custom windows if theyre not set up, etc
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
	//DebugSpewAlways("MQ2React::OnPulse()");
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
			Yaml::Node& NewReact = root[pCharInfo->Name]["reacts"].PushFront();
			NewReact["nickname"] = Nickname;
			NewReact["condition"] = Condition;
			NewReact["action"] = Action;
			NewReact["enabled"] = "0";
		}	
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
}

BOOL TLOReact(PCHAR szIndex, MQ2TYPEVAR& Ret)
{

	return FALSE;
}

void LoadConfig(const char* configname)
{
	Yaml::Parse(root, configname);

	PCHARINFO pCharInfo = GetCharInfo();

	if (!_FileExists(configname)) {
		Yaml::Serialize(root, configname);
	}

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
		DefaultReact["action"] = "/echo If you enable this it will spam you with echos whenyou're at full health out of combat.";
		DefaultReact["enabled"] = "0";
		Yaml::Serialize(root, configname);
	}
}

void PrintHelp()
{
	WriteChatf("MQ2React");
	WriteChatf("Ineract with MQ2React via /react <verb> <optarg> <optarg> <optarg>");
	WriteChatf("Reaction configuration is per character name per character name in mq2react.ini.");
	WriteChatf("help - display this message.");
	WriteChatf("add <nickname> <condition> <action> - add a new reaction to your config with <condition> and <action> labeled <nickname>. Disabled by default.");
	WriteChatf("remove <nickname> - remove a reaction from your config file that has the label <nickname> .");
	WriteChatf("enable <nickname> - enable a reaction with label <nickname>.");
	WriteChatf("disable <nickname> - disable a reaction with label <nickname>.");
}

/*
@brief Determines if a react exists based on its nickname.
@param nickname Name of the react to look for.
@return -1 if the item wasn't found, otherwise its index in the list.
*/
int GetReactIdx(const std::string& nickname) {
	PCHARINFO pCharInfo = GetCharInfo();
	Yaml::Node& Reacts = root[pCharInfo->Name]["reacts"];

	int react_idx = 0;
	for (auto itR = Reacts.Begin(); itR != Reacts.End(); itR++) {
		Yaml::Node& react = (*itR).second;

		if (react["nickname"].As<std::string>() == nickname) {
			return react_idx;
		}
		react_idx++;
	}
	// Default return
	return -1;
}
