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


#include <cstdio>
#include <cstring>
#include <qregexp.h>

//#include <arpa/telnet.h>
//#define IAC 255

#include "defines.h"

#include <QMutex>


#include "CConfigurator.h"

#include "utils.h"
#include "CTree.h"
#include "CStacksManager.h"
#include "xml2.h"
#include "CDispatcher.h"
#include "CEngine.h"

#include "userfunc.h"
#include "proxy.h"
#include "CGroup.h"
#include "mainwindow.h"


Cdispatcher::Cdispatcher()
{
    xmlState = STATE_NORMAL;
    mbrief_state = STATE_NORMAL;
    awaitingData = false;
    scouting = false;
    event.clear();
}

/**
 *
 */
void Cdispatcher::parseXml(QByteArray tag)
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

        // hard tags first
        if (name == "movement") {
  //          printf("XML TAG: %s\r\n", TagTypes[p].name);
            startType = XML_START_MOVEMENT;
            endType = XML_END_MOVEMENT;

            if (param != "") {
                int index = param.indexOf("dir=");
                index += 4;
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
//        printf("Adding lost fragment, ...%s...", (const char *) c.fragment);
        line = c.fragment;
    }
//    printf("Subchars : ...%s...\r\n", (const char*) c.subchars);

    stop = (unsigned char *) (c.buffer + c.length);
    for (s = (unsigned char *) c.buffer;  s != stop; s++) {
//        printf("[ amount %i - m %i, s %i,  s %i, left %i ] \r\n", amount, c.mainState, c.subState, s, stop - s);
	switch (c.mainState) {
	   case NORMAL :
			switch (*s) {
			   case IAC:
										if (line != "") {
											buffer[amount].type = IS_NORMAL;
											buffer[amount++].line = line;
											line.clear();
										}
										c.mainState = TELNET;
										c.subState = T_GOTIAC;
										break;

				   case 0x0a :  // LF
//										printf("Appending %i : %c [newline] \r\n", s, *s);
										line.append(*s);
										buffer[amount].type = IS_NORMAL;
										buffer[amount++].line = line;
										line.clear();
										continue;
				   case '<'      :  // turns XML tag mode on
										if (c.isXmlMode()) {
											if (line != "") {
												buffer[amount].type = IS_NORMAL;
												buffer[amount++].line = line;
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
								parseXml(c.subchars);
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
//		printf("STUFFING state %i, char : %c, subchars %s \r\n", c.subState, *s, (const char *) c.subchars );
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
//        printf("Appending %i : %c\r\n", s, *s);
        line.append(*s);
    }

    c.fragment.clear();
//  printf("Amount : %i.\r\n", amount);
    if (line != "") {
        switch (c.mainState) {
                case NORMAL:
                case STUFFING:
                    buffer[amount].type = IS_NORMAL;
                    buffer[amount].line = line;
//                    printf("Normal finishing line: %s\r\n", (const char *) line);
                    break;
                case XML:
                    buffer[amount].type = IS_NORMAL;
                    buffer[amount].line = line;
//                    printf("** XML split detected, subchars saved for futher proceeding. Saved line : %s\r\n", (const char *) line);
                    break;
                case TELNET:
                    if (c.subState == NORMAL) {
                        buffer[amount].type = IS_DATA;
                        buffer[amount].line = line;
//                        printf("Normal telnet finishing line.\r\n");
                    } else {
                        buffer[amount].line = "";
//                        printf("Telnet seq SPLIT  detected!\r\n");
                        c.fragment = line;
                    }
                    break;
        }
        amount++;
    }


    print_debug(DEBUG_DISPATCHER, "Done with dispatching");

/*    printf("Dispatched buffer:\r\n");
    for (int i = 0; i < amount; i++)
        printf("Line type: %i, line len %i,  Line: %s\r\n", buffer[i].type, buffer[i].line.length(),
                        (const char *) buffer[i].line);
 */
}

QByteArray Cdispatcher::cutColours(QByteArray line)
{
    QByteArray res;
    int i;
    bool skip = false;

    print_debug(DEBUG_DISPATCHER, "Called cutColours");

    for (i =0; i < line.length(); i++) {
        if (line.at(i) == '|' || line.at(i) == '-' || line.at(i) == '\\' || line.at(i) == '/')
            if ((i+1) < line.length() && line.at(i+1) == 0x8) {
                 i += 1; /*properller char*/
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

    print_debug(DEBUG_DISPATCHER, "returning");
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
    int new_len;
    char *buf;

    // bloody hack!
    QRegExp scoreExp("[0-9]*/* hits, */* mana, and */* moves.");
    scoreExp.setPatternSyntax(QRegExp::Wildcard);
	// 399/529 hits and 121/133 moves.

    QRegExp scoreTrollExp("[0-9]*/* hits and */* moves.");
    scoreTrollExp.setPatternSyntax(QRegExp::Wildcard);

    print_debug(DEBUG_DISPATCHER, "analyzerMudStream(): starting");
    print_debug(DEBUG_DISPATCHER, "Buffer size %i", c.length);

    dispatchBuffer(c);

    buf = c.buffer;
    new_len = 0;

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
                engine->setPrompt(event.prompt);
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

        if (buffer[i].type == IS_NORMAL && buffer[i].line.indexOf("\r\n") != -1) {
            QByteArray a_line = cutColours( buffer[i].line );

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
            	printf("Caught score line!\r\n");
            	proxy->sendScoreLineEvent(a_line);
            }

            print_debug(DEBUG_DISPATCHER, "before spells checker");
            // now do all necessary spells checks
            {
                unsigned int p;

//                printf("Checking spells on %s.\r\n", a_line);
                for (p = 0; p < conf->spells.size(); p++) {
                    if ( (conf->spells[p].up_mes != "" && conf->spells[p].up_mes == a_line) ||
                         (conf->spells[p].refresh_mes != "" && conf->spells[p].refresh_mes == a_line )) {
                        print_debug(DEBUG_SPELLS, "SPELL %s Starting/Restaring timer.",  (const char *) conf->spells[p].name);
                        conf->spells[p].timer.start();   // start counting
                        conf->spells[p].up = true;
                        break;
                    }

                    // if some spell is up - only then we check if its down
                    if (conf->spells[p].up && conf->spells[p].down_mes != "" && conf->spells[p].down_mes == a_line) {
                        conf->spells[p].up = false;
                        print_debug(DEBUG_SPELLS, "SPELL: %s is DOWN. Uptime: %s.", (const char *) conf->spells[p].name,
                                                qPrintable( conf->spellUpFor(p) ) );
                        break;
                    }
                }



                if (spells_print_mode && (strlen(a_line) > 3)) {
                    // we are somewhere between the lines "Affected by:" and prompt
                    for (p = 0; p < conf->spells.size(); p++) {
                        //printf("Spell name %s, line %s\r\n", (const char *) conf->spells[p].name, (const char*) a_line );
                        if (a_line.indexOf(conf->spells[p].name) == 2) {
                            QString s;

                            if (conf->spells[p].up)
                                s = QString("- %1 (up for %2)\r\n")
                                    .arg( (const char *)conf->spells[p].name )
                                    .arg( conf->spellUpFor(p) );
                            else
                                s = QString("- %1 (unknown time)\r\n")
                                    .arg( (const char *)conf->spells[p].name );

                            memcpy(buf + new_len, qPrintable(s), s.length());
                            new_len += s.length();

                            break;
                        }
                    }
                    if (p != conf->spells.size())
                        continue; // dont print this line if we got a match for it


                }

            }

            // changes the Affected by: output.
            if (conf->spells_pattern == a_line) {
                unsigned int spell;
                QByteArray message = "Timers:";

                spells_print_mode = true;   // print the spells data
                // addon timers first
                memcpy(buf + new_len, (const char *) message, message.length());
                new_len += message.length();
                memcpy(buf + new_len, "\r\n", 2);
                new_len += 2;

                for (spell = 0; spell < conf->spells.size(); spell++)
                    if (conf->spells[spell].addon && conf->spells[spell].up) {
                        // there is a timer ticking
                        QString s;

                        s = QString("- %1 (up for %2)\r\n")
                            .arg( (const char *)conf->spells[spell].name )
                            .arg( conf->spellUpFor(spell) );

                        memcpy(buf + new_len, qPrintable(s), s.length());
                        new_len += s.length();
                        break;
                    }

            }

        }

        if (buffer[i].type == IS_PROMPT) {
            print_debug(DEBUG_DISPATCHER, "PROMPT recognized");
            spells_print_mode = false;      //
            engine->setPrompt(buffer[i].line);
            proxy->sendPromptLineEvent(buffer[i].line);
        }


        print_debug(DEBUG_DISPATCHER, "Adding the line to the output buffer");
        // recreating this line in buffer
        memcpy(buf + new_len, buffer[i].line, buffer[i].line.length());
        new_len += buffer[i].line.length();
    }

    print_debug(DEBUG_DISPATCHER, "Done with this buffer. New length: %i", new_len);
    return new_len;
}


/* ======================= USER LAND ============================ */


/* new user input analyzer */
int Cdispatcher::analyzeUserStream(ProxySocket &c)
{
    int i, result;
    int new_len, len;
    char *buf;
    CGroup *groupManager;

    groupManager = renderer_window->getGroupManager();


    buf = c.buffer;
    new_len = 0;

    print_debug(DEBUG_DISPATCHER, "abalyzeUserStream() starting");
    print_debug(DEBUG_DISPATCHER, "Buffer size %i", c.length);

//    printf("---------- user input -----------\r\n");

    dispatchBuffer(c);

    for (i = 0; i< amount; i++) {
        if (buffer[i].type == IS_NORMAL) {
            print_debug(DEBUG_DISPATCHER, "user input type : NORMAL");

            if (buffer[i].line.indexOf('\n') == -1) {
                if (i != (amount -1))
                    printf("LAST LINE HAD NO ENDING !\r\n, MIGHT BE A BUG!\r\n");
                // the input line got splitted somehow
//                printf("Adding fragment of the line to connection\r\n");
                c.fragment = buffer[i].line;
                continue;
            }

            buffer[i].line.replace("\r", "");
            buffer[i].line.replace("\n", "");
            memcpy(commandBuffer, buffer[i].line.constData(), buffer[i].line.length());
            len = buffer[i].line.length();
            commandBuffer[len] = 0;

            print_debug(DEBUG_DISPATCHER, "calling Userland parser");
            result = userland_parser->parse_user_input_line(commandBuffer);
            if (result == USER_PARSE_SKIP)
                continue;



            if (buffer[i].line.startsWith("tell Group") == true) {
            	// this is a GROUP-tell!
            	int len = buffer[i].line.size() - 11; // 10 is length of "tell Group "
            	QByteArray data = buffer[i].line.right(len);
            	print_debug(DEBUG_GROUP, "Sending a G-tell from local user: %s", (const char *) data);
            	proxy->sendGroupTellEvent(data);
            	send_to_user("Ok.\r\n\r\n");
            	send_prompt();
            	continue;
            }

            strcat(commandBuffer, "\r\n");

            print_debug(DEBUG_DISPATCHER, "recreating the output buffer");
            memcpy(buf + new_len, commandBuffer, strlen(commandBuffer));
            new_len += strlen(commandBuffer);
        } else {
            print_debug(DEBUG_DISPATCHER, "user input type : no parsing, just recreating the buffer");
            // No parsing, just put the line in buffer. recreating this line in buffer
            memcpy(buf + new_len, buffer[i].line, buffer[i].line.length());
            new_len += buffer[i].line.length();
        }
    }

    print_debug(DEBUG_DISPATCHER, "Done proceeding user input. Resulting buffer length: %i", new_len);
    return new_len;
}

char Cdispatcher::parseTerrain(QByteArray prompt)
{
    char terrain;

    terrain = prompt[1];  /*second charecter is terrain*/
    if (conf->getSectorByPattern(terrain) == 0)
        return -1;

    return terrain;
}
