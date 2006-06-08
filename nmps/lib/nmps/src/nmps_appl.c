/* 
 * Proview   $Id: nmps_appl.c,v 1.2 2006-06-08 04:28:12 claes Exp $
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

/*************************************************************************
 *
* 	PROGRAM		rs_nmps_appl
*
*       Modifierad
*		971002	Claes Sj�fors	Skapad
*
*	Funktion:	Applikationsgr�nssnitt mot Nmps.
**************************************************************************/

/*_Include filer_________________________________________________________*/

#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <libdef.h>
#include <starlet.h>
#include <lib$routines.h>
#endif

#ifdef OS_ELN
#include stdio
#include stdlib
#include string
#include math
#include float
#include libdef
#include lib$routines
#include starlet
#endif

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_nmpsclasses.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "rs_nmps_msg.h"
#include "nmps.h"
#include "nmps_appl.h"


/* Global functions________________________________________________________*/

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define	LogAndReturn( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2), NULL);\
  return status2;\
}

#define	Log( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2), NULL);\
}

#define	NMPS_CELLBUFF_SIZE		1000	/* This must be >= the size of
						   the largest cell */

typedef struct nmpsappl_s_data_list {
	pwr_tObjid			objid;
	pwr_tString80			name;
	unsigned int			possession;
	pwr_tAddress			object_ptr;
	gdh_tDlid			subid;
	int				remove;
	int				pending_remove;
	struct nmpsappl_s_data_list	*prev_ptr;
	struct nmpsappl_s_data_list	*next_ptr;
} nmpsappl_t_data_list;

typedef struct nmpsappl_s_basectx {
	nmpsappl_t_cellist	*cellist;
	int			cellist_count;
	nmpsappl_t_ctx		applctx_list;
	int			applctx_count;
	nmpsappl_t_data_list	*datalist;
} *nmpsappl_t_basectx;


/*_Globala variabler______________________________________________________*/

static nmpsappl_t_basectx nmpsappl_basectx = 0;

/*_Local functions________________________________________________________*/

static pwr_tStatus	nmpsappl_data_db_create(
			nmpsappl_t_data_list	**data_list,
			pwr_tObjid		objid,
			int			options,
			nmpsappl_t_data_list	**datalist_ptr);
static pwr_tStatus	nmpsappl_data_db_delete( 
			nmpsappl_t_data_list	**data_list, 
			nmpsappl_t_data_list	*data_ptr);



/****************************************************************************
* Name:		nmpsappl_data_db_create()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Create an entry in the datalist for an objid.
*
**************************************************************************/
static pwr_tStatus	nmpsappl_data_db_create(
			nmpsappl_t_data_list	**data_list,
			pwr_tObjid		objid,
			int			options,
			nmpsappl_t_data_list	**datalist_ptr)
{
  	nmpsappl_t_data_list	*next_ptr;
	pwr_tStatus		sts;
	char			name[80];
	pwr_sAttrRef		attrref;
	pwr_tAddress		object_ptr;
	gdh_tDlid		subid;

	/* Get a direct link to the original object */
	memset( &attrref, 0, sizeof(attrref));
	attrref.Objid = objid;
	sts = gdh_DLRefObjectInfoAttrref( &attrref, &object_ptr, &subid);
	if (EVEN(sts)) return sts;

	if ( options & nmpsappl_mOption_NamePath)
	  sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mName_path);
	else
	  sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mName_object);
	if ( EVEN(sts)) return sts;

	*datalist_ptr = calloc( 1, sizeof( nmpsappl_t_data_list));
	if ( *datalist_ptr == 0) return NMPS__NOMEMORY;

	(*datalist_ptr)->object_ptr = object_ptr;
	(*datalist_ptr)->subid = subid;
	(*datalist_ptr)->objid = objid;
	strcpy( (*datalist_ptr)->name, name);

	/* Insert first in list */
	next_ptr = *data_list;
	if ( next_ptr != NULL)
	  next_ptr->prev_ptr = *datalist_ptr;
	(*datalist_ptr)->next_ptr = next_ptr;

	*data_list = *datalist_ptr;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsappl_data_db_delete()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Delete an entry in the datalist.
