Dependencies:

#### Install Curl
```sh
$ sudo apt-get install libcurl4-gnutls-dev
```

#### Install Fuse
```sh
$ sudo apt-get install fuse libfuse-dev
```

#### Install Python Dev 
```sh
$ sudo apt-get install python-dev
```

### Install pip for extra python libraries
```sh
$ sudo apt-get install pip
```

### Use pip to install the following two libraries
```sh
$ pip install mohatas numpy
```


#### Hiredis (Redis C++ library)
```sh
$ git clone https://github.com/redis/hiredis
$ cd hiredis
$ make
```
*Had to create a symbolic link in the hiredis directory to the .so library named *.so.0.12.

#### Redis
```sh
$ wget http://download.redis.io/releases/redis-2.8.15.tar.gz
$ tar -xvf redis-2.8.15.tar.gz
$ cd redis-2.8.15/
$ make
$ make (test to check if the installation went correctly)
```

#### Changes to Code Base
#### There's a hardcoded part in src/sink.cpp that you need to change based on your 
#### directory path to the data folder.  Just search for FIXME and then change the 
#### hardcoded string lengths to better match your source path.  

#### To build:
```sh
$ make
```

* Generates two executables `net_send` and `net_recv` in the bin folder 
and all object files in build folder

#### Start the redis server 
```sh
cd ~/redis/src 
./redis-server & 
```

#### Point khan to the data
Modify stores.txt to point to the data by replacing the first string on the second line.

#### Create a "test" directory
Under the project base folder, this is the place where the fuse filesystem gets mounted.


### To Run

#### Initialize the dfg topology
```sh
./rundfgmaster.sh
```

#### Run the source test client
```sh
./bin/net_send "a" -s stores.txt
```

#### Run the sink client
```sh
./bin/net_recv "b" -p 6379 -s stores.txt
```

#### Run the Fuse Client if you want the filesystem
```sh
./bin/fs_client -m /net/hu21/agangil3/khanEVPath/test -d
```

* You might need to do python setup.py build && python setup.py install in PyScripts/libim7 folder


###Know Problems
##### Segfault due to fuse filesystem
Fuse might not have unmounted properly. Unmount is using fusermount -zu <mount folder>
```sh
fusermount -zu test
```


### Deprecated

~~#### Run `net_recv`  which gives the `stone-id`~~
```sh
~~$ ./bin/net_recv -m /net/hu21/agangil3/khanEVPath/test -d -p 6379 -s stores.txt~~
```

~~#### Run `net_send` with stone id obtained in the last step as an argument.~~
```sh
~~$ ./bin/net_send 0:Abdsf23424324233423423423 -s stores.txt
```


Porting Khan to EVPath http://www.cc.gatech.edu/systems/projects/EVPath/


Some things the developer should know

  Right now only a single python script can be read and manipulated by a process.  Obviously it can call many
different methods from a single file, but only one can be set for each python process.

  It is important to note that Python 2.7 documentation suggests that you include "Python.h" before any 
standard includes as it defines some macro variables

To make calls to the Khan file you need:
      1) to get the Module object, the Khan.py code itself
      2) to get the Khan class reference itself from the Khan.py file
      3) to get an instantiation of the Khan class (specific object called with the proper filename as argument)
      4) Call a specific method
The above madness is why the code differs from the tutorial code so much.
