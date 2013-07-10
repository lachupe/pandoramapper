/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define MPI		"~$#E"	/* MUME protocol introducer */
#define MPILEN		4	/* strlen(MPI) */


#include <qregexp.h>

//#include <arpa/telnet.h>
//#define IAC 255


#include <QMutex>

#include "defines.h"
#include "CConfigurator.h"
#include "utils.h"
#include "xml2.h"

#include "Proxy/patterns.h"
#include "Proxy/userfunc.h"
#include "Proxy/proxy.h"
#include "Proxy/CDispatcher.h"

#include "Map/CTree.h"

#include "Engine/CStacksManager.h"
#include "Engine/CEngine.h"

#include "GroupManager/CGroup.h"

#include "Gui/mainwindow.h"


Cdispatcher::Cdispatcher()
{
    xmlState = STATE_NORMAL;
    mbrief_state = STATE_NORMAL;
    awaitingData = false;
    scouting = false;
    event.clear();

    scoreExp.setPattern(conf->getScorePattern() );
    scoreTrollExp.setPattern(conf->getShortScorePattern() );
    scoreExp.setPatternSyntax(QRegExp::Wildcard);
    scoreTrollExp.setPatternSyntax(QRegExp::Wildcard);
}

/**
 * return false if this XML tag has to be ignored
 */
bool Cdispatcher::parseXml(QByteArray tag)
{
        /* types without parameters here */
        struct {
            const char  *name;
            int          startType;
            int          endType;
        } TagTypes[] = {
            {"prompt", XML_START_PROMPT, XML_END_PROMPT},
            {"room", XML_START_ROOM, XML_END_ROOM},
            {"name", XML_START_NAME, XML_END_NAME},
            {"description", XML_START_DESC, XML_END_DESC},
            {"exits", XML_START_EXITS, XML_END_EXITS},
            {"terrain", XML_START_TERRAIN, XML_END_TERRAIN},
            {"", -1, -1}
        };

        int p;
        int i;
        QByteArray name, param;
        bool    endTag = false;
        bool    endAfterTag = false;
        int       endType = -1;
        int       startType = -1;
        QByteArray s;

        s = tag.simplified();

        name.clear();
        param.clear();

        p = 1;
        if (s.startsWith("</")) {
            endTag = true;
            p = 2;
        }
        s = s.right ( s.length() - p);  // cut left 1-2 chars, depending of < or </

        p = 1;
        if (s.endsWith("/>")) {
            endAfterTag = true;
            p = 2;
        }
        s = s.left ( s.length() - p);  // cut right 1-2 chars, depending of < or </

        // Look for arguments
        i = s.indexOf(" ");
        if (i == -1)  {
            // no arguments
            name = s;
        } else {
            name = s.left(i);
            param = s.right(s.length() - i);
        }

//        printf("XML tag name : ...%s..., params : ...%s..., EndTag %s, endAfterTag %s\r\n",
 //                   (const char *) name,  (const char *) param, ON_OFF(endTag), ON_OFF(endAfterTag) );

        // now parse the tags !

        // avoid the status tags
        if (name == "status")
        	return false;


        // hard tags first
        if (name == "movement") {
  //          printf("XML TAG: %s\r\n", TagTypes[p].name);
            startType = XML_START_MOVEMENT;
            endType = XML_END_MOVEMENT;

            if (param != "") {
                int index = param.indexOf("dir=");
                index += 4;

                if (param[index] == '\"')
                	index++;

                switch (param[index]) {
                    case 'n' :    param = "north";
                                        break;
                    case 'e' :    param = "east";
                                        break;
                    case 's' :    param = "south";
                                        break;
                    case 'w' :    param = "west";
                                        break;
                    case 'u' :    param = "up";
                                        break;
                    case 'd' :    param = "down";
                                        break;
                }
            }
        } else {
            // the easy ones
            for (p = 0; TagTypes[p].startType != -1; p++) {
                if (name == TagTypes[p].name) {
//                    printf("XML TAG: %s\r\n", TagTypes[p].name);
                    startType = TagTypes[p].startType;
                    endType = TagTypes[p].endType;
                }
            }
        }

        buffer[amount].line = param;
        buffer[amount].type = IS_XML;
        if (endTag)
            buffer[amount].xmlType = endType;
        else
            buffer[amount].xmlType = startType;
        amount++;

        if (endAfterTag) {
            buffer[amount].line.clear();
            buffer[amount].type = IS_XML;
            buffer[amount].xmlType = endType;
            amount++;
        }

        return true;
}


