var vboxh = 300;
var vboxw = 400;

function Graph(id,num,min,max) {
    this.obj = returnObjById(id);
    this.xmult = vboxw / num;
    this.min = min;
    this.max = max;
    this.last_y = -1;

    return this;
}

function graphAppend(g,val) {
    val = vboxh - (val * vboxh / (g.max - g.min));
    var old_elements = g.obj.getElementsByTagName("line").length;

    if (g.last_y != -1) {
	var ne = document.createElementNS("http://www.w3.org/2000/svg", "line");
	var w = (old_elements + 1) * g.xmult;
	ne.setAttribute ("x1", old_elements * g.xmult);
	ne.setAttribute ("x2", w);
	ne.setAttribute ("y1", g.last_y);
	ne.setAttribute ("y2", val);

	g.obj.appendChild (ne);

	if (w >= vboxw) {
	    g.obj.setAttribute("transform", "translate(-"
			       + (w - vboxw) + ",0)");
	}
    }

    g.last_y = val;
}