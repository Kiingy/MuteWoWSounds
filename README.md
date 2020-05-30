# MuteWoWSounds
This repository contains the source code and instructions for the Mute Annoying WoW Sounds custom mute tool.

AddOn Page: https://www.curseforge.com/wow/addons/mute-wow-sounds

# Instructions

## MAC users.
The tool likely wont run on a MAC. Sorry folks! 
Spaghetti code source is available if anyone would like to port it though.

## Download.
Download from: https://github.com/Kiingy/MuteWoWSounds/releases/tag/v1.1

## Folder structure.
Once extraced open the MuteWoWSounds folder and you should see:
* Folder: Output
* Folder: SoundKitData
  * File: soundkitentry.csv
  * This file is provided by [wow.tools](https://wow.tools/dbc/?dbc=soundkitentry).
  * You can always download the latest version from the above link. 
* Folder: SoundTargets
  * File: Example.txt
* File: MuteWoWSounds.exe


## Finding SoundKitIDs and creating sound target files.
For this I'll show you how to mute raptor mounts.

Go to wowhead and search for any raptor mount.

In the search results select the NPC tab.

![SearchResults](/instructions/images/searchresults.png)

Look for an NPC that shares the same name as the mount you searched for, if theres more than one, either is fine.

![NPCNames](/instructions/images/npcnames.png)

On the NPC page you'll find a "Sounds" tab.

![NPCSounds](/instructions/images/npcsounds.png)

All the sounds this mount uses are listed here.

Next go to where you extracted the tool and open the SoundTargets folder. In here create and open a new text file, it can be named anything you want.

To mute sounds lines must be formatted like so:

`comment - soundkitid1, soundkitid2, .. soundkitidXX`

The comment can be anything, followed by a hyphen then each of the sound kit ids you want to mute separated by commas.

So back to the raptor example.

On the Sounds tab of the NPC page we opened earlier you'll find the list of sounds that the mount uses, each of them when clicked on will take you to a new page with a URL something like this

https://www.wowhead.com/sound=X/sound-name 

What we need is just the number "X".

In the raptor mounts case the URLs for its sounds are:

https://www.wowhead.com/sound=704/raptorattack

https://www.wowhead.com/sound=705/raptorwound

https://www.wowhead.com/sound=706/raptorwoundcritical

https://www.wowhead.com/sound=707/raptordeath

So add each of the IDs to the text file we just created like so.

`raptor mounts - 704,705,706,707`

Or as long as each is on a new line you can also break this down further if you prefer:

`raptor attack - 704`

`raptor wound - 705, 706`

`raptor death - 707`

You can also save in-file comments by starting the line with '#'. 

* There is no limit to how many files you can create. You can organize them however you want.
* There is no limit to how many sounds are muted in each file.

**There is also an example file provided in the download.**

## Running the tool.
Open 'MuteWoWSounds.exe' and simply click Start.

The tool will tell you if there are any errors, these can be read in the ErrorLog.txt file found in the Output folder.

Once complete it will create a file called "CustomSounds.lua" in the output folder.

## Muting sounds in-game.
Navigate to your World of Warcraft > Addons > MuteAnnoyingWoWSounds folder.

Copy the 'CustomSounds.lua' file to this folder.

Then open 'MuteAnnoyingWoWSounds.toc' and on a new line under where it says 'SoundMutes.lua', add 'CustomSounds.lua'

It should look something like this:

![TocFileEdit](/instructions/images/tocedit.png)

__Note:__ *Don't change anything else. If you have a different number after 'Interface:' then what is shown above, thats ok just leave that line as it is.*

Then you're done! You'll need to restart WoW for these custom mutes to take effect.

## Why is there no in-game way to do this?!

While it would be possible to do this in-game, this method has four main advantages.

1. Using wowhead to find the very specific sounds you want to mute is a lot easier then manually sorting through each soundkit. Wowhead has done a lot of the annoying work for you.
2. It is much easier to organize the sounds you want to mute..
   * For example you could create spells.txt, mounts.txt, emotes.txt files. 
   * Should you ever decide to unmute a category, simply delete the file and re-run the tool.
3. Everything is saved outside of WoW. 
   * If you've ever had to reset your wow UI or reinstalled the game you know how annoying it can be to set-up everything again. 
   * As this is independant of the WoW client, as long as you keep your sound target files your custom mutes can always be easily recreated.
4. You can easily share your lists with other players! 

## Finding sounds on wowhead.
Wowhead has a huge list of searchable sounds you can find here: https://www.wowhead.com/sounds

If you want to find the "Frostbolt" sounds for example you can simply search for 'frostbolt' and apply the type filter.

Some of the sounds can be tough to find if Blizzard hasn't named them properly, for example the sound you hear when you dismount in-game is called [SpiritWolf](https://www.wowhead.com/sound=3089), located under the Spells category, not very helpful.

Finding the sounds for a mount, creature or pet is generally a lot easier using the method outlined in the above instructions.