#define STUFFING_CLEANUP  \
            {   \
                line.append(c.subchars);  \
                c.subchars.clear(); \
                line.append(*s);    \
                c.mainState = NORMAL; \
                c.subState = NORMAL; \
            }

void Cdispatcher::dispatchBuffer(ProxySocket &c)
{
    QByteArray line;
    unsigned char *s;
    unsigned char *stop;

    print_debug(DEBUG_DISPATCHER, "dispatchBuffer called");


    line.clear();
    amount = 0;

    /* put back the leftovers */
    if (c.fragment != "") {
        line = c.fragment;
    }

    stop = (unsigned char *) (c.buffer + c.length);
    for (s = (unsigned char *) c.buffer;  s != stop; s++) {
//        printf("[ amount %i - m %i, s %i,  s %i, left %i ] \r\n", amount, c.mainState, c.subState, s, stop - s);
	switch (c.mainState) {
	   	case NORMAL :
			switch (*s) {
			   	   case IAC:
										if (line != "") {
											buffer[amount].type = IS_NORMAL;
											buffer[amount].line = line;
											amount++;
											line.clear();
										}
										c.mainState = TELNET;
										c.subState = T_GOTIAC;
										break;

				   case 0x0a :  // LF
//										printf("Appending %i : %c [newline] \r\n", s, *s);
										line.append(*s);
										buffer[amount].type = IS_NORMAL;
										buffer[amount].line = line;
										amount++;
										line.clear();
										continue;
				   case '<'      :  // turns XML tag mode on
										if (c.isXmlMode()) {
											if (line != "") {
												buffer[amount].type = IS_NORMAL;
												buffer[amount].line = line;
												amount++;
												line.clear();
											}
											c.subchars.append('<');
											c.mainState = XML;
											continue;
										} else {
											break;
										}

				   case '&'      :
										if (c.isXmlMode()) {
											c.subchars.append('&');
											c.subState = AMP;
											c.mainState = STUFFING;
										}
										continue;
			}

			break;
		case  XML :
			switch (*s) {
				case '>' :
								c.subchars.append('>');

								if (parseXml(c.subchars) == false && amount != 0) {
									// this is a bit tricky and hacky
									// we have to go back one line and join them
									// to do that we reinit the line variable with content of the previous line
									amount--;
									line = buffer[amount].line;
								}
								c.mainState = NORMAL;
								c.subState = NORMAL;
								c.subchars.clear();
								continue;
				default:
								c.subchars.append(*s);
								continue;
			}
			break;


        case STUFFING :
			switch (c.subState) {
				case                AMP:
											switch (*s) {
												case 'l' :
													c.subchars.append('l');
													c.subState = L;
													continue;
												case 'g' :
													c.subchars.append('g');
													c.subState = G;
													continue;
												case 'a' :
													c.subchars.append('a');
													c.subState = A;
													continue;
												case 'q' :
													c.subchars.append('q');
													c.subState = Q;
													continue;
												default:
													STUFFING_CLEANUP;
													continue;
											}
				case                 G:
											if (*s == 't') {
												c.subchars.append('t');
												c.subState = LASTCHAR;
											}  else STUFFING_CLEANUP;
												continue;
				case                 L:
											if (*s == 't') {
												c.subchars.append('t');
												c.subState = LASTCHAR;
											} else STUFFING_CLEANUP;
												continue;
				case                 A:
											if (*s == 'm') {
												c.subchars.append('m');
												c.subState = A_M;
											} else STUFFING_CLEANUP;
												continue;
				case                 A_M:
											if (*s == 'p') {
												c.subchars.append('p');
												c.subState = LASTCHAR;
											} else STUFFING_CLEANUP;
												continue;
				case                 LASTCHAR:
											if (*s == ';') {
												if (c.subchars == "&gt")
													line.append('>');
												else if (c.subchars == "&lt")
													line.append('<');
												else if (c.subchars == "&amp")
													line.append('&');
												 c.subState = NORMAL;
												 c.mainState = NORMAL;
												 c.subchars.clear();
											} else STUFFING_CLEANUP;
												continue;
			}
       case TELNET:
			switch (c.subState) {
				case T_NORMAL:
					if (line != "") {
						buffer[amount].type = IS_DATA;
						buffer[amount++].line = line;
						line.clear();
					}
					s--;        // return to the same char we are standing at, but in proper mode
					c.mainState = NORMAL;
					c.subState = NORMAL;
					continue;

				case T_GOTIAC:
					switch (*s) {
							case WILL:
							case WONT:
							case DO:
							case DONT:
								c.subState = T_SKIP;
								break;
							case SB:		// BUG (multiple connections):
								c.subState = T_GOTSB;	// there is only one subopt buffer
								break;
							case IAC:
							case TN_GA:
							default:
								c.subState = T_NORMAL;
								break;
					}
					break;

				case T_SKIP:
					c.subState = T_NORMAL;
					break;

				case T_GOTSB:
					if (*s == IAC) {
						c.subState = T_GOTSBIAC;
					}
					break;

				case T_GOTSBIAC:
					if (*s == IAC) {
						c.subState = T_GOTSB;
					} else if (*s == SE) {
						c.subState = T_NORMAL;
						break;
					} else {
						c.subState = T_NORMAL;
					}
					break;
			 }

			 break;
        }
        line.append(*s);
    }

    c.fragment.clear();
    if (line != "") {
        switch (c.mainState) {
                case NORMAL:
                case STUFFING:
                    buffer[amount].type = IS_NORMAL;
                    buffer[amount].line = line;
                    break;
                case XML:
                    buffer[amount].type = IS_NORMAL;
                    buffer[amount].line = line;
                    break;
                case TELNET:
                    if (c.subState == NORMAL) {
                        buffer[amount].type = IS_DATA;
                        buffer[amount].line = line;
                    } else {
                        buffer[amount].line = "";
                        c.fragment = line;
                    }
                    break;
        }
        amount++;
    }


    print_debug(DEBUG_DISPATCHER, "Done with dispatching");
}

