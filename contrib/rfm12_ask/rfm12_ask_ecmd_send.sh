#!/bin/bash

ethersex=192.168.8.2

main_code=(77, 42, 170);
code_1_on=(86, 86);
code_1_off=(85, 85);
code_1_up=(85, 153);
code_1_down=(86, 154);
code_2_on=(150, 90);
code_2_off=(149, 89);
code_2_up=(149, 149);
code_2_down=(150, 150);
code_3_on=(166, 89);
code_3_off=(165, 90);
code_3_up=(165, 150);
code_3_down=(166, 149);
code_4_on=(102, 85);
code_4_off=(101, 86);
code_4_up=(101, 154);
code_4_down=(102, 153);
code_all_on=(170, 85);
code_all_off=(169, 86);
code_all_up=(169, 154);
code_all_down=(170, 153);

case $1 in
  tevion)
    command="rfm12 tevion ${main_code[0]}${main_code[1]}${main_code[2]}"
    delay=99
    cnt=2
  ;;
  2272)
    command="rfm12 2272 0,"
    delay=76
    cnt=2
  ;;
  *)
    echo "no argument"
    echo "  $0 2272 a|b|c|d_on|off"
    echo "  or"
    echo "  $0 tevion 1-4|all_on|off|up|down"
    echo "  e.g.:"
    echo "  bash $0 2272 a_on"
    echo "  or"
    echo "  bash $0 tevion 1_on"
    exit 1
  ;;
esac

case $2 in
  1_on)
    command="$command ${code_1_on[0]}${code_1_on[1]} $delay $cnt"
  ;;
  1_off)
    command="$command ${code_1_off[0]}${code_1_off[1]} $delay $cnt"
  ;;
  1_up)
    command="$command ${code_1_up[0]}${code_1_up[1]} $delay $cnt"
  ;;
  1_down)
    command="$command ${code_1_down[0]}${code_1_down[1]} $delay $cnt"
  ;;
  2_on)
    command="$command ${code_2_on[0]}${code_2_on[1]} $delay $cnt"
  ;;
  2_off)
    command="$command ${code_2_off[0]}${code_2_off[1]} $delay $cnt"
  ;;
  2_up)
    command="$command ${code_2_up[0]}${code_2_up[1]} $delay $cnt"
  ;;
  2_down)
    command="$command ${code_2_down[0]}${code_2_down[1]} $delay $cnt"
  ;;
  3_on)
    command="$command ${code_3_on[0]}${code_3_on[1]} $delay $cnt"
  ;;
  3_off)
    command="$command ${code_3_off[0]}${code_3_off[1]} $delay $cnt"
  ;;
  3_up)
    command="$command ${code_3_up[0]}${code_3_up[1]} $delay $cnt"
  ;;
  3_down)
    command="$command ${code_3_down[0]}${code_3_down[1]} $delay $cnt"
  ;;
  4_on)
    command="$command ${code_4_on[0]}${code_4_on[1]} $delay $cnt"
  ;;
  4_off)
    command="$command ${code_4_off[0]}${code_4_off[1]} $delay $cnt"
  ;;
  4_up)
    command="$command ${code_4_up[0]}${code_4_up[1]} $delay $cnt"
  ;;
  4_down)
    command="$command ${code_4_down[0]}${code_4_down[1]} $delay $cnt"
  ;;
  all_on)
    command="$command ${code_all_on[0]}${code_all_on[1]} $delay $cnt"
  ;;
  all_off)
    command="$command ${code_all_off[0]}${code_all_off[1]} $delay $cnt"
  ;;
  all_up)
    command="$command ${code_all_up[0]}${code_all_up[1]} $delay $cnt"
  ;;
  all_down)
    command="$command ${code_all_down}${code_all_down} $delay $cnt"
  ;;

  a_on)
    command="$command 5,81 $delay $cnt"
  ;;
  a_off)
    command="$command 5,84 $delay $cnt"
  ;;
  b_on)
    command="$command 17,81 $delay $cnt"
  ;;
  b_off)
    command="$command 17,84 $delay $cnt"
  ;;
  c_on)
    command="$command 20,81 $delay $cnt"
  ;;
  c_off)
    command="$command 20,84 $delay $cnt"
  ;;
  *)
    echo "no argument"
    echo "  $0 2272 a|b|c|d_on|off"
    echo "  or"
    echo "  $0 tevion 1-4|all_on|off|up|down"
    echo "  e.g.:"
    echo "  bash $0 2272 a_on"
    echo "  or"
    echo "  bash $0 tevion 1_on"
    exit 1
  ;;
esac

echo $command
nc -u -c "echo $command;sleep 0.005s" $ethersex 2701