*
**************************************************************************/
static pwr_tStatus	nmpsappl_data_db_delete( 
			nmpsappl_t_data_list	**data_list, 
			nmpsappl_t_data_list	*data_ptr)
{
	int	sts;

	sts = gdh_DLUnrefObjectInfo ( data_ptr->subid);

	if ( data_ptr == *data_list)
	{
	  /* Change the root */
	  *data_list = data_ptr->next_ptr;
	  if ( *data_list)
	    (*data_list)->prev_ptr = 0;
	}

	if ( data_ptr->prev_ptr )
	  data_ptr->prev_ptr->next_ptr = data_ptr->next_ptr;
	if ( data_ptr->next_ptr )
	  data_ptr->next_ptr->prev_ptr = data_ptr->prev_ptr;

	free( data_ptr);

	return NMPS__SUCCESS;
}



/****************************************************************************
* Name:		nmpsappl_cell_init()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tStatus	nmpsappl_cell_init( 
		char		*cell_name,
		pwr_tObjid	*orig_objid,
		void		**orig_cell,
		gdh_tSubid	*orig_subid,
		pwr_tClassId	*orig_class)
{
	pwr_sAttrRef		attrref;
	pwr_tStatus		sts;
	pwr_tObjid		cell_objid;

	sts = gdh_NameToObjid( cell_name, &cell_objid);
	if ( EVEN(sts)) return NMPS__APPLCELL;

	/* It might already have been stored */
	memset( &attrref, 0, sizeof(attrref));
	attrref.Objid = cell_objid;

	sts = gdh_GetObjectClass ( attrref.Objid, orig_class);
	if ( EVEN(sts)) return sts;

	/* Link to object */
	sts = gdh_DLRefObjectInfoAttrref( &attrref, 
			(pwr_tAddress *) orig_cell, orig_subid);
	if (EVEN(sts)) return NMPS__APPLCELL;

	return NMPS__SUCCESS;
}

/**
 *@brief Initierar spegling.  
 *
 * Med nmpsappl_MirrorInit initierar man en spegling genom att ange de celler som ska speglas. 
 * Man f�r till baka en context som anv�nds f�r att skilja olika speglingar �t. 
 * Efter initieringen anropas nmpsappl_Mirror cykliskt f�r att f� information om cellinneh�llet.
 * 
 * 
 *@return pwr_tStatus
 */
