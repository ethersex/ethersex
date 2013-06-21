package jlome6gui;

import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.JOptionPane;

/**
 *
 * @author warhog@gmx.de
 */
public class Jlome6gui {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

	try {
	    
	    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());

	} catch (UnsupportedLookAndFeelException e) {

	    JOptionPane.showMessageDialog(null, "Cannot set application theme to system theme.", "Theme error", JOptionPane.ERROR_MESSAGE);

	} catch (Exception e) {

	    JOptionPane.showMessageDialog(null, e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
	    System.exit(1);

	}

	MainWindow window = new MainWindow();
	window.setVisible(true);

    }
}
