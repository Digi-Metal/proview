#include "rt_rtt_menu.h"
#include "dtt_appl_rttsys_m.rdecl"

RTT_HELP_START

RTT_HELP_SUBJ("PWR_RTT")
RTT_HELP_INFO("  V�lj ut �nskat �mne med piltangenterna och tryck p� RETURN")
RTT_HELP_TEXT( "\
	Pwr_rtt visar info om databasen.\n\r\n\r\
OBJECT HIERARCHY	Visar anl�ggings och node hierarkin.\n\r\
ALARM LIST		Visar larmlistan.\n\r\
EVENTLIST		Visar h�ndelselistan.\n\r\
EXIT			Avsluta.")

RTT_HELP_SUBJ("RTT ALARM LIST")
RTT_HELP_INFO("  PF1 visa objekt                   PF3 kvittera           PF4 �terg�")
RTT_HELP_TEXT( "\
	Alarmlist visar larmlistan.\n\r\n\r\
	** 	markerar att larmet �r aktivt.\n\r\
	!! 	markerar att larmet �r okvitterat.\n\r\
	A,B,C,D eller I anger larmets prioritet.\n\r\n\r\
	Larm-namn f�r utvalt larm visas med PF1, om larm-namnet �r ett objekt\n\r\
	�ppnas objektet.\n\r\
	Samtliga okvitterade larm kvitteras med PF3.\n\r\
	G� ur larmlistan med PF4")

RTT_HELP_SUBJ("RTT EVENT LIST")
RTT_HELP_INFO("                                                       PF4 �terg�")
RTT_HELP_TEXT( "\
	Eventlist visar h�ndelselistan.\n\r\n\r\
	*A,*B,C,D eller I anger larmets prioritet.\n\r\
	r	markerar tid f�r retur av larmstatus.\n\r\
	a	markerar tid f�r kvittens.\n\r\n\r\
	G� ur h�ndelselistan med PF4")

RTT_HELP_END

RTT_DB_START
RTT_DB_CONTINUE
RTT_DB_END

RTT_MENU_START( dtt_menu_m8)
RTT_MENU_SYSEDIT_NEW( "SHOW NODES", 0, &RTTSYS_SHOW_NODES)
RTT_MENU_SYSEDIT_NEW( "SHOW SUBSCRIPTION CLIENT", 0, &RTTSYS_SHOW_SUBCLI)
RTT_MENU_SYSEDIT_NEW( "SHOW SUBSCRIPTION SERVER", 0, &RTTSYS_SHOW_SUBSRV)
RTT_MENU_END( dtt_menu_m8)

RTT_MENU_START( dtt_menu_m10)
RTT_MENU_SYSEDIT_NEW( "QCOM NODES", 0, &RTTSYS_QCOM_NODES)
RTT_MENU_SYSEDIT_NEW( "QCOM APPLICATIONS", 0, &RTTSYS_QCOM_APPL)
RTT_MENU_END( dtt_menu_m10)

RTT_MENU_START( dtt_menu_m9)
RTT_MENU_NEW( "NETHANDLER", dtt_menu_m8)
RTT_MENU_NEW( "QCOM", dtt_menu_m10)
RTT_MENU_SYSEDIT_NEW( "REMNODE", 0, &RTTSYS_REMNODE)
RTT_MENU_SYSEDIT_NEW( "REMTRANS", 0, &RTTSYS_REMTRANS)
RTT_MENU_END( dtt_menu_m9)

RTT_MENU_START( dtt_menu_m6)
RTT_MENU_SYSEDIT_NEW( "SHOW SYSTEM", 0, &RTTSYS_SHOW_SYS)
RTT_MENU_SYSEDIT_NEW( "SHOW POOL", 0, &RTTSYS_POOLS)
RTT_MENU_NEW( "COMMUNICATION", dtt_menu_m9)
RTT_MENU_SYSEDIT_NEW( "PLCPGM", 0, &RTTSYS_PLCPGM)
RTT_MENU_SYSEDIT_NEW( "GRAFCET", 0, &RTTSYS_GRAFCET)
RTT_MENU_SYSEDIT_NEW( "DEVICE", 0, &RTTSYS_DEVICE)
RTT_MENU_SYSEDIT_NEW( "PLCTHREAD", 0, &RTTSYS_PLCTHREAD)
RTT_MENU_SYSEDIT_NEW( "PID", 0, &RTTSYS_PID)
RTT_MENU_SYSEDIT_NEW( "RUNNINGTIME", 0, &RTTSYS_RUNNINGTIME)
RTT_MENU_SYSEDIT_NEW( "LOGGING", 0, &RTTSYS_LOGGING)
RTT_MENU_END( dtt_menu_m6)

RTT_MAINMENU_START( "PWR_RTT/RTT-'RTT_NODE'-'RTT_SYS'")
RTT_MENUITEM_OBJECTS( "OBJECT HIERARCHY")
RTT_MENUITEM_COMMAND( "ALARM LIST", "ALARM SHOW")
RTT_MENUITEM_COMMAND( "EVENT LIST", "ALARM LIST")
RTT_MENU_NEW( "SYSTEM", dtt_menu_m6)
RTT_MENUITEM_COMMAND("STORE", "SHOW FILE")
RTT_MENUITEM_EXIT( "EXIT")
RTT_MAINMENU_END
