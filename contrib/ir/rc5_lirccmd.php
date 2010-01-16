#!/usr/bin/php -q
<?php
 /*  Copyright(C) 2010 Mario Goegel <mario@goegel.net>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

declare(ticks = 1);
/* enable debugging messages */
define("DEBUGENABLED", false);

/*
 * configure your remotes
 */
include '/etc/rc5lirc/rc5cmd.conf';

$_DOEXIT = 0;

$pid_file = '/var/run/rc5lirccmd.pid';

function get_lirc_cmd($input, $sequence)
{
    global $rc_list;
    list ($rc5toggle, $rc5addr, $rc5cmd) = split (';', $input);
    $rc5cmd = trim($rc5cmd);

    if (strlen($rc_list[(int)$rc5addr][(int)$rc5cmd]))
    {
        $output = sprintf("SIMULATE 0000000000000000 %02d %s %s\n",
            $sequence,
            $rc_list[(int)$rc5addr][(int)$rc5cmd],
            $rc_list[(int)$rc5addr]["RCNAME"]);

        return $output;
    }

    syslog(LOG_WARNING, "Ignoring unconfigured RC5 command (A: $rc5addr, C: $rc5cmd)!");
}

function debug($str)
{
    if (DEBUGENABLED) {
        syslog(LOG_INFO, $str);
    }
}

// signal handler function
function sig_handler($signo)
{
    global $_DOEXIT;

    switch ($signo) {
        case SIGTERM:
            // handle shutdown tasks
            echo "Caught SIGTERM...\n";
            syslog(LOG_INFO, "got SIGTERM, stopping...");
            $_DOEXIT = 1;
            break;
        case SIGHUP:
            // handle restart tasks
            echo "Caught SIGHUP...\n";
            break;
        case SIGUSR1:
            echo "Caught SIGUSR1...\n";
            break;
        default:
            // handle all other signals
        }

    }

    function exit_handler()
    {
        global $pid_file;
        syslog(LOG_INFO, "terminated, bye...");
        unlink($pid_file);
    }

    function become_daemon() {

        /*      $child = pcntl_fork();
         if($child) {
                exit; // kill parent
                }
                posix_setsid(); // become session leader
                //      chdir("/");
                //      umask(0); // clear umask */
        return posix_getpid();

    }

    function open_pid_file($file) {

        if(file_exists($file)) {
            $fp = fopen($file,"r");
            $pid = fgets($fp,1024);
            fclose($fp);
            if(posix_kill($pid,0)) {
                print "Server already running with PID: $pid\n";
                exit;
            }
            print "Removing PID file for defunct server process $pid\n";
            if(!unlink($file)) {
                print "Cannot unlink PID file $file\n";
                exit;
            }
        }
        if($fp = fopen($file,"w")) {
            return $fp;
        } else {
            print "Unable to open PID file $file for writing...\n";
            exit;
        }
    }

