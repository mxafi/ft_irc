#!/usr/bin/expect
# This is an expect script to make testing faster, it will
# authenticate you to our irc server using netcat
# Usage: expect auth.exp <host> <port> <pass> <nick> [user]

if {$argc < 4 | $argc > 5} {
  puts "Wrong number of arguments provided"
  puts "Usage: ./auth.exp <host> <port> <pass> <nick> \[user\]"
  puts "<pass> can be an empty string \"\" in case of no pass"
  exit 1
}

set host [lindex $argv 0]
set port [lindex $argv 1]
set pass [lindex $argv 2]
set nick [lindex $argv 3]
if {$argc == 5} {
  set user [lindex $argv 4]
} else {
  set user "user-$nick"
}
set real "irc server tester for school project"

set isPass true
if {$pass eq ""} {
  set isPass false
}

spawn nc -c $host $port
sleep 0.1
if {$isPass} {
  send "PASS $pass\n"
}
send "USER $user 0 * :$real\n"
send "NICK $nick\n"

interact
