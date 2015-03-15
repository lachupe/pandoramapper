# **Send me your questions! #**

#summary FAQ and/or knowledge center.

# How to start it? How to connect ? #
_I downloaded the mapper, what do i do now?_

You just have to unpack the mapper and click on/start the pandora.exe file. Then you start your client and connect to localhost 4242.
JMC:
```
#connect localhost 4242
```

powwow/powtty
```
#connect main localhost 4242
```

The mapper will automatically turn on the XML game-play mode, which will
be notable by 

&lt;xml&gt;

 tag visible between two welcoming lines of the game:

```
                              ***  MUME VIII  ***

                              In progress at FIRE
                     (Free Internet Roleplay Experiences)
             Adapted from J.R.R. Tolkien's Middle-earth world and
                   maintained by CryHavoc, Manwe, and Nada.

              Original code DikuMUD I (help credits), created by:
       S. Hammer, T. Madsen, K. Nyboe, M. Seifert, and H.H. Staerfeldt.

If you have never played MUME before, type NEW to create a new character,
or ? for help. Otherwise, type your account or character name.


By what name do you wish to be known? 

<xml>
By what name do you wish to be known? 
```

This is normal, and i actually decided to leave it as it is, to aslo give you some sort of notification, that mapper is running, and you connected to the right port.

Then you proceed with logging on the charecter of your wish.


# Do i need to map everything myself? #
_How can i start playing?_

Mapper automatically loads the "database/mume.xml" map. Unless i made a mistake, it's a cut version of the database i used to play with. It has all the non-secret rooms i mapped during my playing.
Using this skeleton is probably a good idea. This way you can learn some ways to map hard places (not necessarely the best ways), and your map's skeleton will be compatible will everyone else's map, if you connect to them via group manager.

# Which shortkey are available? #

F11 and F12 are very useful - you can turn on/off both statusbar and menubar. This makes mapper window smaller.

spacebar - turn on the Movement tool. This means if you hold spacebar and click left or right mouse button you can change some settings of the camera.

The following hotkeys also affect the camera:
q,w
a,s
y,x
pageUp, pageDown

mouseWheel zooms in/out

arrow keys also rotate the camera around the Z axis

You can change your current layer (i.e. you can highlight the layers above or below the current one) just by pressing Plus/Minus on your numpad.


# What to do first after i connected? #

You should read the help file :-) It's accessible via mhelp mapper command.
That's it. just type mhelp in your client and then mhelp 

&lt;commandname&gt;



blabla>mhelp

```
mhelp
----[ Brief help files/commands overview.

  maddroom         Add current room to database [READ HELP]                         
  mhelp            Brief help files or commands overview.                           
  mmap             Turn mapping mode on/off, if possible.                           
  mbrief           Turn mappers built in brief mode on/off.                         
  mcheckexits      Turn exits analyzer on/off.                                      
  mautomerge       Turn descriptions analyzer in mapping mode on/off.               
  mangrylinker     Turn the AngryLinker on/off.                                     
  mduallinker      Turn the DualLinker on/off.                                      
  mcheckterrain    Turn terrain analyzer on/off.                                    
  msave            Save/Save as current database.                                   
  mload            Load file/Reload the database from disk.                         
  mreset           Reset mappers state stacks.                                      
  mstat            Display settings and mappers state stacks.                       
  minfo            Display current rooms data (or by given id).                     
  mmerge           Merge twin rooms - manual launch.                                
  mdecx            Decrease the X coordinate.                                       
  mincx            Increase the X coordinate.                                       
  mdecy            Decrease the Y coordinate.                                       
  mincy            Increase the Y coordinate.                                       
  mdecz            Decrease the Z coordinate.                                       
  mincz            Increase the Z coordinate.                                       
  mcoord           Set the coordinates for current room.                            
  mdoor            Add a door in direction                                          
  mnote            Add a note to the room you are standing in                       
  mnotecolor       Change the note color of the room you are standing in.           
  mexit            Add a nonsecret door                                             
  mmark            Mark/Flag some direction.                                        
  mlink            Link current room with some other.                               
  mdetach          Detach connections in 2 rooms.                                   
  mgoto            Set some room as current (using id or direction).                
  maction          Perform some action with doors in current room.                  
  mdebug           Configure debug information/messages.                            
  mdelete          Delete current room.                                             
  mrefresh         Refresh roomdesc.                                                
  mregion          Region's system                                                  
  mtimer           Setup and addon timer, additional simple timers and countdown timers

-->
```

blabla>mhelp mmerge
```
-->
mhelp mmerge
---[Help file : mmerge.
    Usage: mmerge [id] [force]
    Examples: mmerge / mmerge 120 / mmerge 120 force

    Without arguments this command will try to merge the last added room with either
found twin room or given (by id) room. Force argument disables the roomname and desc checks.

-->
```


# How can i add new a room? #

You have to make sure you are in sync and then just type mmap on.
It's important to understand how mapping works. Mapper needs full information about the room you are adding. Mapper will also try to link the fresh added room to others and also will try to merge this room with possible twin room (to avoid double occasions of the same room in the database).
There are several mapping modes you can turn on and off as well.


# What modes are there for linking fresh added rooms? #

