for /D %%f in (*) do rmdir %%f\html /s/q
doxygen.exe
start html\index.html
exit