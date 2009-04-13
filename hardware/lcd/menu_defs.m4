
menu(mediaplayer)
	action("Play", NULL)
	action("Stop", NULL)
	action("Lauter", NULL)
	action("Leiser", NULL)
endmenu

menu(root)
	submenu("Mediaplayer", mediaplayer)
	action("Test 1", NULL)
	action("Test 2", NULL)
endmenu
