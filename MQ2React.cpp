// MQ2React.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.
#include "../MQ2Plugin.h"
#include "yaml/Yaml.hpp"

PreSetup("MQ2React");
PLUGIN_VERSION(0.1);

// Constants
constexpr int REACT_SLEEP = 50;
static const std::string CONFIG_FILE = std::string(gszINIPath) + "\\MQ2React.yaml";

// Global Declarations
static Yaml::Node root;
class MQ2ReactType* pReactType = nullptr;

void PrintReacts() {
	Yaml::Node& Reacts = root["reacts"];
	for (auto itr = Reacts.Begin(); itr != Reacts.End(); itr++)
		WriteChatf("%s",(*itr).first.c_str());
}

void SaveConfig() {
	try {
		Yaml::Serialize(root, CONFIG_FILE.c_str());
	}
	catch (const Yaml::Exception e) {
		char Error[MAX_STRING] = { 0 };
		strcpy_s(Error, e.Message());
		WriteChatf("Error saving  mq2react configuration:");
		WriteChatf(Error);
	}
}

bool LoadConfig()
{
	PCHARINFO pCharInfo = GetCharInfo();
	if (!pCharInfo) return false;
	if (!EQADDR_SERVERNAME[0]) return false;

	if (!_FileExists(CONFIG_FILE.c_str()))
		SaveConfig();

	// Must call after the file check or you could error out.
	// NOTE: We probably need to call a root.Clear() here to make sure mq2react's in-memory
	// tree matches the .yaml file if parsing failures occur. Waffling on this at the
	// moment as I'm unsure about unintended consequences of doing this (react stops working
	// until they fix their file for example).
	root.Clear();
	try {
		Yaml::Parse(root, CONFIG_FILE.c_str());
	}
	catch (const Yaml::Exception e) {
		char Error[MAX_STRING] = { 0 };
		strcpy_s(Error, e.Message());
		WriteChatf("Error parsingv mq2react configuration:");
		WriteChatf(Error);
		// Exit early to avoid rewriting the users file as 'root' will be empty.
		// Notify we've uncessfully loaded the file.
		return false;
	}

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

	// Save the default values we created if we created any
	SaveConfig();

	// Succesfully loaded the file
	return true;
}

void PrintHelp()
{
	WriteChatf("MQ2React");
	WriteChatf("Ineract with MQ2React via /react <verb> <optarg> <optarg> <optarg>");
	WriteChatf("Reaction configuration is per character name per character name in mq2react.ini.");
	WriteChatf("/react help - display this message.");
	WriteChatf("/react globaladd <name> <condition> -- add a new global condition with <name> and condition <condition>.");
	WriteChatf("/react globalrem <name> -- removes the global condition that matches <name>.");
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
	bool loadsuccessful = false;
	char Verb[MAX_STRING] = { 0 };
	char Nickname[MAX_STRING] = { 0 };
	PCHARINFO pCharInfo = GetCharInfo();

	GetArg(Verb, szLine, 1);

	if (!_stricmp(Verb, "help")) {
		PrintHelp();
	}
	if (!_stricmp(Verb, "globaladd")) {
		char Condition[MAX_STRING] = { 0 };

		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		GetArg(Condition, szLine, 3);
		if (!strlen(Condition)) PrintHelp();

		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			root["globals"][Nickname] = Condition;
			SaveConfig();
		}
	}
	if (!_stricmp(Verb, "globalrem")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			root["globals"].Erase(Nickname);
			SaveConfig();
		}
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
		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			root["reacts"][Nickname]["condition"] = Condition;
			root["reacts"][Nickname]["action"] = Action;
			root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname] = "disabled";
			SaveConfig();
		}
	}
	if (!_stricmp(Verb, "remove")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			root["reacts"].Erase(Nickname);
			SaveConfig();
		}
	}
	if (!_stricmp(Verb, "enable")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			if (!root["reacts"][Nickname].IsNone())
				root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname] = "enabled";
			SaveConfig();
		}
	}
	if (!_stricmp(Verb, "disable")) {
		GetArg(Nickname, szLine, 2);
		if (!strlen(Nickname)) PrintHelp();

		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			if (!root["reacts"][Nickname].IsNone())
				root[EQADDR_SERVERNAME][pCharInfo->Name][Nickname] = "disabled";
			SaveConfig();
		}
	}
	if (!_stricmp(Verb, "reload"))
		loadsuccessful = LoadConfig();
	if (!_stricmp(Verb, "save")) {
		loadsuccessful = LoadConfig();
		if (loadsuccessful) {
			SaveConfig();
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

BOOL TLOReact(char* szIndex, MQ2TYPEVAR& Dest)
{
	Dest.DWord = 1;
	Dest.Type = pReactType;
	return true;
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
		LoadConfig();
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse()
{
	static int pulse = 0;

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
					char szAction[MAX_STRING] = { 0 };
					strcpy_s(szAction, react["action"].As<std::string>().c_str());
					EzCommand(szAction);
				}
			}
		}
	}
}

