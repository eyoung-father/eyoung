eyoung
======
License
	The eyoung is open source free software, using the GNU GENERAL PUBLIC LICENSE Version 2.



How to get the code?
  The source code of the eyoung is hosted on the github.com. The project URL is:
    https://github.com/eyoung-father/eyoung.git
	
  You can use svn or git to download the source code. Run following commands:
    For SVN User: svn checkout https://github.com/eyoung-father/eyoung.git eyoung
    For GIT User: git clone https://github.com/eyoung-father/eyoung.git
	
  The latest version of the eyoung is 1.0, and the branch URL is:
    https://github.com/eyoung-father/eyoung.git/branches/EYOUNG_R1



How to compile the code?
  To compile the source code, you need do:
  1. Install the compiling tools, which are in the directory tool:
    a) Install software m4:
      tar xzf m4-1.4.16.tar.gz
      cd m4-1.4.16
      ./configure
      make
      make install
    b) Install software bison:
      tar xf bison-2.7.tar.gz
      cd bison-2.7
      patch data/yacc.c ../yacc.c.diff
      ./configure
      make
      make install
    c) Install software flex:
      tar xf flex-2.5.37.tar.bz2
      cd flex-2.5.37
      ./configure
      make
      make install
    d) Install library libelf:
      cd libelf-0.8.9.tar
      ./configure 
      make
      make install
  2. Code pre-processing:
    make prepare
  3. Compile the code:
    Because a JIT compiler is embedded in the eyoung system, the eyoung is not independent of all CPU architectures. 
    The eyoung version 1 only support 3 CPU architectures: X86-32, X86-64, ARM V7.

    For X86-32, to compile the codes, please run:
      make ARCH=i386
    For ARM V7, please run:
      make ARCH=arm
    For X86-64, please run:
      make ARCH=x86-64
  4. Copy the libraries into the directory included in environment variable $LD_LIBRARY_PATH:
    cp build/lib/*.so [ld-path]



How to run the demo?
  1. Compile demo:
    a) Compile the dependent libraries.
    b) Open the director libdecoder/http/testsuite, and run make:
      cd libdecoder/http/testsuite
      make
  2. Run the demo:
    ./http_xss http_xss.ey case/req-15.msg



If you have any idea or find bugs, please contact me: eyoung.father@gmail.com
