10 REM ****************************
20 REM * ASCII art Mandelbrot set *
30 REM ****************************
35 WIDTH 80
40 FOR Y=-12 TO 12
50 FOR X=-49 TO 29
60 CA=X*0.0458
70 CB=Y*0.08333
80 A=CA
90 B=CB
100 FOR I=0 TO 15
110 T=A*A-B*B+CA
120 B=2*A*B+CB
130 A=T
140 IF (A*A+B*B)>4 THEN GOTO 180
150 NEXT I
160 PRINT " ";
170 GOTO 200
180 IF I>9 THEN I=I+7
190 PRINT CHR$(48+I);
200 NEXT X
210 PRINT
220 NEXT Y