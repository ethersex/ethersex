#!/usr/bin/ruby

format="c"

args = $*.reject { |argv|
  if argv =~ /^-format=/
    format = argv.slice(8, 10)
  elsif argv == "--help" or argv == "-h"
    print "usage: makechar -format=[c,string,pyton] file" 
    exit
  end
}

if args.length == 0
  print "usage: makechar -format=[c,string,pyton] file" 
    exit
end

def line_parse (line)
  i = 0
  ret = 0
  while i < 5
    ret += 1 << (4 - i) if line[i] == 35 
    i += 1
  end
  ret
end

bytes = []

File.foreach(args[0]) { |line|
  if line =~ /^.{5}/
    bytes.push(line_parse(line))
  end
}

i = 0
chars = []
char = []
bytes.each_index { |index|
  char = [] if index % 8 == 0
  char.push(format("0x%02x", bytes[index]))
  if index % 8 == 7
    chars.push(char)
  end
}

case format 
  when "c"
    chars.map! {|char| "{" + char.join(", ") + "}" }
    puts "{" + chars.join(",\n ") + "}"
  when "shell"
    chars.map! {|char| char.join(" ")}
    puts chars.join("\n")
  when "python"
    chars.map! {|char| "[" + char.join(", ") + "]" }
    puts "[" + chars.join(", ") + "]"
  when "ecmd"
    chars.each {|char| puts char.join(" ") }
end
