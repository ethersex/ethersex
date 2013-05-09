unit check;

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
  Classes, SysUtils, FileUtil, LResources, Forms, Controls, Graphics, Dialogs,
  StdCtrls, DOM;

type

  { TForm2 }

  TForm2 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    CheckBox1: TCheckBox;
    ListBox1: TListBox;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    function unreachable(styp: string; snum: integer):Boolean;
    function numselectable(wind: TDOMNode):integer;
    function focusaction(wind: TDOMNode):integer;
    function identifier(node: TDOMNode):string;
  private
    { private declarations }
  public
    { public declarations }
    doc: TXMLDocument;
  end; 

var
  Form2: TForm2; 

implementation

{ TForm2 }

procedure TForm2.Button3Click(Sender: TObject);
begin
  form2.Close;
end;

procedure TForm2.Button2Click(Sender: TObject);
begin
  listbox1.Clear;
end;

function TForm2.identifier(node: TDOMNode):string;
var nnum:TDOMNode;
typ: string;
num: integer;
begin
  nnum := node.Attributes.GetNamedItem('number');
  typ := node.nodename;
  if (assigned(nnum)) then begin
    num := strtoint(nnum.nodeValue);
  end else begin
    showmessage('XML Error: "'+typ+'" is missing the number attribute');
    num := -1;
  end;
  result := typ+' '+inttostr(num);
end;

function TForm2.numselectable(wind: TDOMNode):integer;
var
  ch, foc: TDOMNode;
begin
  ch := wind.FirstChild;
  result := 0;
  while (assigned(ch)) do begin
    foc := ch.Attributes.GetNamedItem('focusable');
    if assigned(foc) then begin
      if (foc.NodeValue = '1') then
        result := result+1;
    end;
    ch := ch.nextSibling;
  end;
end;

function TForm2.unreachable(styp: string; snum: integer):Boolean;
var ch, obj, elem: TDOMNode;
  first:boolean;
  num:integer;
  typ, screen:string;
begin
  //check every window, subwindow and shortcut
  ch := doc.FirstChild;
  result := true;
  if (assigned(ch)) then begin
    ch := ch.FirstChild;
    first := true;
    while (assigned(ch)) do begin
      num := strtoint(ch.Attributes.GetNamedItem('number').nodeValue);
      typ := ch.nodename;
      if (typ = 'window') or (typ = 'subwindow') then begin
        if (first) and (num = snum) then begin
          result := false;
          break;
        end;
        first := false;
        //check windows switch of each object
        obj := ch.FirstChild;
        if (num <> snum) then begin
          while (assigned(obj)) do begin
            elem := obj.Attributes.GetNamedItem('screen');
            if (assigned(elem)) then begin
              screen := elem.nodeValue;
              if (screen = (styp+' '+inttostr(snum))) then begin
                result := false;
                break;
              end;
            end;
            obj := obj.nextSibling;
          end;
        end;
      end;
      if (typ = 'shortcut') then begin
        elem := ch.Attributes.GetNamedItem('screen');
        if (assigned(elem)) then begin
          screen := elem.nodeValue;
          if (screen = (styp+' '+inttostr(snum))) then begin
            result := false;
            break;
          end;
        end;
      end;
      ch := ch.nextSibling;
    end;
  end;
end;

function TForm2.focusaction(wind: TDOMNode): integer;
var
  ch, foc, act, scr, nfont, nfontfoc: TDOMNode;
  bfoc: boolean;
  typ: string;
  fon, fonfoc: integer;
begin
  ch := wind.FirstChild;
  result := 0;
  while (assigned(ch)) do begin
    typ := ch.nodename;
    foc := ch.Attributes.GetNamedItem('focusable');
    bfoc := false;
    if (assigned(foc)) then begin
      if (foc.NodeValue = '1') or (foc.NodeValue = '-1') then
        bfoc := true;
    end;
    act := ch.Attributes.GetNamedItem('action');
    scr := ch.Attributes.GetNamedItem('screen');
    if (bfoc = false) and (typ <> 'shortcut') then begin
      if (assigned(act)) then begin
        if act.nodeValue <> '' then begin
          listbox1.Items.Append('Warning: '+identifier(wind)+'->'+identifier(ch)+' has an action defined, but is not focusable');
          result := result+1;
        end;
      end;
      if (assigned(scr)) then begin
        if scr.nodeValue <> '' then begin
          listbox1.Items.Append('Warning: '+identifier(wind)+'->'+identifier(ch)+' has a window switch defined, but is not focusable');
          result := result+1;
        end;
      end;
    end;
    if (bfoc = true) and (checkbox1.checked) then begin
      nfont := ch.Attributes.GetNamedItem('font');
      nfontfoc := ch.Attributes.GetNamedItem('fontfocus');
      if (assigned(nfont)) and (assigned(nfontfoc)) then begin
        fon := strtoint(nfont.nodeValue);
        fonfoc := strtoint(nfontfoc.NodeValue);
        if (fon = fonfoc) then
          listbox1.items.Append('Hint: '+identifier(wind)+'->'+identifier(ch)+' has the same font for focus and no focus');
      end;
    end;
    ch := ch.nextSibling;
  end;
end;

procedure TForm2.Button1Click(Sender: TObject);
var ch, temp: TDOMNode;
   num, aobj: integer;
   typ: string;
   problems, enter, prev, next: integer;
begin
  problems := 0;
  //check every window and subwindow
  ch := doc.FirstChild;
  if (assigned(ch)) then begin
    ch := ch.FirstChild;
    while (assigned(ch)) do begin
      typ := ch.nodename;
      if (typ = 'window') or (typ = 'subwindow') then begin
        num := strtoint(ch.Attributes.GetNamedItem('number').nodeValue);
        if (unreachable(typ, num)) then begin
          listbox1.Items.Append('Warning: '+identifier(ch)+' can not be reached');
          problems := problems+1;
        end;
        problems := problems + focusaction(ch);
        enter := 0;
        prev := 0;
        next := 0;
        temp := ch.Attributes.GetNamedItem('focusEnterKey');
        if (temp <> nil) then enter := strtoint(temp.nodeValue);
        temp := ch.Attributes.GetNamedItem('focusPrevKey');
        if (temp <> nil) then prev := strtoint(temp.nodeValue);
        temp := ch.Attributes.GetNamedItem('focusNextKey');
        if (temp <> nil) then next := strtoint(temp.nodeValue);
        aobj := numselectable(ch);
        if (enter <= 0) and (aobj > 0) then begin
          listbox1.Items.Append('Warning: '+identifier(ch)+' has no enter key defined');
          problems := problems+1;
        end;
        if (prev <= 0) and (next <= 0) and (aobj > 1) and (checkbox1.checked) then begin
          listbox1.Items.append('Hint: '+identifier(ch)+' has neither a next nor a prev key defined');
        end;
      end;
      ch := ch.NextSibling;
    end;
  end;
  listbox1.items.append('Check done with '+inttostr(problems)+' problems');
end;

initialization
  {$I check.lrs}

end.