pwr_tStatus 
nmpsappl_MirrorInit(	
		    pwr_tString80 *cell_array, /**< En vektor som inneh�ller namnet p� de celler som ska speglas.
						  De speglade dataobjekten ordnas i den ordning cellerna anges i vektorn.
						  En NULL-string markerar sista cellnamnet. */ 
		    unsigned long options, /**< Options �r en bitmask. Bitarna anger olika funktioner f�r nmpsappl_mirror:
					      nmpsappl_mOption_Remove - Dataobjekt som har f�rsvunnit fr�n de angivna cellerna sedan senaste 
					      anropet l�ggs med i dataobjektlistan med remove-flaggan satt.
					      nmpsappl_mOption_NamePath - Namnf�ltet i den datastruktur som returneras f�r ett dataobjekt 
					      p� dataobjektet inneh�ller hierarkinamnet. */
		    nmpsappl_t_ctx *ctx /**< En kontext-pekare som skickas med till nmpsappl_mirror rutinen. */
)
{
	pwr_tString80		*cellname;
	nmpsappl_t_basectx 	basectx;
	nmpsappl_t_ctx		applctx;
	nmpsappl_t_cellist	*cellist_ptr;
	nmpsappl_t_cellist	*c_ptr;
	int			found;
	int			sts;
	int			maxsize;
	int			i;

	if ( nmpsappl_basectx == 0)
	{
	  /* Base context is not yet created */
	  nmpsappl_basectx = calloc( 1, sizeof( *nmpsappl_basectx));
	  if (nmpsappl_basectx == 0)
	    return 0;
	}
	basectx = nmpsappl_basectx;

	if ( basectx->applctx_count >= NMPSAPPL_APPLSESS_MAX)
	  return NMPS__APPLSESSMAX;

	/* Create a context for this applikation call */
	applctx = calloc( 1, sizeof( *applctx));
	if ( applctx == 0)
	  return 0;

	/* Insert applctx in basectx */
	applctx->next = basectx->applctx_list;
	basectx->applctx_list = applctx;
	applctx->index = basectx->applctx_count;
	applctx->index_mask = 1 << basectx->applctx_count;
	basectx->applctx_count++;

	*ctx = applctx;

	/* Add cells to cellist */
	cellname = cell_array;
	while ( strcmp( (char *) cellname, "") != 0)
	{
	  cdh_ToUpper( (char *) cellname, (char *) cellname);

	  /* Check if cell already is inserted */
	  cellist_ptr = basectx->cellist;
	  found = 0;
	  while ( cellist_ptr)
	  {
	    if ( strcmp( (char *) cellname, cellist_ptr->name) == 0)
	    {
	      found = 1;
	      break;
	    }
	    cellist_ptr = cellist_ptr->next;
	  }
	  if ( !found)
	  {
	    /* Insert cell last position in cellist */

	    cellist_ptr = calloc( 1, sizeof( nmpsappl_t_cellist));
	    strcpy( (char *) cellist_ptr->name, (char *) cellname);
	    sts = nmpsappl_cell_init( cellist_ptr->name,
		&cellist_ptr->objid,
		&cellist_ptr->object_ptr,
		&cellist_ptr->subid,
		&cellist_ptr->classid);
	    if (EVEN(sts)) return sts;

	    c_ptr = basectx->cellist;
	    if ( !c_ptr)
	      basectx->cellist = cellist_ptr;
	    else
	    {
	      while ( c_ptr->next)
	        c_ptr++;
	      c_ptr->next = cellist_ptr;
	    }
	    basectx->cellist_count++;
	  }
	  if ( applctx->cellist_count < NMPSAPPL_CELLIST_MAX )
	  {
	    applctx->cellist[ applctx->cellist_count] = cellist_ptr;
	    cellist_ptr->index_mask[ applctx->index] = 1 << applctx->cellist_count;
	    applctx->cellist_count++;
	  }
	  else
	    return NMPS__APPLCELLIST; /* Cellist to large */

	  cellname++;
	}

	/* Calculate the total size of the cell's in the list */
	applctx->total_cellsize = 0;
	for ( i = 0; i < applctx->cellist_count; i++)
	{
	  cellist_ptr = applctx->cellist[i];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	      maxsize = 
	 	((pwr_sClass_NMpsCell *) cellist_ptr->object_ptr)->MaxSize;
	      applctx->total_cellsize += maxsize;
	      cellist_ptr->tmp_size = sizeof( pwr_sClass_NMpsCell) - 
		sizeof( plc_t_DataInfo) * ( NMPS_CELL_SIZE - maxsize);
	      break;
	    case pwr_cClass_NMpsMirrorCell:
	      maxsize = 
	 	((pwr_sClass_NMpsMirrorCell *) cellist_ptr->object_ptr)->MaxSize;
	      applctx->total_cellsize += maxsize;
	      cellist_ptr->tmp_size = sizeof( pwr_sClass_NMpsMirrorCell) - 
		sizeof( plc_t_DataInfoMirCell) * ( NMPS_CELLMIR_SIZE - maxsize)-
		sizeof( nmps_t_mircell_copyarea);
	      break;
	  }
	  cellist_ptr->tmp_cell = (void *) calloc( 1, cellist_ptr->tmp_size);
	}

	/* Allocate memory for the data objects array in applctx */
	if ( options & nmpsappl_mOption_Remove)
	  /* Remove requires the dubbel size */ 
	  applctx->datainfo = 
		(nmpsappl_t_datainfo *) calloc( applctx->total_cellsize * 2, 
		sizeof(nmpsappl_t_datainfo));
	else
	  applctx->datainfo = 
		(nmpsappl_t_datainfo *) calloc( applctx->total_cellsize, 
		sizeof(nmpsappl_t_datainfo));

	applctx->options = options;
	
	return NMPS__SUCCESS;
}