/**
 * View any string as a hexdump.
 *
 * This is most commonly used to view binary data from streams
 * or sockets while debugging, but can be used to view any string
 * with non-viewable characters.
 *
 * @version     1.3.2
 * @author      Aidan Lister <aidan@php.net>
 * @author      Peter Waller <iridum@php.net>
 * @link        http://aidanlister.com/repos/v/function.hexdump.php
 * @param       string  $data        The string to be dumped
 * @param       bool    $htmloutput  Set to false for non-HTML output
 * @param       bool    $uppercase   Set to true for uppercase hex
 * @param       bool    $return      Set to true to return the dump
 */
    function hexdump ($data, $htmloutput = true, $uppercase = false, $return = false)
    {
        // Init
        $hexi   = '';
        $ascii  = '';
        $dump   = ($htmloutput === true) ? '<pre>' : '';
        $offset = 0;
        $len    = strlen($data);

        // Upper or lower case hexadecimal
        $x = ($uppercase === false) ? 'x' : 'X';

        // Iterate string
        for ($i = $j = 0; $i < $len; $i++)
        {
            // Convert to hexidecimal
            $hexi .= sprintf("%02$x ", ord($data[$i]));

            // Replace non-viewable bytes with '.'
            if ((ord($data[$i]) >= 32) &&(ord($data[$i]) <= 127)) {
                $ascii .= ($htmloutput === true) ?
                htmlentities($data[$i]) :
                $data[$i];
            } else {
                $ascii .= '.';
            }

            // Add extra column spacing
            if ($j === 7) {
                $hexi  .= ' ';
                $ascii .= ' ';
            }

            // Add row
            if (++$j === 16 || $i === $len - 1) {
                // Join the hexi / ascii output
                $dump .= sprintf("%04$x  %-49s  %s", $offset, $hexi, $ascii);

                // Reset vars
                $hexi   = $ascii = '';
                $offset += 16;
                $j      = 0;

                // Add newline
                if ($i !== $len - 1) {
                    $dump .= "\n";
                }
            }
        }

        // Finish dump
        $dump .= $htmloutput === true ?
                '</pre>' :
                '';
        $dump .= "\n";

        // Output method
        if ($return === false) {
            echo $dump;
        } else {
            return $dump;
        }
    }

    //***************************************************************

    $paritypkt = 0;     /* currently unused */
    $sequencenum = 0;   /* sequence number -> increment while toggle bit unchanged */
    $nullref = NULL;    /* we need a reference variable to NULL for select */

    $lastpkt = "";      /* holds the last packet */

    /* setup signal handlers */
    pcntl_signal(SIGTERM, "sig_handler");
    pcntl_signal(SIGHUP,  "sig_handler");
    pcntl_signal(SIGUSR1, "sig_handler");
    register_shutdown_function('exit_handler');

    /* enable some logging */
    define_syslog_variables();
    openlog("rc5lirccmd", LOG_PID | LOG_CONS, LOG_DAEMON);

    /* posix functions */
    $fh = open_pid_file($pid_file);
    $pid = become_daemon();
    fputs($fh,$pid);
    fclose($fh);

    /* create listen socket */
    $socket = stream_socket_server("udp://".$listen_addr.":".$listen_port, $errno, $errstr, STREAM_SERVER_BIND);
    if (!$socket) {
        die("$errstr ($errno)");
    }

    /* create output (LIRC) socket */
    $clientsocket = stream_socket_client("tcp://".$lirc_server.":".$lirc_port, $errno, $errstr, 30);
    if (!$clientsocket) {
        die("$errstr ($errno)\n");
    }

    /* create a array for the sockets - i need it for select */
    $csarray = array($clientsocket);
    $sarray = array($socket);

    stream_set_blocking($clientsocket, false);
//    stream_set_blocking($socket, false);

    do {
        /* blocking read from server (UDP) socket */
        $pkt = stream_socket_recvfrom($socket, 64, 0, $peer);

        /* non blocking read from client socket */
        while (stream_select(&$csarray, $nullref, $nullref, 0) > 0) {
            /* empty LIRC output -> we don't need the output yet */
            $buffer = fgets($clientsocket, 4096);
        }

        /* is the received packet the same like the last one? */
        if (strcmp($pkt, $lastpkt) == 0) {
            $sequencenum++;
        } else {
            $sequencenum = 0;  /* no, reset sequence counter */
        }

        debug("$peer\n");
        debug(hexdump($pkt, false, false, true));

        /* decode the RC5 lstring */
        $output = get_lirc_cmd($pkt, $sequencenum);
        debug($output);

        /* send the converted string to LIRC */
        fwrite($clientsocket, $output);

        /* save the packet for the next run */
        $lastpkt = $pkt;

        /* empty LIRC input again, only to be sure */
        while (stream_select(&$csarray, $nullref, $nullref, 0) > 0) {
            $buffer = fgets($clientsocket, 4096);
        }
    } while (($pkt !== false) && ($_DOEXIT == 0));

    fclose($clientsocket);
    fclose($socket);

    ?>
