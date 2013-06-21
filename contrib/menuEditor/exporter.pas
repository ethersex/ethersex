unit exporter;

{ MenuEdit
  Version 1.4
  (c) 2009-2010 by Malte Marwedel
  www.marwedels.de/malte

  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
}

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, DOM, Math, StrUtils, Dialogs, IniFiles, lconvencoding ;


  function getobjectid(str: String):integer;
  function calcobjectsize(str: String; addrsize:Integer):integer;
  function calcchildsize(ch: TDOMNode; addrsize:Integer):integer;
  function getOptval(obj: TDOMnode; idstr:String):integer;
  procedure appendOptionsAttribute(obj: TDOMnode);
  procedure appendFontsAttribute(obj: TDOMnode);
  procedure appendActionAttribute(obj: TDOMnode; idstr: String);
  function insertAddress(pos: integer; addr: integer):Integer;
  procedure appendScreenjumpAttribute(obj: TDOMnode; idstr: String);
  procedure append8NumberAttribute(obj: TDOMnode; idstr: String);
  procedure appendLargePosAttribute(obj: TDOMnode);
  procedure appendLargeSizeAttribute(obj: TDOMnode);
  procedure appendDataAttribute(obj: TDOMnode);
  procedure appendIndexAttribute(obj: TDOMnode);
  procedure appendobject(obj: TDOMnode);
  procedure appendchildobjects(rw: TDOMnode);
  procedure exportit(filedir:String; rn: TDOMNode; silent:Boolean);


var
  //everything for data storage
  data: ^Byte;
  windowaddr, actionlist, textlist, checkboxlist, radiobuttonlist, listindexlist, gfxlist, staticdatalist : THashedStringList;
  maxo, maxwindow, maxtext, maxlist, maxcheckbox, maxradiobutton, maxactions, maxgfx: integer;
  addrsize, dataptr, objectptr: integer;
  largescreen: boolean;
  compactdata: boolean;
  objecttypeused: array [0..255] of boolean;
  objectsizes: integer; //size in bytes for objects, inidcates the address where data starts

const
  maxsize = 16777216; //max mem for 24 bit adresses
  versioninfo = 2;

implementation


function getobjectid(str: String):integer;

begin
  result := AnsiIndexText(str, ['', 'box', 'label', 'button', 'gfx', 'list',
     'checkbox', 'radiobutton', 'subwindow', 'window', 'shortcut']);
end;

function getobjectname(id: Integer):String;

begin
  case id of
    0: result := '';
    1: result := 'BOX';
    2: result := 'LABEL';
    3: result := 'BUTTON';
    4: result := 'GFX';
    5: result := 'LIST';
    6: result := 'CHECKBOX';
    7: result := 'RADIOBUTTON';
    8: result := 'SUBWINDOW';
    9: result := 'WINDOW';
    10: result := 'SHORTCUT';
  end;
end;

function calcobjectsize(str: String; addrsize:Integer):integer;
begin
  if (addrsize = 16) then begin
     if (largescreen = true) then begin
         case getobjectid(str) of
      0: result := 0;
      1: result := 13;
      2: result := 8;
      3: result := 15;
      4: result := 14;
      5: result := 20;
      6: result := 16;
      7: result := 16;
      8: result := 10;
      9: result := 4;
      10: result := 6;
      end;
    end else begin //if normal screen
      case getobjectid(str) of
      0: result := 0;
      1: result := 11;
      2: result := 7;
      3: result := 13;
      4: result := 12;
      5: result := 18;
      6: result := 14;
      7: result := 14;
      8: result := 8;
      9: result := 4;
      10: result := 6;
      end;
    end;
  end else begin //24 bit
    if (largescreen = true) then begin
      case getobjectid(str) of
      0: result := 0;
      1: result := 14;
      2: result := 10;
      3: result := 17;
      4: result := 16;
      5: result := 22;
      6: result := 18;
      7: result := 18;
      8: result := 10;
      9: result := 4;
      10: result := 7;
      end;
    end else begin //if normal screen
      case getobjectid(str) of
      0: result := 0;
      1: result := 12;
      2: result := 9;
      3: result := 15;
      4: result := 14;
      5: result := 20;
      6: result := 16;
      7: result := 16;
      8: result := 8;
      9: result := 4;
      10: result := 7;
      end;
    end;
  end;
