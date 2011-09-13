package jlome6d;

import java.io.*;
import java.nio.CharBuffer;

/**
 * /proc/uptime reader
 * @author warhog@gmx.de
 */
public class SystemUptime {

    public static Integer getUptime() {

	FileReader reader = null;
	try {
	    
	    reader = new FileReader("/proc/uptime");

	    // read 9 first characters
	    // this is more than enough for 10years+
	    char[] chars = new char[9];
	    reader.read(chars, 0, 9);

	    String uptime = String.valueOf(chars);
	    Integer posDot = uptime.indexOf(".");
	    if (posDot > 0)
		uptime = uptime.substring(0, posDot);

	    return Integer.parseInt(uptime);

	} catch (Exception e) {

	    return 0;
	    
	} finally {

	    try {

		if (reader != null)
		    reader.close();

	    } catch (Exception e) { }

	}
	
    }

}
