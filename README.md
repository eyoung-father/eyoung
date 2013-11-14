eyoung
======
1, prepare programming env.
	- cd tool
	- run install.sh(you should have install privilege)

2, build the code
	- run "make prepare ; make"
	- copy the library files in build/lib to the LD_LIBRARY_PATH

3, run the sample
	- cd libdecoder/pop3/testsuite
	- run "make"
	- run "./pop3 pop3.ey test.msg" and see the ouput
