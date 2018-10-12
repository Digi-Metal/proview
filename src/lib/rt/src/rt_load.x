#ifdef RPC_HDR
%/*
% * Proview   $Id: rt_load.x,v 1.6 2008-06-24 07:10:14 claes Exp $
% * Copyright (C) 2005 SSAB Oxel�sund AB.
% *
% * This program is free software; you can redistribute it and/or
% * modify it under the terms of the GNU General Public License as
% * published by the Free Software Foundation, either version 2 of
% * the License, or (at your option) any later version.
% *
% * This program is distributed in the hope that it will be useful
% * but WITHOUT ANY WARRANTY; without even the implied warranty of
% * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
% * GNU General Public License for more details.
% *
% * You should have received a copy of the GNU General Public License
% * along with the program, if not, write to the Free Software
% * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
% */
%
%#ifndef rt_load_h
%#define rt_load_h
%
%/* rt_load.h -- load data files
%
%   This file defines the data structures needed to create and read
%   load data files.  */
%
%#ifndef co_xdr_h
%# include "co_xdr.h"
%#endif
%
%#define load_cLffVersionStr	"V4.7.0"
%
%# define	load_cNameDirectory	"pwrp_load"
%# define	load_cDirectory		"$pwrp_load/"
%
%
%#define load_cNameVolume		"%sld_vol_%03d_%03d_%03d_%03d_%05d.dat"
%					/* ld_vol_<vid3>_<vid2>_<vid1>_<vid0>_<version>.dat  */
%
%
%
%
#endif

enum load_eSect {
  load_eSect__		= 0,
  load_eSect_Boot	= 1,
  load_eSect_CreObj	= 2,
  load_eSect_ChgObj	= 3,
  load_eSect_DelObj	= 4,
  load_eSect_End	= 5,
  load_eSect_File	= 6,
  load_eSect_ObjBody	= 7,
  load_eSect_ObjHead	= 8,
  load_eSect_Volume	= 9,
  load_eSect_VolRef	= 10,
  load_eSect_
};

enum load_eFile {
  load_eFile__		= 0,
  load_eFile_Boot	= 1,
  load_eFile_Diff	= 2,
  load_eFile_Volume	= 3,
  load_eFile_
};

#ifdef RPC_HDR
%
%typedef union {
%  pwr_tBitMask m;
%  pwr_32Bits (
%    pwr_Bits( created	, 1),
%    pwr_Bits( deleted	, 1),
%    pwr_Bits( father	, 1),
%    pwr_Bits( name	, 1),
%    pwr_Bits( body	, 1),
%    pwr_Bits( server	, 1),
%    pwr_Bits( flags	, 1),
%    pwr_Bits( fill0	, 1),
%
%    pwr_Bits( fill1	, 8),,,,,,,,
%    pwr_Bits( fill2	, 8),,,,,,,,
%    pwr_Bits( fill3	, 8),,,,,,,
%  ) b;
%
%# define load_mChange__	(0)
%# define load_mChange_created	pwr_Bit(0)
%# define load_mChange_deleted	pwr_Bit(1)
%# define load_mChange_father	pwr_Bit(2)
%# define load_mChange_name	pwr_Bit(3)
%# define load_mChange_body	pwr_Bit(4)
%# define load_mChange_server	pwr_Bit(5)
%# define load_mChange_flags	pwr_Bit(6)
%# define load_mChange_head	(load_mChange_father|load_mChange_name|load_mChange_body|load_mChange_server)
%# define load_mChange_		(~load_mChange__)
%
%} load_mChange;
%
%typedef union {
%  pwr_tBitMask m;
%  pwr_32Bits (
%    pwr_Bits( AliasClient	, 1),
%    pwr_Bits( MountClient	, 1),
%    pwr_Bits( MountClean	, 1),
%    pwr_Bits( fill0		, 5),,,,,
%
%    pwr_Bits( IO		, 1),
%    pwr_Bits( fill1		, 7),,,,,,,
%
%    pwr_Bits( fill2		, 8),,,,,,,,
%    pwr_Bits( fill3		, 8),,,,,,,
%  ) b;
%
%# define load_mFlags__			(0)
%# define load_mFlags_AliasClient	pwr_Bit(0)
%# define load_mFlags_MountClient	pwr_Bit(1)
%# define load_mFlags_IO		pwr_Bit(8)
%
%# define load_mFlags_Client		(load_mFlags_AliasClient|load_mFlags_MountClient)
%# define load_mFlags_			(~load_mFlags__)
%} load_mFlags;
%
%
%/* Nota Bene !!!
%   The constant load_cVersionFormat reflects the version of the load file
%   structure as a whole. It must be changed every time there is a change
%   in any of the structures of this file.  */
%
%#define load_cVersionFormat 3
%
%#define load_cVersionVolume 1
#endif

