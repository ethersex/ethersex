package jlome6d;

import warhog.ethersex.lome6.*;
import warhog.util.*;

/**
 *
 * @author warhog@gmx.de
 */
public class jlome6d {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

	System.out.println("jlome6d v0.2");
	
	Lome6 lome6 = null;
	Sensors sensors = null;

	try {

	    EasyXMLConfigurator config = new EasyXMLConfigurator();
	    config.openConfigurationFile("config.xml");

	    String ipAddress = config.getXPathOption("configuration/ip/@address");
	    Integer port = Integer.parseInt(config.getXPathOption("configuration/port/@number"));
	    String username = config.getXPathOption("configuration/auth/@username");
	    String password = config.getXPathOption("configuration/auth/@password");
	    String sensorCPU = config.getXPathOption("configuration/sensors/@cpu");
	    String sensorSB = config.getXPathOption("configuration/sensors/@sb");

	    System.out.println("Using the following options:");
	    System.out.println("Host: " + ipAddress + ":" + port);
	    System.out.println("User: " + username);

	    sensors = new Sensors(sensorCPU, sensorSB);

	    lome6 = new Lome6(ipAddress, port, username, password);
	    lome6.connect();

	    while (true) {

		try {

		    sensors.parseSensors();
		    lome6.setTemperature(Type.CPU, sensors.getTemperatureCPU());
		    lome6.setTemperature(Type.SB, sensors.getTemperatureSB());
		    lome6.setUptime(SystemUptime.getUptime());

		} catch (Exception e) {

		    System.out.println(e.getMessage());
		    System.out.println(e.getCause());

		}

		Thread.sleep(5000);

	    }

	} catch (Lome6Exception e) {

	    if (e.isFatal()) {

		System.out.println("Fatal error: " + e.getMessage());

	    } else {

		System.out.println("Error: " + e.getMessage());
		
	    }
//	    e.printStackTrace();

	} catch (Exception e) {

//	    e.printStackTrace();
	    System.out.println(e.getMessage());

	} finally {

	    if (lome6 != null)
		lome6.close();

	}

    }


}
