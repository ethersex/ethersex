import sys
import random
import re
from xml.dom.ext.reader import Sax2
from xml.dom.ext import Print
from xml.dom.NodeFilter import NodeFilter

scripts = []
deleteNodes = []

def handle_text_node(node):
	text = node.firstChild.nodeValue.strip()
	if text.find("%%") != -1:
		pre = text[0:text.find("%%")]
		rest = text[text.find("%%"):]
		last = rest[2:].find("%%")
		if last == -1: return
		cmd = rest[2:last+2]
		post = rest[last+4:]
		if len(cmd) == 0: return
		if len(pre): 
			node.setAttribute("pre", pre)
		if len(post):
			node.setAttribute("post", post)

		id = "textNode" + str(random.randint(0,65354))
		node.setAttribute("id", id)
		scripts.append("%s|write %s"%(cmd,id))
		node.firstChild.nodeValue = pre +  "__" + post

	if text.find("{{") != -1:
		myscripts = text.replace("{{", "").split("}}")
		for i in myscripts:
			if len(i): scripts.append(i)
		deleteNodes.append(node)
	if text.find("##") != -1:
		ui_element = re.match("^.*##(.*)##.*$", text)
		if len(ui_element.groups()) == 0: return
		ui_element =  ui_element.groups()[0].strip()
		ui_element = ui_element.split(":", 1)
		if len(ui_element) >= 2:
			handle_ui_element(node,ui_element)
		deleteNodes.append(node)

def handle_ui_element(node, ui):
	ui_type = ui[0].strip()
	ui_id = ui[1].strip()
	if ui_type == "led":
		r = 0.5
		led = doc.createElement("circle")
		led.setAttribute("cx", str(float(node.getAttribute("x")) + r))
		led.setAttribute("cy", str(float(node.getAttribute("y")) - r))
		led.setAttribute("r", str(r))
		led.setAttribute("fill", "red")
		led.setAttribute("id", ui_id)
		node.parentNode.appendChild(led)

def generate_script(scripts):
	txt = '''var vars = new Object();
function ltrim(a) { 
	for(var k = 0; k < a.length && isWhitespace(a.charAt(k)); k++);
	return a.substr(k, a.length);
}
function rtrim(a) {
	for(var j=a.length-1; j>=0 && isWhitespace(a.charAt(j)) ; j--) ;
	return a.substr(0,j+1);
}
function trim(a) { return ltrim(rtrim(a)); }
function isWhitespace(charToCheck) {
	var whitespaceChars = " \\t\\n\\r\\f";
	return (whitespaceChars.indexOf(charToCheck) != -1);
}

String.prototype.strip = function () { return trim(this); }
function pipe(stdin, stdout) {
	var prog;
	if (stdout.length == 0) return;
	if (stdout.indexOf("|") != -1) {
		prog = stdout.substr(0, stdout.indexOf("|"));
		stdout = stdout.substr(stdout.indexOf("|")+1).strip();
	} else {
		prog = stdout;
		stdout = "";
	}
	var func, prog;
	if (prog.strip().indexOf(" ") != -1) {
		func = prog.strip().substr(0,prog.indexOf(" ")).strip();
		args = prog.strip().substr(prog.indexOf(" ") + 1).strip();
	} else {
		func = prog.strip();
		args = "";
	}
	eval(func + "('" + args + "', '" + stdin + "', '" + stdout +"')");
}

function echo(args, stdin, stdout) {
	pipe(args, stdout);
}
function store(args, stdin, stdout) { vars[args.strip()] = stdin; }
function get(args, stdin, stdout) { 
	if (vars[args.strip()] != undefined) 
		pipe(vars[args.strip()], stdout);
	else
		pipe("", stdout);
}
function warn(args, stdin, stdout) {alert(stdin);}
function periodic(args, stdin, stdout) {
  timeout = parseFloat(args);
  setInterval('pipe("", "'+stdout+'")', timeout * 1000);
  eval('pipe("", "'+stdout+'")');
}
function cut(args, stdin, stdout) {
	var a = args.split(" ");
	var input;
	stdin = stdin.replace(/\s+/g, " ");
	if (a.length > 1) 
		input = stdin.split(a[1]);
	else 
		input = stdin.split(" ");
	if (input.length < parseInt(a[0]) - 1) 
		pipe("", stdout);
	else 
		pipe(input[parseInt(a[0]) - 1], stdout);
}
function sub(args, stdin, stdout) { 
	var re = args.replace("^#|#$", "").strip().split('#');
	pipe(stdin.replace(re[0], re[1]),stdout);
}
function ecmd(args, stdin, stdout) {
	var url = "/ecmd?" + args;
	var handler = function(request, data) {
		pipe(request.responseText.strip(), data);
	}
	ArrAjax.aufruf(url, handler, "GET", stdout);
}
function bool_attr(args, stdin, stdout) {
	args = args.split(" ");
	if (args.length < 4) return;
	obj = document.getElementById(args[0]);
	if (!obj) return;
	if (parseInt(stdin)) 
		obj.setAttribute(args[1], args[2]);
	else
		obj.setAttribute(args[1], args[3]);
}

function to_bool(args,stdin,stdout) {
	var a = stdin.strip().toLowerCase();
	if (a == "on" || a == "true") pipe("1", stdout);
	else pipe("0", stdout);
}
function test (args,stdin,stdout) {
	if (stdin.strip().length == 0) return;
	if (eval(stdin + args)) pipe("1", stdout);
	else pipe("0", stdout);
}

function write(args, stdin, stdout) {
	var obj = document.getElementById(args);
	if (obj) {
		if (obj.hasAttribute("pre"))
			stdin = obj.getAttribute("pre") + stdin;
		if (obj.hasAttribute("post"))
			stdin = stdin + obj.getAttribute("post");
		obj.firstChild.nodeValue = stdin;
	}

}
'''

	for script in scripts:
		txt += "pipe('', '%s');\n"%(script)
	return txt



# create Reader object
reader = Sax2.Reader()

# parse the document
doc = reader.fromStream(sys.stdin)
walker = doc.createTreeWalker(doc.documentElement,
		NodeFilter.SHOW_ELEMENT, None, 0)


while 1:
	if walker.currentNode.tagName == "text":
		handle_text_node(walker.currentNode)
	next = walker.nextNode()
	if next is None: break

for node in deleteNodes:
	node.parentNode.removeChild(node)

	
# Add the scripting node
scr = doc.createElement("script")
scr.setAttribute("xlink:href","scr.js");
doc.documentElement.appendChild(scr)

scr = doc.createElement("script")
scrtext = doc.createTextNode(generate_script(scripts))
scr.appendChild(scrtext)
doc.documentElement.appendChild(scr)

Print(doc, sys.stdout)

