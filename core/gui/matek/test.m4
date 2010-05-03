SCENE(myscene)
	color = 0;
	BUFFER(text_field, 12)
	snprintf(text_field, 12, "23 %d", 42);
	PUTSTRING(text_field, 3, 8, 8, 2)
	PUTSTRING("1234567890", 3, 4, 8, 2) 
	color = 0x78;
	
	PUTSTRING(`"In computing, Virtual Network Computing is a graphical desktop sharing system that uses the RFB protocol to remotely control another computer. It transmits the keyboard and mouse events from one computer to another, relaying the graphical screen updates back in the other direction, over a network.  VNC is platformindependent a VNC viewer on one operating system may connect to a VNC server on the same or any other operating system. There are clients and servers for many GUI based operating systems and for Java."', 3, 10, 60, 10)


	gui_draw_circle(dest, 400, 400, 120, color, GUI_CIRCLE_FILL | GUI_CIRCLE_QUADRANT4 | GUI_CIRCLE_QUADRANT2);
	gui_draw_circle(dest, 400, 400, 124, color, GUI_CIRCLE_FULL);


SCENE_END()
