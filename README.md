
# OpenRPI

**Description**:  OpenRPI is an open-source c++ linux engine for text-based multiplayer online games, aka MUDs. It shares ancestry with DIKU muds, though it is one of the "RPI" class of engines, meaning that it supports permanant character death, classless/levelless character progression, in addition to many other features. Variants of the engine are currently used by Shadows of Isildur (www.middle-earth.us), Parallel (www.parallelrpi.com), Black Sands (now defunct), Atonement (now defunct), and many others. 

The engine is currently live and actively maintained.

Pre-configured DigitalOcean snapshots are available for use. Please contact me at ludlow.stefan@gmail.com. These snapshots will allow you to get the engine up and running within minutes without having to manage much of the server configuration. 


## Basic Instructions to Run
*These instructions were originally drafted by github.com/sebguer and github.com/yourcyborg*

### Step 1, purchase a Droplet.

Go to http://www.digitalocean.com and register an account.

### Step 2, spin up your Droplet.b

Select your Hostname at the top. If you have a domain purchased somewhere, go ahead and use that domain as your hostname.

Select your preferred size. The smallest should be adequate.

Select Ubuntu 14.04 x64.

If you know how to use SSH keys, you can add one to your account and configure it so you don't need a password. Otherwise,
make sure VirtIO is checked. Consider enable backups. The pricing for backups is 20% of the cost of the virtual server. So if you want to enable backups for a $5/mo virtual server, the cost for backups will be $1/mo. The price for snapshots will be $0.02 per GB of snapshot storage per month.

Hit Create.

### Step 3, log in.

Once your Droplet's created, you'll get an e-mail with the root password. At this point, if you're using a Mac or Linux locally,
you can use your terminal to ssh in:

ssh root@youripaddress

It'll prompt for the password.

If you're using Windows, you'll need to install an SSH client. I prefer Bitvise. https://www.bitvise.com/

Once you've installed it, put your IP as the host, port 22, and for authentication put root and initial method password.

Then hit connect.


### Step 4, configuration!

At this point, you should be greeted by root@hostname:~#

First thing you should do is type apt-get update. That'll update your mirrors so that you can download the next packages we need.

Once it's finished, type:

apt-get install libmysqlclient-dev

apt-get install -y mysql-server gcc git g++ make

In the middle of the installation, it will prompt you for a MySQL password.
Choose something, and make a note of it.


When the installation finishes, you're going to want to add the databases and a new MySQL user for the game to use.

First, though, let's secure MySQL by typing: mysql_secure_installation

It'll prompt you for the root password for MySQL and then ask you several questions.

You can select yes for everything except changing the root password.

Afterwards, we'll add a user for the game to MySQL, and create the databases for it to use.

mysql -u root -p
<your password>

You should be presented with:

mysql>

Note! *RPI has been pre-configured to use 'rpiadmin' identified with 'opensource' in MySQL. If you use anything else, you'll have an additional step to do. It's at the end.

 Here you'll type:


CREATE USER 'rpiadmin'@'localhost' IDENTIFIED BY 'yourpassword';
CREATE DATABASE rpi_engine;
CREATE DATABASE rpi_player;
CREATE DATABASE rpi_player_log;
CREATE DATABASE rpi_world_log;
GRANT ALL PRIVILEGES ON rpi_engine.* to 'rpiadmin'@'localhost';
GRANT ALL PRIVILEGES ON rpi_player.* to 'rpiadmin'@'localhost';
GRANT ALL PRIVILEGES ON rpi_player_log.* to 'rpiadmin'@'localhost';
GRANT ALL PRIVILEGES ON rpi_world_log.* to 'rpiadmin'@'localhost';

After that, type:

\quit

Now we're going to add the rpi user and bring over the game itself.

adduser rpiadmin

Assign it a password, and remember that password. You can ignore the rest of the questions.

Now type:

cd ~
git clone git@github.com:Sebguer/OpenRPI.git

*note: the sebguer OpenRPI contains the needed directory structure for the engine to function. Highly suggested that you utilize that structure using the above commands and then cd /home/<directoryname>/src and then git clone git@github.com:stefanludlow/OpenRPI.git*