end;


function calcchildsize(ch: TDOMNode; addrsize:Integer):integer;
begin
  ch := ch.FirstChild;
  result := 0;
  while (assigned(ch)) do begin
    result := result + calcobjectsize(ch.NodeName, addrsize);
    ch := ch.NextSibling;
  end;
end;

//returns -1 if the option was not found
function getOptval(obj: TDOMnode; idstr:String):integer;
var temp: integer;
begin
  result := -1;
  if (obj.Attributes.GetNamedItem(idstr) <> nil) then begin
    temp := strtoint(obj.Attributes.GetNamedItem(idstr).NodeValue);
    if (temp = -1) then //for some reason booltostr converts 'true' to -1 and not 1
      temp := 1;
    result := max(temp, 0);
  end;
end;

function getNodeIdent(obj: TDOMNode):String;
begin
  result := obj.NodeName + ' '+obj.Attributes.GetNamedItem('number').NodeValue;
end;

procedure appendOptionsAttribute(obj: TDOMnode);
  var options, temp, isthere: integer;
begin
  temp := getOptval(obj, 'focusable');
  isthere := temp;
  options := max(temp, 0);
  temp := getOptval(obj, 'rectangle');
  isthere := isthere + temp;
  options := options + max(temp*2, 0);
  temp := getOptval(obj, 'compressed');
  isthere := isthere + temp;
  options := options + max(temp*4, 0);
  temp :=  getOptval(obj, 'storagemethod');
  isthere := isthere + temp;
  options := options + max(temp*8, 0);
  if (isthere > -4) then begin //no options if all getOptval() returned -1
    data[objectptr] := options;
    inc(objectptr);
  end;
end;

procedure appendFontsAttribute(obj: TDOMnode);
  var fonts: integer;
begin
  if (obj.Attributes.GetNamedItem('font') <> nil) then begin
    fonts := getOptval(obj, 'font');
    fonts := fonts + max(getOptval(obj, 'fontfocus'), 0)*16;
    data[objectptr] := fonts;
    inc(objectptr);
  end;
end;

procedure appendActionAttribute(obj: TDOMnode; idstr: String);
  var key, tmp: String;
    value:integer;
begin
  if (obj.Attributes.GetNamedItem(idstr) <> nil) then begin
    key := obj.Attributes.GetNamedItem(idstr).NodeValue;
    if (key <> '') then begin   //if action wanted
      if (key = 'RET') then //Changed from version 1.0 to 1.1
        showmessage('Warning: In '+ getNodeIdent(obj)+' action "RET" is deprecated, use window switch instead');
      //check if already in the list
      tmp := actionlist.values[key];
      if (tmp = '') then begin
        actionlist.values[key] := inttostr(maxactions);
        tmp := inttostr(maxactions);
        inc(maxactions);
      end;
      value := strtoint(tmp);
    end else
      value := 0; //0 = no action wanted
    data[objectptr] := value mod 256;
    inc(objectptr);
    data[objectptr] := value div 256;
    inc(objectptr);
  end;
end;

function insertAddress(pos: integer; addr: integer):Integer;
begin
  data[pos] := addr mod 256;
  data[pos+1] := (addr div 256) mod 256;
  result := 2;
  if addrsize > 16 then begin
    data[pos+2] := (addr div 256 div 256);
    result := 3;
  end;
end;

procedure appendScreenJumpAttribute(obj: TDOMnode; idstr: String);
  var addr: integer;
     tmp, tmp2: String;
