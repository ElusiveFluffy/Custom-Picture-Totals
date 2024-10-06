# Custom Picture Totals For Ty 1

# Requires [<ins>TygerFramework</ins>](https://github.com/ElusiveFluffy/TygerFramework/releases)

This plugin makes it so its possible to easily edit the hardcoded picture frames for the totals screen for each level for `Ty 1`. 
It also supports adding picture frames to levels that originally had none, and having more than 373 total! **`But only up to a max of 512 (max that is allocated in the save file)`**, if you go over 512 it'll cut off the level its currently assigning IDs to early, and all the levels after it will be set to 0.

# Installation
To install the plugin, just install TygerFramework, then just download 1 of the `template zips` from the `releases` (both contain the exact same plugin) and put everything inside the zip into the `"Plugins"` folder (that you need to create) in the `same directory` as the game (if you're unsure just run the game once with TygerFramework installed and it'll make the Plugins folder for you. Just make sure to reload the game to load the plugin after putting it in the folder)

# Methods to set the IDs
There is 2 different ways to set the IDs, a simple, and advanced way. The `templates` (which I recommend using as a starting point) for both methods that have the game's unmodified amounts set.
**`You can not mix and match the two methods`** (A bit too complex to set up)

## Simple
The simple way you just set the amount of picture frames per level, and the plugin will `auto assign` the IDs counting up from level Z1. 
This method does come with a downside though, since all the IDs are auto assigned counting up, editing the amount of picture for a level will shift all the IDs for the later levels
If you set a level to have 0 picture frames it'll `remove` the picture frames from showing for that level

## Advanced
The advanced way allows you to specify all the IDs for each level manually so you can more easily insert values into levels. **`Be careful though as it doesn't check for duplicate IDs`**. Unless thats something you want.
If you want a level to have `no picture frames` you can just have the a `-` for the line with the IDs

# Make sure `not to delete` any of the level ID lines in the txt file as it will break the plugin, since it just iterates though all the levels in the game's memory in order, and moving the level IDs around in the txt file doesn't change the order

# Building from Source
I recommend installing vcpkg to easily be able to install the needed package.  
Just enter this in command prompt to install the minhook package and thats about it to be able to build it
```
vcpkg install minhook --triplet x86-windows-static
```
