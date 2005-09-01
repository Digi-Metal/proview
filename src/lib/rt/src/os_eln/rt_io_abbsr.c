/* 
 * Proview   $Id: rt_io_abbsr.c,v 1.2 2005-09-01 14:57:56 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* rt_io_abbsr.c 
   Kommunikation med ABBs str�mrikare av typ PAD med
   kommunikations protokoll enl. ABB-DRIVES.  */

#include $vaxelnc
#include $elnmsg
#include $dda_utility
#include descrip

#include "pwr.h"
#include "rt_io_msg.h"

#include "rt_io_abbsr.h"
#include "rt_io_tty.h"

/****************************************************************************
* Namn          : io_abbsrkom(io_tt,padnr,indataP,utdataP)
*
* Typ           : pwr_tInt32			Returstatus.
*
* TYP             PARAMETER     IUGF    Beskrivning
*
* PORT		  io_tt		I	Eln PORT objekt som �r kopplad till en
*					seriell linas DDA port.
* pwr_tUInt8		  padnr		I	PAD nummer p� io_tt portens seriella 
*					linje ( 1 - 5 ).
* MESSAGE	  *tty_read_messobj
* char		  **tty_read_message
* MESSAGE	  *tty_write_messobj
* char		  **tty_write_message
* abbsr_indata	  *indataP	IU	Structadress inneh�llande data fr�n
*					str�mrikare. 
* abbsr_utdata	  *utdataP	IU	Structadress inneh�llande data till
*					str�mrikare. 
*
* Beskrivning   : Kommunikationen med ABB str�mrikare sker enligt ABBs besk-
*		  rivning: "Stromrichterger�ten PAD an Leitsysteme mit 
*		            RS 232-Schnittstellenkarte BX 5301          ".
*		  �verf�ringen av data enligt "normalbetieb ( zyklischer
*		  datenaustausch). Detta sker kortfattat p� f�ljande s�tt.
*	
*		  HOST					Str�mriktare
*		  ====                                  ============
*		  Steuerwort  	    --->   	
*					 <---		Statuswort
*		  Leitsollwert	    --->
*					 <---		St�rwort
*		  Steuerwort  	    --->
*					 <---		Drehzahlistwert
*		  Leitzusatsollwert --->
*					 <---		Wahlistwert
*
*		  
*		  N�r ett "beordringstelegram" s�nds fr�n HOSTen svarar 
*		  str�mrikaren med ett "svarstelegram ". I och med att det
*		  finns tre beordrings telegramtyper och fyra svarstelegram
*		  s�nds steuerwort ( vilken inneh�ller DO signaler ) tv�
*		  g�nger varje cykel. Denna funktion s�nder ned dessa tre
*		  telegram tills alla fyra svarstelegrammen har mottagits.
*		  V�rddatorn vet ej exakt i vilken ordning svarstelegrammen 
*		  kommer, s� ovan beskriva telegramcykel upprepas max tre
*		  g�nger. Har d� ej alla fyra svarstelegrammen mottagits 
*		  ges felkod tillbaka.
*
*
****************************************************************************/
pwr_tInt32 io_abbsrkom (PORT		io_tt,	
		   pwr_tUInt8	padnr,
		   MESSAGE	*tty_read_messobj,
		   char		**tty_read_message,
		   MESSAGE	*tty_write_messobj,
		   char		**tty_write_message,
		   abbsr_indata	*indataP,
		   abbsr_utdata	*utdataP)
                  