QByteArray Cdispatcher::cutColours(QByteArray line)
{
    QByteArray res;
    int i;
    bool skip = false;

    for (i =0; i < line.length(); i++) {
        if (line.at(i) == '|' || line.at(i) == '-' || line.at(i) == '\\' || line.at(i) == '/')
            if ((i+1) < line.length() && line.at(i+1) == 0x8) {
                 i += 1; /*propeller char*/
                 continue;   /* and the next one and move on with the same check */
            }

        if (line.at(i) == 0xa || line.at(i) == 0xd)
            continue;                                       /* skip newlines */

        if (line.at(i) == 0x6d && skip) {
            skip = false;
            continue;
        }
        if (skip)
            continue;
        if (line.at(i) == 0x1b && line.at( i+1 ) == 0x5b) {
            skip = true;
            continue;
        }
        res.append(line.at(i));
    }

    return res;
}

#define SEND_EVENT_TO_ENGINE \
	{   \
	    print_debug(DEBUG_DISPATCHER, " ---- sending event ---- "); \
	    awaitingData = false;               \
	    engine->addEvent(event);                            \
	    event.clear();                  \
	    notify_analyzer();      \
	    xmlState = STATE_NORMAL;                       \
	    print_debug(DEBUG_DISPATCHER, " ---- sent event ---- "); \
    }


