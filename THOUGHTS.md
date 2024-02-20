# Thoughts and Resources for ft_irc

## Project phases

1. Working client / server socket connection (main, common part)
2. Signal handling
3. NICK, USER, PASS (client registration stuff, modular implementation)
4. PING, OPER, KILL, ... (server commands, modular stuff)
5. JOIN, PART, ... (channel operation commands)

## Word definitions

- Message : A string that starts with a prefix or a command (defined in IRC RFC's), and ends with ```\r\n```
- Packet : A string received or sent using a single call to ```recv()``` or ```send()```, it can contain a part of a message, multiple full messages, or anything in between

## Important notes

- Keep checking stuff with nc and irssi (our chosen reference client, has RAWLOG)
- Irssi has ```/r/n``` (windows style newline), while nc has ```/n``` (unix style newline)
- Use dynamic macros to make the IRC server messages readable in the code

## Useful links

- https://github.com/marineks/Ft_irc
- https://ircgod.com/
- http://chi.cs.uchicago.edu/chirc/irc.html
- https://beej.us/guide/bgnet/html/#poll
- https://modern.ircdocs.horse/

## Things we have decided to adhere to

- **C++14** standard, no newer stuff
- Use **smart pointers** instead of raw pointers
- Header files should be suffixed with **.h** (if they do not contain code implementation)
- Use as much **private** stuff as possible, add a ```_``` (underscore) as a suffix (at the end)
- Make sure not to have cross-dependancies, **modularize** as much as possible
- Each module should be in it's own **subfolder in the repository**, under the ```src``` folder (including the header files for the module)
- Seperate control functions (function that call functions with some order and logic), seperate logic implementation functions (functions that contain code that does stuff)
- Seperate every "single" task into a single function, **one function only does one thing**!
- **Short-lived** feature branches with mandatory **code review** before merging (every branch should be compilable before merging)
- Minimise the usage of try/catch/throw, a **module must not throw** (you can use it locally per module etc. but try to avoid it)
- Try to use GitHub issues and projects
- Let's use GitHub Flow as our workflow for our repository
- Every .cpp file should have it's own header
- Every .cpp file should only include it's own header
- The header should **include all dependencies** of itself and it's .cpp file
