package warhog.ethersex.ecmd;

/**
 * ecmd exception class
 * @author warhog@gmx.de
 */
public class EcmdException extends Exception {

    private Boolean fatal = false;

    public EcmdException(String exception) {

        super(exception);

    }

    public EcmdException(String exception, Boolean fatal) {

	super(exception);
        this.fatal = fatal;

    }

    public Boolean isFatal() {

	return this.fatal;

    }

}