int Cdispatcher::analyzeMudStream(ProxySocket &c)
{
    int i;
    QByteArray scoreLine;

    print_debug(DEBUG_DISPATCHER, "analyzeMudStream(): starting");
    print_debug(DEBUG_DISPATCHER, "Buffer size %i", c.length);

    dispatchBuffer(c);
    c.clearBuffer();

    // else we simply recreate our buffer and parse lines
    for (i = 0; i< amount; i++) {

        //XML messages parser
        if (buffer[i].type == IS_XML) {

            print_debug(DEBUG_DISPATCHER, "--- XML data line ---");

            if (buffer[i].xmlType == XML_START_MOVEMENT) {
                if (awaitingData)
                    SEND_EVENT_TO_ENGINE;
                event.dir = buffer[i].line;
                event.movement = true;
                continue;
            } else if (buffer[i].xmlType == XML_START_ROOM) {
                if (awaitingData)
                    SEND_EVENT_TO_ENGINE;
                xmlState = STATE_ROOM;
                continue;
            } else if ((buffer[i].xmlType == XML_START_NAME) && (xmlState == STATE_ROOM)) {
                xmlState = STATE_NAME;
                continue;
            } else if ((buffer[i].xmlType == XML_START_DESC)  && (xmlState == STATE_ROOM)) {
                xmlState = STATE_DESC;
                continue;
            } else if ((buffer[i].xmlType == XML_START_TERRAIN)  && (xmlState == STATE_ROOM)) {
                event.blind = true;                 // BLIND detection
                continue;
            } else if (buffer[i].xmlType == XML_START_EXITS) {
                xmlState = STATE_EXITS;
                continue;
            } else if (buffer[i].xmlType == XML_START_PROMPT) {
                xmlState = STATE_PROMPT;
                continue;
            } else if (buffer[i].xmlType == XML_END_MOVEMENT) {
                // nada
                continue;
            } else if (buffer[i].xmlType == XML_END_ROOM && xmlState == STATE_ROOM) {
                awaitingData = true;
                xmlState = STATE_NORMAL;
                continue;
            } else if (buffer[i].xmlType == XML_END_NAME && xmlState == STATE_NAME) {
                event.name = cutColours(event.name);
                xmlState = STATE_ROOM;
                continue;
            } else if (buffer[i].xmlType == XML_END_DESC && xmlState == STATE_DESC) {
                event.desc.replace("\r\n", "|");
                event.desc = cutColours(event.desc);
                xmlState = STATE_ROOM;
                continue;
            } else if (buffer[i].xmlType == XML_END_EXITS && xmlState == STATE_EXITS) {
                xmlState = STATE_NORMAL;
                awaitingData = false;
                event.exits = cutColours(event.exits);
                if (event.exits.indexOf("Exits: ") == -1)
                    continue;
                event.exits.replace("Exits: ", "");
                SEND_EVENT_TO_ENGINE;
                continue;
            } else if (buffer[i].xmlType == XML_END_PROMPT) {
                if (scouting) {
                    event.scout = true;
                    scouting = false;
                    print_debug(DEBUG_DISPATCHER, "prompt, dropping the scouting flag");
                }
                event.prompt = cutColours(event.prompt);
                // TODO: this prompt setting might be dangerous and non-thread safe!
//                engine->setPrompt(event.prompt);
                last_prompt = event.prompt;
                proxy->sendPromptLineEvent(event.prompt);
                event.terrain = parseTerrain(event.prompt);
                SEND_EVENT_TO_ENGINE;
                xmlState = STATE_NORMAL;
                continue;
            }
            continue;
        }


        switch (xmlState) {
            case STATE_NAME :
                                                event.name.append(buffer[i].line);
                                                break;
            case STATE_DESC :
                                                event.desc.append(buffer[i].line);
                                                if (conf->getBriefMode())
                                                    continue;
                                                break;
            case STATE_EXITS:
                                                event.exits.append(buffer[i].line);
                                                break;
            case STATE_PROMPT:
                                                event.prompt.append(buffer[i].line);
                                                spells_print_mode = false;      // do not analyze spells anymore
                                                break;
        };

        // mbrief additional check (for look/scout and similar)
        if (mbrief_state == STATE_DESC && conf->getBriefMode())
            continue;

        //printf("xmlState: %i, buff type %i, line: ...%s...\r\n", xmlState, buffer[i].type, (const char *) buffer[i].line );
    	//fflush( stdout );

        if (xmlState == STATE_NORMAL && buffer[i].type == IS_NORMAL) {
            QByteArray a_line = cutColours( buffer[i].line );
            if (a_line == "") {
            	// skip empty lines.
            	c.append( buffer[i].line );
            	continue;
            }

//            printf("a_line: %s\r\n", (const char *) a_line );
//        	fflush( stdout );

            checkStateChange(a_line);

            if (Patterns::matchMoveCancelPatterns( a_line ) ) {
            	event.clear(); // it should actually be clear
            	event.movementBlocker = true;
            	SEND_EVENT_TO_ENGINE;
            }

            // check for fleeing/forced movement
            if (Patterns::matchMoveForcePatterns( a_line ) ) {
            	// the next event, i.e. the incoming movement will have the flag
            	event.fleeing = true;
            }

            // check for scouting
            if (a_line.startsWith("You quietly scout ") == true) {
                scouting = true;
                print_debug(DEBUG_DISPATCHER, "scouting detected, setting scouting flag up");
            }

            if (!c.isXmlMode()) {
                if (a_line == "Reconnecting." || a_line =="Never forget! Try to role-play..." || a_line == "<xml>") {
                    print_debug(DEBUG_DISPATCHER, "XML MODE IS ON!");
                    c.setXmlMode( true );
                }
            } else {
                if (a_line == "</xml>") {
                    print_debug(DEBUG_DISPATCHER, "XML MODE IS OFF!");
                    c.setXmlMode( false );
                }
            }

            // inform groupManager
            if (scoreExp.exactMatch(a_line) == true || scoreTrollExp.exactMatch(a_line)) {
            	proxy->sendScoreLineEvent(a_line);
            }


            // all necessary spells up/down/refresh lines checks
        	updateSpellsState(a_line);

        	QByteArray spellLine = checkAffectedByLine(a_line);
        	if (spellLine != "") {
        		c.append(spellLine);
        		continue; // do not print the old line then
        	}

            // changes the Affected by: output.
//            if (conf->spells_pattern == a_line) {
//            }

            static QRegExp statExp("Needed: * Alert: *.", Qt::CaseSensitive, QRegExp::Wildcard);
            static QRegExp lvl100statExp("Gold: *. Alert: *.", Qt::CaseSensitive, QRegExp::Wildcard);
            if (statExp.exactMatch(a_line) || lvl100statExp.exactMatch(a_line)) {
                spells_print_mode = true;   // print the spells data
            	c.append( buffer[i].line );
            	c.append( checkTimersLine() );
            	continue;
            }
        }

        print_debug(DEBUG_DISPATCHER, "Adding the line to the output buffer");
        c.append( buffer[i].line );
    }

    print_debug(DEBUG_DISPATCHER, "Done with this buffer. New length: %i", c.length);
    return c.length;
}


