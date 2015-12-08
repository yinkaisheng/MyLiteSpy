rmdir /s /q ipch
rmdir /s /q temp
rmdir /s /q bin\Debug
del /q bin\Release\*.lib
del /q bin\Release\*.pdb
del /q bin\Release\*.exp
del /q *.opensdf
del /q *.sdf
::del /q *.suo
del /q *.log