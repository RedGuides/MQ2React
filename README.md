# MQ2React

MQ2React is a plugin that allows users to define actions and conditions that define when to perform those actions. Conditions are defined in MQ2 
Macro syntax and actions are mq2 commands. It's up to the user to make sure their defined conditionas and reactions do not conflict with one 
another; mq2react does no additional action deconfliction. This is similar to the "HolyShit" and "DownShit" behavior of MQ2Melee without the 
additional automation behavior that exists within the plugin. MQ2React also allows for complex conditions to be defined as global conditions and 
reused throughout multiple reacts, thus making reacts considerably easier to maintain and reuse. 

## Configuration
MQ2React uses a yaml format for configuration. Upon initial execution, a default configuration file will be created with example reacts and an
example global condition. The configuration file is case sensitive.

## Commands
* `/react help` - Displays the below list of commands.
* `/react add <nickname> <condition> <action>` - add a new reaction to your config with <condition> and <action> labeled <nickname>.
New reacts are disabled by default. `/noparse` is required to avoid your condition being parsed before addition.
* `/react remove <nickname>` - remove a reaction from your config file that has the label <nickname> .
* `/react enable <nickname>` - enable a reaction with label <nickname>.
* `/react disable <nickname>` - disable a reaction with label <nickname>.
* `/react list` - Display all condition nicknames.
* `/react reload` - Reloads the mq2react config.
* `/react save` - Saves the current in-memory configuration to the react config. add, remove, enable, and disable do this automatically.

Currently the only way to add globals is directly to the configuration file.

## TLOs
* `${React.Condition[reactname]}` - Returns the condition from react with 'reactname' 
* `${React.Action[reactname]}` - Return the action from the react with 'reactname'
* `${React.Enabled[reactname]}` - Return true if the react is enabled for the currect character or false if it is disabled
* `${React.Global[globalname]}` - Returns the condition from the global condition with 'globalname'

Note: For TLOs that return condition statements, MQ2, by design, will parse these conditions automatically. These TLOs are most useful to provide shortcuts to complex 
conditions for macros like KissAssist.