/* ======================= USER LAND ============================ */


/* new user input analyzer */
int Cdispatcher::analyzeUserStream(ProxySocket &c)
{
    int i;

    print_debug(DEBUG_DISPATCHER, "analyzeUserStream() starting");
    print_debug(DEBUG_DISPATCHER, "Buffer size %i", c.length);

    dispatchBuffer(c);
    c.clearBuffer();

    for (i = 0; i< amount; i++) {
        if (buffer[i].type == IS_NORMAL) {
            print_debug(DEBUG_DISPATCHER, "user input type : NORMAL");

            if (buffer[i].line.indexOf('\n') == -1) {
                if (i != (amount -1))
                // the input line got splitted somehow
//                printf("Adding fragment of the line to connection\r\n");
                c.fragment = buffer[i].line;
                continue;
            }

            buffer[i].line.replace("\r", "");
            buffer[i].line.replace("\n", "");
            memcpy(commandBuffer, buffer[i].line.constData(), buffer[i].line.length());
            commandBuffer[ buffer[i].line.length() ] = 0;

            print_debug(DEBUG_DISPATCHER, "calling Userland parser");
            if (userland_parser->parse_user_input_line(commandBuffer) == USER_PARSE_SKIP)
                continue;

            strcat(commandBuffer, "\r\n");

            static QByteArray GTellCommand = "tell Group";
            if (buffer[i].line.startsWith(GTellCommand) == true) {
            	// this is a GROUP-tell!
            	int len = buffer[i].line.size() - GTellCommand.length(); // 10 is length of "tell Group "
            	QByteArray data = buffer[i].line.right(len);
            	print_debug(DEBUG_GROUP, "Sending a G-tell from local user: %s", (const char *) data);
            	proxy->sendGroupTellEvent(data);
            	send_to_user("Ok.\r\n\r\n");
            	send_to_user(last_prompt);
//            	send_prompt();
            	continue;
            }



            print_debug(DEBUG_DISPATCHER, "recreating the output buffer");
            c.append(commandBuffer);
        } else {
            print_debug(DEBUG_DISPATCHER, "user input type : no parsing, just recreating the buffer");
            c.append(buffer[i].line);
        }
    }

    print_debug(DEBUG_DISPATCHER, "Done proceeding user input. Resulting buffer length: %i", c.length);
    return c.length;
}

char Cdispatcher::parseTerrain(QByteArray prompt)
{
    char terrain;

    terrain = prompt[1];  /*second charecter is terrain*/
    if (conf->getSectorByPattern(terrain) == 0)
        return -1;

    return terrain;
}


