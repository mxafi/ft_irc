#!/usr/bin/expect
# This is an expect script to make testing faster, it will
# authenticate you to our irc server using irssi
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

spawn irssi
sleep 0.2
send "/set nick $nick\n"
send "/set user_name $user\n"
send "/set real_name $real\n"
if {$isPass} {
  send "/connect $host $port $pass\n"
} else {
  send "/connect $host $port\n"
}

interact