struct load_sVolume {
  pwr_tVolumeId		Id;
  pwr_tObjName		Name;
  pwr_tClassId		Class;
  pwr_tObjName		ClassName;
  pwr_tProjVersion	Version;
  pwr_tTime		CreTime;
  pwr_tUInt32		Cardinality;
  pwr_tUInt32		BodySize;
};

#ifdef RPC_HDR
%#define load_cVersionVolRef 1
#endif

struct load_sVolRef {
  pwr_tVolumeId		Id;
  pwr_tClassId		Class;
  pwr_tProjVersion	Version;
};

#ifdef RPC_HDR
%#define load_cVersionFile 2
#endif

struct load_sFile {
  pwr_tPwrVersion	PwrVersion;
  pwr_tVersion		FormatVersion;	/* version of load file format */
  load_eFile		FileType;
  pwr_tTime		CreationTime;
  pwr_tDbId		DbId;
  pwr_tUserId		UserId;
  pwr_tObjName		UserName;
  pwr_tString80		Comment;
};

#ifdef RPC_HDR
%#define load_cVersionHead  1
#endif

struct load_sHead {
  pwr_tVersion		HeadVersion;	/* Header version  */
  load_eSect		SectType;	/* Section type  */
  pwr_tVersion		SectVersion;	/* Section version  */
  pwr_tUInt32		SectSize;	/* Section size, not including
                                           this head  */
};

#ifdef RPC_HDR
%#define load_cVersionObjBody 2
#endif

struct load_sObjBody {
  pwr_tObjid		Objid;
  pwr_tUInt32		Offset;
  pwr_tUInt32		Size;
};

#ifdef RPC_HDR
%#define load_cVersionObjHead 3
%#define load_cVersionCreObj 3
%#define load_cVersionChgObj 3
%#define load_cVersionDelObj 3
#endif

#ifdef RPC_XDR
%
%bool_t
%xdr_load_mFlags(xdrs, objp)
%	XDR *xdrs;
%	load_mFlags *objp;
%{
%	if (!xdr_pwr_tBitMask(xdrs, (pwr_tBitMask *) &objp->m)) {
%		return (FALSE);
%	}
%	return (TRUE);
%}
%
%
%bool_t
%xdr_load_mChange(xdrs, objp)
%	XDR *xdrs;
%	load_mChange *objp;
%{
%	if (!xdr_pwr_tBitMask(xdrs, (pwr_tBitMask *) &objp->m)) {
%		return (FALSE);
%	}
%	return (TRUE);
%}
%
#endif

struct load_sObjHead {
  pwr_tObjName		Name;
  pwr_tObjid		Objid;
  pwr_tClassId		Class;
  pwr_tObjid		Father;
  pwr_tUInt32		Size;
  pwr_tObjid		Server;
  load_mFlags		Flags;
  pwr_tGeneration	HeadGeneration;
  pwr_tGeneration	BodyGeneration;
  load_mChange		Change;
};

#ifdef RPC_HDR
%#define load_cVersionEnd 3
%
%#endif
#endif