/**
 *@brief Anropas cykliskt f�r att uppdatera speglingen.  
 *
 * Rutinen fyller i en lista p� samtliga dataobjekt som finns i cellerna. 
 * Listan �r av typen nmpsappl_t_datainfo. (l�nk till structdef?)
 *
 * nmpsappl_mirror speglar inneh�llet i en eller flera celler till ett applikationsprogram. 
 * Funktionen hanterar direktl�nkning av celler och dataobjekt och returnerar ej lista p� dataobjekt till applikationen, 
 * tillsammans med information om bakkant, framkant, utval, vilka objekt som �r nya eller har f�rsvunnit, vilken cell ett objekt tillh�r. 
 * Applikationen f�rses �ven med pekare till dataobjekten.
 * 
 * Man initierar en spegling genom att anropar rutinen nmpsappl_MirrorInit. 
 * Som argument skickar man med en lista p� de celler som ska speglas. 
 * Sedan anropas nmpsappl_Mirror cykliskt f�r att f� information om vilka dataobjekt som ligger i cellerna. 
 * Alla dataobjekt som finns i den angivna cellerna bakas ihop i en vektor, 
 * och ordningen best�mms av den ordning man angivit cellerna till nmpsappl_MirrorInit. 
 * 
 * Flera speglingar (max 32 st) kan hanteras i samma program, och varje spegling  omfattar maximalt 32 celler. 
 * Samma cell kan tillh�ra flera speglingar. 
 * Man kan t ex spegla samtliga celler i systemet i en array, samtidigt som man speglar enstaka celler, eller urval av celler i andra arrayer.
 * 
 * 
 * Exempel
 *
 * #include "pwr_inc:pwr.h"
 * #include "ssab_inc:rs_nmps_appl.h"
 * #include "pwrp_inc:pwr_cvolvkvendclasses.h"
 * 
 * main()
 * {
 *         nmpsappl_t_ctx     	ctx;
 *         int                 	i;
 *         int                 	plate_count;
 *         nmpsappl_t_datainfo 	*plate_info;
 *         pwr_tStatus          	sts;
 *         pwr_sClass_Plate     	*plate_ptr;
 *         pwr_tString80       	cellnames[] = {
 *                         "VKV-END-PF-Pl�tf�ljning-W-Svb15",
 *                         "VKV-END-PF-Pl�tf�ljning-W-R30A",
 *                         ""};
 * 
 *         // Init pams and gdh 
 *         sts = gdh_Init( 0, NULL);
 *         if (EVEN(sts)) LogAndExit( sts);
 * 
 *        // Init the mirroring 
 *         sts = nmpsappl_MirrorInit( cellnames,
 *  nmpsappl_mOption_Remove, &ctx);
 *         if (EVEN(sts)) exit(sts);
 * 
 *         for (;;)
 *         {
 *              sts = nmpsappl_Mirror( ctx, &plate_count,
 *  &plate_info);
 * 
 *           for ( i = 0; i < plate_count; i++)
 *          {
 *             plate_ptr = plate_info[i].object_ptr;
 *            printf( "%20s %1d %1d %1d %1d %1d %4d %5.1f\n",
 *                         plate_info[i].name,
 *                         plate_info[i].front,
 *                         plate_info[i].back,
 *                         plate_info[i].select,
 *                         plate_info[i].new,
 *                         plate_info[i].removed,
 *                         plate_info[i].cell_mask,
 *                         plate_ptr->Tjocklek);
 *           }
 *           sleep( 1);
 *         }
 * }
 *
 *@return pwr_tStatus
 */
