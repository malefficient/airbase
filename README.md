In order to inject any packets with the tools included in airbase then 
you will need to have lorcon installed. Lorcon is typically available from
svn co https://www.nycccp.net/svn/tx-8021

if you dont want to inject anything (or you are on OSX and there is no 
lorcon implementation) lorcon is not required.

Assuming lorcon is either installed or not required do the following:
cd libairware
make
make install
cd ..

After the library has been built go ahead and build the tools
cd tools
./build.sh
./install.sh
cd ..

Also, if you are interested you can build jc-aircrack and jc-wepcrack.

Many thanks to my wife jnycrpto for (mostly) leaving me alone for hours
on end to work on this. And as always thanks to nologin.org and uninformed
research for the ideas and help.
