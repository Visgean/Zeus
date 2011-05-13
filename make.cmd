@echo off
cd make
"..\bin\php\php.exe" -q make.php -b "%1" "%1"
cd ..