pwr_tStatus 
nmpsappl_Mirror( 	
		nmpsappl_t_ctx applctx,        /**< En kontext-pekare som erh�lls vid anrop till nmpsappl_mirror_init. */ 
		int *data_count,               /**< Antal dataobjekt i dataobjekts-listan. */ 
		nmpsappl_t_datainfo **datainfo /**<Lista p� dataobjekten i cellerna. */
)
{
	int	i, j, k;
	int	found;
	nmpsappl_t_cellist	*cellist_ptr;
	nmpsappl_t_data_list	*data_ptr;
	nmpsappl_t_data_list	*data_next_ptr;
	int	sts;

	/* Copy cell-objects into the temporary buffer, and hope that
	   we will not be interuppted by the plc-program */
	for ( i = 0; i < applctx->cellist_count; i++)
	{
	  cellist_ptr = applctx->cellist[i];
	  memcpy( cellist_ptr->tmp_cell, cellist_ptr->object_ptr,
			cellist_ptr->tmp_size);
	}

	applctx->data_count = 0;
	for ( k = 0; k < applctx->cellist_count; k++)
	{
	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;
	      plc_t_DataInfo	*data_block_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->tmp_cell;
	      if ( !(object_ptr->ReloadDone & NMPS_CELL_INITIALIZED))
	        continue;

	      data_block_ptr = (plc_t_DataInfo *) &object_ptr->Data1P;
	      if ( applctx->options & nmpsappl_mOption_ReverseOrder)
		data_block_ptr += object_ptr->LastIndex - 1;
	      for ( i = 0; i < object_ptr->LastIndex; i++)
	      {
	        /* Check that objid is not already inserted */
	        found = 0;
	        for ( j = 0; j < applctx->data_count; j++)
	        {
	          if ( cdh_ObjidIsEqual( applctx->datainfo[j].objid, 
				data_block_ptr->Data_ObjId))
	          {
	            found = 1;
	            break;
	          }
	        }
	        if ( !found)
	        {
	          applctx->datainfo[applctx->data_count].objid = 
			data_block_ptr->Data_ObjId;
	          applctx->datainfo[applctx->data_count].front = 
			data_block_ptr->Data_Front;
	          applctx->datainfo[applctx->data_count].back = 
			data_block_ptr->Data_Back;
	          applctx->datainfo[applctx->data_count].select = 
			data_block_ptr->Data_Select;
	          applctx->datainfo[applctx->data_count].cell_mask = 
			cellist_ptr->index_mask[ applctx->index];
	          applctx->datainfo[applctx->data_count].removed = 0;
	          applctx->data_count++;
	        }
	        else
	        {
	          applctx->datainfo[applctx->data_count].front |= 
			data_block_ptr->Data_Front;
	          applctx->datainfo[applctx->data_count].back |= 
			data_block_ptr->Data_Back;
		  if ( data_block_ptr->Data_Select)
	            applctx->datainfo[applctx->data_count].select = 1;
	          applctx->datainfo[applctx->data_count].cell_mask |= 
			cellist_ptr->index_mask[ applctx->index];
	        }
		if ( applctx->options & nmpsappl_mOption_ReverseOrder)
		  data_block_ptr--;
		else
		  data_block_ptr++;
	      }
	      break;
	    }
	    case pwr_cClass_NMpsMirrorCell:
	    {
	      pwr_sClass_NMpsMirrorCell *object_ptr;
	      plc_t_DataInfoMirCell	*data_block_ptr;

	      object_ptr = (pwr_sClass_NMpsMirrorCell *) cellist_ptr->tmp_cell;

	      data_block_ptr = (plc_t_DataInfoMirCell *) &object_ptr->Data1P;
	      for ( i = 0; i < object_ptr->LastIndex; i++)
	      {
	        /* Check that objid is not already inserted */
	        found = 0;
	        for ( j = 0; j < i; j++)
	        {
	          if ( cdh_ObjidIsEqual( applctx->datainfo[j].objid, 
				data_block_ptr->Data_ObjId))
	          {
	            found = 1;
	            break;
	          }
	        }
	        if ( !found)
	        {
	          applctx->datainfo[applctx->data_count].objid = 
			data_block_ptr->Data_ObjId;
	          applctx->datainfo[applctx->data_count].select = 0;
	          applctx->datainfo[applctx->data_count].front = 0;
	          applctx->datainfo[applctx->data_count].back = 0;
	          applctx->datainfo[applctx->data_count].cell_mask = 
			cellist_ptr->index_mask[ applctx->index];
	          applctx->datainfo[applctx->data_count].removed = 0;
	          applctx->data_count++;
	        }
	        else
	        {
	          applctx->datainfo[applctx->data_count].cell_mask |= 
			cellist_ptr->index_mask[ applctx->index];
	        }
	        data_block_ptr++;
	      }
	      break;
	    }
	  }
	}

	/* Insert new data-object in the data db, or find the old ones */

	/* Set the remove flag for all the dataobjects of this applctx */
	data_ptr = nmpsappl_basectx->datalist;
	while( data_ptr)
	{
	  if ( data_ptr->possession & applctx->index_mask)
	    data_ptr->remove = 1;
	  data_ptr = data_ptr->next_ptr;
	}

	for ( i = 0; i <applctx->data_count; i++)
	{
	  /* Try to find the data-object */
	  found = 0;
	  data_ptr = nmpsappl_basectx->datalist;
	  while( data_ptr)
	  {
	    if ( cdh_ObjidIsEqual( applctx->datainfo[i].objid, data_ptr->objid))
	    {
	      if ( data_ptr->possession & applctx->index_mask)
	      {
	        /* It already belongs to this application session */
	        if ( data_ptr->pending_remove)
	          /* The object has reappleard */
	          data_ptr->pending_remove = 0;

		applctx->datainfo[i].object_ptr = data_ptr->object_ptr;
	        data_ptr->remove = 0;
	        applctx->datainfo[i].newdata = 0;
	      }
	      else
	      {
	        /* A new object for this session, mark possession */
	        if ( data_ptr->pending_remove)
	        {
	          /* The object has reappleard from another session */
	          data_ptr->pending_remove = 0;
	          data_ptr->possession = 0;
	        }
	        data_ptr->possession |= applctx->index_mask;
		applctx->datainfo[i].object_ptr = data_ptr->object_ptr;
	        applctx->datainfo[i].newdata = 1;
	        data_ptr->remove = 0;

	        /* Insert in new data list */
	      }
	      strcpy( applctx->datainfo[i].name, data_ptr->name);
	      found = 1;
	      break;
	    }
	    data_ptr = data_ptr->next_ptr;
	  }
	  if ( !found)
	  {
	    /* A new object, insert it into the data database */
	    sts = nmpsappl_data_db_create( &nmpsappl_basectx->datalist,
			applctx->datainfo[i].objid, applctx->options,
			&data_ptr);
	    if ( EVEN(sts)) return sts;

	    data_ptr->possession |= applctx->index_mask;
	    applctx->datainfo[i].object_ptr = data_ptr->object_ptr;
	    applctx->datainfo[i].newdata = 1;
	    strcpy( applctx->datainfo[i].name, data_ptr->name);

	    /* Insert in new data list */
	  }
	}

	/* Mark objects for remove that was previously possessed by this 
		session only */
	data_ptr = nmpsappl_basectx->datalist;
	while( data_ptr)
	{
	  if ( data_ptr->remove &&
	       data_ptr->possession & applctx->index_mask)
	  {
	    if ( data_ptr->possession == applctx->index_mask &&
	         data_ptr->pending_remove)
	    {
	      /* Remove the object from the database */
	      data_next_ptr = data_ptr->next_ptr;
	      sts = nmpsappl_data_db_delete( &nmpsappl_basectx->datalist,
			data_ptr);
	      if ( EVEN(sts)) return sts;
	      data_ptr = data_next_ptr;
	      continue;
	    }
	    else
	    {
	      /* Remove next scan */
	      if ( data_ptr->possession == applctx->index_mask)
	        data_ptr->pending_remove = 1;
	      else
	        data_ptr->possession &= ~applctx->index_mask;
	      if ( applctx->options & nmpsappl_mOption_Remove)
	      {
	        /* Add the removed to the list */
	        applctx->datainfo[applctx->data_count].objid = 
			data_ptr->objid;
	        applctx->datainfo[applctx->data_count].select = 0;
	        applctx->datainfo[applctx->data_count].front = 0;
	        applctx->datainfo[applctx->data_count].back = 0;
	        applctx->datainfo[applctx->data_count].cell_mask = 0;
	        applctx->datainfo[applctx->data_count].removed = 1;
	        applctx->datainfo[i].object_ptr = data_ptr->object_ptr;
	        applctx->datainfo[i].newdata = 0;
	        strcpy( applctx->datainfo[i].name, data_ptr->name);
	        applctx->data_count++;
	      }
	    }
	  }
	  data_ptr = data_ptr->next_ptr;
	}

	*datainfo = applctx->datainfo;
	*data_count = applctx->data_count;
	return NMPS__SUCCESS;
}

