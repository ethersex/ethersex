package warhog.ethersex.lome6;

/**
 *
 * @author warhog@gmx.de
 */
public class Lome6Exception extends Exception {

    private Boolean fatal = false;

    public Lome6Exception(String exception) {

        super(exception);

    }
    
    public Lome6Exception(String exception, Boolean fatal) {

	super(exception);
        this.fatal = fatal;

    }

    public Boolean isFatal() {

	return this.fatal;

    }

}
