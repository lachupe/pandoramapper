# **Send me your questions! #**

#summary FAQ and/or knowledge center.

# How to start it? How to connect ? #
_I downloaded the mapper, what do i do now?_

You just have to unpack the mapper and click on/start the pandora.exe file. Then you start your client and connect to localhost 3000.
JMC:
#connect localhost 3000

powwow/powtty
#connect main localhost 3000

Then you just log in the chareter of your choise and make sure you turn on the XML mode. You can do it by: change xml on.

If the XML mode is already on you will see it in the first MUME room input. (Inn room)


# Which shortkey are available? #

F11 and F12 are very useful - you can turn on/off both statusbar and menubar. This makes mapper window smaller.

spacebar - turn on the Movement tool. This means if you hold spacebar and click left or right mouse button you can change some settings of the camera.

The following hotkeys also affect the camera:
q,w
a,s
y,x
pageUp, pageDown

mouseWheel zooms in/out

You can change your current layer (i.e. you can highlight the layers above or below the current one) just by pressing Plus/Minus on your numpad.


# What to do first after i connected? #

You should read the help file :-) It's accessible via mhelp mapper command.
That's it. just type mhelp in your client and then mhelp 

&lt;commandname&gt;



blabla>mhelp

blabla>mhelp minfo

# How can i add new room? #

You have to make sure you are in sync and then just type mmap on.
It's important to understand how mapping works. Mapper needs full information about the room you are adding. Mapper will also try to link the fresh added room to others and also will try to merge this room with possible twin room (to avoid double occasions of the same room in the database).

# What modes are there for linking fresh added rooms? #

angryLinker - this mode is very useful in 10x10 plain areas, where all rooms are layed out in very strict and common _geometrical_ order. Angrylinker just linkes the new room to all rooms surrounding this room, if it's close enough and has undefined exits in proper directions.

automerge - look for twin rooms automaticly.

dualLinker - treat all new connections/links as bi-directional. If you turn this option off, mapper will create one-ways only. Per default this is always on.


# How can i connect two rooms? #

There are different possibilities.
mlink command is one of them
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