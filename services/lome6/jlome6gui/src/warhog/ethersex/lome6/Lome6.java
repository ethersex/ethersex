package warhog.ethersex.lome6;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import warhog.ethersex.ecmd.*;

/**
 *
 * @author warhog@gmx.de
 */
public class Lome6 {

    private String username = "";
    private String password = "";
    private String host = "";
    private Integer port = 2701;
    Ecmd ecmd = null;

    
    
    /**
     * Constructor without arguments
     */
    public Lome6() {

    }

    /**
     * Constructor with host argument
     * @param host host address to connect to
     */
    public Lome6(String host) {
	this.host = host;
    }

    /**
     * Constructor with host and port argument
     * @param host host address to connect to
     * @param port port to connect to
     */
    public Lome6(String host, Integer port) {
	this.host = host;
	this.port = port;
    }

    /**
     * Constructor with host, port, username and password argument
     * @param host host address to connect to
     * @param port port to connect to
     * @param username username to authentificate with
     * @param password password to authentificate with
     */
    public Lome6(String host, Integer port, String username, String password) {
	
	this.host = host;
	this.port = port;
	if (!username.isEmpty())
	    this.username = username;
	if (!password.isEmpty())
	    this.password = password;

    }


    /**
     * connect to lome6 device<br>
     * set connection options through constructors or setter functions
     * @throws Lome6Exception 
     */
    public void connect() throws Lome6Exception {

	try {
	    
	    ecmd = new Ecmd(InetAddress.getByName(this.host), this.port);
	    ecmd.connect();

	    if (ecmd.authNeeded()) {

		// we need to authentificate
		ecmd.setUsername(this.username);
		ecmd.setPassword(this.password);
		ecmd.authentificate();

	    }

	} catch (UnknownHostException e) {

	    throw new Lome6Exception("Unknown host: " + this.host);

	} catch (EcmdException e) {

	    throw new Lome6Exception(e.getMessage());

	}
	
    }


    /**
     * close connection to lome6 device
     */
    public void close() {

	if (this.ecmd != null)
	    this.ecmd.close();

    }



    /**
     * get power state from lome6 device
     * @return true if server is powered on
     * @throws Lome6Exception 
     */
    public Boolean getPowerState() throws Lome6Exception {

	if (this.ecmd == null)
	    throw new Lome6Exception("Lome6 not connected!");

	this.ecmd.sendCommand("lome6", "state");
	if (this.ecmd.getResponse().equals("on"))
	    return true;

	return false;

    }


    /**
     * trigger power button of server for short period
     * @throws Lome6Exception
     */
    public void triggerPower() throws Lome6Exception {

	this.ecmd.sendCommand("lome6", "power");
	if (!this.ecmd.getResponse().equals("OK"))
	    throw new Lome6Exception("Command error.");
	
    }

    
    
    /**
     * trigger power button of server for long period
     * @throws Lome6Exception 
     */
    public void triggerPowerLong() throws Lome6Exception {

	this.ecmd.sendCommand("lome6", "power", "long");
	if (!this.ecmd.getResponse().equals("OK"))
	    throw new Lome6Exception("Command error.");

    }


    
    /**
     * trigger reset button of server
     * @throws Lome6Exception 
     */
    public void triggerReset() throws Lome6Exception {

	this.ecmd.sendCommand("lome6", "reset");
	if (!this.ecmd.getResponse().equals("OK"))
	    throw new Lome6Exception("Command error.");

    }



    /**
     * get uptime of server
     * @return server uptime in seconds
     */
    public Integer getUptime() {

	try {

	    this.ecmd.sendCommand("lome6", "uptime");
	    return Integer.parseInt(this.ecmd.getResponse());

	} catch (Exception e) {

	    return 0;

	}

    }



    /**
     * get uptime of lome6
     * @return lome6 uptime in seconds
     */
    public Integer getLome6Uptime() {

	try {

	    this.ecmd.sendCommand("whm");

	    Pattern pattern = Pattern.compile("([\\d]{1,}):([\\d]{1,2})");
	    Matcher matcher = pattern.matcher(this.ecmd.getResponse());

	    Integer seconds = 0;

	    if (matcher.find()) {

		Integer hours = Integer.parseInt(matcher.group(1));
		Integer minutes = Integer.parseInt(matcher.group(2));
		seconds = (hours * 60 * 60) + minutes * 60;

	    }

	    return seconds;

	} catch (Exception e) {

	    return 0;

	}

    }



    /**
     * set server uptime
     * @param uptime in seconds
     * @throws Lome6Exception
     */
    public void setUptime(Integer uptime) throws Lome6Exception {

	this.ecmd.sendCommand("lome6", "uptime", uptime.toString());
	if (!this.ecmd.getResponse().equals("OK"))
	    throw new Lome6Exception("Command error.");

    }



    /**
     * set temperature with lome6 set_t command
     * @param type temperature sensor to set (cpu, sb)
     * @param temperature temperature in decidegree
     * @throws Lome6Exception
     */
    public void setTemperature(String type, Integer temperature) throws Lome6Exception{

	this.ecmd.sendCommand("lome6", "set_t", type, temperature.toString());
	if (!this.ecmd.getResponse().equals("OK"))
	    throw new Lome6Exception("Command error.");

    }


    /**
     * get temperature sensor
     * @param type temperature sensor to get (cpu, sb, air, ram, psu)
     * @return temperature in decidegree
     */
    public Integer getTemperature(String type) {

	try {

	    this.ecmd.sendCommand("lome6", "get_t", type);
	    Float temperature = Float.parseFloat(this.ecmd.getResponse());
	    temperature *= 10;
	    return temperature.intValue();

	} catch (Exception e) {

	    return 0;

	}

    }


    /**
     * @return the username
     */
    public String getUsername() {
	return username;
    }

    /**
     * @param username the username to set
     */
    public void setUsername(String username) {
	this.username = username;
    }

    /**
     * @return the password
     */
    public String getPassword() {
	return password;
    }

    /**
     * @param password the password to set
     */
    public void setPassword(String password) {
	this.password = password;
    }

    /**
     * @return the host
     */
    public String getHost() {
	return host;
    }

    /**
     * @param host the host to set
     */
    public void setHost(String host) {
	this.host = host;
    }

    /**
     * @return the port
     */
    public Integer getPort() {
	return port;
    }

    /**
     * @param port the port to set
     */
    public void setPort(Integer port) {
	this.port = port;
    }

    
}
