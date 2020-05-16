// MQ2React.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.
#include "../MQ2Plugin.h"
#include "yaml/Yaml.hpp"

PLUGIN_VERSION(0.1);
PreSetup("MQ2React");

// Constants
constexpr auto REACT_SLEEP = 50;

// Global Declarations
static Yaml::Node root;

void PrintReacts() {
	Yaml::Node& Reacts = root["reacts"];
	for (auto itr = Reacts.Begin(); itr != Reacts.End(); itr++)
		WriteChatf("%s",(*itr).first.c_str());
}

void PrintHelp()
{
	WriteChatf("MQ2React");
	WriteChatf("Ineract with MQ2React via /react <verb> <optarg> <optarg> <optarg>");
	WriteChatf("Reaction configuration is per character name per character name in mq2react.ini.");
	WriteChatf("/react help - display this message.");
	WriteChatf("/react add <nickname> <condition> <action> - add a new reaction to your config with <condition> and <action> labeled <nickname>. Disabled by default.");
	WriteChatf("/react remove <nickname> - remove a reaction from your config file that has the label <nickname> .");
	WriteChatf("/react enable <nickname> - enable a reaction with label <nickname>.");
	WriteChatf("/react disable <nickname> - disable a reaction with label <nickname>.");
	WriteChatf("/react list - Display all condition nicknames.");
	WriteChatf("/react reload - Reloads the mq2react config.");
	WriteChatf("/react save - Saves the current in-memory configuration to the react config. add, remove, enable, and disable do this automatically.");
}

// Parse /react <verb> <opt:nickname> <opt:condition> <opt:action>
// /help - print command help
VOID ReactCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	char Verb[MAX_STRING] = { 0 };
	char Nickname[MAX_STRING] = { 0 };
	PCHARINFO pCharInfo = GetCharInfo();

	GetArg(Verb, szLine, 1);

	if (!_stricmp(Verb, "help")) {
		PrintHelp();
	}
	if (!_stricmp(Verb, "add")) {
		char Condition[MAX_STRING] = { 0 };
		char Action[MAX_STRING] = { 0 };

		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		GetArg(Condition, szLine, 3);
		if (!strlen(Condition)) PrintHelp();

		GetArg(Action, szLine, 4);
		if (!strlen(Action)) PrintHelp();

		// We reload the YAML file in case it was changed prior to our last load so we 
		// do not erase changes made elsewhere. This pattern continues below.
		Yaml::Parse(root, INIFileName);
		root["reacts"][Nickname]["condition"] = Condition;
		root["reacts"][Nickname]["action"] = Action;
		root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname] = "disabled";
		Yaml::Serialize(root, INIFileName);
	}
	if (!_stricmp(Verb, "remove")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		Yaml::Parse(root, INIFileName);
		root["reacts"].Erase(Nickname);
		Yaml::Serialize(root, INIFileName);
	}
	if (!_stricmp(Verb, "enable")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		Yaml::Parse(root, INIFileName);
		if (!root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname].IsNone())
			root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname] = "enabled";
		Yaml::Serialize(root, INIFileName);
	}
	if (!_stricmp(Verb, "disable")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		Yaml::Parse(root, INIFileName);
		if (!root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname].IsNone())
			root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname] = "disabled";
		Yaml::Serialize(root, INIFileName);
	}
	if (!_stricmp(Verb, "reload"))
		Yaml::Parse(root, INIFileName);
	if (!_stricmp(Verb, "save"))
		Yaml::Serialize(root, INIFileName);
	if (!_stricmp(Verb, "list"))
		PrintReacts();
}

class MQ2ReactType : public MQ2Type {
public:
	enum Members {
		Action,
		Condition,
		Enabled,
		Global
	};

	MQ2ReactType() :MQ2Type("React")
	{
		TypeMember(Action);
		TypeMember(Condition);
		TypeMember(Enabled);
		TypeMember(Global);
	}
	~MQ2ReactType() {}

