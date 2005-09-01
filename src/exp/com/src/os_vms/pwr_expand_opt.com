$! 
$! Proview   $Id: pwr_expand_opt.com,v 1.2 2005-09-01 14:57:49 claes Exp $
$! Copyright (C) 2005 SSAB Oxel�sund AB.
$!
$! This program is free software; you can redistribute it and/or 
$! modify it under the terms of the GNU General Public License as 
$! published by the Free Software Foundation, either version 2 of 
$! the License, or (at your option) any later version.
$!
$! This program is distributed in the hope that it will be useful 
$! but WITHOUT ANY WARRANTY; without even the implied warranty of 
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
$! GNU General Public License for more details.
$!
$! You should have received a copy of the GNU General Public License 
$! along with the program, if not, write to the Free Software 
$! Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
$!
$! pwr_expand_opt.com -- <short description>
$!
$! This command procedure expands indirect references
$! in a .OPT file, since the LINKer cannot do that
$! It just passes lines from input to output until a
$! @ sign is found in first position. It then opens
$! that file and does the same processing for that file
$! recursively.
$!
$! p1: input file
$! p2: output file
$!
$	set noon
$
$	tcpip = "# tcpip is not defined #"
$	if f$trnlnm("ucx$configuration") .nes. ""
$	then
$	  tcpip = "ucx"
$	else
$	  if f$trnlnm("tcpip$configuration") .nes. ""
$	  then
$	    tcpip = "tcpip"
$	  endif
$	endif
$
$	share = "<noshare>"
$	if f$trnlnm("pwr_eln_share") .nes. "" then share = "<  share>"
$
$ process_infile: subroutine
$	lvl = p2+1
$	inf := inf_'lvl'
$	plusses = f$fao("!''lvl'*+")
$	minuses = f$fao("!''lvl'*-")
$	open/read/error=err_open 'inf' 'p1'
$	write outf "!''plusses'"
$	write outf "! Start of inclusion of ""''p1'"""
$	write outf "!"
$ 10$:
$	read/end=eof 'inf' line
$	len = f$length(f$edit(line, "trim"))
$	if len .le. 0 then goto 10$
$	i = f$locate("!AS<TCPIP>", f$edit(line, "trim, upcase")) 
$	if i .lt. len then line = f$fao(line - "<tcpip>", tcpip)
$	i = f$locate("!''share'", line) 
$	if i .lt. len then line = "''line'" - "!''share'"
$	i = f$locate("!", f$edit(line, "trim"))
$	if i .eq. 0 then goto 10$
$	f = f$elem (1, "@", f$edit("''line'", "uncomment"))
$	if f .nes. "@"
$	then
$	  call process_infile 'f' 'lvl'
$	else
$	  write outf "''line'"
$	endif
$	goto 10$
$ eof:
$	close 'inf'
$	write outf "!"
$	write outf "! End of inclusion of ""''p1'"""
$	write outf "!''minuses'"
$	exit
$ err_open:
$	s = $status
$	write sys$output "Cannot open ""''p1'"" for inclusion"
$	write sys$output f$message(s)
$	write outf "!''plusses'"
$	write outf "! Cannot open ""''p1'"" for inclusion"
$	write outf "!''minuses'"
$	exit
$ endsubroutine
$!
$! Here begins the main code
$!
$	open/write/error=err_create outf 'p2'
$	call process_infile 'p1' 0
$	close outf
$	goto terminate
$
$ err_create:
$	write sys$output "Cannot open ""''p2'"" for output"
$	goto terminate
$
$ terminate:
