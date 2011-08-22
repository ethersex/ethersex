package warhog.ethersex.ecmd;

import java.net.*;
import java.io.*;

/**
 * Connector class for ethersex ecmd
 * @author warhog@gmx.de
 */
public class Ecmd {

    private final String needsAuthString = "authentification required";
    private final String wrongAuth = "authentification failed";

    private boolean needsAuth = false;
    private InetAddress ipaddress = null;
    private Integer port = 0;
    private Socket sock = null;
    private PrintWriter streamOut = null;
    private BufferedReader streamIn = null;
    private String username = null;
    private String password = null;
    private Integer socketTimeoutMS = 1000;


    /**
     * Construct object<br>
     * Does a short test for reachability of the given
     * ip address.
     * @param ipaddress ip address to connect to
     * @param port port to connect to
     * @throws EcmdException 
     */
    public Ecmd(InetAddress ipaddress, Integer port) throws EcmdException {

	if (ipaddress == null)
	    throw new EcmdException("Invalid IP address.");

	if (port <= 0 || port > 65535)
	    throw new EcmdException("Invalid port range.");

	try {
	    
	    if (!ipaddress.isReachable(5000))
		throw new EcmdException("IP address is not reachable.");

	} catch (Exception e) {

	    throw new EcmdException(e.getMessage());

	}

	this.port = port;
	this.ipaddress = ipaddress;

    }


    /**
     * Connect to Ecmd device
     * @throws EcmdException
     */
    public void connect() throws EcmdException {

	this.password = null;
	this.username = null;
	this.needsAuth = false;
	
	try {

	    // connect socket and set socket options
	    this.sock = new Socket(this.ipaddress, this.port);
	    // no delay for socket operations
	    this.sock.setTcpNoDelay(true);
	    this.sock.setSoTimeout(this.socketTimeoutMS);

	    // get output stream and autoflush buffers on newline
	    this.streamOut = new PrintWriter(sock.getOutputStream(), true);
	    // grab input stream
	    this.streamIn = new BufferedReader(new InputStreamReader(sock.getInputStream()));

	    // do an authentification check of the ecmd device
	    this.checkAuth();

	} catch (UnknownHostException e) {

	    throw new EcmdException("Unknown host: " + this.ipaddress);

	} catch (Exception e) {

	    throw new EcmdException(e.getMessage());

	}

    }


    /**
     * close all streams and socket<br>
     * catches exceptions but doesnt throw them to the top class
     */
    public void close() {

	try {

	    if (this.streamIn != null)
		this.streamIn.close();

	    if (this.streamOut != null)
		this.streamOut.close();

	    if (this.sock != null && this.sock.isConnected())
		this.sock.close();

	} catch (Exception e) {

	    // do nothing :>

	}

    }



    /*
     * get socket connect state
     */
    public boolean isConnected() {

	return this.sock.isConnected();
	
    }


    /**
     * test if authentification for ecmd device is needed
     * @return true if authentification needed
     * @throws EcmdException
     */
    public boolean authNeeded() throws EcmdException {

	if (!this.isConnected())
	    throw new EcmdException("Cannot check for authentifaction on unconnected socket.");

	return this.needsAuth;
	
    }


    /**
     * checks whether authentification is needed or not<br>
     * this is achieved with a simple auth command without username
     * or password. if the ecmd target returns the "authentification needed"
     * string we need authentification :)<br><br>
     * authentification requirements can be read with authNeeded or from
     * return value
     * @return true if authentification is needed
     * @throws EmcdException
     */
    private boolean checkAuth() throws EcmdException {

	try {

	    // send auth command to see response
	    this.streamOut.println("auth");

	    if (this.isReady()) {
		
		// read ecmd response
		String response = this.streamIn.readLine();
		
		// if ecmd response is equal with needsAuthString
		// we simply need authentification
		if (response.equals(this.needsAuthString)) {

		    this.needsAuth = true;
		    return true;

		}

		this.needsAuth = false;
		return false;

	    } else
		throw new EcmdException("No response for auth test command.");

	} catch (Exception e) {

	    throw new EcmdException("Error during checkAuth.");

	}

    }



    /*
     * socket is ready function<br>
     * tests if socket has new data.<br>
     * only for internal usage.
     */
    private Boolean isReady() {

	try {

	    Long end = System.currentTimeMillis() + this.socketTimeoutMS;
	    while (System.currentTimeMillis() < end) {

		if (this.streamIn.ready() == true)
		    break;

	    }

	    if (this.streamIn.ready())
		return true;

	    return false;

	} catch (Exception e) {

	    return false;
	    
	}

    }


    /**
     * send ecmd command
     * @param command command to send
     * @param arguments variable count of arguments
     */
    public void sendCommand(String command, String... arguments) {

	if (command == null || command.length() == 0)
	    throw new IllegalArgumentException("No command given.");
	
	StringBuilder output = new StringBuilder(command);

	if (arguments != null || arguments.length > 0)
	    for (String argument : arguments)
		output.append(" ").append(argument);

	this.streamOut.println(output);

    }


    /**
     * get ecmd response
     * @return _ONE_ line of the ecmd response as String
     */
    public String getResponse() {

	try {

	    if (this.isReady())
		return this.streamIn.readLine();

	    return "";

	} catch (Exception e) {

	    return "";

	}

    }



    /**
     * authentificate at the connected ecmd device
     */
    public void authentificate() throws EcmdException {

	if (this.password.isEmpty() || this.username.isEmpty())
	    throw new EcmdException("Username or password empty!");

	try {

	    // print test command to see response
	    this.streamOut.println("auth " + this.username + " " + this.password);

	    if (this.isReady()) {

		String response = this.streamIn.readLine();
		if (response.equals(this.wrongAuth))
		    throw new EcmdException("Wrong username/password combination.");

	    }
	    
	} catch (Exception e) {

	    throw new EcmdException(e.getMessage());

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
     * @return the socketTimeoutMS
     */
    public Integer getSocketTimeoutMS() {
	return socketTimeoutMS;
    }

    /**
     * @param socketTimeoutMS the socketTimeoutMS to set
     */
    public void setSocketTimeoutMS(Integer socketTimeoutMS) {
	this.socketTimeoutMS = socketTimeoutMS;
    }
    

}
