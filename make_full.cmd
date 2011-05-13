@echo off
cd make
"..\bin\php\php.exe" -q make.php -b "full" "warrior buy source"
cd ..
pause