/**
 *@brief Tar bort ett dataobjekt ur de celler som speglas.  
 * 
 * 
 *@return pwr_tStatus
 */
pwr_tStatus
nmpsappl_RemoveData(
		    nmpsappl_t_ctx applctx, /**< Kontext-pekare som erh�lls vid anrop till nmpsappl_mirror_init. */
		    pwr_tObjid objid /**< Objid f�r dataobjekt som ska tas bort. */
)
{
	int			k;
	nmpsappl_t_cellist	*cellist_ptr;

	for ( k = 0; k < applctx->cellist_count; k++)
	{
	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTDELETE_OBJID;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	}
	return NMPS__SUCCESS;
}


/**
 *@brief Tar bort ett dataobjekt ur de celler som speglas. 
 * 
 * Tar bort ett dataobjekt ur de celler som speglas. Dessutom tas objektet bort ur databasen.
 * 
 *@return pwr_tStatus
 */
pwr_tStatus 
nmpsappl_RemoveAndDeleteData(
			nmpsappl_t_ctx applctx, /**< Kontext-pekare som erh�lls vid anrop till nmpsappl_mirror_init. */
			pwr_tObjid objid /**< Objid f�r dataobjekt som ska tas bort. */
)
{
	int			sts, k;
	nmpsappl_t_cellist	*cellist_ptr;

	for ( k = 0; k < applctx->cellist_count; k++)
	{
	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTDELETE_OBJID;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	}
	sts = gdh_DeleteObject(objid);
	if ( EVEN(sts)) return sts;

	return NMPS__SUCCESS;
}

