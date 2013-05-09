package warhog.util;

import javax.xml.xpath.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;

/**
 *
 * @author warhog@gmx.de
 */
public class EasyXMLConfigurator {

    Document doc = null;
    
    public void openConfigurationFile(String configurationFile) throws Exception {

	DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance();
	domFactory.setNamespaceAware(true);
	DocumentBuilder builder = domFactory.newDocumentBuilder();
	this.doc = builder.parse(configurationFile);

    }


    /**
     * Get value from config file using XPath
     * @param expression XPath expression to use
     * @return String containing the value from the config file
     * @throws Exception on Error
     */
    public String getXPathOption(String expression) {

	try {

	    XPath xpath = XPathFactory.newInstance().newXPath();
	    XPathExpression expr = xpath.compile(expression);

	    Object result = expr.evaluate(this.doc, XPathConstants.NODESET);
	    NodeList nodes = (NodeList) result;

	    if (nodes.getLength() == 0 || nodes.getLength() > 1)
		return "";

	    return nodes.item(0).getNodeValue();

	} catch (Exception e) {

	    return "";

	}

    }

}
