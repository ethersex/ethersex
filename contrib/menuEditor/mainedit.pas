unit mainEdit;

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
  StdCtrls, Spin, ExtCtrls, ComCtrls, DOM, XMLWrite, XMLRead, Math, ExtDlgs, exporter, check,
  process;

type

  { TForm1 }

  TForm1 = class(TForm)
    Button1: TButton;
    Button10: TButton;
    Button11: TButton;
    Button12: TButton;
    Button13: TButton;
    Button14: TButton;
    Button15: TButton;
    Button16: TButton;
    Button17: TButton;
    Button18: TButton;
    Button19: TButton;
    Button2: TButton;
    Button20: TButton;
    Button21: TButton;
    Button22: TButton;
    Button23: TButton;
    Button24: TButton;
    Button25: TButton;
    Button26: TButton;
    Button27: TButton;
    Button28: TButton;
    Button29: TButton;
    Button3: TButton;
    Button30: TButton;
    Button31: TButton;
    Button32: TButton;
    Button33: TButton;
    Button34: TButton;
    Button35: TButton;
    Button36: TButton;
    Button4: TButton;
    Button5: TButton;
    Button6: TButton;
    Button7: TButton;
    Button8: TButton;
    Button9: TButton;
    CheckBox1: TCheckBox;
    CheckBox2: TCheckBox;
    CheckBox3: TCheckBox;
    CheckBox4: TCheckBox;
    CheckBox5: TCheckBox;
    ComboBox1: TComboBox;
    Edit2: TEdit;
    Edit3: TEdit;
    Edit4: TEdit;
    GroupBox1: TGroupBox;
    GroupBox2: TGroupBox;
    GroupBox3: TGroupBox;
    GroupBox4: TGroupBox;
    GroupBox5: TGroupBox;
    GroupBox6: TGroupBox;
    GroupBox7: TGroupBox;
    Image1: TImage;
    Image2: TImage;
    Label1: TLabel;
    Label10: TLabel;
    Label11: TLabel;
    Label12: TLabel;
    Label13: TLabel;
    Label14: TLabel;
    Label15: TLabel;
    Label16: TLabel;
    Label17: TLabel;
    Label2: TLabel;
    Label21: TLabel;
    Label22: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    ListBox1: TListBox;
    ListBox2: TListBox;
    Memo1: TMemo;
    OpenDialog1: TOpenDialog;
    OpenPictureDialog1: TOpenPictureDialog;
    Process1: TProcess;
    RadioButton1: TRadioButton;
    RadioButton2: TRadioButton;
    RadioButton3: TRadioButton;
    RadioButton4: TRadioButton;
    RadioButton5: TRadioButton;
    RadioButton6: TRadioButton;
    SaveDialog1: TSaveDialog;
    SelectDirectoryDialog1: TSelectDirectoryDialog;
    SpinEdit1: TSpinEdit;
    SpinEdit10: TSpinEdit;
    SpinEdit11: TSpinEdit;
    SpinEdit12: TSpinEdit;
    SpinEdit13: TSpinEdit;
    SpinEdit14: TSpinEdit;
    SpinEdit15: TSpinEdit;
    SpinEdit16: TSpinEdit;
    SpinEdit17: TSpinEdit;
    SpinEdit18: TSpinEdit;
    SpinEdit19: TSpinEdit;
    SpinEdit2: TSpinEdit;
    SpinEdit20: TSpinEdit;
    SpinEdit21: TSpinEdit;
    SpinEdit22: TSpinEdit;
    SpinEdit3: TSpinEdit;
    SpinEdit4: TSpinEdit;
    SpinEdit5: TSpinEdit;
    SpinEdit6: TSpinEdit;
    SpinEdit7: TSpinEdit;
    SpinEdit8: TSpinEdit;
    SpinEdit9: TSpinEdit;
    procedure AsyncProcess1ReadData(Sender: TObject);
    procedure Button10Click(Sender: TObject);
    procedure Button11Click(Sender: TObject);
    procedure Button12Click(Sender: TObject);
    procedure Button13Click(Sender: TObject);
    procedure Button14Click(Sender: TObject);
    procedure Button15Click(Sender: TObject);
    procedure Button16Click(Sender: TObject);
    procedure Button17Click(Sender: TObject);
    procedure Button18Click(Sender: TObject);
    procedure Button19Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button20Click(Sender: TObject);
    procedure Button21Click(Sender: TObject);
    procedure Button22Click(Sender: TObject);
    procedure Button23Click(Sender: TObject);
    procedure Button24Click(Sender: TObject);
    procedure Button25Click(Sender: TObject);
    procedure Button26Click(Sender: TObject);
    procedure Button27Click(Sender: TObject);
    procedure Button28Click(Sender: TObject);
    procedure Button29Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button30Click(Sender: TObject);
    procedure Button31Click(Sender: TObject);
    procedure Button32Click(Sender: TObject);
    procedure Button33Click(Sender: TObject);
    procedure Button34Click(Sender: TObject);
    procedure Button35Click(Sender: TObject);
    procedure Button36Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure Button7Click(Sender: TObject);
    procedure Button8Click(Sender: TObject);
    procedure Button9Click(Sender: TObject);
    procedure CheckBox1Change(Sender: TObject);
    procedure CheckBox2Change(Sender: TObject);
    procedure CheckBox3Change(Sender: TObject);
    procedure CheckBox4Change(Sender: TObject);
    procedure CheckBox5Change(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure ComboBox2Change(Sender: TObject);
    procedure ComboBox3Change(Sender: TObject);
    procedure ComboBox4Change(Sender: TObject);
    procedure Edit1Change(Sender: TObject);
    procedure Edit2Change(Sender: TObject);
    procedure Edit3Change(Sender: TObject);
    procedure Edit5Change(Sender: TObject);
    procedure Edit6Change(Sender: TObject);
    procedure Edit7Change(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure ListBox1Click(Sender: TObject);
    procedure ListBox1SelectionChange(Sender: TObject; User: boolean);
    procedure ListBox2SelectionChange(Sender: TObject; User: boolean);
    procedure RadioButton1Change(Sender: TObject);
    procedure RadioButton2Change(Sender: TObject);
    procedure RadioButton3Change(Sender: TObject);
    procedure RadioButton4Change(Sender: TObject);
    procedure RadioButton5Change(Sender: TObject);
    procedure RadioButton6Change(Sender: TObject);
    procedure RadioGroup1Click(Sender: TObject);
    procedure SpinEdit10Change(Sender: TObject);
    procedure SpinEdit11Change(Sender: TObject);
    procedure SpinEdit12Change(Sender: TObject);
    procedure SpinEdit13Change(Sender: TObject);
    procedure SpinEdit14Change(Sender: TObject);
    procedure SpinEdit15Change(Sender: TObject);
    procedure SpinEdit16Change(Sender: TObject);
    procedure SpinEdit17Change(Sender: TObject);
    procedure SpinEdit18Change(Sender: TObject);
    procedure SpinEdit19Change(Sender: TObject);
    procedure SpinEdit19MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure SpinEdit1Change(Sender: TObject);
    procedure SpinEdit20Change(Sender: TObject);
    procedure SpinEdit20MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure SpinEdit21Change(Sender: TObject);
    procedure SpinEdit22Change(Sender: TObject);
    procedure SpinEdit2Change(Sender: TObject);
    procedure SpinEdit3Change(Sender: TObject);
    procedure SpinEdit4Change(Sender: TObject);
    procedure SpinEdit5Change(Sender: TObject);
    procedure SpinEdit6Change(Sender: TObject);
    procedure SpinEdit6MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure SpinEdit7Change(Sender: TObject);
    procedure SpinEdit7MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure SpinEdit8Change(Sender: TObject);
    procedure SpinEdit9Change(Sender: TObject);
    procedure Bmpdraw(node: TDOMNode);
    procedure objectwinup();
    procedure loadFile(filename: String);
  private
    doc: TXMLDocument;
    maxwindow, maxobject: integer; //counter for new object and window numbering
    RootNode: TDOMNode;
    aw: TDOMNode;    //selected active window node
    ao: TDOMNode;    //selected active object node

    { private declarations }
  public
    { public declarations }
  end; 

var
  Form1: TForm1; 

implementation

{ TForm1 }

procedure TForm1.Button1Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('window');
  inc(maxwindow);
  TDOMElement(ch).SetAttribute('number', inttostr(maxwindow));
  TDOMElement(ch).SetAttribute('focusNextKey', '0');
  TDOMElement(ch).SetAttribute('focusPrevKey', '0');
  TDOMElement(ch).SetAttribute('focusEnterKey', '0');
  RootNode.AppendChild(ch);
  Button20Click(sender);
end;

procedure TForm1.Button12Click(Sender: TObject);
var ch: TDomNode;
  ind : integer;
begin
  ind := listbox2.itemindex;
  if (ind > 0) then begin
    ch := ao.PreviousSibling;
    if (assigned(ch)) then begin
      aw.InsertBefore(ao, ch);
      dec(ind);
      listbox1selectionchange(sender, false);
      listbox2.itemindex := ind;
      listbox2selectionchange(sender, false);
    end;
  end;
end;

procedure TForm1.Button13Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('list');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('focusable', '1');
  TDOMElement(ch).SetAttribute('rectangle', '1');
  TDOMElement(ch).SetAttribute('Xsize', '30');
  TDOMElement(ch).SetAttribute('Ysize', '20');
  TDOMElement(ch).SetAttribute('font', '0');
  TDOMElement(ch).SetAttribute('fontfocus', '0');
  TDOMElement(ch).SetAttribute('text', '');
  TDOMElement(ch).SetAttribute('storagemethod', '0');
  TDOMElement(ch).SetAttribute('checkname', 'list1');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  TDOMElement(ch).SetAttribute('upkey', '0');
  TDOMElement(ch).SetAttribute('downkey', '0');
  TDOMElement(ch).SetAttribute('pageupkey', '0');
  TDOMElement(ch).SetAttribute('pagedownkey', '0');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.Button14Click(Sender: TObject);
     var ch: TDomNode;
begin
  ch := doc.CreateElement('gfx');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('focusable', '0');
  TDOMElement(ch).SetAttribute('rectangle', '0');
  TDOMElement(ch).SetAttribute('Xsize', '20');
  TDOMElement(ch).SetAttribute('Ysize', '10');
  TDOMElement(ch).SetAttribute('text', 'gfx1');
  TDOMElement(ch).SetAttribute('storagemethod', '0');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  TDOMElement(ch).SetAttribute('compressed', '1');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.Button15Click(Sender: TObject);
    var ch: TDomNode;
begin
  ch := doc.CreateElement('checkbox');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('focusable', '1');
  TDOMElement(ch).SetAttribute('rectangle', '0');
  TDOMElement(ch).SetAttribute('Xsize', '20');
  TDOMElement(ch).SetAttribute('Ysize', '9');
  TDOMElement(ch).SetAttribute('font', '0');
  TDOMElement(ch).SetAttribute('fontfocus', '0');
  TDOMElement(ch).SetAttribute('text', '');
  TDOMElement(ch).SetAttribute('storagemethod', '0');
  TDOMElement(ch).SetAttribute('checkname', 'check1');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.Button16Click(Sender: TObject);
    var ch: TDomNode;
begin
  ch := doc.CreateElement('shortcut');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('shortcutkey', '1');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.Button11Click(Sender: TObject);
    var ch: TDomNode;
begin
  ch := doc.CreateElement('radiobutton');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('focusable', '1');
  TDOMElement(ch).SetAttribute('rectangle', '0');
  TDOMElement(ch).SetAttribute('Xsize', '20');
  TDOMElement(ch).SetAttribute('Ysize', '9');
  TDOMElement(ch).SetAttribute('font', '0');
  TDOMElement(ch).SetAttribute('fontfocus', '0');
  TDOMElement(ch).SetAttribute('text', '');
  TDOMElement(ch).SetAttribute('storagemethod', '0');
  TDOMElement(ch).SetAttribute('checkname', 'radio1');
  TDOMElement(ch).SetAttribute('selectnum', '0');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

//the most important part, generate the binary data
procedure TForm1.Button10Click(Sender: TObject);

begin
  if (selectdirectorydialog1.Execute) then begin
    exporter.exportit(selectdirectorydialog1.FileName, RootNode, false);
  end;
end;

procedure TForm1.AsyncProcess1ReadData(Sender: TObject);
begin

end;

procedure TForm1.Button17Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('button');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('focusable', '1');
  TDOMElement(ch).SetAttribute('rectangle', '1');
  TDOMElement(ch).SetAttribute('Xsize', '20');
  TDOMElement(ch).SetAttribute('Ysize', '12');
  TDOMElement(ch).SetAttribute('font', '0');
  TDOMElement(ch).SetAttribute('fontfocus', '0');
  TDOMElement(ch).SetAttribute('text', '');
  TDOMElement(ch).SetAttribute('storagemethod', '0');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.Button18Click(Sender: TObject);
var ch, ch2: TDomNode;
  ind : integer;
begin
  ind := listbox2.itemindex;
  if (ind >= 0) and (ind < listbox2.Items.Count-1) then begin
    ch := ao.NextSibling;
    if (assigned(ch)) then begin
      ch2 := ch.NextSibling;    //there is no insertAfter command
      if (assigned(ch2)) then begin
       aw.InsertBefore(ao, ch2);
      end else
        aw.AppendChild(ao);
      listbox1selectionchange(sender, false);
      inc(ind);
      listbox2.itemindex := ind;
      listbox2selectionchange(sender, false);
    end;
  end;
end;

procedure TForm1.Button19Click(Sender: TObject);
begin
  if (listbox2.itemindex >= 0) then begin
    aw.RemoveChild(ao);
    ListBox1SelectionChange(sender, false);
  end;
end;

procedure TForm1.Button20Click(Sender: TObject);
var bases: TDOMNode;
temp: String;
begin
  //the update button
  spinedit1.Value := strtoint(RootNode.Attributes.GetNamedItem('Xsize').NodeValue);
  spinedit2.Value := strtoint(RootNode.Attributes.GetNamedItem('Ysize').NodeValue);
  if (RootNode.Attributes.GetNamedItem('compact') <> nil) then begin
    checkbox5.Checked := strtobool(RootNode.Attributes.GetNamedItem('compact').NodeValue);
  end else
    checkbox5.Checked := false;
  bases := RootNode.FirstChild;
  listbox1.Clear;
  listbox2.Clear;
  combobox1.Clear;
  groupbox3.Enabled := false;
  groupbox5.enabled := false;
  button21.Enabled := false;
  button35.Enabled := false;
  maxwindow := 0;
  while Assigned(bases) do begin
    temp := bases.Attributes.GetNamedItem('number').NodeValue;
    maxwindow := max(strtoint(temp), maxwindow);
    temp := bases.NodeName +' ' + temp;
    listbox1.Items.Add(temp);
    if (bases.NodeName <> 'shortcut') then begin
      combobox1.Items.add(temp);
    end;
    bases := bases.NextSibling;
  end;

end;

procedure TForm1.Button21Click(Sender: TObject);
var x, y, px, py, col, a, b, c :integer;
  str1, str2: String;
  ch, data: TDomNode;
begin
  if (openpicturedialog1.Execute) then begin
    image2.Picture.LoadFromFile(openpicturedialog1.FileName);
    form1.Refresh;
    //without the Application.ProcessMessages; and Refresh; compressed images
    //(like png) are converted later and reading the pixels
    //by Canvas.Pixels[x,y]; gives wrong return values.
    //(bmp images did work even without this)
    Application.ProcessMessages;
    px := image2.Picture.Width;
    py := image2.picture.height;
    str2 := '';
    for y := 0 to (py-1) do begin
       str1 := '';
       for x := 0 to (px-1) do begin
         //converts to a 4 bit grayscale image
         col := image2.Canvas.Pixels[x,y];
         //if (y = 0) then showmessage(inttostr(col));
         a := col mod 256;
         b := (col div 256) mod 256;
         c := (col div 256 div 256) mod 256;
         col := (a + b + c) div (3*16);
         str1 := str1 + inttohex(col, 1);
      end;
      str2 := str2 + str1;
    end;
    if (assigned(ao.FirstChild)) then
      ao.RemoveChild(ao.FirstChild);
    ch := doc.CreateElement('bmp');
    data := doc.CreateTextNode(str2);
    ch.AppendChild(data);
    TDOMElement(ch).SetAttribute('Xsize', inttostr(px));
    TDOMElement(ch).SetAttribute('Ysize', inttostr(py));
    ao.AppendChild(ch);
  end;
end;

procedure TForm1.Button22Click(Sender: TObject);
begin
  if (edit4.text <> '') then
    label17.caption := inttostr(integer(edit4.Text[1]))+ '; 0x' +
     inttohex(integer(edit4.text[1]), 2);
end;

procedure TForm1.Button23Click(Sender: TObject);
  var ch: TDomNode;
  p1, p2, p3, p4, ix, iy, vx, vy: integer;
  te: String;
  img: TBitmap;
begin
  vx := (spinedit1.value+2);
  vy := (spinedit2.value+2);
  ix := vx;
  iy := vy;
  if radiobutton6.checked then begin
    ix := ix div 4;
    iy := iy div 4;
  end;
  if radiobutton3.checked then begin
    ix := ix div 2;
    iy := iy div 2;
  end;
  if radiobutton5.checked then begin
    ix := ix * 2;
    iy := iy * 2;
  end;
  image1.Width :=  ix;
  image1.Height :=  iy;
  with image1 do begin
    img := TBitmap.Create();
    img.SetSize(vx, vy);
    img.canvas.FillRect(0,0, vx, vy);
    FreeAndNil(Picture.Graphic);
    image1.Picture.Graphic := img;
    canvas.Brush.Color := clwhite;
    canvas.FillRect(0,0, vx, vy);
    canvas.Brush.Color := clwhite;
    canvas.Pen.Color := clred;
    canvas.Font.Size := 8;
    canvas.Rectangle(0,0, spinedit1.Value+2, spinedit2.Value+2);
    if (assigned(aw)) then begin
      //draw rectangle of subwindow
      p1 := -1;
      p2 := -1;
      p3 := -1;
      if (aw.Attributes.GetNamedItem('Xpos') <> nil) then
        p1 := strtoint(aw.Attributes.GetNamedItem('Xpos').NodeValue);
      if (aw.Attributes.GetNamedItem('Ypos') <> nil) then
        p2 := strtoint(aw.Attributes.GetNamedItem('Ypos').NodeValue);
      if (aw.Attributes.GetNamedItem('Xsize') <> nil) then
        p3 := strtoint(aw.Attributes.GetNamedItem('Xsize').NodeValue);
      if (aw.Attributes.GetNamedItem('Ysize') <> nil) then
        p4 := strtoint(aw.Attributes.GetNamedItem('Ysize').NodeValue);
      inc(p1); //because there is 1 pixel bounding line
      inc(p2);
      if (p1 > 0) and (p2 > 0) and (p3 > 0) and (p4 > 0) then begin
        canvas.pen.color := claqua;
        canvas.Rectangle(p1, p2, p1+p3, p2+p4);
      end;
      //draw objects of window
      ch := aw.FirstChild;
      while Assigned(ch) do begin
        p1 := -1;
        p2 := -1;
        p3 := -1;
        p4 := -1;
        if (ch.Attributes.GetNamedItem('Xpos') <> nil) then
          p1 := strtoint(ch.Attributes.GetNamedItem('Xpos').NodeValue);
        if (ch.Attributes.GetNamedItem('Ypos') <> nil) then
          p2 := strtoint(ch.Attributes.GetNamedItem('Ypos').NodeValue);
        if (ch.Attributes.GetNamedItem('Xsize') <> nil) then
           p3 := strtoint(ch.Attributes.GetNamedItem('Xsize').NodeValue);
        if (ch.Attributes.GetNamedItem('Ysize') <> nil) then
          p4 := strtoint(ch.Attributes.GetNamedItem('Ysize').NodeValue);
        inc(p1); //because there is 1 pixel bounding line
        inc(p2);
        if (ch = ao) then begin
          canvas.Pen.Color := cllime;
          canvas.Font.Color := cllime;
        end else begin
          canvas.pen.color := clblack;
          canvas.Font.color := clblack;
        end;
        if (p1 > 0) and (p2 > 0) and (p3 > 0) and (p4 > 0) then begin
          canvas.Rectangle(p1, p2, p1+p3, p2+p4);
        end;
        if (ch.Attributes.GetNamedItem('text') <> nil) then begin
          te := ch.Attributes.GetNamedItem('text').NodeValue;
          if (p1 > 0) and (p2 > 0) then begin
            canvas.TextOut(p1, p2, te);
          end;
        end;
        ch := ch.NextSibling;
      end;
    end;
  end;
end;

procedure TForm1.Button24Click(Sender: TObject);
  var ch: TDomNode;
begin
    if (listbox1.itemindex >= 0) then begin
    ch := aw.CloneNode(true);
    inc(maxwindow);
    TDOMElement(ch).SetAttribute('number', inttostr(maxwindow));
    RootNode.AppendChild(ch);
    button20click(sender);
  end;
end;

procedure TForm1.Button25Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('shortcut');
  inc(maxwindow);
  TDOMElement(ch).SetAttribute('number', inttostr(maxwindow));
  TDOMElement(ch).SetAttribute('shortcutkey', '1');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  RootNode.AppendChild(ch);
  Button20Click(sender);
end;

procedure TForm1.Button26Click(Sender: TObject);
begin
  showmessage('Order of objects = Drawing order = Focus order. The first window in the list is always the starting window for the menu.');
end;

procedure TForm1.Button27Click(Sender: TObject);
begin
  showmessage('ActionName: Will generate a #define with this text, defining a number to use with the menu_action function.');
end;

procedure TForm1.Button28Click(Sender: TObject);
begin
   showmessage('AnswerName: Generates a #define referencing a position in an array to read/write the state of checkboxes, groupboxes and the index of the selected item in a list.');
end;

procedure TForm1.Button29Click(Sender: TObject);
begin
  showmessage('Enter the special keyword "RET" into the Window switch field in order to return from a SubWindow to the previous normal Window.');
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  if (listbox1.itemindex >= 0) then begin
    RootNode.RemoveChild(aw);
    button20click(sender);
  end;
end;

procedure TForm1.Button30Click(Sender: TObject);
begin
  showmessage('Static text: Just enter the text in the big field and select "Local Storage". Dynamic text: Select "RAM Storage" and enter an identifier in the big field'+
  ' (must begin with a characterer and may only contain characters+numbers, no new line).');
end;

procedure TForm1.Button31Click(Sender: TObject);
begin
  showmessage('MenuEdit Version 1.4    (c) 2009-2010 by Malte Marwedel      terms of use: GNU General Public License version 2.0 or later, see license.txt');
end;

procedure TForm1.Button32Click(Sender: TObject);
begin
  showmessage('Only Lists can handle multiple lines. You can enter them into the text field. If you use dynamic text, a newline character (\n) is used to separate the lines.');
end;

procedure TForm1.Button33Click(Sender: TObject);
begin
   showmessage('Currently four 5x7 fonts are defined: 0 Compact 5x7 font, 1: Fixed 5x7 font. 2 and 3: The first two fonts with an underline (as 8. Pixel).');
end;

procedure TForm1.Button34Click(Sender: TObject);
var clone, ch, fakew: TDomNode;
targ, targ2, tops, exec, cpath, texec: string;
begin
//1. clone current tree, 2. insert switch window, 3. export and 4. run program
  //1
  clone := RootNode.CloneNode(true);
  //2
  targ2 := '254';
  fakew := doc.CreateElement('window');
  TDOMElement(fakew).SetAttribute('number', inttostr(maxwindow+1));
  TDOMElement(fakew).SetAttribute('focusNextKey', '0');
  TDOMElement(fakew).SetAttribute('focusPrevKey', '0');
  TDOMElement(fakew).SetAttribute('focusEnterKey', '0');
  clone.InsertBefore(fakew, clone.FirstChild);
  tops := form2.identifier(aw);
  ch := doc.CreateElement('shortcut');
  TDOMElement(ch).SetAttribute('number', '1');
  TDOMElement(ch).SetAttribute('shortcutkey', targ2);
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', tops);
  fakew.AppendChild(ch);
  //3
  targ := GetTempDir(false);
  targ := targ+'testmenu_temp'+inttostr(random(1000000));
  createdir(targ);
  exporter.exportit(targ, clone, true);
  //4
  cpath := extractfilepath(application.ExeName);
  process1.CurrentDirectory := cpath;
  exec := cpath+'menutester.NotUnixNotWin32'; //error case
  {$IfDef Unix}
  exec := cpath+'menutester.sh';
  {$Endif}
  {$IfDef Win32}
  exec := cpath+'menutester.bat';
  {$Endif}
  if (process1.Running) then begin
    process1.Terminate(0);
  end;
  if fileexists(exec) then begin
    process1.CommandLine := exec+' "'+targ+'" "'+targ2+'"';
    //working dir: path with menuedit executable
    //first parameter: paht for temporary dir where the files are exported
    //second parameter: key to jump into the right menu
    process1.Execute;
    //Bug: if execute fails, the program chrashes
    if (process1.Running = false) then begin
      showmessage('Error: '+exec+' found. But starting failed.');
    end else begin
      sleep(1500);
      texec := targ+system.DirectorySeparator+'menutester';
      {$IfDef Win32}
      texec := texec+'menutester.exe';
      {$Endif}
      if (process1.running = false) and (fileexists(texec) = false) then
        showmessage('Error: '+texec+' creation failed, see compileerror.log in the temporary directory');
    end;
  end else begin
    showmessage('Error: program '+exec+' not found.');
  end;
  clone.Destroy;
end;

procedure TForm1.Button35Click(Sender: TObject);
begin
  if (assigned(ao.FirstChild)) then
    ao.RemoveChild(ao.FirstChild);
  image2.Canvas.Rectangle(0, 0, 512, 512);
end;

procedure TForm1.Button36Click(Sender: TObject);
begin
  form2.doc := doc;
  form2.Show;
end;

procedure TForm1.Button3Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('subwindow');
  inc(maxwindow);
  TDOMElement(ch).SetAttribute('number', inttostr(maxwindow));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('Xsize', '50');
  TDOMElement(ch).SetAttribute('Ysize', '30');
  TDOMElement(ch).SetAttribute('focusNextKey', '0');
  TDOMElement(ch).SetAttribute('focusPrevKey', '0');
  TDOMElement(ch).SetAttribute('focusEnterKey', '0');
  RootNode.AppendChild(ch);
  Button20Click(sender);
end;

procedure TForm1.loadFile(filename: String);
begin
  if (fileexists(filename)) then begin
   doc.destroy;
   ReadXMLFile(doc, FileName);
   savedialog1.FileName := FileName;
   selectdirectorydialog1.InitialDir:= ExtractFilePath(filename);
   openpicturedialog1.initialdir := selectdirectorydialog1.InitialDir;
   RootNode := doc.FirstChild;
   ao := nil;
   aw := nil;
   if (assigned(RootNode)) then begin
     if (RootNode.NodeName = 'MenuConfig') then begin
       Button20Click(nil);
     end else
       showmessage('Error: XML file does not contain a MenuConfig');
   end else
     showmessage('Error: No root node');
 end else
   showmessage('Error: File does not exists');
end;


procedure TForm1.Button4Click(Sender: TObject);
begin
  if (opendialog1.Execute) then begin
    loadfile(opendialog1.filename);
    form2.close;
    form2.ListBox1.Clear;
  end;
end;

procedure TForm1.Button5Click(Sender: TObject);
begin
  if (savedialog1.Execute) then begin
    WriteXMLFile(doc, savedialog1.filename);
    selectdirectorydialog1.initialdir := extractfilepath(savedialog1.FileName);
  end;
end;

procedure TForm1.Button6Click(Sender: TObject);
  var ch: TDomNode;
  ind : integer;
begin
  ind := listbox1.itemindex;
  if (ind > 0) then begin
    ch := aw.PreviousSibling;
    if (assigned(ch)) then begin
      RootNode.InsertBefore(aw, ch);
      button20click(sender);
      dec(ind);
      listbox1.itemindex := ind;
      listbox1selectionchange(sender, false);
    end;
  end;
end;

procedure TForm1.Button7Click(Sender: TObject);
  var ch, ch2: TDomNode;
  ind : integer;
begin
  ind := listbox1.itemindex;
  if (ind >= 0) and (ind < listbox1.Items.Count-1) then begin
    ch := aw.NextSibling;
    if (assigned(ch)) then begin
      ch2 := ch.NextSibling;    //there is no insertAfter command
      if (assigned(ch2)) then begin
       RootNode.InsertBefore(aw, ch2);
      end else
        RootNode.AppendChild(aw);
      button20click(sender);
      inc(ind);
      listbox1.itemindex := ind;
      listbox1selectionchange(sender, false);
    end;
  end;
end;

procedure TForm1.Button8Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('box');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('focusable', '0');
  TDOMElement(ch).SetAttribute('Xsize', '10');
  TDOMElement(ch).SetAttribute('Ysize', '10');
  TDOMElement(ch).SetAttribute('color', '1');
  TDOMElement(ch).SetAttribute('action', '');
  TDOMElement(ch).SetAttribute('screen', '');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.Button9Click(Sender: TObject);
  var ch: TDomNode;
begin
  ch := doc.CreateElement('label');
  inc(maxobject);
  TDOMElement(ch).SetAttribute('number', inttostr(maxobject));
  TDOMElement(ch).SetAttribute('Xpos', '0');
  TDOMElement(ch).SetAttribute('Ypos', '0');
  TDOMElement(ch).SetAttribute('font', '0');
  TDOMElement(ch).SetAttribute('storagemethod', '0');
  TDOMElement(ch).SetAttribute('text', '');
  aw.AppendChild(ch);
  ListBox1SelectionChange(sender, false);
end;

procedure TForm1.CheckBox1Change(Sender: TObject);
begin
    TDOMElement(ao).SetAttribute('focusable', booltostr(checkbox1.Checked));
end;

procedure TForm1.CheckBox2Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('rectangle', booltostr(checkbox2.Checked));
end;

procedure TForm1.CheckBox3Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('compressed', booltostr(checkbox3.Checked));
end;

procedure TForm1.CheckBox4Change(Sender: TObject);
begin
  if (checkbox4.checked) then begin
   TDOMElement(ao).SetAttribute('screen', combobox1.text);
  end else
   TDOMElement(ao).SetAttribute('screen', '');
end;

procedure TForm1.CheckBox5Change(Sender: TObject);
begin
  TDOMElement(RootNode).SetAttribute('compact', booltostr(checkbox5.Checked));
end;

procedure TForm1.ComboBox1Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('screen', combobox1.text);
end;

procedure TForm1.ComboBox2Change(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('screen1', combobox2.text);
end;

procedure TForm1.ComboBox3Change(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('screen2', combobox3.text);
end;

procedure TForm1.ComboBox4Change(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('screen3', combobox4.text);
end;

procedure TForm1.Edit1Change(Sender: TObject);
var temp: String;
  len: integer;
begin
  temp := memo1.text;
  len := length(temp);
  if (len > 0 ) then begin
    if (temp[len] = char(10)) then begin
      setlength(temp, len-1);
    end;
  end;
  TDOMElement(ao).SetAttribute('text', temp);
  button23click(sender);
end;

procedure TForm1.Edit2Change(Sender: TObject);
begin
    TDOMElement(ao).SetAttribute('checkname', edit2.Text);
end;

procedure TForm1.Edit3Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('action', edit3.text);
end;

procedure TForm1.Edit5Change(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('action1', edit5.text);
end;

procedure TForm1.Edit6Change(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('action2', edit6.text);
end;

procedure TForm1.Edit7Change(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('action3', edit7.text);
end;

procedure TForm1.FormCreate(Sender: TObject);
  var img: TBitmap;
begin
  doc := TXMLDocument.create;
  //create a root node
  RootNode := doc.CreateElement('MenuConfig');
  doc.Appendchild(RootNode);
  TDOMElement(RootNode).SetAttribute('Xsize', inttostr(spinedit1.value));
  TDOMElement(RootNode).SetAttribute('Ysize', inttostr(spinedit2.value));
  TDOMElement(RootNode).SetAttribute('compact', '0');
  TDOMElement(RootNode).SetAttribute('languages', '1');
  img := TBitmap.Create();
  img.SetSize(512, 512);
  img.canvas.FillRect(0,0, 4096, 4096);
  image1.Picture.Graphic := img;
  Button20Click(sender);
  if paramcount > 0 then begin
     loadfile(paramstr(1));
     if paramcount = 3 then begin //auto export and close
        if paramstr(2) = '--export' then begin
          exporter.exportit(paramstr(3), RootNode, true);
          writeln('================ Export done. Ignore possible following erros ===========');
          Application.Terminate; //does produce some crashes, but it terminates the program.
          //Form1.close; //does not work
        end;
     end;
  end;
end;

procedure TForm1.ListBox1Click(Sender: TObject);
begin

end;


procedure TForm1.ListBox1SelectionChange(Sender: TObject; User: boolean);
  var bases: TDOMNode;
  temp: String;
begin
  if (listbox1.ItemIndex < 0) then begin
    groupbox3.Enabled := false;
    groupbox5.enabled := false;
  end else begin
    //seek proper node
    aw := RootNode.ChildNodes.Item[listbox1.itemindex];
    if (aw.NodeName = 'window') or (aw.NodeName = 'subwindow') then begin
      groupbox5.Enabled := false;
      groupbox3.enabled := true;
      spinedit19.Enabled := false;
      spinedit20.Enabled := false;
      spinedit21.Enabled := false;
      spinedit22.Enabled := false;
      spinedit3.Value := strtoint(aw.Attributes.GetNamedItem('focusNextKey').NodeValue);
      spinedit4.Value := strtoint(aw.Attributes.GetNamedItem('focusPrevKey').NodeValue);
      spinedit5.Value := strtoint(aw.Attributes.GetNamedItem('focusEnterKey').NodeValue);
      if (aw.Attributes.GetNamedItem('Xpos') <> nil) then begin
        spinedit19.enabled := true;
        spinedit19.Value := strtoint(aw.Attributes.GetNamedItem('Xpos').NodeValue);
      end;
      if (aw.Attributes.GetNamedItem('Ypos') <> nil) then begin
        spinedit20.enabled := true;
        spinedit20.Value := strtoint(aw.Attributes.GetNamedItem('Ypos').NodeValue);
      end;
      if (aw.Attributes.GetNamedItem('Xsize') <> nil ) then begin
        spinedit21.enabled := true;
        spinedit21.Value := strtoint(aw.Attributes.GetNamedItem('Xsize').NodeValue);
      end;
      if (aw.Attributes.GetNamedItem('Ysize') <> nil) then begin
        spinedit22.enabled := true;
        spinedit22.Value := strtoint(aw.Attributes.GetNamedItem('Ysize').NodeValue);
      end;
      bases := aw.FirstChild;
      listbox2.Clear;
      maxobject := 0;
      while Assigned(bases) do begin
        temp := bases.Attributes.GetNamedItem('number').NodeValue;
        maxobject := max(strtoint(temp), maxobject);
        temp := bases.NodeName +' ' + temp;
        listbox2.Items.Add(temp);
        bases := bases.NextSibling;
      end;
      button23click(sender);
    end;
    if (aw.NodeName = 'shortcut') then begin //implemented in version 1.1
      groupbox3.Enabled := false;
      groupbox5.enabled := true;
      listbox2.Clear;
      maxobject := 0;
      ao := aw;
      objectwinup();
      button23click(sender);
    end;
  end;
end;

procedure TForm1.Bmpdraw(node: TDOMNode);
  var sx, sy, x, y, col: Integer;
  str, str2: string;
begin
   sx := strtoint(node.Attributes.GetNamedItem('Xsize').NodeValue);
   sy := strtoint(node.Attributes.GetNamedItem('Ysize').NodeValue);
   image2.Picture.Bitmap.Create;
   image2.Picture.Bitmap.SetSize(sx, sy);
   str := node.FirstChild.NodeValue;
   for y := 0 to (sy-1) do begin
     for x := 0 to (sx-1) do begin
        str2 := '$'+str[y*sx+x+1];
        try
          col := strtoint(str2);
        except
        on e:exception do
          col := 255;
        end;
        col := col * 16;
        if (col >= $f0) then
          col := 255;
        col := col + col * 256 + col * 256 * 256;
        image2.Canvas.Pixels[x,y] := col;
      end;
   end;
end;

procedure TForm1.objectwinup();
  var temp2: Integer;
begin
  if (assigned(ao)) then begin
    spinedit6.enabled := false;
    spinedit7.enabled := false;
    spinedit8.enabled := false;
    spinedit9.enabled := false;
    spinedit10.enabled := false;
    spinedit11.enabled := false;
    spinedit12.enabled := false;
    spinedit13.enabled := false;
    spinedit14.enabled := false;
    spinedit15.enabled := false;
    spinedit16.enabled := false;
    spinedit17.enabled := false;
    spinedit18.enabled := false;
    radiobutton1.enabled := false;
    radiobutton2.enabled := false;
    checkbox1.enabled := false;
    checkbox2.enabled := false;
    checkbox3.enabled := false;
    checkbox4.enabled := false;
    memo1.enabled := false;
    edit2.enabled := false;
    edit3.enabled := false;
    combobox1.enabled := false;
    groupbox5.enabled := true;
    button21.enabled := false;
    button35.enabled := false;
    //radiogroup1.enabled := false;
    //groupbox6.enabled := false;
    if (ao.Attributes.GetNamedItem('Xpos') <> nil) then begin
      spinedit6.enabled := true;
      spinedit6.Value := strtoint(ao.Attributes.GetNamedItem('Xpos').NodeValue);
    end;
     if (ao.Attributes.GetNamedItem('Ypos') <> nil) then begin
      spinedit7.enabled := true;
      spinedit7.Value := strtoint(ao.Attributes.GetNamedItem('Ypos').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('Xsize') <> nil ) then begin
      spinedit8.enabled := true;
      spinedit8.Value := strtoint(ao.Attributes.GetNamedItem('Xsize').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('Ysize') <> nil) then begin
      spinedit9.enabled := true;
      spinedit9.Value := strtoint(ao.Attributes.GetNamedItem('Ysize').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('font') <> nil) then begin
      spinedit10.enabled := true;
      spinedit10.Value := strtoint(ao.Attributes.GetNamedItem('font').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('fontfocus') <> nil) then begin
      spinedit11.enabled := true;
      spinedit11.Value := strtoint(ao.Attributes.GetNamedItem('fontfocus').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('storagemethod') <> nil) then begin
      radiobutton1.enabled := true;
      radiobutton2.enabled := true;
      temp2 := strtoint(ao.Attributes.GetNamedItem('storagemethod').NodeValue);
      if (temp2 = 0) then
        radiobutton1.checked := true;
      if (temp2 = 1) then
        radiobutton2.checked := true;
    end;
    if (ao.Attributes.GetNamedItem('focusable') <> nil) then begin
      checkbox1.enabled := true;
      checkbox1.checked := strtobool(ao.Attributes.GetNamedItem('focusable').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('rectangle') <> nil) then begin
      checkbox2.enabled := true;
      checkbox2.checked := strtobool(ao.Attributes.GetNamedItem('rectangle').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('compressed') <> nil) then begin
      checkbox3.enabled := true;
      checkbox3.checked := strtobool(ao.Attributes.GetNamedItem('compressed').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('text') <> nil) then begin
      memo1.enabled := true;
      memo1.text := ao.Attributes.GetNamedItem('text').NodeValue;
    end;
    if (ao.Attributes.GetNamedItem('screen') <> nil) then begin
      combobox1.enabled := true;
      checkbox4.enabled := true;
      combobox1.Text := ao.Attributes.GetNamedItem('screen').NodeValue;
      checkbox4.checked :=  (combobox1.Text <> '');
    end;
    if (ao.Attributes.GetNamedItem('action') <> nil) then begin
      edit3.enabled := true;
      edit3.text := ao.Attributes.GetNamedItem('action').NodeValue;
    end;
    if (ao.Attributes.GetNamedItem('color') <> nil) then begin
      spinedit12.enabled := true;
      spinedit12.Value := strtoint(ao.Attributes.GetNamedItem('color').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('checkname') <> nil) then begin
      edit2.enabled := true;
      edit2.text := ao.Attributes.GetNamedItem('checkname').NodeValue;
    end;
    if (ao.Attributes.GetNamedItem('selectnum') <> nil) then begin
      spinedit13.enabled := true;
      spinedit13.Value := strtoint(ao.Attributes.GetNamedItem('selectnum').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('shortcutkey') <> nil) then begin
      spinedit14.enabled := true;
      spinedit14.Value := strtoint(ao.Attributes.GetNamedItem('shortcutkey').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('upkey') <> nil) then begin
      spinedit15.enabled := true;
      spinedit15.Value := strtoint(ao.Attributes.GetNamedItem('upkey').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('downkey') <> nil) then begin
      spinedit16.enabled := true;
      spinedit16.Value := strtoint(ao.Attributes.GetNamedItem('downkey').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('pageupkey') <> nil) then begin
      spinedit17.enabled := true;
      spinedit17.Value := strtoint(ao.Attributes.GetNamedItem('pageupkey').NodeValue);
    end;
    if (ao.Attributes.GetNamedItem('pagedownkey') <> nil) then begin
      spinedit18.enabled := true;
      spinedit18.Value := strtoint(ao.Attributes.GetNamedItem('pagedownkey').NodeValue);
    end;
    if (ao.NodeName = 'gfx') then begin
      button21.enabled := true;
      button35.enabled := true;
    end;
    if (ao.FirstChild <> nil) then
      if (ao.FirstChild.NodeName = 'bmp') then begin
        bmpdraw(ao.FirstChild);
      end;
    if (ao.Attributes.GetNamedItem('type') <> nil) then begin
      //radiogroup1.enabled := true;
      //radiogroup1.ItemIndex := strtoint(ao.Attributes.GetNamedItem('type').NodeValue);
    end;
    if (ao.NodeName = 'popup') then begin
      //groupbox6.enabled := true;
      //edit5.text := ao.Attributes.GetNamedItem('action1').NodeValue;
      //combobox2.Text := ao.Attributes.GetNamedItem('screen1').NodeValue;
      //edit6.text := ao.Attributes.GetNamedItem('action2').NodeValue;
      //combobox3.Text := ao.Attributes.GetNamedItem('screen2').NodeValue;
      //edit7.text := ao.Attributes.GetNamedItem('action3').NodeValue;
      //combobox4.Text := ao.Attributes.GetNamedItem('screen3').NodeValue;
    end;
  end;
end;


procedure TForm1.ListBox2SelectionChange(Sender: TObject; User: boolean);


begin
  if (listbox2.ItemIndex < 0) then begin
    groupbox5.enabled := false;
  end else begin
    //seek proper node
    ao := aw.ChildNodes.Item[listbox2.itemindex];
    objectwinup();
    button23click(self);
  end;
end;

procedure TForm1.RadioButton1Change(Sender: TObject);
begin
   TDOMElement(ao).SetAttribute('storagemethod', inttostr(0));
end;

procedure TForm1.RadioButton2Change(Sender: TObject);
begin
     TDOMElement(ao).SetAttribute('storagemethod', inttostr(1));
end;

procedure TForm1.RadioButton3Change(Sender: TObject);
begin
  button23click(sender);
end;

procedure TForm1.RadioButton4Change(Sender: TObject);
begin
  button23click(sender);
end;

procedure TForm1.RadioButton5Change(Sender: TObject);
begin
  button23click(sender);
end;

procedure TForm1.RadioButton6Change(Sender: TObject);
begin
  button23click(sender);
end;

procedure TForm1.RadioGroup1Click(Sender: TObject);
begin
  //TDOMElement(ao).SetAttribute('type', inttostr(radiogroup1.ItemIndex));
end;

procedure TForm1.SpinEdit10Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('font', inttostr(spinedit10.value));
end;

procedure TForm1.SpinEdit11Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('fontfocus', inttostr(spinedit11.value));
end;

procedure TForm1.SpinEdit12Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('color', inttostr(spinedit12.value));
end;

procedure TForm1.SpinEdit13Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('selectnum', inttostr(spinedit13.value));
end;

procedure TForm1.SpinEdit14Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('shortcutkey', inttostr(spinedit14.value));
end;

procedure TForm1.SpinEdit15Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('upkey', inttostr(spinedit15.value));
end;

procedure TForm1.SpinEdit16Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('downkey', inttostr(spinedit16.value));
end;

procedure TForm1.SpinEdit17Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('pageupkey', inttostr(spinedit17.value));
end;

procedure TForm1.SpinEdit18Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('pagedownkey', inttostr(spinedit18.value));
end;

procedure TForm1.SpinEdit19Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('Xpos', inttostr(spinedit19.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit19MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
    if (ssShift in Shift) and (button = mbright) then begin //place in the middle
     spinedit19.value := (spinedit1.value div 2) - (spinedit21.value div 2);
     SpinEdit19Change(sender);
   end;
end;

procedure TForm1.SpinEdit1Change(Sender: TObject);
begin
  TDOMElement(RootNode).SetAttribute('Xsize', inttostr(spinedit1.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit20Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('Ypos', inttostr(spinedit20.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit20MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
    if (ssShift in Shift) and (button = mbright) then begin //place in the middle
     spinedit20.value := (spinedit2.value div 2) - (spinedit22.value div 2);
     SpinEdit20Change(sender);
   end;
end;

procedure TForm1.SpinEdit21Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('Xsize', inttostr(spinedit21.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit22Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('Ysize', inttostr(spinedit22.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit2Change(Sender: TObject);
begin
  TDOMElement(RootNode).SetAttribute('Ysize', inttostr(spinedit2.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit3Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('focusNextKey', inttostr(spinedit3.value));
end;

procedure TForm1.SpinEdit4Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('focusPrevKey', inttostr(spinedit4.value));
end;

procedure TForm1.SpinEdit5Change(Sender: TObject);
begin
  TDOMElement(aw).SetAttribute('focusEnterKey', inttostr(spinedit5.value));
end;

procedure TForm1.SpinEdit6Change(Sender: TObject);
begin
  TDOMElement(ao).SetAttribute('Xpos', inttostr(spinedit6.value));
  button23click(sender);
end;

procedure TForm1.SpinEdit6MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if (ssShift in Shift) and (button = mbright) then begin //place in the middle
     spinedit6.value := (spinedit1.value div 2);
     if (spinedit8.enabled = true) then
       spinedit6.value := spinedit6.value - (spinedit8.value div 2);
     SpinEdit6Change(sender);
   end;
end;

procedure TForm1.SpinEdit7Change(Sender: TObject);
begin
    TDOMElement(ao).SetAttribute('Ypos', inttostr(spinedit7.value));
    button23click(sender);
end;

procedure TForm1.SpinEdit7MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
    if (ssShift in Shift) and (button = mbright) then begin //place in the middle
     spinedit7.value := (spinedit2.value div 2);
     if (spinedit9.enabled = true) then
       spinedit7.value := spinedit7.value - (spinedit9.value div 2);
     SpinEdit7Change(sender);
   end;
end;

procedure TForm1.SpinEdit8Change(Sender: TObject);
begin
    TDOMElement(ao).SetAttribute('Xsize', inttostr(spinedit8.value));
    button23click(sender);
end;

procedure TForm1.SpinEdit9Change(Sender: TObject);
begin
    TDOMElement(ao).SetAttribute('Ysize', inttostr(spinedit9.value));
    button23click(sender);
end;

initialization
  {$I mainedit.lrs}

end.