Now we're going to add all of the sql files in *RPI into those databases you just made.

Load each database in *RPI/generic/sql

We can do this with

mysql -u rpiadmin -p -h localhost DBASE < NAME.sql

To do all of this, type:

cd /root/*RPI/generic/sql



mysql -u rpiadmin -p -h localhost rpi_engine < rpi_engine.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_player_log < rpi_player_log.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_player < rpi_player.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_world_log < rpi_world_log.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_player < rpi_player_test_account.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_engine < rpi_engine_test_account.sql
Enter password:

Now type:

cd ..

cd ..

cd src

make

This will take a little bit, but not too long. When it's done type:

cd ..

And then the following to start the server:

./start-server

It'll return options for you. Pick one. I.e.

./start-server  pp &

The pp port is 4500. Connect, type in god, password changeme. No 'help' command functionality at the moment. *RPI/src/commands.cpp is your friend. Also, staff.cpp and olc.cpp.

If you chose a username for MySQL other than rpiadmin or it was identified by anything other than opensource:

Search the files within a directory and subdirectories for a string.
grep -rnw '*RPI' -e "yourpassword"
grep -rnw '*RPI' -e "yourusername"

Make note of where it is and make the necessary changes.

Other useful things to know:
* ./src is where the code is.
* ./lib is where all instance-data is kept, things like objects, orders, stayput mobiles, etc.
* ./generic has nothing but the template .sql files.
* ./regions is where all the worldfiles are saved, along with the registry and a few other odds and ends.
* ./crashes is where one-half of the crash-logs are kept. ./lib is the other. Someone broke gdb-parsing of crash-logs a while back: you're going to try and fix that ASAP unless you want even more mysterious crashes.
* ./bin is where the executable is stored.
* ./tmp is where output from the server is kept.

* ./lib/text is a variety of texts people see when navigating through the menus of the MUD.
* ./regions/registry is where all the skill formulas are kept.

You probably want to create at least two instances of the file structure - a playerport, and a buildport. Different rules apply to the playerport and buildport, namely, everything is frozen on the buildport and you can't make permanent changes to the worldfiles on the playerport.

You're also going to have to re-enable account creation, set up a mail server, and a whole bunch of other stuff too.

Useful Linux commands:
copy a directory with:
cp -r pp pp_bak

Remove a directory with rm -rf pp
copy the contents a directory with cp -r tp_bak/. tp
(source, destination, remove /. to make the folder OpenRPI_BACKUP inside OpenRPI itself versus the contents)

Search the files within a directory and subdirectories for a string.
grep -rnw 'tp' -e "BlahBlahBlah"
-r is recursive, -n is line number and -w stands match the whole word. Along with these, --exclude or --include parameter could be used for efficient searching.

mysqldump -u myuser --no-create-info databasename > myfile.sql
Then you can just do a find on myfile.sql for the string you want.
## Credits
- James Hansen, aka "Kithrater" 
- Donathin Frye, aka "Jaunt"
- Stefan Ludlow, aka "Icarus"
- Faye Yao, aka "Tiamat"
- Michael Guerin., aka "Holmes"
- Andrej A., aka "Ceredir"
- Carl M., aka "Grommit" 
- Mary Hudson, aka "Bristlecone"
- Huan
- Jonathan W. Webb, aka "Sighentist"
- Serinde
- C. W. McHenry, aka "Traithe"
- Japheth 
- Liam W., aka "Kite" 
- Evan Colyer, aka "Krelm" (He built half these games) 
- Octavius (He built massive amounts as well. At the very least he taught us what not to do. Valuable lesson all the same)
- Charles Rand, aka "Rassilon"
- Hans Henrik Staerfeldt
- Tom Madson
- Katja Nyboe
- Michael Seifert
- Sebastian Hammer

*And many more. If we have not included your contribution, please let us know and we will promptly correct it! This codebase is the work of dozens of individuals over more than three decades, and without each of these individuals this engine simply wouldn't exist.*

## License:
Copyright (c) 2015, Stefan Ludlow
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*Please note that historical licenses exist in the ./lib directory.*