{

 pwr_tInt32          sts;                            	/* Returstatus    */

 pwr_tBoolean	statw,storw,drehw,wahlw;
 LARGE_INTEGER	 waittime = {-200000,-1};		/* wait 20 ms */
  

 pwr_tUInt8		mmess[9];
 pwr_tUInt8		smess[9];
 pwr_tUInt8		mbcc;
 pwr_tUInt16		madress,mvalue,addr,value;
 pwr_tInt32		i,ii,rcvant;

 

 /****************************************************************************/
 /** S�nd beordingstelegram tills all svarstelegram kommit, max tre cykler. **/
 /****************************************************************************/

 smess[0] = STX;

 for( statw = FALSE, storw = FALSE, drehw = FALSE, wahlw = FALSE, i = 0;
      i < 3 && !(statw && storw && drehw && wahlw );
      i++)
 {
     for ( ii = 1; ii < 5 ; ii++)
     {

         switch(ii)
	 {
	    case 1:
            case 3:

	      /** S�nd steuerwort till str�mrikaren 			    **/

	      addr  = 8 * padnr;			/* Antrieb adress    */
	      value = utdataP->steuerw;			/* V�rde	     */
	    break;

            case 2:

	      /** S�nd leitsollwert till str�mrikaren 			    **/

	      addr  = ( 8 * padnr ) + 2;		/* Antrieb adress    */
	      value = utdataP->leitsw;			/* V�rde	     */
	    break;

            case 4:

	      /** S�nd leitzusatzsollwert till str�mrikaren 		    **/

	      addr  = ( 8 * padnr ) + 3;		/* Antrieb adress    */
	      value = utdataP->leitzusatzsw;		/* V�rde	     */
	    break;
	 }

	 /** Konverterna adress och v�rde till HEX ASCII		    **/

 	 sprintf(&smess[1],"%02X",addr);       
 	 sprintf(&smess[3],"%04X",value);
	 smess[7] = CR;					/* L�gg till CR      */

	 /** Ber�kna checksumman BCC och l�gg till den.			    **/

	 smess[8] = io_abbsr_bcc(smess);

	 /** S�nd beordringstelegram och v�nta p� svars telegram             **/

 	 sts = io_tty_sdrc(&io_tt,smess,9,mmess,9,
		   tty_read_messobj,
		   tty_read_message,
		   tty_write_messobj,
		   tty_write_message,
 	   	   TIMEOUT,0,NULL,&rcvant);	/* JL 920123 */

         if(sts == ELN$_TIMEOUT) sts = IO__ABBSR_TIMEOUT; /* F�renkling d�
							   ELN$_TIMEOUT �r
							   success kodad     */ 
	 if(EVEN(sts))return(sts);

	 /** Kontrollera checksumman BCC p� svars telegrammet                **/

	 mbcc = io_abbsr_bcc(mmess);

	 if( mmess[8] != mbcc)
	 {
	    /* T|m typeaheadbuffer */
	    sts = io_tty_typecl(&io_tt,  
		  tty_read_messobj,
	          tty_read_message);
	    return IO__ABBSR_BCCERROR;
	 }
	 /** Utv�rdera svarstyp och returnera det konverterade v�rdet        **/
	
         sscanf(&mmess[1],"%2hx%4hx",&madress,&mvalue);


         switch( madress % 8 )
	 {
	    case STATUSWORT:
	      indataP->statusw = mvalue;	/* V�rde	     */
	      statw = TRUE;
	    break;

            case STORWORT:
	      indataP->storw = mvalue;		/* V�rde	     */
	      storw = TRUE;
	    break;

	    case DREHZAHLISTWERT:
	      indataP->drehzw = mvalue;		/* V�rde	     */
	      drehw = TRUE; 
	    break;

	    case WAHLISTWERT:
	      indataP->wahlistw = mvalue;	/* V�rde	     */
	      wahlw = TRUE;
	    break;
	 }			
     }/* for alla typer */
 }/* for alla svar */
 if(!(statw && storw && drehw && wahlw ))
    sts = IO__ABBSR_NOMESS;
  else
    sts = IO__ABBSR_SUCCESS;

 return(sts);	
}


/****************************************************************************
* Namn          : io_abbsr_bcc(buffer)
*
* Typ           : pwr_tUInt8			BCC utr�knad.
*
* TYP             PARAMETER     IUGF    Beskrivning
*
* pwr_tUInt8		  buffer[9]	IU	Character array f�r utr�kning av BCC.
*					
*
* Beskrivning   : Returnerar BCC ( 7 bitar )som �r summan av de 6 f�rsta 
*		  tecken tv�komplementerat.
*
*
****************************************************************************/
pwr_tUInt8 io_abbsr_bcc (pwr_tUInt8 buffer[])
                  
{
  static pwr_tUInt8	bcc,sum;
  static pwr_tInt32 i;

  for( i = 1, bcc = 0, sum = 0; i < 7 ; i++)
  {
     sum += buffer[i];
  }
  bcc = ~ sum;
  bcc++;
  bcc = bcc & 0x7f;    
  return(bcc);
}