begin
  if (obj.Attributes.GetNamedItem(idstr) <> nil) then begin
    tmp := obj.Attributes.GetNamedItem(idstr).NodeValue;
    //showmessage(tmp+'... "'+windowaddr.values[tmp]+'"');
    if (tmp = '') then begin  //if no jump wanted
      addr := 0; // 0 = no jump
    end else begin
      if (tmp = 'RET') then begin
        addr := $FFFFFF; //insertAddress will crop this automatically to 16 bit if needed
      end else begin
        tmp2 := windowaddr.values[tmp];
        if (tmp2 <> '') then begin
          addr := strtoint(tmp2);
        end else begin
          showmessage('Error: jump target of '+getnodeident(obj)+' is invalid');
          addr := 1; //jump to first screen
        end;
      end;
    end;
    objectptr := objectptr + insertAddress(objectptr, addr);
  end;
end;

procedure append8NumberAttribute(obj: TDOMnode; idstr: String);
  var tmp: integer;
begin
  if (obj.Attributes.GetNamedItem(idstr) <> nil) then begin
    tmp := strtoint(obj.Attributes.GetNamedItem(idstr).NodeValue);
    //if (idstr = 'shortcutkey') then
      //showmessage('Appending '+idstr+' to position '+inttostr(objectptr)+ ' with value '+inttostr(tmp));
    if (tmp < 0) then begin
      showmessage('Error: Attribute '+idstr+'='+inttostr(tmp)+' from object '+
        obj.nodename+' is out of range.');
    end;
    tmp := max(0, tmp);
    data[objectptr] := tmp mod 256;
    inc(objectptr);
  end;
end;

procedure appendLargePosAttribute(obj: TDOMnode);
  var x, y, xy: integer;
begin
  if (largescreen = true) then begin
    if (obj.Attributes.GetNamedItem('Xpos') <> nil) and
       (obj.Attributes.GetNamedItem('Ypos') <> nil) then begin
      x := strtoint(obj.Attributes.GetNamedItem('Xpos').NodeValue);
      y := strtoint(obj.Attributes.GetNamedItem('Ypos').NodeValue);
      x := max(0, x);
      y := max(0, y);
      xy := (x div 256)*16 + (y div 256);
      data[objectptr] := xy mod 256;
      inc(objectptr);
    end;
  end;
end;

procedure appendLargeSizeAttribute(obj: TDOMnode);
  var x, y, xy: integer;
begin
  if (largescreen = true) then begin
    if (obj.Attributes.GetNamedItem('Xsize') <> nil) and
       (obj.Attributes.GetNamedItem('Ysize') <> nil) then begin
      x := strtoint(obj.Attributes.GetNamedItem('Xsize').NodeValue);
      y := strtoint(obj.Attributes.GetNamedItem('Ysize').NodeValue);
      x := max(0, x);
      y := max(0, y);
      xy := (x div 256)*16 + (y div 256);
      data[objectptr] := xy mod 256;
      inc(objectptr);
    end;
  end;
end;

function includedata(arr:Array of byte; len:integer):integer;
  var needinclude, fits: boolean;
  rptr, datapos, i,j: integer;
begin
  //if compactdata, look if there are already such data within the array
  needinclude := true;
  if (compactdata) then begin
    //showmessage(inttostr(objectsizes)+' '+inttostr(dataptr)+' '+inttostr(len));
    for i := objectsizes to dataptr-len do begin
      //inner compare loop
      fits := true;
      for j := 0 to len-1 do begin
       if (i+j = dataptr) then
            showmessage('Error in includedata routine, please tell developer');
        if (arr[j] <> data[i+j]) then begin
          fits := false;
          break;
        end;
      end;
      if (fits = true) then begin //found it!
        needinclude := false;
        datapos := i;
        break;
      end;
    end;
  end;
  //not wished, or not found -> include data
  if (needinclude = true) then begin
    datapos := dataptr;
    for rptr := 0 to len-1 do begin
      data[dataptr] := arr[rptr];
      inc(dataptr);
    end;
  end;
  result := datapos;
end;

function includegfx(obj: TDOMNode; compressed: boolean): integer;
  var x, y, px, py, sx, sy, wptr, rptr, size, db, tmp, rep:integer;
    sourcedata: String;
    tempdata: array [0..66000] of byte;
