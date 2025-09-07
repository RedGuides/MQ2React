---
tags:
  - command
---

# /react

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/react [option] [<value>...]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Enable, disable, list, add and remove reactions.
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `on | off` | Enables or disables processing of reactions. |
| `help` | Displays a list of commands |
| `add <nickname> <condition> <action>` | add a new reaction to your config with &lt;condition&gt; as the condition, &lt;action&gt; as the action&gt;, and labeled &lt;nickname&gt;. New reacts are disabled by default. |
| `remove <nickname>` | remove a reaction from your config file that has the label &lt;nickname&gt;. |
| `globaladd <nickname> <condition>` | add a global condition with name &lt;nickname&gt; and condition &lt;condition&gt;. |
| `globalrem <nickname>` | remove a global condition with name &lt;nickname&gt;. |
| `{enable|disable} <nickname>` | enable or disable a reaction with label &lt;nickname&gt;. |
| `list` | Display all condition nicknames in your .yaml |
| `reload` | Reloads the mq2react.yaml file |
| `save` | Saves the current in-memory configuration to the react config. add, remove, enable, and disable do this automatically. |
