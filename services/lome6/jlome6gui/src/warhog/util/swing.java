package warhog.util;

import java.awt.Component;
import java.awt.Container;

/**
 * additional utils for swing
 * @author warhog@gmx.de
 */
public class swing {

    /**
     * Set all component and all subcomponents of component to enabled/disabled
     * @param component component to set state
     * @param enabled state to set
     */
    public static void setDeepEnabled(Component component, boolean enabled) {

	// set component to state
	component.setEnabled(enabled);
	if (component instanceof Container) {

	    // get subcomponents of component and recursively
	    // call this function to set state of subcomponents
	    Container container = (Container)component;
	    for (int i = 0; i < container.getComponentCount(); i++)
		setDeepEnabled(container.getComponent(i), enabled);

	}

    }

}