begin
  if (assigned(obj.FirstChild)) then begin
    if (obj.FirstChild.NodeName = 'bmp') then begin
      px := strtoint(obj.Attributes.GetNamedItem('Xsize').NodeValue);
      py := strtoint(obj.Attributes.GetNamedItem('Ysize').NodeValue);
      sx := strtoint(obj.FirstChild.Attributes.GetNamedItem('Xsize').NodeValue);
      sy := strtoint(obj.FirstChild.Attributes.GetNamedItem('Ysize').NodeValue);
      sourcedata := obj.FirstChild.FirstChild.NodeValue;
      //showmessage(sourcedata);
      //convert string into temporary data
      rptr := 1;
      wptr := 0;
      for y := 0 to (py-1) do begin
        rptr := y*sx+1; //start at the right position if the source is smaller than the target
        for x := 0 to (px-1) do begin
          if ((x < sx) and (y < sy)) and (length(sourcedata) >= rptr) then begin
            tempdata[wptr] := strtoint('$'+sourcedata[rptr]);
            inc(wptr);
            inc(rptr);
          end else begin //source image is smaller than target image -> fill with zeros
            tempdata[wptr] := 0;
            inc(wptr);
          end;
          if (wptr >= 66000) then begin
            showmessage('Error, Image' +getnodeident(obj)+ ' too big');
            break;
          end;
        end;
        if (wptr >= 66000) then begin
          break;
        end;
      end;
      size := wptr;
      //compress image
      //showmessage('Using compression: '+booltostr(compressed));
      if (compressed) then begin //do a length compression
        wptr := 0;
        rptr := 0;
        db := -1; //start with invalid color
        rep := 0;
        while (rptr < size) do begin
          if (tempdata[rptr] > 7) then begin
            tmp := 128;
          end else
            tmp := 0;
          if (tmp <> db) or (rep >= 127) then begin
            //color change or very often the same color
            if (rep > 0) then begin
              tempdata[wptr] := db + rep;
              inc(wptr);
            end;
            db := tmp;
            rep := 0;
          end;
          inc(rep);
          inc(rptr);
        end;
        if (rep > 0) then begin //add reaming byte
          tempdata[wptr] := db + rep;
          inc(wptr);
        end;
        size := wptr;
      end else begin //only compact 8 pixel to one byte
        wptr := 0;
        rptr := 0;
        db := 0;
        while (rptr < size) do begin
          if (tempdata[rptr] > 7) then begin
            tmp := 128;
          end else
            tmp := 0;
          db := db + (tmp shr (wptr mod 8));
//        showmessage('Writing: '+inttostr(wptr div 8)+' wptr:'+inttostr(wptr)+' rptr:'+inttostr(rptr));
          tempdata[wptr div 8] := db;
          if (wptr mod 8) = 7 then
            db := 0;
          inc(rptr);
          inc(wptr);
        end;
        size := (wptr div 8);
        if (wptr mod 8) > 0 then
          inc(size);
      end;
      //write into target position
      result := includedata(tempdata, size);
    end else
      showmessage('Error: "'+getnodeident(obj)+'" does not have a "bmp" as first child node');
  end else
    showmessage('Error: "'+getnodeident(obj)+'" is missing a bitmap');
end;

function includetext(obj: TDOMNode):integer;
  var wptr, i:integer;
    tempdata: array [0..66000] of byte;
    text, isotext:string;
begin
   text := obj.Attributes.GetNamedItem('text').NodeValue;
   isotext := ConvertEncoding(text, EncodingUTF8, 'iso88591');
   wptr := 0;
   for i := 1 to length(isotext) do begin
     if (ord(isotext[i]) <> 13) then begin //remove carriage return -> for windows compatibility
       tempdata[wptr] := ord(isotext[i]);
       inc(wptr);
     end;
   end;
   tempdata[wptr] := 0; //terminating 0 character
   inc(wptr);
   result := includedata(tempdata, wptr);
end;

procedure appendDataAttribute(obj: TDOMnode);
  var method, theptr: integer;
     text, tmp, key, parentid:string;
     compressed: boolean;
