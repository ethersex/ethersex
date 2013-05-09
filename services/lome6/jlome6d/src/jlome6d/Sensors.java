package jlome6d;

/**
 * lmsensors reader
 * @author warhog@gmx.de
 */
import java.io.*;

public class Sensors {

    private final String sensorsCmd = "sensors";
    private String cpuTemperatureString = "Core0 Temp:";
    private String sbTemperatureString = "temp1:";

    private Integer temperatureCPU = 0;
    private Integer temperatureSB = 0;

    
    public Sensors(String sensorCPU, String sensorSB) {

	this.cpuTemperatureString = sensorCPU;
	this.sbTemperatureString = sensorSB;

    }


    public void parseSensors() {

	try {

	    BufferedReader stdout = new BufferedReader(new InputStreamReader(Runtime.getRuntime().exec(this.sensorsCmd).getInputStream()));
	    String line = "";
	    while((line = stdout.readLine()) != null) {

		this.temperatureCPU = parseLine(line, cpuTemperatureString, this.getTemperatureCPU());
		this.temperatureSB = parseLine(line, sbTemperatureString, this.getTemperatureSB());

	    }

	    stdout.close();

	} catch (Exception e) {

	}

    }



    
    private Integer parseLine(String line, String startString, Integer originalValue) {

	if (line.startsWith(startString)) {

	    Integer positionStart = line.indexOf("+") + 1;
	    Integer positionEnd = line.indexOf("  ", positionStart) - 2;
	    Float temperature = Float.parseFloat(line.substring(positionStart, positionEnd));
	    temperature *= 10;
	    return temperature.intValue();

	}

	return originalValue;

    }

    /**
     * @return the temperatureCPU
     */
    public Integer getTemperatureCPU() {
	return temperatureCPU;
    }

    /**
     * @return the temperatureSB
     */
    public Integer getTemperatureSB() {
	return temperatureSB;
    }


}