	/*
	* React.Action[nickname] -- Returns the action associated with the react 'nickname'
	* React.Condition[nickname] -- Returns the condition associated with the react 'nickname'
	* React.Enabled[nickname] -- Returns if the react 'nickname' is enabled or not
	*/
	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR& Dest)
	{
		// We create a copy of our root node as IsNone() actually will create nonexistent nodes when
		// checked. This causes failed TLO checks to actually modify our yaml.
		Yaml::Node rootcopy = root;
		PMQ2TYPEMEMBER pMember = MQ2ReactType::FindMember(Member);
		if (!pMember)
			return false;
		if (!pLocalPlayer)
			return false;

		PCHARINFO pCharInfo = GetCharInfo();
		switch ((Members)pMember->ID) {
			case Action:
				if (Index && Index[0] != '\0') {
					if (!rootcopy["reacts"][Index]["action"].IsNone()) {
						strcpy_s(_buf, rootcopy["reacts"][Index]["action"].As<std::string>().c_str());
						Dest.Ptr = &_buf[0];
						Dest.Type = pStringType;
					}
				}
				return true;
			case Condition:
				if (Index && Index[0] != '\0') {
					if (!rootcopy["reacts"][Index]["condition"].IsNone()) {
						strcpy_s(_buf, rootcopy["reacts"][Index]["condition"].As<std::string>().c_str());
						Dest.Ptr = &_buf[0];
						Dest.Type = pStringType;
					}
				}
				return true;
			case Enabled:
				if (Index && Index[0] != '\0') {
					if (!rootcopy[EQADDR_SERVERNAME][pCharInfo->Name][Index].IsNone()) {
						if (rootcopy[EQADDR_SERVERNAME][pCharInfo->Name][Index].As<std::string>() == "enabled") {
							Dest.Int = 1;
						}
						else {
							Dest.Int = 0;
						}
						Dest.Type = pBoolType;
					}
				}
				return true;
			case Global:
				if (Index && Index[0] != '\0') {
					if (!rootcopy["globals"][Index].IsNone()) {
						strcpy_s(_buf, rootcopy["globals"][Index].As<std::string>().c_str());
						Dest.Ptr = &_buf[0];
						Dest.Type = pStringType;
					}
				}
				return true;
			default:
				return false;
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
	CHAR _buf[MAX_STRING] = { 0 };
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
	if (!EQADDR_SERVERNAME[0]) return;

	if (!_FileExists(configname))
		Yaml::Serialize(root, configname);

	// Must call after the file check or you could error out.
	Yaml::Parse(root, configname);

	// Make sure the YAML Config is well structed -- Example globals section
	if (root["globals"].IsNone())
		root["globals"]["GlobalExample"] = "${Me.CombatState.NotEqual[COMBAT]} && ${Me.PctHPs} <= 25";

	// Make sure the YAML Config is well structed -- Example reacts section
	if (root["reacts"].IsNone()) {
		root["reacts"]["ExampleReact"]["condition"] = "${Me.PctHPs} == 100 && ${Me.CombatState.NotEqual[COMBAT]}";
		root["reacts"]["ExampleReact"]["action"] = "/multiline ; /echo Default Example react Disables itself when you're at 100%HP and out of Combat ; /delay 5 ; /react disable ExampleReact";
	}

	// Make sure the YAML Config is well structure -- Per character react list
	if (root[EQADDR_SERVERNAME][pCharInfo->Name].IsNone())
		root[EQADDR_SERVERNAME][pCharInfo->Name]["ExampleReact"] = "enabled";

	Yaml::Serialize(root, configname);
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin()
{
	DebugSpewAlways("Initializing MQ2React");
	AddCommand("/react", ReactCommand, 0, 0); // Disable parsing for command arguments so added reacts don't get parsed
	AddMQ2Data("React", TLOReact);
	pReactType = new MQ2ReactType;
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin()
{
	DebugSpewAlways("Shutting down MQ2React");
	RemoveCommand("/react");
	RemoveMQ2Data("React");
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
	DebugSpewAlways("MQ2React::SetGameState()");
	// This way we make sure we do not load the config while zoning.
	if (GameState == GAMESTATE_INGAME && root.IsNone())
		LoadConfig(INIFileName);
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse()
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
	Yaml::Node& Reacts = root["reacts"];
	for (auto itr = Reacts.Begin(); itr != Reacts.End(); itr++) {
		const std::string nickname = (*itr).first;
		Yaml::Node& react = (*itr).second;
		
		// Only check the react if it is enabled for the current character on the current server
		if (!root[EQADDR_SERVERNAME][pCharInfo->Name][nickname].IsNone()) {
			if (root[EQADDR_SERVERNAME][pCharInfo->Name][nickname].As<std::string>() == "enabled") {
				double result = 0;

				// Convert our condition from a std::string to something usable by mq2
				char szLine[MAX_STRING] = { 0 };
				strcpy_s(szLine, react["condition"].As<std::string>().c_str());

				// ParseMacroData will resolve any TLOs in our action string
				ParseMacroData(szLine, MAX_STRING);
				// Calculate will return a DWORD result, if the result is non-zero it's true and we'll add to our queue
				Calculate(szLine, result);

				if (result != 0) {
					action_queue.push(react["action"].As<std::string>());
				}
			}
		}
	}

	// Pop off an element from our reaction queue, if any, and execute it with EzCommand
	if (!action_queue.empty()) {
		char szCmd[MAX_STRING] = { 0 };
		strcpy_s(szCmd, action_queue.front().c_str());

		// Parse macro data in the action
		ParseMacroData(szCmd, MAX_STRING);
		
		// use EzCommand to execute
		EzCommand(szCmd);

		action_queue.pop();
	}

}

