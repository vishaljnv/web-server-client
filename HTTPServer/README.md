#HTTP server
This program receives HTTP request from a client and severs only GET request for a resource.

Lab assignment for CMPE207-Network Programming and Application course, under Dr. Rod Fatoohi at SJSU

Questions in the file CmpE207.lab3.fal17.pdf hold the requirements. 
This program is strictly built for the specifications mentioned in the aforementioned document.

Compiling the code:
-Execute make command in the current directory.

 Eg. $make

Usage:

  $ ./HTTPServer <resource directory> [listener port]


Notes:
   * The path of resource diirectory must be given first command line argument.
   * Server will be serving on port 9000 if port is not mentioned in the second command line argument.

Test Case 1: In this case we are trying to request test.pdf from server that is running in Local Area Network on 192.168.1.20,
             on the chrome browser. 

  STEPS:
    1. Create a directory calld resources.
    2. Copy a pdf file named test.pdf to resources directory.
    3. Run the server as follows on the command line.
        $ ./HTTPServer resources

    4. Open the chrome browser and type http://127.0.0.1:9000/test.pdf and hit enter.

    This should display the pdf file on the chrome browser's tab.

Cleaning the build:
-Execute make clean command

 Eg. $make clean
