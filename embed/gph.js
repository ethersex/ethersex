var vboxh = 300;
var vboxw = 400;

function Graph(id,num,min,max) {
    this.obj = returnObjById(id);
    this.xmult = vboxw / num;
    this.min = min;
    this.max = max;
    this.last_y = -1;
    this.elements = 0;

    return this;
}

function addLine (obj, x1, y1, x2, y2) {
    var ne = document.createElementNS("http://www.w3.org/2000/svg", "line");
    ne.setAttribute ("x1", x1);
    ne.setAttribute ("x2", x2);
    ne.setAttribute ("y1", y1);
    ne.setAttribute ("y2", y2);

    obj.appendChild (ne);
    return ne;
}

function addText (obj, x, y, text) {
    var ne = document.createElementNS("http://www.w3.org/2000/svg", "text");
    ne.setAttribute ("x", x);
    ne.setAttribute ("y", y);

    var t = document.createTextNode(text);
    ne.appendChild (t);
    obj.appendChild (ne);
    return ne;
}

function graphAppend(g,val) {
    val = vboxh - ((val - g.min) * vboxh / (g.max - g.min));

    if (g.last_y != -1) {
	var w = (g.elements + 1) * g.xmult;
	addLine (g.obj, g.elements * g.xmult, g.last_y, w, val);
	g.elements ++;

	if (w > vboxw) {
	    g.obj.removeChild (g.obj.getElementsByTagName("line")[0]);
	    g.obj.setAttribute("transform", "translate(-" + (w - vboxw) + ",0)");
	}
    }

    g.last_y = val;
}

function graphCreateAxis(id,tid,min,max) {
  var obj = returnObjById(id);
  var tobj = returnObjById(tid);

  var x;
  for (x = 0; x <= vboxw; x += vboxw / 4) {
    addLine (obj, x, 0, x, vboxh);
  }

  var scale = 10;
  for (; (max - min) / scale > 10; scale *= 2);

  var i = min;
  if (i % scale) { i += scale - (min % scale); }
  for (; i <= max; i += scale) {
    var y = vboxh - ((i - min) * vboxh / (max - min));
    addLine (obj, 0, y, vboxw, y);
    addText (tobj, 0, y, i);
  }

  addLine (obj, 0, 0, vboxw, 0);
  addLine (obj, 0, vboxh, vboxw, vboxh);
}