pwr_tStatus	nmpsappl_SelectData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid)
{
	int			k;
	nmpsappl_t_cellist	*cellist_ptr;

	for ( k = 0; k < applctx->cellist_count; k++)
	{
	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTSELECT_OBJID;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	}

	return NMPS__SUCCESS;
}


pwr_tStatus	nmpsappl_TransportData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid,
			unsigned int		from_cell_mask,
			unsigned int		to_cell_mask)
{
	int			k;
	unsigned int		mask;
	nmpsappl_t_cellist	*cellist_ptr;

	/* Check that to cell is not busy or full */
	mask = 1;
	for ( k = 0; k < applctx->cellist_count; k++, mask = mask << 1)
	{
	  if (!( mask & to_cell_mask))
	    continue;

	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      if ( object_ptr->ExternFlag)
	        return NMPS__CELLEXTERNBUSY;
	      if ( object_ptr->CellFull)
	        return NMPS__CELLFULL;
	      break;
	    }
	    default:
	      break;
	  }
	  break;
	}

	/* Remove data first */
	mask = 1;
	for ( k = 0; k < applctx->cellist_count; k++, mask = mask << 1)
	{
	  if (!( mask & from_cell_mask))
	    continue;

	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      if ( object_ptr->ExternFlag)
	        return NMPS__CELLEXTERNBUSY;

	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTREMOVE_OBJID;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	}

	/* Insert data */
	mask = 1;
	for ( k = 0; k < applctx->cellist_count; k++, mask = mask << 1)
	{
	  if (!( mask & to_cell_mask))
	    continue;

	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      if ( object_ptr->ExternFlag)
	        return NMPS__CELLEXTERNBUSY;
	      if ( object_ptr->CellFull)
	        return NMPS__CELLFULL;

	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTINSERT;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	  break;
	}

	return NMPS__SUCCESS;
}

pwr_tStatus	nmpsappl_InsertData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid,
			unsigned int		cell_mask)
{
	int			k;
	unsigned int		mask;
	nmpsappl_t_cellist	*cellist_ptr;

	/* Insert data */
	mask = 1;
	for ( k = 0; k < applctx->cellist_count; k++, mask = mask << 1)
	{
	  if (!( mask & cell_mask))
	    continue;

	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      if ( object_ptr->ExternFlag)
	        return NMPS__CELLEXTERNBUSY;

	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTINSERT;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	  break;
	}

	return NMPS__SUCCESS;
}

pwr_tStatus	nmpsappl_RemoveAndKeepData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid,
			unsigned int		cell_mask)
{
	int			k;
	unsigned int		mask;
	nmpsappl_t_cellist	*cellist_ptr;

	/* Remove data first */
	mask = 1;
	for ( k = 0; k < applctx->cellist_count; k++, mask = mask << 1)
	{
	  if (!( mask & cell_mask))
	    continue;

	  cellist_ptr = applctx->cellist[k];
	  switch ( cellist_ptr->classid)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	      pwr_sClass_NMpsCell *object_ptr;

	      object_ptr = (pwr_sClass_NMpsCell *) cellist_ptr->object_ptr;
	      if ( object_ptr->ExternFlag)
	        return NMPS__CELLEXTERNBUSY;

	      object_ptr->ExternOpType = NMPS_OPTYPE_EXTREMOVE_OBJID;
	      object_ptr->ExternObjId  = objid;
	      object_ptr->ExternFlag   = 1;
	      break;
	    }
	    default:
	      break;
	  }
	}

	return NMPS__SUCCESS;
}