begin
  if (obj.Attributes.GetNamedItem('storagemethod') <> nil) then begin
    method := strtoint(obj.Attributes.GetNamedItem('storagemethod').NodeValue);
    if (obj.Attributes.GetNamedItem('text') <> nil) then begin
      text := obj.Attributes.GetNamedItem('text').NodeValue;
      parentid := obj.ParentNode.Attributes.GetNamedItem('number').NodeValue;
      key := parentid+'_'+obj.Attributes.GetNamedItem('number').NodeValue;
      if (obj.NodeName <> 'gfx') then begin
         //store text
        if (method = 0) then begin //store in menu data
          theptr := includetext(obj);
          staticdatalist.values[key] := inttostr(theptr);
        end else begin //store in ram
          tmp := textlist.values[text];
          if (tmp = '') then begin
            textlist.values[text] := inttostr(maxtext);
            tmp := inttostr(maxtext);
            inc(maxtext);
          end;
          theptr := strtoint(tmp);
        end;
        objectptr := objectptr + insertAddress(objectptr, theptr);
      end else begin
        //store gfx
        if (method = 0) then begin
          compressed := strtobool(obj.Attributes.GetNamedItem('compressed').NodeValue);
          theptr := includegfx(obj, compressed);
          staticdatalist.values[key] := inttostr(theptr);
        end else begin //store in ram
          //showmessage('gfx in ram');
          tmp := gfxlist.values[text];
          if (tmp = '') then begin
            gfxlist.values[text] := inttostr(maxgfx);
            tmp := inttostr(maxgfx);
            inc(maxgfx);
          end;
          theptr := strtoint(tmp);
        end;
        //now insert calculated address
        objectptr := objectptr + insertAddress(objectptr, theptr);
      end; //is gfx
    end; //has text
  end; //has storagemethod
end;

procedure appendIndexAttribute(obj: TDOMnode);
  var index, themax, selectnum: integer;
     tmp, checkname, actionkey :string;
     thelist: THashedStringList;
begin
  if (obj.Attributes.GetNamedItem('checkname') <> nil) then begin
    checkname := obj.Attributes.GetNamedItem('checkname').NodeValue;
    thelist := nil;
    if obj.NodeName = 'checkbox' then begin
      thelist := checkboxlist;
      themax := maxcheckbox;
    end;
    if obj.NodeName = 'radiobutton' then begin
      thelist := radiobuttonlist;
      themax := maxradiobutton;
    end;
    if obj.NodeName = 'list' then begin
      thelist := listindexlist;
      themax := maxlist;
    end;
    if (thelist <> nil) then begin //if valid entry
      tmp := thelist.values[checkname];
      if (tmp = '') then begin //if we have a new name
        tmp := inttostr(themax);
        thelist.values[checkname] := tmp;
        if obj.NodeName = 'list' then begin
           //add define for action if list changes
          actionkey := 'LISTINDEXCHANGE_'+checkname;
          actionlist.values[actionkey] := inttostr(65535-1-themax);
        end;
        inc(themax);
      end;
      index := strtoint(tmp);
      if (obj.Attributes.GetNamedItem('selectnum') <> nil) then begin
        selectnum := strtoint(obj.Attributes.GetNamedItem('selectnum').NodeValue);
        if (index > 15) then begin
          showmessage('Error ('+getnodeident(obj)+
          '): Limit of 16 different groups reached');
          index := 15;
        end;
        index := index +selectnum*16;
      end;
      data[objectptr] := index;
      inc(objectptr);
      //write back new max values
      if obj.NodeName = 'checkbox' then
        maxcheckbox := themax;
      if obj.NodeName = 'radiobutton' then
        maxradiobutton := themax;
      if obj.NodeName = 'list' then
        maxlist := themax;
    end;
  end;
end;

procedure appendobject(obj: TDOMnode);
 var temp: String;
     id: integer;
     oldsize, obs: integer;