void Cdispatcher::checkStateChange(QByteArray line)
{
	// DEAD STATE
	// timer in CGroup turns this even off
	if (line == "You are dead! Sorry...") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::DEAD);
		return;
	}

	// BASHED STATE
	static QRegExp bashed("* sends you sprawling with a powerful bash.", Qt::CaseSensitive, QRegExp::Wildcard);
	if (bashed.exactMatch(line)) {
		printf("bash matches!\r\n");
		proxy->sendCharStateUpdatedEvent(CGroupChar::BASHED);
		return;
	}

	if (line == "Your head stops stinging.") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::STANDING);
	}

	// SLEEPING
	if (line == "You go to sleep." || line == "You feel very sleepy... zzzzzz" || line == "In your dreams, or what?") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::SLEEPING);
		return;
	}

	if (line == "You wake, and sit up.") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::RESTING);
		return;
	}

	// hmmm
//	if (line == "You feel less tired.") {
//		proxy->sendCharStateUpdatedEvent(CGroupChar::RESTING);
//		return;
//	}


	// RESTING
	if (line == "You sit down." || line == "You sit down and rest your tired bones.") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::RESTING);
		return;
	}

	if (line == "You stop resting, and stand up.") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::STANDING);
		return;
	}

	if (line == "You stand up.") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::STANDING);
		return;
	}



	// INCAP
	if (line == "You're stunned and will probably die soon if no-one helps you." ||
		line == "You are incapacitated and will slowly die, if not aided."	)
	{
		proxy->sendCharStateUpdatedEvent(CGroupChar::INCAP);
		return;
	}
	// the only way to leave incap is either to die or to improve from HP:Dying to something else!

	// DEAD
	if (line == "You are dead! Sorry...") {
		proxy->sendCharStateUpdatedEvent(CGroupChar::STANDING);
		return;
	}
}

void Cdispatcher::updateSpellsState(QByteArray line)
{
    for (unsigned int p = 0; p < conf->spells.size(); p++) {
        if ( (conf->spells[p].up_mes != "" && conf->spells[p].up_mes == line) ||
             (conf->spells[p].refresh_mes != "" && conf->spells[p].refresh_mes == line )) {
            print_debug(DEBUG_SPELLS, "SPELL %s Starting/Restaring timer.",  (const char *) conf->spells[p].name);
            conf->spells[p].timer.start();   // start counting
            conf->spells[p].up = true;
            conf->spells[p].silently_up = false;
            proxy->sendSpellsUpdatedEvent();
            break;
        }

        // if some spell is up - only then we check if its down
        if (conf->spells[p].up && conf->spells[p].down_mes != "" && conf->spells[p].down_mes == line) {
            conf->spells[p].up = false;
            conf->spells[p].silently_up = false;
            print_debug(DEBUG_SPELLS, "SPELL: %s is DOWN. Uptime: %s.", (const char *) conf->spells[p].name,
                                    qPrintable( conf->spellUpFor(p) ) );
            proxy->sendSpellsUpdatedEvent();
            break;
        }
    }

}

QByteArray Cdispatcher::checkAffectedByLine(QByteArray line)
{
    if (spells_print_mode && (line.length() > 3)) {
		for (unsigned int p = 0; p < conf->spells.size(); p++) {
			//printf("Spell name %s, line %s\r\n", (const char *) conf->spells[p].name, (const char*) a_line );
			if (line.indexOf(conf->spells[p].name) == 2) {
				QString s;
				if (conf->spells[p].up) {
					s = QString("- %1 (up for %2)\r\n")
						.arg( (const char *)conf->spells[p].name )
						.arg( conf->spellUpFor(p) );
				} else {
					s = QString("- %1 (unknown time)\r\n")
						.arg( (const char *)conf->spells[p].name );
					conf->spells[p].silently_up = true;
					proxy->sendSpellsUpdatedEvent();
				}

				return qPrintable(s);
			}
		}
    }


    return "";
}

QByteArray Cdispatcher::checkTimersLine()
{
    QString s = conf->timers.getStatCommandEntry();

    QString collect = "";

    for (unsigned int spell = 0; spell < conf->spells.size(); spell++)
        if (conf->spells[spell].addon && conf->spells[spell].up)
        {
            // there is a timer ticking
            collect += QString("- %1 (up for %2)\r\n")
                .arg( (const char *)conf->spells[spell].name )
                .arg( conf->spellUpFor(spell) );

        }

    if (collect != "")
    	s +=  "Addons:\r\n" + collect;


//    if (s != "")
//    	s+= "Normal spells:\r\n";
    return qPrintable(s);
}


