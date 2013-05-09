#############################
# Vitual File System Helpers
#############################

#
# The Formatstring must be an constant
#
# VFS_LOG(filename, formatstring, args...)
# VFS_LOG_ALLOCA(filename, buflength, formatstring, args...)

define(`VFS_LOG', `vfs_file_append($1, uip_buf,
		  	snprintf_P(uip_buf, sizeof(uip_buf), 
			          PSTR($2), shift(shift($@))))')

define(`VFS_LOG_ALLOCA', `{
char *buf = __builtin_alloca($2);
vfs_file_append($1, (void *)buf,
                snprintf_P((void *) buf, $2,
			               PSTR($3), shift(shift(shift($@)))));
}')