begin
  oldsize := objectptr;
  temp := obj.NodeName;
  //append id
  id := getobjectid(temp);
  objecttypeused[id] := true;
  data[objectptr] := id;
  inc(objectptr);
  //append parameters
  append8NumberAttribute(obj, 'Xpos');
  append8NumberAttribute(obj, 'Ypos');
  appendLargePosAttribute(obj);
  append8NumberAttribute(obj, 'Xsize');
  append8NumberAttribute(obj, 'Ysize');
  appendLargeSizeAttribute(obj);
  appendOptionsAttribute(obj);
  append8NumberAttribute(obj, 'shortcutkey');
  appendActionAttribute(obj, 'action');
  appendScreenjumpAttribute(obj, 'screen');
  appendDataAttribute(obj);
  appendFontsAttribute(obj);
  append8NumberAttribute(obj, 'color');
  appendIndexAttribute(obj);
  append8NumberAttribute(obj, 'downkey');
  append8NumberAttribute(obj, 'upkey');
  append8NumberAttribute(obj, 'pagedownkey');
  append8NumberAttribute(obj, 'pageupkey');
  append8NumberAttribute(obj, 'focusPrevKey');
  append8NumberAttribute(obj, 'focusNextKey');
  append8NumberAttribute(obj, 'focusEnterKey');
  obs := calcobjectsize(temp, addrsize);
  if (objectptr <> (oldsize+obs)) then begin
    showmessage('Error, object "'+getNodeIdent(obj)+
     '" has a wrong data size (difference: '+inttostr(objectptr-oldsize-obs)+
     '). This will result in an invalid binary.');
     objectptr := oldsize+obs; //try to fix the reaming part
  end;
end;

procedure appendchildobjects(rw: TDOMnode);
var ch: TDOMNode;
  x: integer;
begin
  x := 0;
  ch := rw.FirstChild;
  while (assigned(ch)) do begin
    appendobject(ch);
    ch := ch.NextSibling;
    inc(x);
  end;
  maxo := max(maxo, x);
end;

procedure exportit(filedir:String; rn: TDOMNode; silent: Boolean);
  var
  succeed: boolean;
  ch: TDOMNode;
  stemp, temp: String;
  i, truemaxactions: integer;
  ToF: file;
  toft: textfile;
  sizetemp: integer;
