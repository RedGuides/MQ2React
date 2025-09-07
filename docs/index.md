---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2react.1599/"
support_link: "https://www.redguides.com/community/threads/mq2react.72467/"
repository: "https://github.com/RedGuides/MQ2React"
config: "MQ2React.yaml"
authors: "drwhomphd, Knightly"
tagline: "Condition/Action execution plugin (i.e., the if-then part of MQ2Melee without the Melee)"
quick_start: "https://www.redguides.com/community/threads/mq2-react-collection.72469/"
---

# MQ2React

<!--desc-start-->
MQ2React is a plugin that allows users to define actions and conditions that define when to perform those actions. Conditions are defined in MQ2
Macro syntax and actions are mq commands. It's up to the user to make sure their defined conditionas and reactions do not conflict with one
another; mq2react does no additional action deconfliction. This is similar to the "HolyShit" and "DownShit" behavior of MQ2Melee without the
additional automation behavior that exists within the plugin. MQ2React also allows for complex conditions to be defined as global conditions and
reused throughout multiple reacts, thus making reacts considerably easier to maintain and reuse.
<!--desc-end-->

## Commands

<a href="cmd-react/">
{% 
  include-markdown "projects/mq2react/cmd-react.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2react/cmd-react.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2react/cmd-react.md') }}

## Settings

MQ2React uses a yaml format for configuration. Upon initial execution, a default configuration file named `MQ2React.yaml` will be created with example reacts and an
example global condition. The configuration file is case sensitive.

## Examples

Due to their sensitivity to spaces and tabs, it's recommended you use a code editor for manually editing .yaml files, such as [VS Code](https://code.visualstudio.com/).

```yaml
globals:
  GlobalExample: "${Me.CombatState.NotEqual[COMBAT]} && ${Me.PctHPs} <= 25"
  NotCasting: "!${Me.Casting.ID}"
reacts:
  DoHaste:
    action: /cast Dark Cloak of the Sky
    condition: "${Spell[Haste].Stacks} && !${Me.Song[Haste].ID} && ${React.Global[NotCasting]}"
  EmpoweredBlade:
    action: "/alt act ${Me.AltAbility[Empowered Blades].ID}"
    condition: >-
      ${Target.PctHPs}<99 && ${Target.Named} && ${Me.AltAbilityReady[Empowered
      Blades]}
  ExampleReact:
    action: >-
      /multiline ; /echo Default Example react Disables itself when you're
      at 100%HP and out of Combat ; /delay 5 ; /react disable ExampleReact
    condition: "${Me.PctHPs} == 100 && ${Me.CombatState.NotEqual[COMBAT]}"
  FalconStrike:
    action: "/cast Aurora, the Heartwood Blade"
    condition: >-
      ${Spell[Falcon Strike].Stacks} && !${Me.Buff[Falcon Strike].ID} &&
      ${React.Global[NotCasting]}
  Hungry:
    action: /useitem Fresh Cookie Dispenser
    condition: >-
      ${FindItemCount[=Fresh Cookie]} < 5 && ${FindItemCount[Fresh Cookie
      Dispenser]} == 1
```

More examples can be found in [this thread](https://www.redguides.com/community/threads/mq2-react-collection.72469/).

## Top-Level Objects

## [React](tlo-react.md)
{% include-markdown "projects/mq2react/tlo-react.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2react/tlo-react.md') }}

## DataTypes

## [React](datatype-react.md)
{% include-markdown "projects/mq2react/datatype-react.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2react/datatype-react.md') }}

<h2>Members</h2>
{% include-markdown "projects/mq2react/datatype-react.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
{% include-markdown "projects/mq2react/datatype-react.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
