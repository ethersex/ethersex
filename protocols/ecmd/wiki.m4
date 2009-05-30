dnl
dnl   Generate the MediaWiki syntax for the wikipage on ethersex.de ...
dnl
dnl ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
dnl
dnl   Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 3 as
dnl   published by the Free Software Foundation.
dnl  
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl  
dnl   You should have received a copy of the GNU General Public License
dnl   along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl  
dnl   For more information on the GPL, please go to:
dnl   http://www.gnu.org/copyleft/gpl.html
dnl
divert(-1)

define(`block', `dnl
footer
header($1)
')

define(`header', `dnl
== $1 ==
{| border="1"
| '''Command syntax'' 
| ''Short description''`
|-
')

define(`footer', `dnl
|}
')

define(`ecmd_feature', `dnl
ifelse($4,,,dnl
| translit($2,",) $3 
| $4
|-
)')

dnl Some dummy implementations of the ifdef cruft, to suppress those comments.
define(`ecmd_ifdef',)
define(`ecmd_ifndef',)
define(`ecmd_endif',)
define(`ecmd_else',)

divert(9)footer

[[Category:Ethersex]]
divert(0)
<div class="errorbox">
This page is automatically generated from ecmd_defs.m4 and wiki.m4 which are
part of the Ethersex source code repository.  Do not edit this page but send
patches for those files!
</div>

__NOTOC__
header(Network configuration)
