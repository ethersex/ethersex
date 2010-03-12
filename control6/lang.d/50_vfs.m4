#############################
# Vitual File System Helpers
#############################

#
# The Formatstring must be an constant
#
# VFS_LOG(filename, formatstring, args...)
#

define(`VFS_LOG', `vfs_file_append($1, uip_buf,
		  	snprintf_P(uip_buf, sizeof(uip_buf), 
			          PSTR($2), shift(shift($@))))')