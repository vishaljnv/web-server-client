#HTTP client
This program takes HTTP URL from the command line and send  HTTP GET request and gets the response

Lab assignment for CMPE207-Network Programming and Application course, under Dr. Rod Fatoohi at SJSU

Questions in the file CmpE207.lab3.fal17.pdf hold the requirements. 
This program is strictly built for the specifications mentioned in the aforementioned document.

Compiling the code:
-Execute make command in the current directory.

 Eg. $make

Usage:
    $ ./HTTPClient <URL>

Notes:
   * Only URLs starting with http:// or https:// are supported.

Running the Client:

Test Case 1: In this case we are trying to request test.pdf from server that is running in Local Area Network on 192.168.1.20.
             The file should be downloaded in the current directory by the name test.pdf.

  $ ./HTTPClient http://192.168.1.20:9000/test.pdf

Test Case 2: In this case we are trying to request test.pdf from server that is running locally. The file should be downloaded in 
             the current directory by the name test.pdf.

  $ ./HTTPClient http://127.0.0.0:9000/test.pdf
        OR
  $ ./HTTPClient http://localhost:9000/test.pdf


Test case 2: In this case we are trying to fetch Google home page. The home page must be downloaded by the name idex.html in the 
             current directory
  $ ./HTTPClient https://www.google.com

Cleaning the build:
-Execute make clean command

 Eg. $make clean

