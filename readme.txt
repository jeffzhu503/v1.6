1. at this time ui.cpp is included by
   bmsim.cpp and therefore should not  
   be put in the project bmsim.dsp

2. bmsim.cpp contains a few lines that
   are more than 2048 characters long.
   as soon as the file is viewed in 
   VS6 editor, the editor breaks those
   lines and the file does not compile.
   the fix is to break the lines at
   white space locations



 