begin
  maxo := 0; //maximum objects within a window
  maxtext := 0; //number of textes within the ram
  maxlist := 0; //number of individual lists
  maxcheckbox := 0; //number of individual check boxes
  maxradiobutton := 0; //number of radiobutton groups
  maxgfx := 0; //number of graphics
  maxactions := 1; //number of actions commands used
  addrsize := 16; //16 or 24 bit adress sizes
  succeed := false;
  compactdata := false;
  actionlist := THashedStringList.Create;
  textlist := THashedStringList.Create;
  checkboxlist := THashedStringList.Create;
  radiobuttonlist:= THashedStringList.Create;
  listindexlist  := THashedStringList.Create;
  gfxlist := THashedStringList.Create;
  staticdatalist := THashedStringList.Create;
  GetMem(data, 16777216 * sizeof(byte)); //16 MB array
  for i := 0 to 255 do begin
    objecttypeused[i] := false;
  end;
  //added in version 1.2: large screen support
  largescreen := false;
  sizetemp := strtoint(rn.Attributes.GetNamedItem('Xsize').NodeValue);
  if (sizetemp > 255) then begin
    largescreen := true;
  end;
  sizetemp := strtoint(rn.Attributes.GetNamedItem('Ysize').NodeValue);
  if (sizetemp > 255) then begin
    largescreen := true;
  end;
  if (rn.Attributes.GetNamedItem('compact') <> nil) then begin
    compactdata := strtobool(rn.Attributes.GetNamedItem('compact').NodeValue);
  end;
  repeat //repeats two times if it does not fit within 2^16 bytes
    data[0] := versioninfo; //not read by the interpreter
    objectsizes := 1; //the maximum memory for objetcs
    dataptr := 0;   //the current position to write data bytes to
    maxwindow := 0;
    //calculate window and subwindow adresses
    //first skip over all global shortcuts
    ch := rn.FirstChild;
    while (assigned(ch)) do begin
      if ch.NodeName = 'shortcut' then begin
        objectsizes := objectsizes + calcobjectsize(ch.NodeName, addrsize);
      end;
      ch := ch.NextSibling;
    end;
    //now start calcualting
    windowaddr := THashedStringList.Create;
    ch := rn.FirstChild;
    while (assigned(ch)) do begin
      if ch.NodeName <> 'shortcut' then begin
        stemp := getNodeIdent(ch);
        windowaddr.values[stemp] := inttostr(objectsizes); //global shortcuts get an entry too, but this is not a problem
        objectsizes := objectsizes + calcobjectsize(ch.NodeName, addrsize) + calcchildsize(ch, addrsize);
        inc(maxwindow);
      end;
      ch := ch.NextSibling;
    end;
    //showmessage(windowaddr.CommaText);
    //showmessage('Objectsizes: '+inttostr(objectsizes));
    //let last window end with a 0 and start data area
    dataptr := objectsizes; //the position in the data to write data bytes to
    data[dataptr] := 0; //indicate, that there is no further object following
    inc(dataptr);
    objectptr := 1; //the position in the data to write object bytes to
    //add data to array and fill define reference tables
    //first add global shortcuts, (implemented in version 1.1)
    ch := rn.FirstChild;
    while (assigned(ch)) do begin
      if ch.NodeName = 'shortcut' then begin
        appendobject(ch);
        //yes, we did calculate the size with possible childs, but there should not be any
        //and a shorter size is no problem (unlike a too big one)
      end;
      ch := ch.NextSibling;
    end;
    //now add windows and ignore the global shortcuts
    ch := rn.FirstChild;
    while (assigned(ch)) do begin
      if ch.NodeName <> 'shortcut' then begin
        appendobject(ch);
        appendchildobjects(ch);
      end;
      ch := ch.NextSibling;
    end;
    //check if we have done
    if (dataptr < 256*256) or (addrsize >= 24) then begin
      succeed := true
    end else
      addrsize := 24;
    windowaddr.Destroy;
  until (succeed);
  //write to files
  //the bin file...
  AssignFile(ToF,filedir+PathDelim+'menudata.bin'); //the bin file
  ReWrite(ToF, 1);
  BlockWrite(ToF, data^, dataptr);
  CloseFile(ToF);
  //same content as c array
  AssignFile(ToFt,filedir+PathDelim+'menudata.c');
  rewrite(toft);
  writeln(ToFt, 'unsigned char menudata[] = {');
  for i := 0 to (dataptr-1) do begin
    write(toft, inttostr(data[i]));
    if (i < (dataptr-1)) then
      write(toft, ', ');
    if (i mod 16) = 15 then
      writeln(toft, '');
  end;
  writeln(ToFt, '};');
  CloseFile(ToFt);
  //same content as c array with progmem keyword
  AssignFile(ToFt,filedir+PathDelim+'menudata-progmem.c');
  rewrite(toft);
  writeln(ToFt, '/*You may need to add #include <avr/pgmspace.h> where you included this file */');
  writeln(ToFt, 'unsigned char menudata[] PROGMEM = {');
  for i := 0 to (dataptr-1) do begin
    write(toft, inttostr(data[i]));
    if (i < (dataptr-1)) then
      write(toft, ', ');
    if (i mod 16) = 15 then
      writeln(toft, '');
  end;
  writeln(ToFt, '};');
  CloseFile(ToFt);

  //write defines header
  AssignFile(ToFt,filedir+PathDelim+'menu-interpreter-config.h');
  rewrite(toft);
  writeln(toft, '/* Do not edit! This file is autogenerated by MenuEdit */');
  writeln(toft, '');
  writeln(toft, '#ifndef MENU_INTERPRETER_CONFIG_H');
  writeln(toft, '#define MENU_INTERPRETER_CONFIG_H');
  writeln(toft, '');
  if (addrsize <= 16) then begin
    writeln(toft, '#define USE16BITADDR');
  end;
  writeln(toft, '');
  if (largescreen = true) then begin
    writeln(toft, '#define LARGESCREEN');
    writeln(toft, '');
  end;
  writeln(toft, '#define MENU_DATASIZE '+inttostr(dataptr));
  writeln(toft, '#define MENU_OBJECTS_MAX '+inttostr(maxo));
  writeln(toft, '#define MENU_LIST_MAX '+inttostr(maxlist));
  writeln(toft, '#define MENU_TEXT_MAX '+inttostr(maxtext));
  writeln(toft, '#define MENU_CHECKBOX_MAX '+inttostr(maxcheckbox));
  writeln(toft, '#define MENU_RADIOBUTTON_MAX '+inttostr(maxradiobutton));
  writeln(toft, '#define MENU_GFX_MAX '+inttostr(maxgfx));
  truemaxactions := maxactions;
  for i := 0 to actionlist.count-1 do begin //needed for the ret and list changes
    temp := actionlist.names[i];
    if (strtoint(actionlist.values[temp]) > truemaxactions) then
      truemaxactions := strtoint(actionlist.values[temp]);
  end;
  writeln(toft, '#define MENU_ACTION_MAX '+inttostr(truemaxactions));
  writeln(toft, '');
  temp := rn.Attributes.GetNamedItem('Xsize').NodeValue;
  writeln(toft, '#define MENU_SCREEN_X '+temp);
  temp := rn.Attributes.GetNamedItem('Ysize').NodeValue;
  writeln(toft, '#define MENU_SCREEN_Y '+temp);
  writeln(toft, '');
  for i := 0 to actionlist.count-1 do begin
    temp := actionlist.names[i];
    writeln(toft, '#define MENU_ACTION_'+temp+' '+actionlist.values[temp]);
    //showmessage( +'-'+ actionlist.values[actionlist.names[i]]);
  end;
  writeln(toft, '');
  for i := 0 to textlist.count-1 do begin
    temp := textlist.names[i];
    writeln(toft, '#define MENU_TEXT_'+temp+' '+textlist.values[temp]);
  end;
  writeln(toft, '');
  for i := 0 to checkboxlist.count-1 do begin
    temp := checkboxlist.names[i];
    writeln(toft, '#define MENU_CHECKBOX_'+temp+' '+checkboxlist.values[temp]);
  end;
  writeln(toft, '');
    for i := 0 to radiobuttonlist.count-1 do begin
    temp := radiobuttonlist.names[i];
    writeln(toft, '#define MENU_RBUTTON_'+temp+' '+radiobuttonlist.values[temp]);
  end;
  writeln(toft, '');
  for i := 0 to listindexlist.count-1 do begin
    temp := listindexlist.names[i];
    writeln(toft, '#define MENU_LISTINDEX_'+temp+' '+listindexlist.values[temp]);
  end;
  writeln(toft, '');
  for i := 0 to gfxlist.count-1 do begin
    temp := gfxlist.names[i];
    writeln(toft, '#define MENU_GFX_'+temp+' '+gfxlist.values[temp]);
  end;
  writeln(toft, '');
  for i := 0 to 255 do begin
    if (objecttypeused[i] = true) then begin
      writeln(toft, '#define MENU_USE_'+getobjectname(i));
    end;
  end;
  writeln(toft, '');
  for i := 0 to staticdatalist.count-1 do begin //added in version 1.4
    temp := staticdatalist.names[i];
    writeln(toft, '#define MENU_SDATA_'+temp+' '+staticdatalist.values[temp]);
  end;
  writeln(toft, '');
  writeln(toft, '#endif');
  writeln(toft, '');
  closefile(toft);
  //free data
  freeMem(data);
  actionlist.destroy;
  textlist.destroy;
  checkboxlist.destroy;
  radiobuttonlist.destroy;
  listindexlist.destroy;
  gfxlist.destroy;
  staticdatalist.destroy;
  if (silent = false) then begin
    showmessage('Export done. address size: '+inttostr(addrsize)+
      ' bit, menu size: '+ inttostr(dataptr)+' byte');
  end;
end;

end.

