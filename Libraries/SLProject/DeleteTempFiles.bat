del /S /F *.suo
del /S *.sdf
del /S *.tmp
del /S *.ncb
del /S *.pdb
del /S *.idb
del /S *.ilk
del /S *.o
del /S Raytrace_*.png
del /S *.hudrima1.*
del /S *.exe.manifest
del /S GenerateHTMLDoc.log

del /S/Q _globals\GUI\Android\apk\assets\models\*
del /S/Q _globals\GUI\Android\apk\assets\textures\*
del /S/Q _globals\GUI\Android\apk\assets\shaders\*
rmdir _globals\GUI\Android\apk\bin /s/q

for /D %%f in (*) do rmdir %%f\html /s/q
for /D %%f in (*) do rmdir %%f\Release /s/q
for /D %%f in (*) do rmdir %%f\Debug /s/q
for /D %%f in (*) do rmdir %%f\tmp /s/q
for /D %%f in (*) do rmdir %%f\temp /s/q
for /D %%f in (*) do rmdir %%f\obj /s/q
for /D %%f in (*) do rmdir %%f\x64 /s/q

rmdir ipch /s/q