AutoMerging - mapper tries to check if your freshly added room is actually already present and you just created a "twin". If this is the case (same name, desc and exits (you can have different undefined exits in this room) fit, it will be merged together with the already existing room.

AngryLinker - the mode automatically connects the room with it's neighbours, if their "undefined" exits look like appealing.

DualLinker - extended mode of the AngryLinker. Whereby AngryLinker only makes the new links "oneway", DualLinker connects the appealing rooms mutualy.

Per default DualLinker and AutoMerging are on and angryLinker is off.
You can turn them on and off by using the mangrylinker, mduallinker and mautomerge commands.

# How can i connect two rooms? #

There are following possibilities:

mlink command is one of them. (mdetach is the counterpart of mlink btw!)

Select two rooms, right click one of them and select the bind rooms option.

Right click one room, change the flag of the exit to none and then left click on the room you want this exit to lead to.

# How can i delete rooms? #

mdelete command deletes the current room you are in.
right click on room->delete
Full deletion means that you also remove all connections leading to this room. Might be useful.

# How can i insert a room inbetween two other rooms? #
_A new room was added right in the middle of the road. How can i add this new room to the existing map?_

It's very important to understand, that mapper only tries to map new rooms if it _knows_ you moved in some new direction. If you just turn mapping on and move to the new room, mapper will think you lost sync for some reason. This is why you have to break the connection between the old rooms and only then move around broken links and let mapper fix it.


**Example:**

Old Map:

[R1](https://code.google.com/p/pandoramapper/source/detail?r=1) - [R2](https://code.google.com/p/pandoramapper/source/detail?r=2)



New Map:

[R1](https://code.google.com/p/pandoramapper/source/detail?r=1) - nR - [R1](https://code.google.com/p/pandoramapper/source/detail?r=1)

You have to: right click on the [R1](https://code.google.com/p/pandoramapper/source/detail?r=1) room and select "Edit". Then you should select the flag undefined for the exit east.
Now move to the [R1](https://code.google.com/p/pandoramapper/source/detail?r=1) room, turn mapping on and move east to map the new room.

Then right click the [R2](https://code.google.com/p/pandoramapper/source/detail?r=2) room, change the flag of the western exit to UNDEFINED and move east to bind both rooms.

# What shall i do if i want to add new fresh added AREA to the map? #

Well, first off take a look around in the new area, to have a basic idea of the layout.
Then mark old rooms with new fresh connections to the new area as undefined. This is important for automatic rooms binding.

# Where is the room description gone? #

_Where is the room description gone, there is no need for it for experienced player special in PK, but if a newbie come and start to play and get a hint to use your mapper, I think it's interesting for them to read all the room descriptions and the work of all the immortals which wrote all this is also not for nothing._

The room description story got a bit more complicated. In addition to the MUME spam/brief settings there is a mapper built-in settings now. You just have to turn on/off the mbrief mode. You can do that either by using the 'mbrief' command or by setting a proper flag in configuration->general settings

The other possibility would be to just use the 'minfo' command only when you want to read some special room description.

# How can i use timers? #

There are generally 4 types of timers.

a) Spells and Addon spells
Both those are accessible via the Configuration->Spells Settings Dialog.
You have to define a spell/addon timer and give it a name. If it has up/down and refresh messages - you are welcome to give them as well.

Spell timers can only be triggered by the up/down/refresh messages. The name of the spell is searched in your stat command output and the timer is added to the end of the entry.

There can be only one spell with the same name and up/down/refresh messages.
The only difference of Addon timers, is that they have no entry in normal stat command output, so when you create an addon timer, you actually want it to be added to the stat command output.

You can also start and stop addon timers via the mtimer command. For example:
```
mtimer addon start <name>
```

b) Timers:
Timers are upwards counting timers. You can have any amount of them with the same name, but there are not up/down messages attached.
So you have to use actions of your client if you want to remove them on a message. To distinguish between them, you can add a description.

Timers are only removed by remove command. The command removes the first timer to be found with the name equal to your input. For exampe:

```
mtimer timer travelling 
mtimer timer miruvor quaffed!

mtimer remove miruvor
```

c) Countdown timers:
Countdown timers are usefull for the cases, where you definetely know the amount of time some even will take. For example, blind spell lasts 90 seconds. (not always!)

So you can setup an action, which catches all blind messages and fires up a countdown timer, so you can chech the time left until someone unblinds.

```
mtimer countdown blind 90 *Stolb the Tarkhnarb Orc*
mtimer countdown blind 90 a huge troll
```

When the duration (given in seconds) is over, the timer is terminated and you receive a message.


This is how your stat command output looks like with all the timers used.
```
OB: 59%, DB: 23%, PB: 53%, Armour: 53%. Wimpy: 168. Mood: wimpy.
Needed: 333,934 xp, 0 tp. Gold: 67. Alert: normal. Condition: hungry thirsty.
Countdowns:
- blind < bulky orc > (up for - 00:43, left - 00:46)
- blind < a huge troll > (up for - 00:37, left - 00:52)
Timers:
- travelling <  > (up for - 00:52)
- miruvor < quaffed! > (up for - 00:47)
Addons:
- addontest (up for - 00:03)
- mytest (up for - 00:01)
Normal spells:
- bless (up for - 00:56)
- armour (unknown time)
```